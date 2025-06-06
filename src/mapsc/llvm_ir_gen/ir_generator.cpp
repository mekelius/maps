#include "ir_generator.hh"

#include <cassert>
#include <span>
#include <sstream>
#include <system_error>
#include <tuple>
#include <utility>
#include <variant>
#include <vector>

#include "llvm/ADT/APFloat.h"
#include "llvm/ADT/APInt.h"
#include "llvm/ADT/ArrayRef.h"
#include "llvm/ADT/StringRef.h"
#include "llvm/IR/DerivedTypes.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/GlobalVariable.h"
#include "llvm/IR/Value.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/Module.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/Support/FileSystem.h"
#include "llvm/IR/Verifier.h"
#include "llvm/IR/BasicBlock.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/Constants.h"

#include "common/string_helpers.hh"
#include "common/maps_datatypes.h"

#include "mapsc/source_location.hh"
#include "mapsc/logging.hh"

#include "mapsc/compilation_state.hh"

#include "mapsc/types/type.hh"
#include "mapsc/types/function_type.hh"
#include "mapsc/types/type_defs.hh"
#include "mapsc/types/type_store.hh"

#include "mapsc/ast/definition.hh"
#include "mapsc/ast/expression_properties.hh"
#include "mapsc/ast/expression.hh"
#include "mapsc/ast/scope.hh"
#include "mapsc/ast/statement.hh"

using llvm::LLVMContext;
using std::optional, std::nullopt, std::vector, std::tuple, std::get, std::get_if;
using std::unique_ptr, std::make_unique;
using Maps::Helpers::capitalize;

#define BAD_STATEMENT_TYPE StatementType::user_error:\
                      case StatementType::compiler_error:\
                      case StatementType::deleted

#define IGNORED_STATEMENT_TYPE StatementType::empty



