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

    // create module
    std::unique_ptr<LLVMContext> context = std::make_unique<LLVMContext>();
    std::unique_ptr<Module> module_ = std::make_unique<Module>("Test module", *context);
    std::unique_ptr<IRBuilder<>> builder = std::make_unique<IRBuilder<>>(*context);
    

    Type* char_type = Type::getInt8Ty(*context);
    Type* char_array_13_type = ArrayType::get(char_type, 13);

    FunctionType* puts_type = FunctionType::get(Type::getInt64Ty(*context), {char_array_13_type->getPointerTo()}, false);
    Function* puts = Function::Create(puts_type, Function::ExternalLinkage, "puts", module_.get());
    
    
    FunctionType* f_type1 = FunctionType::get(PointerType::getUnqual(char_array_13_type), {}, false);
    Function* function1 = Function::Create(f_type1, Function::ExternalLinkage, "hello", module_.get());
    BasicBlock* block1 = BasicBlock::Create(*context, "entry", function1);
    builder->SetInsertPoint(block1);
    builder->CreateRet(builder->CreateGlobalString("Hello World!"));
    
    Type* char_array_ptr_type = PointerType::getUnqual(PointerType::getUnqual(char_type));
    FunctionType* f_type2 = FunctionType::get(Type::getInt64Ty(*context), {Type::getInt64Ty(*context), char_array_ptr_type}, false);
    Function* function2 = Function::Create(f_type2, Function::ExternalLinkage, "main", module_.get());
    BasicBlock* block2 = BasicBlock::Create(*context, "entry", function2);
    builder->SetInsertPoint(block2);
    CallInst* hello_call = builder->CreateCall(function1, {}, "msg");
    builder->CreateCall(puts, {hello_call});
    
    builder->CreateRet(ConstantInt::get(Type::getInt64Ty(*context), 0));
    
    // FunctionType* f_type = FunctionType::get(char_type, {}, false);
    // StringRef string_ref = StringRef::Create("Hello World!");
    // builder->CreateCall(puts, {builder->CreateGlobalString("Hello World!")});
    // Constant* test_const = ConstantInt::get(char_type, 'h');
    // ArrayRef<char> my_string = "Hello World!";
    // Constant* test_const = ConstantDataArray::get(*context, my_string);
    
    // GlobalVariable* global_string = new GlobalVariable(*module_, char_array_13_type, true, GlobalVariable::ExternalLinkage, test_const, "test_string");
    
    
    // Value* return_value = ConstantPointer::get(cha);
    // Value* return_value = ConstantInt::get(char_type, 'h');
    
    
    verifyFunction(*function1);
    verifyFunction(*function2);
        
    // ----- produce output -----
    print_ir(IR_FILE_NAME, module_.get());
    generate_object_file(object_file_name, module_.get());

    return EXIT_SUCCESS;
}