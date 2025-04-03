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

bool generate_ir(IRGenHelper& generator) {
    IRBuilder<>* builder = generator.get_builder();

    Function* puts = generator.function_declaration("puts", FunctionType::get(generator.int_type, {generator.char_array_ptr_type}, false));
    
    Function* hello = generator.function_definition("hello", FunctionType::get(generator.char_array_ptr_type, {}, false));
    builder->CreateRet(builder->CreateGlobalString("Hello World!"));

    Function* main_f = generator.function_definition("main", FunctionType::get(generator.int_type, {generator.int_type, generator.char_array_ptr_type}, false));
    CallInst* hello_call = builder->CreateCall(hello, {}, "msg");
    builder->CreateCall(puts, {hello_call});

    Value* lhs = ConstantInt::get(generator.int_type, 9);
    Value* rhs = ConstantInt::get(generator.int_type, 6);
    builder->CreateAdd(lhs, rhs);

    builder->CreateRet(ConstantInt::get(generator.int_type, 0));
    
    verifyFunction(*hello);
    verifyFunction(*main_f);

    return true;
}
