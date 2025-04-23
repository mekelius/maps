#ifndef __IR_GEN_HH
#define __IR_GEN_HH

#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/Module.h"

#include <ostream>

#include "../lang/ast.hh"
#include "../lang/pragmas.hh"

#include "ir_types.hh"

constexpr std::string_view REPL_WRAPPER_NAME = "repl_wrapper";

namespace IR {

class FunctionStore {
    using Signature = AST::FunctionTypeComplex::HashableSignature;

public:
    // std::optional<llvm::Function*> get_function(const std::string& name, AST::Type* function_type) const;
    std::optional<llvm::FunctionCallee> get(const std::string& name, const AST::Type& ast_type) const;
    bool insert(const std::string& name, const AST::Type& ast_type, llvm::FunctionCallee function_callee);

private:
    using InnerMapType = std::unordered_map<Signature, llvm::FunctionCallee>;

    std::unordered_map<std::string, std::unique_ptr<InnerMapType>>
        functions_ = std::unordered_map<std::string, std::unique_ptr<InnerMapType>>();
};

// Helper class that holds the module, context, etc. for IR generation
class IR_Generator {
public:
    IR_Generator(llvm::LLVMContext* context, llvm::Module* module, llvm::raw_ostream* error_stream);

    void set_pragmas(Pragma::Pragmas* pragmas) {
        current_pragmas_ = pragmas;
    }

    bool run(const AST::AST& ast, Pragma::Pragmas* pragmas = nullptr);

    // version of run that always processes top-level statements, and wraps them in print calls
    bool repl_run(const AST::AST& ast, Pragma::Pragmas* pragmas = nullptr);
    bool print_ir_to_file(const std::string& filename);
    
    // TODO: move to use logging
    llvm::raw_ostream* errs_;
    
    llvm::LLVMContext* context_;
    llvm::Module* module_;
    std::unique_ptr<llvm::IRBuilder<>> builder_;

    TypeMap types_;
private:
    std::optional<llvm::Function*> function_definition(const std::string& name, const AST::Type& ast_type, llvm::FunctionType* llvm_type, 
        llvm::Function::LinkageTypes linkage = llvm::Function::ExternalLinkage);
    std::optional<llvm::Function*> function_declaration(const std::string& name, const AST::Type& ast_type, llvm::FunctionType* type, 
        llvm::Function::LinkageTypes linkage = llvm::Function::ExternalLinkage);

    std::optional<llvm::Value*> global_constant(const AST::Callable& callable);
    std::optional<llvm::Value*> convert_literal(const AST::Expression& expression) const;
    std::optional<llvm::Value*> convert_numeric_literal(const AST::Expression& expression) const;

    std::optional<llvm::Function*> handle_top_level_execution(const AST::AST& ast, bool in_repl);
    bool handle_global_functions(const AST::AST& ast);
    bool handle_global_definition(const AST::Callable& callable);
    llvm::Value* handle_callable(const AST::Callable& callable);
    bool handle_statement(const AST::Statement& statement);

    // if repl_top_level is true, wraps every expression-statement in the block into a print call for the appropriate print function
    bool handle_block(const AST::Statement& statement, bool repl_top_level = false);
    std::optional<llvm::Value*> handle_expression_statement(const AST::Statement& statement, bool repl_top_level = false);

    std::optional<llvm::Value*> handle_expression(const AST::Expression& expression);
    std::optional<llvm::Function*> handle_function(const AST::Callable& callable);

    llvm::Value* handle_call(const AST::Expression& call);
    llvm::GlobalVariable* handle_string_literal(const AST::Expression& str);
    
    void fail(const std::string& message);

    Pragma::Pragmas* current_pragmas_;
    
    bool has_failed_ = false;
    std::unique_ptr<FunctionStore> function_store_ = std::make_unique<FunctionStore>();

    friend bool insert_builtins(IR::IR_Generator& generator);
};

} // namespace IR

#endif