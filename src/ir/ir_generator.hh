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

    void set_pragmas(Pragma::Pragmas* pragmas) {
        current_pragmas_ = pragmas;
    }

    bool run(AST::AST& ast, Pragma::Pragmas* pragmas = nullptr);

    // TODO: move to use logging
    std::ostream* errs_;
    
    std::unique_ptr<llvm::LLVMContext> context_;
    std::unique_ptr<llvm::Module> module_;
    std::unique_ptr<llvm::IRBuilder<>> builder_;

    TypeMap types_;
private:
    llvm::Function* function_definition(const std::string& name, llvm::FunctionType* type, 
        llvm::Function::LinkageTypes linkage = llvm::Function::ExternalLinkage);
    llvm::Function* function_declaration(const std::string& name, llvm::FunctionType* type, 
        llvm::Function::LinkageTypes linkage = llvm::Function::ExternalLinkage);

    std::optional<llvm::Value*> global_constant(const AST::Callable& callable);
    std::optional<llvm::Value*> convert_literal(const AST::Expression& expression) const;
    std::optional<llvm::Value*> convert_numeric_literal(const AST::Expression& expression) const;

    bool handle_global_definition(const AST::Callable& callable);
    llvm::Value* handle_callable(const AST::Callable& callable);
    llvm::Value* handle_expression(const AST::Expression& expression);
    llvm::Value* handle_statement(const AST::Statement& statement);
    llvm::Value* handle_call(const AST::Expression& call);
    std::optional<llvm::Function*> handle_function(const AST::Callable& callable);
    llvm::GlobalVariable* handle_string_literal(const AST::Expression& str);
    
    void fail(const std::string& message);
    bool function_name_is_ok(const std::string& name);
    std::optional<llvm::FunctionCallee> get_function(const std::string& name) const;
    void start_main();

    friend void insert_builtins(IR_Generator& generator);

    std::unordered_map<std::string, llvm::FunctionCallee> functions_map_;

    Pragma::Pragmas* current_pragmas_;
    bool has_failed_ = false;
};

} // namespace IR

#endif