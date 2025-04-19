#include "ir_generator.hh"

#include <cassert>
#include <variant>
#include <sstream>

#include "../lang/ast.hh"
#include "ir_builtins.hh"

using llvm::LLVMContext;
using std::optional, std::nullopt, std::vector, std::tuple, std::get, std::get_if;
using Logging::log_error, Logging::log_info;
using AST::Expression, AST::Statement, AST::Callable, AST::ExpressionType, AST::StatementType;
using Pragma::Pragmas;

namespace IR {

// ----- IR Generation -----

IR_Generator::IR_Generator(const std::string& module_name, std::ostream* info_stream)
:errs_(info_stream), context_(std::make_unique<LLVMContext>()), types_({*context_}) {
    module_ = std::make_unique<llvm::Module>(module_name, *context_);
    builder_ = std::make_unique<llvm::IRBuilder<>>(*context_);

    insert_builtins(*this);
}

bool IR_Generator::run(AST::AST& ast, Pragma::Pragmas* pragmas) {
    if (pragmas)
        set_pragmas(pragmas);

    // global definitions
    for (std::string name: ast.globals_->identifiers_in_order_) {
        std::optional<AST::Callable*> callable = ast.globals_->get_identifier(name);
        assert(callable && "nonexistent name in ast.globals_.identifiers_in_order");

        handle_global_definition(**callable);
    }

    // check if global eval context
    if (true /*pragma global eval context*/) {
        for (AST::Statement* statement: ast.root_) {
            switch (statement->statement_type) {
                // let statements are hoisted
                case StatementType::let:
                    break;

                case StatementType::assignment:
                case StatementType::expression_statement:
                case StatementType::return_:
                    // run it if true
                    break;

                default:
                    assert(false && "uhandled statement type encountered in genererate_ir");
            }
        }
    }
    // start_main();
    
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


// --------- HELPERS ---------

llvm::Function* IR_Generator::function_definition(const std::string& name, llvm::FunctionType* type, 
    llvm::Function::LinkageTypes linkage) {

    llvm::Function* function = llvm::Function::Create(type, linkage, name, module_.get());
    functions_map_.insert({name, {type, function}});
    llvm::BasicBlock* body = llvm::BasicBlock::Create(*context_, "entry", function);
    builder_->SetInsertPoint(body);
    return function;
}

llvm::Function* IR_Generator::function_declaration(const std::string& name, llvm::FunctionType* type, 
    llvm::Function::LinkageTypes linkage) {
    
    llvm::Function* function = llvm::Function::Create(type, linkage, name, module_.get());
    functions_map_.insert({name, {type, function}});
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

bool IR_Generator::handle_global_definition(const AST::Callable& callable) {
    if (!callable.get_type().is_function()) {
        return global_constant(callable).has_value();
    }

    handle_function(callable);
    return true;
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
        // OK instead of these null values we should panic here
        llvm::Value* value = handle_expression(*arg_expr);
        if (value)
            arg_values.push_back(value);
    }

    return builder_->CreateCall(*function, arg_values);
}

llvm::GlobalVariable* IR_Generator::handle_string_literal(const Expression& expression) {
    return builder_->CreateGlobalString(expression.string_value());
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

llvm::Value* IR_Generator::handle_expression(const Expression& expression) {
    switch (expression.expression_type) {
        case ExpressionType::call:
            return handle_call(expression);
            break;

        case ExpressionType::string_literal:
            return handle_string_literal(expression);
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
            has_failed_ = true;
            return nullptr;
    }
}

void IR_Generator::fail(const std::string& message) {
    has_failed_ = true;
    *errs_ << "error during codegen: " << message << "\n";
}

bool IR_Generator::function_name_is_ok(const std::string& name) {
    return (
        name != "print" &&
        name != "puts" &&
        name != "sprintf"
    );
}

std::optional<llvm::FunctionCallee> IR_Generator::get_function(const std::string& name) const {
    auto it = functions_map_.find(name);
    if (it == functions_map_.end())
        return {};

    return it->second;
}

void IR_Generator::start_main() {
    // create the main function
    function_definition("main", 
        llvm::FunctionType::get(types_.int_t, {types_.int_t, types_.char_array_ptr_t}, false));
}

} // namespace IR