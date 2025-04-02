#include "llvm/ADT/APFloat.h"
#include "llvm/ADT/STLExtras.h"
#include "llvm/IR/BasicBlock.h"
#include "llvm/IR/Constants.h"
#include "llvm/IR/DerivedTypes.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/LegacyPassManager.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/Type.h"
#include "llvm/IR/Verifier.h"
#include "llvm/MC/TargetRegistry.h"
#include "llvm/Support/FileSystem.h"
#include "llvm/Support/TargetSelect.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/Target/TargetMachine.h"
#include "llvm/Target/TargetOptions.h"

// these were missing from the tutorial
#include "llvm/Support/Host.h"
#include "llvm/Support/CodeGen.h"

#include <algorithm>
#include <cassert>
#include <map>
#include <memory>
#include <string>
#include <vector>
#include <optional>
#include <iostream>
#include <fstream>

using namespace llvm;

// ----- Constants -----

const std::string IR_FILE_NAME = "./build/out.ll";

// ----- Lexer -----

enum class Token: int {
    eof,
    identifier,
    number,
    oper
};

// -----------------

void mainloop() {
    // Token current_token = get_token();

    // while (current_token != token::eof) {
    //     switch (current_token) {
    //         case token::eof:
    //         return;
    //     }
    // }
}

// ----- IR Generation -----

// Helper class that holds the module, context, etc. for IR generation
class IRGenerator {
  public:
    IRGenerator(const std::string& module_name = "module");

    Function* function_definition(const std::string& name, FunctionType* type, Function::LinkageTypes linkage = Function::ExternalLinkage);
    Function* function_declaration(const std::string& name, FunctionType* type, Function::LinkageTypes linkage = Function::ExternalLinkage);

    IRBuilder<>* get_builder() {
        return builder_.get();
    }

    Module* get_module() {
        return module_.get();
    }

    LLVMContext* get_context() {
        return context_.get();
    }

    Type* char_type;
    Type* int_type;
    Type* char_array_13_type;
    Type* char_array_ptr_type;
  private:
    // do we just leak these? it crashed when I tried to delete them
    std::unique_ptr<LLVMContext> context_;
    std::unique_ptr<Module> module_;
    std::unique_ptr<IRBuilder<>> builder_;
};

IRGenerator::IRGenerator(const std::string& module_name) {
    context_ = std::make_unique<LLVMContext>();
    module_ = std::make_unique<Module>(module_name, *context_);
    builder_ = std::make_unique<IRBuilder<>>(*context_);

    // get some types
    char_type = Type::getInt8Ty(*context_);
    char_array_13_type = ArrayType::get(char_type, 13);
    char_array_ptr_type = PointerType::getUnqual(PointerType::getUnqual(char_type));
    int_type = Type::getInt64Ty(*context_);
}

Function* IRGenerator::function_definition(const std::string& name, FunctionType* type, Function::LinkageTypes linkage) {
    Function* function = Function::Create(type, linkage, name, module_.get());
    BasicBlock* body = BasicBlock::Create(*context_, "entry", function);
    builder_->SetInsertPoint(body);
    return function;
}

Function* IRGenerator::function_declaration(const std::string& name, FunctionType* type, Function::LinkageTypes linkage) {
    Function* function = Function::Create(type, linkage, name, module_.get());
    return function;
}

bool generate_ir(IRGenerator& generator) {
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

// -------------------------

bool init_llvm() {
    // if (InitializeNativeTarget() != 0)
    //     return false;
    
    InitializeAllTargetInfos();
    InitializeAllTargets();
    InitializeAllTargetMCs();
    InitializeAllAsmParsers();
    InitializeAllAsmPrinters();    

    return true;
}

bool read_source_file(const std::string& filename, std::string& source) {
    std::ifstream source_file{filename, std::ifstream::in};
    if (!source_file.is_open()) {
        std::cerr << "Couldn't open file: " << filename << std::endl;
        return false;
    }

    std::string content((std::istreambuf_iterator<char>(source_file)), std::istreambuf_iterator<char>());
    source = content;

    source_file.close();
    return true;
}

bool generate_object_file(const std::string& filename, Module* module_) {
    // check if we have a target
    auto target_triple = sys::getDefaultTargetTriple();
    std::string error;
    auto target = TargetRegistry::lookupTarget(target_triple, error);
    
    if (!target) {
        std::cout << "No target: " << error << std::endl;
        return false;
    }
    
    auto cpu = "generic";
    auto features = "";

    TargetOptions opt;
    auto target_machine = target->createTargetMachine(target_triple, cpu, features, opt, Reloc::PIC_);

    // prepare the stream
    std::error_code error_code;
    raw_fd_ostream output(filename, error_code, sys::fs::OF_None);

    legacy::PassManager pass;

    if (target_machine->addPassesToEmitFile(pass, output, nullptr, CGFT_ObjectFile)) {
        std::cerr << "Couldn't generate object file for target machine" << std::endl;
        return false;
    }

    module_->setDataLayout(target_machine->createDataLayout());
    module_->setTargetTriple(target_triple);

    // output the file
    pass.run(*module_);
    output.flush();

    return true;
}

bool print_ir(const std::string& filename, Module* module_) {
    // prepare the stream
    std::error_code error_code;
    raw_fd_ostream output(filename, error_code, sys::fs::OF_None);

    module_->print(output, nullptr);
    output.flush();

    return true;
}

int main(int argc, char** argv) {
    if (argc < 3) {
        std::cerr << "USAGE: testc source_file object_file" << std::endl;
        return EXIT_FAILURE; 
    }

    std::string source_filename = argv[1];
    std::string object_file_name = argv[2];

    std::string source = "";
    if (!read_source_file(source_filename, source))
        return EXIT_FAILURE;

    if (!init_llvm()) {
        std::cerr << "Couldn't initialize llvm stuff, exiting" << std::endl;
        return EXIT_FAILURE;
    }

    // create module and IRGenerator helper object
    IRGenerator generator{"Test module"};
    Module* module_ = generator.get_module();
    
    generate_ir(generator);

    // ----- produce output -----
    print_ir(IR_FILE_NAME, module_);
    generate_object_file(object_file_name, module_);

    return EXIT_SUCCESS;
}