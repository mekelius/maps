#include "ir_generator.hh"

using namespace llvm;

// ----- IR Generation -----

IR_Generator::IR_Generator(const std::string& module_name, std::ostream* info_stream)
:errs_(info_stream) {
    context_ = std::make_unique<LLVMContext>();
    module_ = std::make_unique<Module>(module_name, *context_);
    builder_ = std::make_unique<IRBuilder<>>(*context_);

    if (type_map_.size() == 0)
        populate_type_map();
}

Function* IR_Generator::function_definition(const std::string& name, FunctionType* type, Function::LinkageTypes linkage) {
    Function* function = Function::Create(type, linkage, name, module_.get());
    functions_map_.insert({name, {type, function}});
    BasicBlock* body = BasicBlock::Create(*context_, "entry", function);
    builder_->SetInsertPoint(body);
    return function;
}

Function* IR_Generator::function_declaration(const std::string& name, FunctionType* type, Function::LinkageTypes linkage) {
    Function* function = Function::Create(type, linkage, name, module_.get());
    functions_map_.insert({name, {type, function}});
    return function;
}

Value* IR_Generator::handle_call(AST::Expression& expression) {
    auto [callee, args] = expression.call_expr;

    std::optional<FunctionCallee> function = get_function(callee);

    if (!function) {
        fail("attempt to call unknown function: \"" + callee + "\"");
        return nullptr;
    }

    // handle args
    std::vector<Value*> arg_values = {};

    for (AST::Expression* arg_expr : args) {
        // OK instead of these null values we should panic here
        Value* value = handle_expression(*arg_expr);
        if (value)
            arg_values.push_back(value);
    }

    return builder_->CreateCall(*function, arg_values);
}

GlobalVariable* IR_Generator::handle_string_literal(AST::Expression& expression) {
    return builder_->CreateGlobalString(expression.string_value);
}

Value* IR_Generator::handle_expression(AST::Expression& expression) {
    switch (expression.expression_type) {
        case AST::ExpressionType::call:
            return handle_call(expression);
            break;

        case AST::ExpressionType::string_literal:
            return handle_string_literal(expression);
            break;

        // case AST::ExpressionType::native_operator:
        //     handle_native_operator(expression);
        //     break;

        // case AST::ExpressionType::function_body:
        //     *errs_ << "error during codegen: encountered function_body without a callable wrapper\n";
        //     has_failed_ = true;
        //     return nullptr;

        // case AST::ExpressionType::callable_expression:
        //     *errs_ << "error during codegen: encountered callable_expression\n";
        //     has_failed_ = true;
        //     return nullptr;

        default:
            *errs_ << "error during codegen: encountered unhandled expression type\n";
            has_failed_ = true;
            return nullptr;
    }
}

std::optional<Function*> IR_Generator::handle_function(AST::Callable& callable) {
    // auto [name, expression, arg_types] = callable;
    // const AST::Type* return_type = expression->type;

    // check expression type
    // if (expression->expression_type == AST::ExpressionType::native_function) {
    //     // what to do here
    //     if (name == "print")
    //         return puts_;
    //     if (name == "sprintf")
    //         return sprintf_;

    //     fail("unknown native function definition: " + name);
    //     return {};
    // }

    // if (expression->expression_type != AST::ExpressionType::function_body) {
    //     fail("encountered callable expression: \"" + name + "\" with a non-reduced body");
    //     return {};
    // }

    // if (!function_name_is_ok(name)) {
    //     fail("invalid function name \"" + name + "\" (likely conflicts with a builtin or an internal name)");
    //     return {};
    // }

    // check that types are reduced
    // if (!return_type->is_native) {
    //     fail(
    //         "encountered callable expression: \"" + name + 
    //         "\" with a non-reduced return type: " + static_cast<std::string>(return_type->name)
    //     );
    //     return {};
    // }

    // for (const AST::Type* arg_type : arg_types) {
    //     if (!arg_type->is_native) {
    //         fail(
    //             "encountered callable expression: \"" + name + 
    //             "\" with a non-reduced parameter type: " + static_cast<std::string>(return_type->name)
    //         );
    //         return {};
    //     }
    // }

    // ----- construct the function signature -----
    
    // TODO: this sucks, simplify with enum
    // auto return_type_ = convert_type(return_type);
    // if (!return_type_) {
    //     fail("cannot map type to native type: " + static_cast<std::string>(return_type->name));
    //     return {};
    // }
    
    // std::vector<Type*> arg_types_ = {};
    
    // for (auto arg_type : arg_types) {
    //     auto arg_type_ = convert_type(arg_type);
    //     if (!arg_type_) {
    //         fail("cannot map type to native type: " + static_cast<std::string>(return_type->name));
    //         return {};
    //     }
    //     arg_types_.push_back(*arg_type_);
    // }

    // // TODO: handle vararg
    // FunctionType* signature = FunctionType::get(*return_type_, arg_types_, false);
    // function_definition(name, signature);

    // SINGLE EXPRESSION FUNCTION BODIES SHOULD JUST BE HANDLED AND RETURN
    // builder_->CreateRet(handle_expression(*expression));

    // ???? recursion? does it need to know it's own name for something
    // handle_function_body(body, signature);

    return nullptr;
}

bool IR_Generator::generate_ir(AST::AST* ast) {
    ast_ = ast;

    create_builtins();
    // start_main();
    
    // fix main to have the correct type
    std::optional<AST::Callable*> main = ast_->get_identifier("main");
    if (main) {
        (*main)->arg_types = { &AST::Int, &AST::String };
    }

    // for (auto& function : ast_->callables_) {
    //     handle_function(*function.get());
    // }

    return !has_failed_;
}

void IR_Generator::fail(const std::string& message) {
    has_failed_ = true;
    *errs_ << "error during codegen: " << message << "\n";
}

void IR_Generator::create_builtins() {
    // declare c functions to be used as builtins
    puts_ = function_declaration("puts", FunctionType::get(int_type, {char_array_ptr_type}, false));
    functions_map_.insert({"print", puts_});
    sprintf_ = function_declaration("sprintf", FunctionType::get(void_type, {char_array_13_type, char_array_13_type, double_type}, true));
}

void IR_Generator::populate_type_map() {
    // get some types
    char_type = Type::getInt8Ty(*context_);
    char_array_13_type = ArrayType::get(char_type, 13);
    char_array_ptr_type = PointerType::getUnqual(PointerType::getUnqual(char_type));
    int_type = Type::getInt64Ty(*context_);
    double_type = Type::getDoubleTy(*context_);
    void_type = Type::getVoidTy(*context_);

    type_map_ = {
        { "Int", int_type },
        { "String", char_array_ptr_type },
        { "Void", void_type },
        // { "Boolean", bool_type },
        // { "",  },
    };
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


std::optional<Type*> IR_Generator::convert_type(const AST::Type* type) const {
    auto it = type_map_.find(type->name);
    if (it == type_map_.end())
        return std::nullopt;

    return it->second;
}

void IR_Generator::start_main() {
    // create the main function
    function_definition("main", FunctionType::get(int_type, {int_type, char_array_ptr_type}, false));
}

bool generate_ir(IR_Generator& generator) {
    IRBuilder<>* builder = generator.get_builder();

    Value* lhs = ConstantInt::get(generator.int_type, 9);
    Value* rhs = ConstantInt::get(generator.int_type, 6);
    builder->CreateAdd(lhs, rhs);
    
    return true;
}
