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

#include "common/string_helpers.hh"

#include "mapsc/logging.hh"
#include "mapsc/ast/ast_store.hh"

#include "mapsc/llvm/ir_builtins.hh"

using llvm::LLVMContext;
using std::optional, std::nullopt, std::vector, std::tuple, std::get, std::get_if, std::unique_ptr, std::make_unique;
using Logging::log_error, Logging::log_info;
using Maps::Expression, Maps::Statement, Maps::Callable, Maps::ExpressionType, Maps::StatementType, 
    Maps::PragmaStore;
using Maps::Helpers::capitalize;

#define BAD_STATEMENT_TYPE StatementType::broken:\
                      case StatementType::illegal:\
                      case StatementType::deleted

#define IGNORED_STATEMENT_TYPE StatementType::operator_definition:\
                          case StatementType::empty

namespace IR {

namespace {

// creates the internal name for a function based on arg types
std::string create_internal_name(const std::string& name, const Maps::FunctionType& ast_type) {
    std::string internal_name = name;

    // prepend arg names
    for (const Maps::Type* arg_type: ast_type.arg_types_) {
        internal_name += "_" + static_cast<std::string>(arg_type->name());
    }

    return internal_name;
}

} // namespace

// ----- IR GENERATOR -----

IR_Generator::IR_Generator(llvm::LLVMContext* context, llvm::Module* module, const Maps::AST_Store& ast, 
    PragmaStore& pragmas, llvm::raw_ostream* error_stream, Options options)
:errs_(error_stream), 
 context_(context), 
 module_(module), 
 types_({*context_}), 
 options_(options), 
 pragmas_(&pragmas), 
 ast_(&ast), 
 maps_types_(ast.types_.get()) {

    if (!types_.is_good_)
        fail("Initializing type map failed");

    builder_ = std::make_unique<llvm::IRBuilder<>>(*context_);
}

bool IR_Generator::run() {
    // Not allowed to run if we have already failed
    if (has_failed_)
        return false;

    if (!handle_global_functions())
        return false;

    // TODO 0.2: check if global eval context
    
    // TODO 0.1: add main wrapper
    // fix main to have the correct type
    // std::optional<AST::Callable*> main = ast->globals_->get_identifier("main");
    // if (main) {
    //     (*main)->arg_types = { &AST::Int, &AST::String };
    // }

    if (!verify_module()) {
        fail("Module verification failed");
        return false;
    }

    return !has_failed_;
}

bool IR_Generator::repl_run() {
    // Not allowed to run if we have already failed
    if (has_failed_)
        return false;

    if (!handle_global_functions())
        return false;

    if (!eval_and_print_root())
        return false;

    if (!verify_module()) {
        fail("Module verification failed");
        return false;
    }

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

void IR_Generator::fail(const std::string& message) {
    has_failed_ = true;
    *errs_ << "error during codegen: " << message << "\n";
    errs_->flush();
}

optional<llvm::Function*> IR_Generator::function_definition(const std::string& name, 
    const Maps::FunctionType& ast_type, llvm::FunctionType* llvm_type, llvm::Function::LinkageTypes linkage) {

    if (!ast_type.is_function()) {
        log_error("IR::Generator::function_definition called with a non-function type: " + ast_type.to_string());
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
bool IR_Generator::close_function_definition(const llvm::Function& function, llvm::Value* return_value) {
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

// --------- HIGH-LEVEL HANDLERS ---------

optional<llvm::Function*> IR_Generator::eval_and_print_root() {
    if (holds_alternative<std::monostate>(ast_->root_->body))
        return nullopt;

    optional<llvm::Function*> top_level_function;

    auto root_function = handle_global_definition(*ast_->root_);

    if (!root_function) {
        fail("Creating REPL wrapper failed");
        return nullopt;
    }

    top_level_function = function_definition(static_cast<std::string>(REPL_WRAPPER_NAME), 
        *ast_->types_->get_function_type(Maps::Void, {}), types_.repl_wrapper_signature);
    
    optional<llvm::FunctionCallee> print = 
        function_store_->get("print", *ast_->types_->get_function_type(Maps::Void, {ast_->root_->get_type()}));
    
    builder_->CreateCall(*print, builder_->CreateCall(*root_function));

    if (!close_function_definition(**top_level_function)) {
        fail("REPL wrapper failed verification");
        return nullopt;    
    }

    return top_level_function;
}

bool IR_Generator::handle_global_functions() {
    for (auto [_1, callable]: *ast_) {
        if (!handle_global_definition(*callable))
            return false;
    }
    
    return true;
}

std::optional<llvm::FunctionCallee> IR_Generator::wrap_value_in_function(const std::string& name, const Maps::Expression& expression) {
    const Maps::FunctionType* maps_type = 
        ast_->types_->get_function_type(*expression.type, {});

    optional<llvm::FunctionType*> llvm_type = types_.convert_function_type(
        *maps_type->return_type_, maps_type->arg_types_);

    if (!llvm_type) {
        fail("Converting \" Void -> " + maps_type->to_string() + "\" into an llvm type failed");
        return nullopt;
    }
    
    auto wrapper = function_definition(name, *maps_type, *llvm_type);

    auto value = global_constant(expression);

    if (!value) {
        fail("Converting " + expression.log_message_string() + " to a value failed");
        return nullopt;
    }

    builder_->CreateRet(*value);

    return wrapper;
}

std::optional<llvm::FunctionCallee> IR_Generator::handle_global_definition(
    const Maps::Callable& callable) {
    
    if (callable.get_type()->is_function())
        return handle_function(callable);
    
    if (const Expression* const* expression = std::get_if<Maps::Expression*>(&callable.body)) {
        return wrap_value_in_function(callable.name, **expression);
    }

    fail("In IR_Generator::handle_global_definition: callable didn't have a function type but wasn't an expression");
    return nullopt;
}

std::optional<llvm::FunctionCallee> IR_Generator::handle_function(const Maps::Callable& callable) {
    assert(callable.get_type()->is_function() && 
        "IR_Generator::handle function called with a non-function callable");

    const Maps::FunctionType* function_type = dynamic_cast<const Maps::FunctionType*>(
        callable.get_type());

    optional<llvm::FunctionType*> signature = types_.convert_function_type(
        *function_type->return_type_, function_type->arg_types_);

    if (!signature) {
        assert(callable.location && 
            "in IR_Generator::handle_function: callable missing location, did it try to handle a builtin");
        Logging::log_error("unable to convert type signature for " + callable.name, 
            *callable.location);
        return nullopt;
    }

    optional<llvm::Function*> function = function_definition(callable.name, 
        *dynamic_cast<const Maps::FunctionType*>(callable.get_type()), *signature);

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

        case StatementType::let:
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

std::optional<llvm::Value*> IR_Generator::handle_expression_statement(const Maps::Statement& statement, 
    bool repl_top_level) {
    
    assert(statement.statement_type == StatementType::expression_statement && 
        "IR_Generator::handle_expression_statement called with non-expression statement");

    Maps::Expression* expression = get<Expression*>(statement.value);
    optional<llvm::Value*> value = handle_expression(*expression);

    if (!value)
        return nullopt;

    if (!repl_top_level)
        return value;

    optional<llvm::FunctionCallee> print = 
        function_store_->get("print", *maps_types_->get_function_type(Maps::Void, {expression->type}));

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
            return handle_call(std::get<Maps::CallExpressionValue>(expression.value));

        case ExpressionType::value:
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

llvm::Value* IR_Generator::handle_call(const Maps::CallExpressionValue& call) {
    auto [callee, args] = call;

    std::optional<llvm::FunctionCallee> function = function_store_->get(
        callee->name, *dynamic_cast<const Maps::FunctionType*>(callee->get_type()));

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

// ----- VALUE HANDLERS -----

llvm::Value* IR_Generator::handle_value(const Maps::Expression& expression) {
    // TODO: make typeids known at compile time so this can be a switch
    if (*expression.type == Maps::Int) {
        assert(std::holds_alternative<long>(expression.value) && 
            "type on expression didn't match value");
        return llvm::ConstantInt::get(*context_, 
            llvm::APInt(64, std::get<long>(expression.value), true));
    // } else if (*expression.type == Maps::String) {

    } else {
        assert(false && "type not implemented in IR_Generator::handle_value");
        return nullptr;
    }
}

// TODO: some assertions here for variant types
optional<llvm::Value*> IR_Generator::convert_value(const Expression& expression) {
    switch (expression.type->id_) {
        case Maps::Int.id_:
            assert(std::holds_alternative<long>(expression.value) && 
                "In IR_Generator::convert_value: expression type didn't match value");
            return llvm::ConstantInt::get(*context_, llvm::APInt(64, std::get<long>(expression.value)));

        case Maps::Float.id_:
            assert(std::holds_alternative<double>(expression.value) && 
                "In IR_Generator::convert_value: expression type didn't match value");
            return llvm::ConstantFP::get(*context_, llvm::APFloat(std::get<double>(expression.value)));

        case Maps::String.id_:
            assert(std::holds_alternative<std::string>(expression.value) && 
                "In IR_Generator::convert_value: expression type didn't match value");
            return builder_->CreateGlobalString(expression.string_value());

        case Maps::Boolean.id_:
            assert(std::holds_alternative<bool>(expression.value) && 
                "In IR_Generator::convert_value: expression type didn't match value");
            return llvm::ConstantInt::get(*context_, llvm::APInt(1, std::get<bool>(expression.value)));

        default:
            fail("Unable to create a value from type " + expression.type->to_string());
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

    if (expression.expression_type == ExpressionType::value) {
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