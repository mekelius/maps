#ifndef __IR_GEN_HH
#define __IR_GEN_HH

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

#include "ast.hh"

// Helper class that holds the module, context, etc. for IR generation
class IRGenHelper {
public:
    IRGenHelper(const std::string& module_name = "module");

    llvm::Function* function_definition(const std::string& name, llvm::FunctionType* type, llvm::Function::LinkageTypes linkage = llvm::Function::ExternalLinkage);
    llvm::Function* function_declaration(const std::string& name, llvm::FunctionType* type, llvm::Function::LinkageTypes linkage = llvm::Function::ExternalLinkage);

    llvm::IRBuilder<>* get_builder() {
        return builder_.get();
    }

    llvm::Module* get_module() {
        return module_.get();
    }

    llvm::LLVMContext* get_context() {
        return context_.get();
    }

    void generate_ir(AST::AST* ast);

    llvm::Type* char_type;
    llvm::Type* int_type;
    llvm::Type* char_array_13_type;
    llvm::Type* char_array_ptr_type;
    llvm::Type* double_type;
    llvm::Type* void_type;
private:
    void handleCall(AST::Expression* call);
    void handleExpression(AST::Expression* expression);
    llvm::GlobalVariable* handleStringLiteral(AST::Expression* str);

    std::unique_ptr<llvm::LLVMContext> context_;
    std::unique_ptr<llvm::Module> module_;
    std::unique_ptr<llvm::IRBuilder<>> builder_;

    // builtin function
    llvm::Function* puts_;
    llvm::Function* sprintf_;
    llvm::Function* main_;
};

bool generate_ir(IRGenHelper& generator);

#endif