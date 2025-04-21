#include "ir_generator.hh"

#include <cassert>
#include <variant>
#include <sstream>

#include "llvm/Support/raw_ostream.h"
#include "llvm/Support/FileSystem.h"

#include "llvm/IR/Verifier.h"

#include "llvm/IR/BasicBlock.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/Constants.h"

#include "../lang/ast.hh"
#include "ir_builtins.hh"

using llvm::LLVMContext;
using std::optional, std::nullopt, std::vector, std::tuple, std::get, std::get_if, std::unique_ptr, std::make_unique;
using Logging::log_error, Logging::log_info;
using AST::Expression, AST::Statement, AST::Callable, AST::ExpressionType, AST::StatementType;
using Pragma::Pragmas;

#define BAD_STATEMENT_TYPE StatementType::broken: case StatementType::illegal
#define IGNORED_STATEMENT_TYPE StatementType::operator_s: case StatementType::empty

namespace IR {

// ----- IR Generation -----

IR_Generator::IR_Generator(llvm::LLVMContext* context, llvm::Module* module, llvm::raw_ostream* error_stream)
:errs_(error_stream), context_(context), module_(module), types_({*context_}) {
    builder_ = std::make_unique<llvm::IRBuilder<>>(*context_);
}

bool IR_Generator::run(const AST::AST& ast, Pragma::Pragmas* pragmas) {
    if (pragmas)
        set_pragmas(pragmas);

    // global definitions
    handle_global_functions(ast);
    // check if global eval context
    
    // fix main to have the correct type
    // std::optional<AST::Callable*> main = ast->globals_->get_identifier("main");
    // if (main) {
    //     (*main)->arg_types = { &AST::Int, &AST::String };
    // }

    // for (auto& function : ast_->callables_) {
    //     handle_function(*function.get());
    // }

    return !has_failed_;
}

bool IR_Generator::repl_run(const AST::AST& ast, Pragma::Pragmas* pragmas) {
    if (pragmas)
        set_pragmas(pragmas);

    if (!handle_global_functions(ast))
        return false;

    if (!handle_top_level_execution(ast, true))
        return false;

    return true;
}


bool IR_Generator::print_ir_to_file(const std::string& filename) {
    // prepare the stream
    std::error_code error_code; //??? what to do with this?
    llvm::raw_fd_ostream ostream{filename, error_code, llvm::sys::fs::OF_None};

    module_->print(ostream, nullptr);
    ostream.flush();

    return true;
}


// --------- HELPERS ---------

llvm::Function* IR_Generator::function_definition(const std::string& name, llvm::FunctionType* type, 
    llvm::Function::LinkageTypes linkage) {

    llvm::Function* function = llvm::Function::Create(type, linkage, name, module_);
    insert_function(name, type, function);
    llvm::BasicBlock* body = llvm::BasicBlock::Create(*context_, "", function);
    builder_->SetInsertPoint(body);
    return function;
}

llvm::Function* IR_Generator::function_declaration(const std::string& name, llvm::FunctionType* type, 
    llvm::Function::LinkageTypes linkage) {
    
    llvm::Function* function = llvm::Function::Create(type, linkage, name, module_);
    insert_function(name, type, function);
    return function;
}

optional<llvm::Value*> IR_Generator::global_constant(const Callable& callable) {
    if (Expression* const* expression = std::get_if<Expression*>(&callable.body)) {
        if (!(*expression)->is_reduced_value())
            return nullopt;

        convert_literal(**expression);

    } else if (Statement* const* statement = std::get_if<Statement*>(&callable.body)) {
        if ((*statement)->statement_type != StatementType::expression_statement)
            return nullopt;

        Expression* expression = get<Expression*>((*statement)->value);
        if (!expression->is_reduced_value())
            return nullopt;

        convert_literal(*expression);
    }
    
    assert(false && 
        "IR::Generator::global constant called with a callable that wasn't a expression or an expression statement");
    return nullopt;
}

std::optional<llvm::FunctionCallee> IR_Generator::get_function(const std::string& name, llvm::FunctionType* function_type) const {
    auto outer_it = functions_->find(name);

    if (outer_it == functions_->end())
        return nullopt;

    // if function type wasn't given, check if there's only one
    if (!function_type) {
        if (outer_it->second->size() == 1)
            return outer_it->second->begin()->second;

        return nullopt;
    }

    auto inner_map = outer_it->second.get();
    auto inner_it = inner_map->find(function_type);

    if (inner_it == inner_map->end())
        return nullopt;

    return inner_it->second;
}

bool IR_Generator::insert_function(const std::string& name, llvm::FunctionType* type, llvm::Function* function) {
    using inner_map_t = std::unordered_map<llvm::FunctionType*, llvm::FunctionCallee>;
    auto outer_it = functions_->find(name);

    if (outer_it == functions_->end()) {
        functions_->insert({name, make_unique<inner_map_t>()});
        functions_->at(name)->insert({type, {type, function}});
        return true;
    }

    auto inner_map = outer_it->second.get();
    if (inner_map->find(type) != inner_map->end()) {
        fail("tried to insert a function overload that already exists");
        return false;
    }

    inner_map->insert({type, {type, function}});
    return true;
}

optional<llvm::Value*> IR_Generator::convert_literal(const Expression& expression) const {
    if (expression.type == AST::String)
        return builder_->CreateGlobalString(expression.string_value());

    if (expression.type.is_numeric() == AST::DeferredBool::true_)
        return convert_numeric_literal(expression);

    return nullopt;
}

optional<llvm::Value*> IR_Generator::convert_numeric_literal(const Expression& expression) const {
    assert(expression.type.is_numeric() == AST::DeferredBool::true_ 
        && "convert_numeric_literal called with a non-num value");
    
    double num_value;
    if (!(std::stringstream{expression.string_value()} >> num_value))
        return nullopt;

    return llvm::ConstantFP::get(*context_, llvm::APFloat(num_value));
}


// --------- HANDLERS ---------

bool IR_Generator::handle_global_functions(const AST::AST& ast) {
    for (std::string name: ast.globals_->identifiers_in_order_) {
        std::optional<AST::Callable*> callable = ast.globals_->get_identifier(name);
        assert(callable && "nonexistent name in ast.globals_.identifiers_in_order");

        if (!handle_global_definition(**callable))
            return false;
    }

    return true;
}

optional<llvm::Function*> IR_Generator::handle_top_level_execution(const AST::AST& ast, bool in_repl) {
    if (holds_alternative<std::monostate>(ast.root_->body))
        return nullopt;

    llvm::Function* top_level_function;

    if (in_repl) {
        top_level_function = function_definition(static_cast<std::string>(REPL_WRAPPER_NAME), types_.repl_wrapper_signature);
    } else {
        top_level_function = function_definition("main", types_.cmain_signature);
    }
    
    if (Statement* const * statement = get_if<Statement*>(&ast.root_->body)) {
        // auto str = builder_->CreateGlobalString("asd");

        // auto print = get_function("print");

        // builder_->CreateCall(*print, str);
        // builder_->CreateRetVoid();
        // return top_level_function;

        switch ((*statement)->statement_type) {
            case StatementType::block:
                handle_block(**statement, true);
                break;

            case StatementType::expression_statement:
                assert(false && "not implemented");
                return nullopt;

            default:
                assert(false && "unexpected statement type as top-level statement");
                fail("unexpected statement type as top-level statement");
                return nullopt;
        }
        
        builder_->CreateRetVoid();
        return top_level_function;

    } else if (Expression* const * expression = get_if<Expression*>(&ast.root_->body)) {
        optional<llvm::Value*> expression_value = handle_expression(**expression);

        if (!expression_value) {
            fail("codegen for top-level expression failed");
            return nullopt;
        }

        optional<llvm::FunctionCallee> print = 
            get_function("print", llvm::FunctionType::get(types_.void_t, {(*expression_value)->getType()}, false));
            
        if (!print) {
            fail("no print function for top level expression");
            return nullopt;
        }

        builder_->CreateCall(*print, *expression_value);
        return top_level_function;
    }

    assert(false && "unhandled callable type in handle_top_level");
    return nullopt;
}

bool IR_Generator::handle_global_definition(const AST::Callable& callable) {
    if (!callable.get_type().is_function()) {
        return global_constant(callable).has_value();
    }

    handle_function(callable);
    return true;
}

llvm::Value* IR_Generator::handle_callable(const Callable& callable) {
    assert(false && "not implemented");
    return nullptr; //!!!
}

bool IR_Generator::handle_statement(const Statement& statement) {
    switch (statement.statement_type) {
        case StatementType::block: 
            return handle_block(statement);

        case StatementType::expression_statement:
            return static_cast<bool>(handle_expression(*get<AST::Expression*>(statement.value)));

        case StatementType::return_: {
            optional<llvm::Value*> value = handle_expression(*get<Expression*>(statement.value));
            if (!value)
                fail("Missing value while createing return statement");
            return builder_->CreateRet(*value);
        }

        case StatementType::let:
        case StatementType::assignment:
            assert(false && "not implemented");

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

    for (const Statement* sub_statement: get<AST::Block>(statement.value)) {
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

std::optional<llvm::Value*> IR_Generator::handle_expression_statement(const AST::Statement& statement, bool repl_top_level) {
    assert(statement.statement_type == StatementType::expression_statement && 
        "IR_Generator::handle_expression_statement called with non-expression statement");

    optional<llvm::Value*> value = handle_expression(*get<Expression*>(statement.value));

    if (!value)
        return nullopt;

    if (!repl_top_level)
        return value;

    optional<llvm::FunctionCallee> print = 
        get_function("print", llvm::FunctionType::get(types_.void_t, {(*value)->getType()}, false));

    if (!print) {
        fail("no print function for top level expression");
        return nullopt;
    }

    builder_->CreateCall(*print, *value);

    return value;
}

optional<llvm::Value*> IR_Generator::handle_expression(const Expression& expression) {
    switch (expression.expression_type) {
        case ExpressionType::call:
            return handle_call(expression);
            break;

        case ExpressionType::string_literal:
            return handle_string_literal(expression);
            break;

        case ExpressionType::numeric_literal:
            return convert_numeric_literal(expression);
            break;

        // case ExpressionType::native_operator:
        //     handle_native_operator(expression);
        //     break;

        // case ExpressionType::function_body:
        //     *errs_ << "error during codegen: encountered function_body without a callable wrapper\n";
        //     has_failed_ = true;
        //     return nullptr;

        // case ExpressionType::callable_expression:
        //     *errs_ << "error during codegen: encountered callable_expression\n";
        //     has_failed_ = true;
        //     return nullptr;

        default:
            *errs_ << "error during codegen: encountered unhandled expression type\n";
            errs_->flush();

            has_failed_ = true;
            return nullopt;
    }
}

std::optional<llvm::Function*> IR_Generator::handle_function(const AST::Callable& callable) {
    assert(callable.get_type().is_function() && "IR_Generator::handle function called with a non-function callable");

    auto [return_type_, arg_types_, _1, _2, _3, _4] 
        = *callable.get_type().function_type();

    optional<llvm::FunctionType*> signature = types_.convert_function_type(return_type_, arg_types_);

    if (!signature) {
        assert(callable.location && "in IR_Generator::handle_function: callable missing location, did it try to handle a builtin");
        Logging::log_error(*callable.location, "unable to convert type signature for " + callable.name);
        return nullopt;
    }

    if (!function_name_is_ok(callable.name)) {
        fail("invalid function name \"" + callable.name + "\" (likely conflicts with a builtin or an internal name)");
        return nullopt;
    }

    llvm::Function* function = function_definition(callable.name, *signature);

    // if (AST::Statement** statement = std::get_if<AST::Statement*>(&callable.body)) {
    //     return handle_statement(**statement);
    // }
    // if (Expression** expression = std::get_if<Expression*>(&callable.body)) {
    //     return handle_expression(**expression);
    // }
    
    return function;
}


llvm::Value* IR_Generator::handle_call(const Expression& expression) {
    auto [callee, args] = get<AST::CallExpressionValue>(expression.value);

    std::optional<llvm::FunctionCallee> function = get_function(callee->name);

    if (!function) {
        fail("attempt to call unknown function: \"" + callee->name + "\"");
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

llvm::GlobalVariable* IR_Generator::handle_string_literal(const Expression& expression) {
    return builder_->CreateGlobalString(expression.string_value());
}

void IR_Generator::fail(const std::string& message) {
    has_failed_ = true;
    *errs_ << "error during codegen: " << message << "\n";
    errs_->flush();
}

bool IR_Generator::function_name_is_ok(const std::string& name) {
    return (
        name != "print" &&
        name != "puts" &&
        name != "sprintf"
    );
}

} // namespace IR