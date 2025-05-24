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

#include "mapsc/source.hh"
#include "mapsc/logging.hh"

#include "mapsc/compilation_state.hh"

#include "mapsc/types/type.hh"
#include "mapsc/types/function_type.hh"
#include "mapsc/types/type_defs.hh"
#include "mapsc/types/type_store.hh"

#include "mapsc/ast/definition.hh"
#include "mapsc/ast/expression.hh"
#include "mapsc/ast/scope.hh"
#include "mapsc/ast/statement.hh"

using llvm::LLVMContext;
using std::optional, std::nullopt, std::vector, std::tuple, std::get, std::get_if;
using std::unique_ptr, std::make_unique;
using Maps::Expression, Maps::Statement, Maps::Definition, Maps::ExpressionType, 
    Maps::StatementType, Maps::PragmaStore;
using Maps::Helpers::capitalize;

#define BAD_STATEMENT_TYPE StatementType::user_error:\
                      case StatementType::compiler_error:\
                      case StatementType::deleted

#define IGNORED_STATEMENT_TYPE StatementType::empty


using Log = Maps::LogInContext<Maps::LogContext::ir_gen>;
                          
namespace IR {

namespace {

// creates the internal name for a function based on arg types
std::string create_internal_name(const std::string& name, const Maps::FunctionType& ast_type) {
    std::string internal_name = name;

    // prepend arg names
    for (const Maps::Type* arg_type: ast_type.param_types()) {
        internal_name += "_" + static_cast<std::string>(arg_type->name());
    }

    return internal_name;
}

} // namespace

// ----- IR GENERATOR -----

IR_Generator::IR_Generator(llvm::LLVMContext* context, llvm::Module* module,
    const Maps::CompilationState* compilation_state, llvm::raw_ostream* error_stream, 
    Options options)
:errs_(error_stream),
 context_(context),
 module_(module),
 types_({*context_}),
 options_(options),
 compilation_state_(compilation_state), 
 pragmas_(&compilation_state->pragmas_),
 maps_types_(compilation_state->types_) {

    if (!types_.is_good_)
        fail("Initializing type map failed");

    builder_ = std::make_unique<llvm::IRBuilder<>>(*context_);
}

bool IR_Generator::run(Maps::Scopes scopes) {
    return run(scopes, {});
}

bool IR_Generator::run(Maps::Scopes scopes, std::span<Maps::Definition* const> additional_definitions) {
    // Not allowed to run if we have already failed
    if (has_failed_)
        return false;

    for (auto scope: scopes) {
        if (!handle_global_functions(*scope))
            return false;
    }

    for (auto definition: additional_definitions) {
        if (!handle_global_definition(*definition)) {
            fail("Couldn't generate ir for " + definition->name_string());
            return false;
        }
    }

    if (!verify_module()) {
        fail("Module verification failed");
        return false;
    }

    return !has_failed_;
}

void IR_Generator::fail(const std::string& message) {
    has_failed_ = true;
    *errs_ << "error during codegen: " << message << "\n";
    errs_->flush();
}

optional<llvm::Function*> IR_Generator::function_definition(const std::string& name, 
    const Maps::FunctionType& ast_type, llvm::FunctionType* llvm_type, 
    llvm::Function::LinkageTypes linkage) {

    if (!ast_type.is_function()) {
        Log::compiler_error("IR::Generator::function_definition called with a non-function type: " + 
            ast_type.name_string(), NO_SOURCE_LOCATION);
        assert(false && "IR_Generator::function_definition called with non-function ast type");
        return nullopt;
    }

    llvm::Function* function = llvm::Function::Create(llvm_type, linkage, 
        create_internal_name(name, ast_type), module_);
    function_store_->insert(name, ast_type, {llvm_type, function});
    llvm::BasicBlock* body = llvm::BasicBlock::Create(*context_, "", function);
    builder_->SetInsertPoint(body);
    return function;
}

// ??? how to do recursion?
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
    const Maps::FunctionType& ast_type, llvm::FunctionType* llvm_type,
    llvm::Function::LinkageTypes linkage) {
    
    llvm::Function* function = llvm::Function::Create(llvm_type, linkage, 
        create_internal_name(name, ast_type), module_);
    function_store_->insert(name, ast_type, function);
    return function;
}

bool IR_Generator::verify_module() {
    if (!options_.verify_module)
        return true;

    return (!llvm::verifyModule(*module_, errs_));
}

bool IR_Generator::handle_global_functions(const Maps::RT_Scope& scope) {
    for (auto definition: scope) {
        if (!handle_global_definition(*definition))
            return false;
    }
    
    return true;
}

std::optional<llvm::FunctionCallee> IR_Generator::handle_global_definition(
    const Maps::Definition& definition) {
    
    if (definition.get_type()->is_function())
        return handle_function(definition);

    auto definition_body = definition.const_body();

    if (const Expression* const* expression = 
            std::get_if<const Maps::Expression*>(&definition_body))
        return wrap_value_in_function(definition.name_string(), **expression);

    fail("In IR_Generator::handle_global_definition: definition didn't have a function type but wasn't an expression");
    return nullopt;
}

