#include "ir_gen.hh"

using namespace llvm;

// ----- IR Generation -----

IRGenHelper::IRGenHelper(const std::string& module_name) {
    context_ = std::make_unique<LLVMContext>();
    module_ = std::make_unique<Module>(module_name, *context_);
    builder_ = std::make_unique<IRBuilder<>>(*context_);

    // get some types
    char_type = Type::getInt8Ty(*context_);
    char_array_13_type = ArrayType::get(char_type, 13);
    char_array_ptr_type = PointerType::getUnqual(PointerType::getUnqual(char_type));
    int_type = Type::getInt64Ty(*context_);
    double_type = Type::getDoubleTy(*context_);
    void_type = Type::getVoidTy(*context_);

    // declare puts
    puts_ = function_declaration("puts", FunctionType::get(int_type, {char_array_ptr_type}, false));
    sprintf_ = function_declaration("sprintf", FunctionType::get(void_type, {char_array_13_type, char_array_13_type, double_type}, true));

    // create the main function
    main_ = function_definition("main", FunctionType::get(int_type, {int_type, char_array_ptr_type}, false));
}

Function* IRGenHelper::function_definition(const std::string& name, FunctionType* type, Function::LinkageTypes linkage) {
    Function* function = Function::Create(type, linkage, name, module_.get());
    BasicBlock* body = BasicBlock::Create(*context_, "entry", function);
    builder_->SetInsertPoint(body);
    return function;
}

Function* IRGenHelper::function_declaration(const std::string& name, FunctionType* type, Function::LinkageTypes linkage) {
    Function* function = Function::Create(type, linkage, name, module_.get());
    return function;
}

// TODO: name to snake_case
void IRGenHelper::handleCall(AST::Expression* expression) {
    auto [callee, args] = expression->call_expr;

    std::vector<Value*> arg_values;

    for (auto arg: args) {
        arg_values.push_back(handleStringLiteral(arg));
    }

    Function* callee_ref;
    if (callee == "puts" || callee == "print") {
        callee_ref = puts_;
    } else if (callee == "sprintf") {
        callee_ref = sprintf_;
    } else {
        // error !!!
        return;
    }

    builder_->CreateCall(callee_ref, arg_values);
}

// TODO: name to snake case
GlobalVariable* IRGenHelper::handleStringLiteral(AST::Expression* expression) {
    return builder_->CreateGlobalString(expression->string_value);
}

void IRGenHelper::handleExpression(AST::Expression* expression) {
    switch (expression->type) {
        case AST::ExpressionType::call:
            handleCall(expression);
        case AST::ExpressionType::string_literal:
            handleStringLiteral(expression);
    }
}

void IRGenHelper::generate_ir(AST::AST* ast) {
    // handleCall(AST::CallExpression{7.8});

    // builder_->CreateCall(puts_, {builder_->CreateGlobalString("jii")});
    handleExpression(ast->entry_point);

    builder_->CreateRet(ConstantInt::get(int_type, 0));

    verifyFunction(*main_);
    return;
}

bool generate_ir(IRGenHelper& generator) {
    IRBuilder<>* builder = generator.get_builder();

    Function* puts = generator.function_declaration("puts", FunctionType::get(generator.int_type, {generator.char_array_ptr_type}, false));
    
    Function* hello = generator.function_definition("hello", FunctionType::get(generator.char_array_ptr_type, {}, false));
    builder->CreateRet(builder->CreateGlobalString("Hello World!"));

    CallInst* hello_call = builder->CreateCall(hello, {}, "msg");
    builder->CreateCall(puts, {hello_call});

    Value* lhs = ConstantInt::get(generator.int_type, 9);
    Value* rhs = ConstantInt::get(generator.int_type, 6);
    builder->CreateAdd(lhs, rhs);
    
    verifyFunction(*hello);

    return true;
}
