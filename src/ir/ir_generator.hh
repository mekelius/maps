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

#include "../lang/ast.hh"

#include <ostream>

// Helper class that holds the module, context, etc. for IR generation
class IR_Generator {
public:
    IR_Generator(const std::string& module_name, std::ostream* info_stream);

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

    bool generate_ir(AST::AST* ast);

    llvm::Type* char_type;
    llvm::Type* int_type;
    llvm::Type* char_array_13_type;
    llvm::Type* char_array_ptr_type;
    llvm::Type* double_type;
    llvm::Type* void_type;
private:
    llvm::Value* handle_call(AST::Expression& call);
    llvm::Value* handle_expression(AST::Expression& expression);
    std::optional<llvm::Function*> handle_function(AST::Identifier& callable);
    llvm::GlobalVariable* handle_string_literal(AST::Expression& str);
    
    void fail(const std::string& message);
    void create_builtins();
    void populate_type_map();
    bool function_name_is_ok(const std::string& name);
    std::optional<llvm::FunctionCallee> get_function(const std::string& name) const;
    std::optional<llvm::Type*> convert_type(const AST::Type*) const;
    void start_main();

    std::unique_ptr<llvm::LLVMContext> context_;
    std::unique_ptr<llvm::Module> module_;
    std::unique_ptr<llvm::IRBuilder<>> builder_;
    AST::AST* ast_ = nullptr;
    std::ostream* errs_;
    // this could probably be static, but the types do depend on the context so non-static it is
    // also just an if statement would be faster, hey enum with a switch case yaas
    std::unordered_map<std::string_view, llvm::Type*> type_map_;
    std::unordered_map<std::string, llvm::FunctionCallee> functions_map_;

    // builtin functions
    llvm::Function* puts_;
    llvm::Function* sprintf_;

    bool has_failed_ = false;
};

bool generate_ir(IR_Generator& generator);

#endif