std::optional<llvm::FunctionCallee> IR_Generator::wrap_value_in_function(
    const std::string& name, const Maps::Expression& expression) {
    
    const Maps::FunctionType* maps_type = 
        maps_types_->get_function_type(expression.type, {}, expression.type->is_pure());

    optional<llvm::FunctionType*> llvm_type = types_.convert_function_type(
        *maps_type->return_type(), maps_type->param_types());

    if (!llvm_type) {
        fail("Converting \"Void => " + maps_type->name_string() + "\" into an llvm type failed");
        return nullopt;
    }
    
    auto wrapper = function_definition(name, *maps_type, *llvm_type);
    auto value = handle_expression(expression);

    if (!value) {
        fail("Converting " + expression.log_message_string() + " to a value failed");
        return nullopt;
    }

    if ((*value)->getType() != types_.void_t) {
        builder_->CreateRet(*value);
    } else {
        builder_->CreateRetVoid();
    }

    return wrapper;
}

std::optional<llvm::FunctionCallee> IR_Generator::handle_function(const Maps::Definition& definition) {
    assert(definition.get_type()->is_function() && 
        "IR_Generator::handle function called with a non-function definition");

    const Maps::FunctionType* function_type = dynamic_cast<const Maps::FunctionType*>(
        definition.get_type());

    optional<llvm::FunctionType*> signature = types_.convert_function_type(
        *function_type->return_type(), function_type->param_types());

    if (!signature) {
        Log::error("unable to convert type signature for " + 
            definition.name_string(), definition.location());
        return nullopt;
    }

    optional<llvm::Function*> function = function_definition(definition.name_string(), 
        *dynamic_cast<const Maps::FunctionType*>(definition.get_type()), *signature);

    if (!function)
        return nullopt;
    
    return function;
}

// ----- STATEMENT HANDLERS -----

bool IR_Generator::handle_statement(const Statement& statement) {
    switch (statement.statement_type) {
        case StatementType::block: 
            return handle_block(statement);

        case StatementType::expression_statement:
            return static_cast<bool>(handle_expression(*get<Maps::Expression*>(statement.value)));

        case StatementType::return_: {
            optional<llvm::Value*> value = handle_expression(*get<Expression*>(statement.value));
            if (!value)
                fail("Missing value while creating return statement");
            return builder_->CreateRet(*value);
        }

        case StatementType::assignment:
            assert(false && "not implemented");
            return false;

        case IGNORED_STATEMENT_TYPE:
            return false;

        case BAD_STATEMENT_TYPE:
            // TODO: print statements nice
            fail("Bad statement in handle_statement: ");
            assert(false && "bad statement got through to IR_Generator");
            return false;
    }
}

