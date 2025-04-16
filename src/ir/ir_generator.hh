#ifndef __IR_GEN_HH
#define __IR_GEN_HH

#include "llvm/IR/BasicBlock.h"
#include "llvm/IR/Constants.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/Verifier.h"

#include <ostream>

#include "../lang/ast.hh"
#include "../lang/pragmas.hh"

#include "ir_types.hh"

namespace IR {

// Helper class that holds the module, context, etc. for IR generation
class IR_Generator {
public:
    IR_Generator(const std::string& module_name, std::ostream* info_stream);

    llvm::Function* function_definition(const std::string& name, llvm::FunctionType* type, 
        llvm::Function::LinkageTypes linkage = llvm::Function::ExternalLinkage);
    llvm::Function* function_declaration(const std::string& name, llvm::FunctionType* type, 
        llvm::Function::LinkageTypes linkage = llvm::Function::ExternalLinkage);

    void set_pragmas(Pragma::Pragmas* pragmas) {
        current_pragmas_ = pragmas;
    }

    bool run(AST::AST& ast, std::optional<Pragma::Pragmas*> pragmas = std::nullopt);

    TypeMap types_;

    std::unique_ptr<llvm::LLVMContext> context_;
    std::unique_ptr<llvm::Module> module_;
    std::unique_ptr<llvm::IRBuilder<>> builder_;
private:
    llvm::Value* handle_callable(AST::Callable& callable);
    llvm::Value* handle_expression(AST::Expression& expression);
    llvm::Value* handle_statement(AST::Statement& statement);
    llvm::Value* handle_call(AST::Expression& call);
    std::optional<llvm::Function*> handle_function(AST::Callable& callable);
    llvm::GlobalVariable* handle_string_literal(AST::Expression& str);
    
    void fail(const std::string& message);
    bool function_name_is_ok(const std::string& name);
    std::optional<llvm::FunctionCallee> get_function(const std::string& name) const;
    void start_main();

    // TODO: move to use logging
    std::ostream* errs_;

    std::unordered_map<std::string, llvm::FunctionCallee> functions_map_;

    Pragma::Pragmas* current_pragmas_;
    bool has_failed_ = false;
};

} // namespace IR

#endif