namespace Maps {

using Log = LogInContext<LogContext::ir_gen>;

namespace LLVM_IR {

// ----- IR GENERATOR -----

IR_Generator::IR_Generator(llvm::LLVMContext* context, llvm::Module* module,
    const CompilationState* compilation_state, llvm::raw_ostream* error_stream, 
    Options options)
:errs_(error_stream),
 context_(context),
 module_(module),
 types_({*context_}),
 options_(options),
 compilation_state_(compilation_state), 
 pragmas_(&compilation_state->pragmas_),
 maps_types_(compilation_state->types_) {

    if (!types_.is_good_) {
        LogInContext<LogContext::compiler_init>::compiler_error(COMPILER_INIT_SOURCE_LOCATION) << 
            "Initializing type map failed";
        fail();
    }

    builder_ = std::make_unique<llvm::IRBuilder<>>(*context_);
}

bool IR_Generator::run(const Scope& scope) {
    return run(scope, {});
}

bool IR_Generator::run(const Scope& scope, 
    std::span<DefinitionHeader* const> additional_definitions) {
    
    // Not allowed to run if we have already failed
    if (has_failed_)
        return false;

    if (!handle_global_functions(scope))
        return false;

    for (auto definition: additional_definitions) {
        if (!handle_global_definition(*definition)) {
            Log::compiler_error(definition->location()) << 
                "Couldn't generate ir for " << *definition;
            fail();
            return false;
        }
    }

    if (!verify_module()) {
        Log::compiler_error(NO_SOURCE_LOCATION) << "Module verification failed" << Endl;
        fail();
        return false;
    }

    return !has_failed_;
}

void IR_Generator::fail() {
    has_failed_ = true;
}

std::nullopt_t IR_Generator::fail_optional() {
    fail();
    return std::nullopt;
}

optional<llvm::Function*> IR_Generator::function_definition(const std::string& name, 
    llvm::FunctionType* llvm_type, llvm::Function::LinkageTypes linkage) {

    llvm::Function* function = llvm::Function::Create(llvm_type, linkage, name, module_);
    function_store_->insert(name, {llvm_type, function});
    llvm::BasicBlock* body = llvm::BasicBlock::Create(*context_, "", function);
    builder_->SetInsertPoint(body);
    return function;
}

optional<llvm::Function*> IR_Generator::overloaded_function_definition(const std::string& name, 
    const FunctionType& maps_type, llvm::FunctionType* llvm_type, 
    llvm::Function::LinkageTypes linkage) {

    if (!maps_type.is_function()) {
        Log::compiler_error(NO_SOURCE_LOCATION) << 
            "IR::Generator::function_definition called with a non-function type: " << maps_type;
        assert(false && "IR_Generator::function_definition called with non-function ast type");
        return nullopt;
    }

    auto suffixed_name = name + FunctionStore::get_suffix(maps_type);

    llvm::Function* function = llvm::Function::Create(llvm_type, linkage, suffixed_name, module_);
    function_store_->insert_overloaded(name, maps_type, {llvm_type, function});
    llvm::BasicBlock* body = llvm::BasicBlock::Create(*context_, "", function);
    builder_->SetInsertPoint(body);
    return function;
}

bool IR_Generator::close_function_definition(const llvm::Function& function, 
    llvm::Value* return_value) {
    
    if (return_value) {
        builder_->CreateRet(return_value);
    } else {
        builder_->CreateRetVoid();
    }

    if (!options_.verify_functions)
        return true;

    // llvm::verifyFunction returns true on fail for some reason
    return (!llvm::verifyFunction(function, errs_));
}

optional<llvm::Function*> IR_Generator::forward_declaration(const std::string& name, 
    llvm::FunctionType* llvm_type, llvm::Function::LinkageTypes linkage) {
    
    llvm::Function* function = llvm::Function::Create(llvm_type, linkage, name, module_);
    function_store_->insert(name, function);
    return function;
}

optional<llvm::Function*> IR_Generator::overloaded_forward_declaration(const std::string& name, 
    const FunctionType& maps_type, llvm::FunctionType* llvm_type,
    llvm::Function::LinkageTypes linkage) {
    
    auto suffixed_name = name + FunctionStore::get_suffix(maps_type);

    llvm::Function* function = llvm::Function::Create(llvm_type, linkage, suffixed_name, module_);
    function_store_->insert_overloaded(name, maps_type, function);
    return function;
}

bool IR_Generator::block_has_terminated() const {
    return builder_->GetInsertBlock()->getTerminator() != nullptr;
}

bool IR_Generator::verify_module() {
    if (!options_.verify_module)
        return true;

    return (!llvm::verifyModule(*module_, errs_));
}

bool IR_Generator::handle_global_functions(const Scope& scope) {
    for (auto definition: scope) {
        if (!handle_global_definition(*definition))
            return false;
    }
    
    return true;
}

std::optional<llvm::FunctionCallee> IR_Generator::handle_global_definition(
    const DefinitionHeader& definition) {

    if (!definition.body_)
        assert(false && "not implemented");

    Log::debug_extra(definition.location()) << "Generating ir for " << definition << Endl;

    if (definition.get_type()->is_function())
        return handle_function(definition);

    auto definition_body = (*definition.body_)->get_value();

    if (const Expression* const* expression = 
            std::get_if<Expression*>(&definition_body))
        return wrap_value_in_function(definition.name_string(), **expression);

    Log::compiler_error(definition.location()) << "In IR_Generator::handle_global_definition:" <<
        " definition didn't have a function type but wasn't an expression";
    return fail_optional();
}

std::optional<llvm::FunctionCallee> IR_Generator::wrap_value_in_function(
    const std::string& name, const Expression& expression) {
    
    const FunctionType* maps_type = 
        maps_types_->get_function_type(expression.type, {}, expression.type->is_pure());

    optional<llvm::FunctionType*> llvm_type = types_.convert_function_type(
        *maps_type->return_type(), maps_type->param_types());

    if (!llvm_type) {
        Log::compiler_error(expression.location) << 
            "Converting " << *maps_type << "\" into an llvm type failed";
        return fail_optional();
    }
    
    auto wrapper = function_definition(name, *llvm_type);
    auto value = handle_expression(expression);

    if (!*value) {
        Log::compiler_error(expression.location) << "Converting " << expression << " to a value failed" << Endl;
        return fail_optional();
    }

    if ((*value)->getType() != types_.void_t) {
        builder_->CreateRet(*value);
    } else {
        builder_->CreateRetVoid();
    }

    return wrapper;
}

std::optional<llvm::FunctionCallee> IR_Generator::handle_function(
    const DefinitionHeader& definition) {
    
    assert(definition.get_type()->is_function() && 
        "IR_Generator::handle function called with a non-function definition");

    const FunctionType* function_type = dynamic_cast<const FunctionType*>(
        definition.get_type());

    optional<llvm::FunctionType*> signature = types_.convert_function_type(
        *function_type->return_type(), function_type->param_types());

    if (!signature) {
        Log::error(definition.location()) << "unable to convert type signature for " << definition << Endl;
        return nullopt;
    }

    optional<llvm::Function*> function = function_definition(definition.name_string(), *signature);

    if (!function)
        return nullopt;
    
    bool success = std::visit(overloaded{
        [this](const Expression* expression) {
            auto value = handle_expression(*expression);
            if (!value)
                return false;

            builder_->CreateRet(*value);
            return true;
        },
        [this](const Statement* statement) {
            return handle_statement(*statement);
        },
        [this, &definition](auto) {
            Log::compiler_error(definition.location()) << 
                "Unhandled definition body encountered during ir gen";
            Log::compiler_error(definition.location()) << "IR gen for " << definition << " failed" << Endl;
            fail();
            return false;
        }
    }, (*definition.body_)->get_value());

    if (!success)
        return nullopt;

    if (!builder_->GetInsertBlock()->getTerminator())
        builder_->CreateRetVoid();

    return function;
}

// ----- STATEMENT HANDLERS -----

bool IR_Generator::handle_statement(const Statement& statement) {
    switch (statement.statement_type) {
        case StatementType::block: 
            return handle_block(statement);

        case StatementType::expression_statement:
            return static_cast<bool>(handle_expression(*get<Expression*>(statement.value)));

        case StatementType::return_: {
            optional<llvm::Value*> value = handle_expression(*get<Expression*>(statement.value));
            if (!*value) {
                Log::compiler_error(statement.location) << 
                    "Missing value while creating return statement";
                fail();
                return false;
            }
            return builder_->CreateRet(*value);
        }

        case StatementType::conditional:
            return handle_conditional(statement);

        case StatementType::switch_s:
            return handle_switch(statement);

        case StatementType::loop:
            return handle_loop(statement);
        
        case StatementType::guard:
            return handle_guard(statement);
        
        case StatementType::assignment:
            assert(false && "not implemented");
            return false;
            
        case IGNORED_STATEMENT_TYPE:
            return false;

        case BAD_STATEMENT_TYPE:
            Log::compiler_error(statement.location) << 
                "Bad statement " << statement << " in handle_statement" << Endl;
            fail();
            assert(false && "bad statement got through to IR_Generator");
            return false;
    }
}

bool IR_Generator::handle_block(const Statement& statement) {
    assert(statement.statement_type == StatementType::block && 
        "IR_Generator::handle_block called with a statement that wasn't a block");

    for (const Statement* sub_statement: get<Block>(statement.value)) {
        switch (sub_statement->statement_type) {
            case StatementType::expression_statement:
                if (!handle_expression_statement(*sub_statement))
                    return false;
                break;

            case StatementType::block:
                if (!handle_block(*sub_statement))
                    return false;
                break;

            default:
                if (!handle_statement(*sub_statement))
                    return false;
                break;
        }
    }
    
    return true;
}

bool IR_Generator::handle_conditional(const Statement& statement) {
    auto [condition, then_statement, else_statement] = statement.get_value<ConditionalValue>();

    bool both_terminated = true;

    auto condition_value = handle_expression(*condition);

    if (!condition_value) {
        Log::compiler_error(statement.location) << "Handling conditional condition failed" << Endl;
        fail();
        return false;
    }

    auto trunc_condition = 
        builder_->CreateTrunc(*condition_value, llvm::Type::getInt1Ty(*context_));

    auto current_function = builder_->GetInsertBlock()->getParent();

    llvm::BasicBlock* then_branch = llvm::BasicBlock::Create(*context_, "then", current_function);
    llvm::BasicBlock* onward_branch = llvm::BasicBlock::Create(*context_, "onward", current_function);

    if (!else_statement) {
        builder_->CreateCondBr(trunc_condition, then_branch, onward_branch);
        builder_->SetInsertPoint(then_branch);
        
        if (!handle_statement(*then_statement)) {
            Log::compiler_error(statement.location) << 
                "Handling conditional statement then branch failed";
            fail();
            return false;
        }

        if (!block_has_terminated())
            builder_->CreateBr(onward_branch);

        builder_->SetInsertPoint(onward_branch);
        return true;
    }

    llvm::BasicBlock* else_branch = llvm::BasicBlock::Create(*context_, "else", current_function);

    builder_->CreateCondBr(trunc_condition, then_branch, else_branch);
    builder_->SetInsertPoint(then_branch);

    if (!handle_statement(*then_statement)) {
        Log::compiler_error(statement.location) << "Handling conditional statement then branch failed" << Endl;
        fail();
        return false;
    }

    if (!block_has_terminated()) {
        both_terminated = false;
        builder_->CreateBr(onward_branch);
    }

    builder_->SetInsertPoint(else_branch);
    if (!handle_statement(**else_statement)) {
        Log::compiler_error(statement.location) << "Handling conditional statement else branch failed" << Endl;
        fail();
        return false;
    }

    if (!block_has_terminated()) {
        both_terminated = false;
        builder_->CreateBr(onward_branch);
    }

    if (both_terminated) {
        onward_branch->eraseFromParent();
        return true;
    }

    builder_->SetInsertPoint(onward_branch);
    return true;
}

bool IR_Generator::handle_switch(const Statement& statement) {
    assert(false && "not implemented");
    return true;
}

bool IR_Generator::handle_loop(const Statement& statement) {
    assert(false && "not implemented");
    return true;
}

bool IR_Generator::handle_guard(const Statement& statement) {
    assert(false && "not implemented");
    return true;
}

std::optional<llvm::Value*> IR_Generator::handle_expression_statement(
    const Statement& statement) {
    
    assert(statement.statement_type == StatementType::expression_statement && 
        "IR_Generator::handle_expression_statement called with non-expression statement");

    Expression* expression = get<Expression*>(statement.value);
    optional<llvm::Value*> value = handle_expression(*expression);

    if (!*value)
        return nullopt;

    return value;
}

// ----- EXPRESSION HANDLERS -----

optional<llvm::Value*> IR_Generator::handle_expression(const Expression& expression) {
    switch (expression.expression_type) {
        case ExpressionType::call:
            return handle_call(expression);

        case ExpressionType::known_value:
            return handle_value(expression);

        default:
            Log::error(expression.location) << "error during codegen: unhandled: " 
                   << expression << "\n";

            has_failed_ = true;
            return nullopt;
    }
}

const FunctionType* deduce_function_type(TypeStore& types, 
    const Expression& call) {

    auto [callee, args] = std::get<CallExpressionValue>(call.value);

    // hack to make printing work for now
    auto callee_type = dynamic_cast<const FunctionType*>(callee->get_type());
    auto return_type = callee_type->return_type();
    std::vector<const Type*> arg_types;

    for (auto arg: args)
        arg_types.push_back(arg->type);

    assert(arg_types.size() == callee_type->arity() && 
        "handle call encountered a call with wrong number of args");

    return types.get_function_type(
        return_type, arg_types, callee_type->is_pure());
}

llvm::Value* IR_Generator::handle_call(const Expression& call) {
    using llvm::ConstantInt, llvm::APInt;
    auto [callee, args] = std::get<CallExpressionValue>(call.value);

    Log::debug_extra(call.location) << "Creating call to " << *callee << Endl;

    // auto deduced_type = deduce_function_type(*compilation_state_->types_, call);
    // Log::debug_extra("Deduced function type to be " + deduced_type->name_string(), call.location);

    std::optional<llvm::FunctionCallee> function = function_store_->get(*callee);

    if (!function) {
        Log::compiler_error(call.location) << "Attempt to call unknown function: \"" << *callee << "\"" << Endl;
        fail();
        return nullptr;
    }

    // handle args
    std::vector<llvm::Value*> arg_values = {};

    for (Expression* arg_expr : args) {
        if (arg_expr->expression_type == 
            ExpressionType::known_value && *arg_expr->type == MutString) {

            maps_MutString maps_str = std::get<maps_MutString>(arg_expr->value);
            llvm::Constant* data = builder_->CreateGlobalString(maps_str.data);
            
            auto mut_str = llvm::ConstantStruct::get(types_.mutstring_t, {
                    data,
                    ConstantInt::get(*context_, 
                        APInt(8*sizeof(maps_UInt), maps_str.length, false)), 
                    ConstantInt::get(*context_, 
                        APInt(8*sizeof(maps_MemUInt), maps_str.mem_size, false))
            });

            auto alloca = builder_->CreateAlloca(types_.mutstring_t, 0, "test");
            builder_->CreateStore(mut_str, alloca);

            arg_values.push_back(alloca);

        } else {
            optional<llvm::Value*> value = handle_expression(*arg_expr);
            if (value) {
                arg_values.push_back(*value);
            }
        }
    }

    auto llvm_call = builder_->CreateCall(*function, arg_values);

    // if (callee->name_string() == "concat_MutString_MutString") {
    //     llvm_call->addParamAttr(0, llvm::Attribute::ByVal);
    //     llvm_call->addParamAttr(1, llvm::Attribute::ByVal);
    // }

    return llvm_call;
}

// ----- VALUE HANDLERS -----

llvm::Value* IR_Generator::handle_value(const Expression& expression) {
    using llvm::ConstantInt, llvm::APInt;

    // TODO: make typeids known at compile time so this can be a switch
    if (*expression.type == Int) {
        assert(std::holds_alternative<maps_Int>(expression.value) && 
            "type on expression didn't match value");
        return llvm::ConstantInt::get(*context_, 
            llvm::APInt(32, std::get<maps_Int>(expression.value), true));

    } else if (*expression.type == Float) {
        return llvm::ConstantFP::get(*context_, 
            llvm::APFloat(std::get<maps_Float>(expression.value)));
        
    } else if (*expression.type == Boolean) {
        return llvm::ConstantInt::get(*context_, 
            llvm::APInt(8, std::get<maps_Boolean>(expression.value), true));
                
    } else if (*expression.type == String) {
        return builder_->CreateGlobalString(std::get<std::string>(expression.value)); 

    } else if (*expression.type == MutString) {
        maps_MutString maps_str = std::get<maps_MutString>(expression.value);
        llvm::Constant* data = builder_->CreateGlobalString(maps_str.data);

        return llvm::ConstantStruct::get(types_.mutstring_t, {
            data,
            ConstantInt::get(*context_, APInt(8*sizeof(maps_UInt), maps_str.length, false)), 
            ConstantInt::get(*context_, APInt(8*sizeof(maps_MemUInt), maps_str.mem_size, false))
        });

    } else {
        assert(false && "type not implemented in IR_Generator::handle_value");
        return nullptr;
    }
}

// TODO: some assertions here for variant types
optional<llvm::Value*> IR_Generator::convert_value(const Expression& expression) {
    auto concrete_type = dynamic_cast<const ConcreteType*>(expression.type);
    assert(concrete_type && "IR_Generator::convert_value called with not a concrete type");
    switch (concrete_type->concrete_type_id_) {
        case Int_ID:
            assert(std::holds_alternative<maps_Int>(expression.value) && 
                "In IR_Generator::convert_value: expression type didn't match value");
            return llvm::ConstantInt::get(*context_, 
                llvm::APInt(32, std::get<maps_Int>(expression.value)));

        case Float_ID:
            assert(std::holds_alternative<maps_Float>(expression.value) && 
                "In IR_Generator::convert_value: expression type didn't match value");
            return llvm::ConstantFP::get(*context_, 
                llvm::APFloat(std::get<maps_Float>(expression.value)));

        case String_ID:
            assert(std::holds_alternative<std::string>(expression.value) && 
                "In IR_Generator::convert_value: expression type didn't match value");
            return builder_->CreateGlobalString(expression.string_value());

        case Boolean_ID:
            assert(std::holds_alternative<bool>(expression.value) && 
                "In IR_Generator::convert_value: expression type didn't match value");
            return llvm::ConstantInt::get(*context_, 
                llvm::APInt(8, std::get<bool>(expression.value)));

        default:
            Log::compiler_error(expression.location) << 
                "Unable to create a value from type " << *expression.type;
            return fail_optional();
    }
}

optional<llvm::Value*> IR_Generator::global_constant(const Expression& expression) {
    if (!is_reduced_value(expression)) {
        Log::compiler_error(expression.location) << expression << " is not a reduced value" << Endl;
        return fail_optional();
    }

    if (expression.expression_type == ExpressionType::known_value) {
        return convert_value(expression);
    }

    Log::compiler_error(expression.location) << "Could not create a global constant from " << expression << Endl;
    return fail_optional();
}

optional<llvm::Value*> IR_Generator::convert_literal(const Expression& expression) {
    if (*expression.type == String)
        return builder_->CreateGlobalString(expression.string_value());

    if (*expression.type == NumberLiteral)
        return convert_numeric_literal(expression);

    Log::compiler_error(expression.location) << expression << " is not a literal" << Endl;
    assert(false && "IR_Generator::convert_literal called with a non-literal");

    return fail_optional();
}

// ??? shouldn;t this have been done earlier
optional<llvm::Value*> IR_Generator::convert_numeric_literal(const Expression& expression) {
    double num_value;
    if (!(std::stringstream{std::string{expression.string_value()}} >> num_value))
        return nullopt;

    return llvm::ConstantFP::get(*context_, llvm::APFloat(num_value));
}

} // namespace LLVM_IR
} // nameespace Maps