bool IR_Generator::handle_block(const Statement& statement, bool repl_top_level) {
    assert(statement.statement_type == StatementType::block && 
        "IR_Generator::handle_block called with a statement that wasn't a block");

    for (const Statement* sub_statement: get<Maps::Block>(statement.value)) {
        switch (sub_statement->statement_type) {
            case StatementType::expression_statement:
                if (!handle_expression_statement(*sub_statement, repl_top_level))
                    return false;
                break;

            case StatementType::block:
                if (!handle_block(*sub_statement, repl_top_level))
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

std::optional<llvm::Value*> IR_Generator::handle_expression_statement(
    const Maps::Statement& statement, bool repl_top_level) {
    
    assert(statement.statement_type == StatementType::expression_statement && 
        "IR_Generator::handle_expression_statement called with non-expression statement");

    Maps::Expression* expression = get<Expression*>(statement.value);
    optional<llvm::Value*> value = handle_expression(*expression);

    if (!value)
        return nullopt;

    if (!repl_top_level)
        return value;

    optional<llvm::FunctionCallee> print = 
        function_store_->get("print", *maps_types_->get_function_type(
            &Maps::Void, {expression->type}, false));

    if (!print) {
        fail("no print function for top level expression");
        return nullopt;
    }

    builder_->CreateCall(*print, *value);

    return value;
}

// ----- EXPRESSION HANDLERS -----

optional<llvm::Value*> IR_Generator::handle_expression(const Expression& expression) {
    switch (expression.expression_type) {
        case ExpressionType::call:
            return handle_call(expression);

        case ExpressionType::known_value:
            return handle_value(expression);

        case ExpressionType::string_literal:
            return handle_string_literal(expression);

        case ExpressionType::numeric_literal:
            return convert_numeric_literal(expression);

        default:
            *errs_ << "error during codegen: encountered unhandled expression type\n";
            errs_->flush();

            has_failed_ = true;
            return nullopt;
    }
}

const Maps::FunctionType* deduce_function_type(Maps::TypeStore& types, 
    const Maps::Expression& call) {

    auto [callee, args] = std::get<Maps::CallExpressionValue>(call.value);

    // hack to make printing work for now
    auto callee_type = dynamic_cast<const Maps::FunctionType*>(callee->get_type());
    auto return_type = callee_type->return_type();
    std::vector<const Maps::Type*> arg_types;

    for (auto arg: args)
        arg_types.push_back(arg->type);

    assert(arg_types.size() == callee_type->arity() && 
        "handle call encountered a call with wrong number of args");

    return types.get_function_type(
        return_type, arg_types, callee_type->is_pure());
}

llvm::Value* IR_Generator::handle_call(const Maps::Expression& call) {
    auto [callee, args] = std::get<Maps::CallExpressionValue>(call.value);

    std::optional<llvm::FunctionCallee> function = function_store_->get(
        callee->name_string(), *deduce_function_type(*compilation_state_->types_, call));

    if (!function) {
        fail("attempt to call unknown function: \"" + callee->name_string() + "\"");
        return nullptr;
    }

    // handle args
    std::vector<llvm::Value*> arg_values = {};

    for (Expression* arg_expr : args) {
        optional<llvm::Value*> value = handle_expression(*arg_expr);
        if (value)
            arg_values.push_back(*value);
    }

    return builder_->CreateCall(*function, arg_values);
}

// ----- VALUE HANDLERS -----

llvm::Value* IR_Generator::handle_value(const Maps::Expression& expression) {
    // TODO: make typeids known at compile time so this can be a switch
    if (*expression.type == Maps::Int) {
        assert(std::holds_alternative<maps_Int>(expression.value) && 
            "type on expression didn't match value");
        return llvm::ConstantInt::get(*context_, 
            llvm::APInt(32, std::get<maps_Int>(expression.value), true));

    } else if (*expression.type == Maps::Float) {
        return llvm::ConstantFP::get(*context_, 
            llvm::APFloat(std::get<maps_Float>(expression.value)));
        
    } else if (*expression.type == Maps::Boolean) {
        return llvm::ConstantInt::get(*context_, 
            llvm::APInt(8, std::get<maps_Boolean>(expression.value), true));
                
    } else if (*expression.type == Maps::String) {
        return builder_->CreateGlobalString(std::get<std::string>(expression.value)); 

    } else {
        assert(false && "type not implemented in IR_Generator::handle_value");
        return nullptr;
    }
}

// TODO: some assertions here for variant types
optional<llvm::Value*> IR_Generator::convert_value(const Expression& expression) {
    auto concrete_type = dynamic_cast<const Maps::ConcreteType*>(expression.type);
    assert(concrete_type && "IR_Generator::convert_value called with not a concrete type");
    switch (concrete_type->concrete_type_id_) {
        case Maps::Int_ID:
            assert(std::holds_alternative<maps_Int>(expression.value) && 
                "In IR_Generator::convert_value: expression type didn't match value");
            return llvm::ConstantInt::get(*context_, 
                llvm::APInt(32, std::get<maps_Int>(expression.value)));

        case Maps::Float_ID:
            assert(std::holds_alternative<maps_Float>(expression.value) && 
                "In IR_Generator::convert_value: expression type didn't match value");
            return llvm::ConstantFP::get(*context_, 
                llvm::APFloat(std::get<maps_Float>(expression.value)));

        case Maps::String_ID:
            assert(std::holds_alternative<std::string>(expression.value) && 
                "In IR_Generator::convert_value: expression type didn't match value");
            return builder_->CreateGlobalString(expression.string_value());

        case Maps::Boolean_ID:
            assert(std::holds_alternative<bool>(expression.value) && 
                "In IR_Generator::convert_value: expression type didn't match value");
            return llvm::ConstantInt::get(*context_, 
                llvm::APInt(8, std::get<bool>(expression.value)));

        default:
            fail("Unable to create a value from type " + expression.type->name_string());
            return nullopt;
    }
}

optional<llvm::Value*> IR_Generator::global_constant(const Maps::Expression& expression) {
    if (!expression.is_reduced_value()) {
        fail(capitalize(expression.log_message_string()) + " is not a reduced value");
        return nullopt;
    }

    if (expression.is_literal())
        return convert_literal(expression);

    if (expression.expression_type == ExpressionType::known_value) {
        return convert_value(expression);
    }

    fail("Could not create a global constant from " + expression.log_message_string());
    return nullopt;
}

optional<llvm::Value*> IR_Generator::convert_literal(const Expression& expression) {
    if (*expression.type == Maps::String)
        return builder_->CreateGlobalString(expression.string_value());

    if (*expression.type == Maps::NumberLiteral)
        return convert_numeric_literal(expression);

    assert(false && "IR_Generator::convert_literal called with a non-literal");
    fail(capitalize(expression.log_message_string()) + " is not a literal");

    return nullopt;
}

llvm::GlobalVariable* IR_Generator::handle_string_literal(const Expression& expression) {
    return builder_->CreateGlobalString(expression.string_value());
}

// ??? shouldn;t this have been done earlier
optional<llvm::Value*> IR_Generator::convert_numeric_literal(const Expression& expression) {
    double num_value;
    if (!(std::stringstream{expression.string_value()} >> num_value))
        return nullopt;

    return llvm::ConstantFP::get(*context_, llvm::APFloat(num_value));
}

} // namespace IR