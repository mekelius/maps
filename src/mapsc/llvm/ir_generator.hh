#ifndef __IR_GEN_HH
#define __IR_GEN_HH

#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/Module.h"

#include <ostream>

#include "mapsc/ast/ast.hh"
#include "mapsc/pragma.hh"

#include "type_mapping.hh"

namespace IR {

constexpr std::string_view REPL_WRAPPER_NAME = "repl_wrapper";

class FunctionStore {
    using Signature = Maps::Type::HashableSignature;

public:
    // std::optional<llvm::Function*> get_function(const std::string& name, AST::Type* function_type) const;
    std::optional<llvm::FunctionCallee> get(const std::string& name, 
        const Maps::FunctionType& ast_type) const;
    bool insert(const std::string& name, const Maps::FunctionType& ast_type, 
        llvm::FunctionCallee function_callee);

private:
    using InnerMapType = std::unordered_map<Signature, llvm::FunctionCallee>;

    std::unordered_map<std::string, std::unique_ptr<InnerMapType>>
        functions_ = std::unordered_map<std::string, std::unique_ptr<InnerMapType>>();
};

// Helper class that holds the module, context, etc. for IR generation
class IR_Generator {
public:
    IR_Generator(llvm::LLVMContext* context, llvm::Module* module, const Maps::AST& ast, 
        Maps::Pragmas& pragmas, llvm::raw_ostream* error_stream);

    bool run();

    // version of run that always processes top-level statements, and wraps them in print calls
    bool repl_run();
    bool print_ir_to_file(const std::string& filename);

    // TODO: move to use logging
    llvm::raw_ostream* errs_;
    
    llvm::LLVMContext* context_;
    llvm::Module* module_;
    std::unique_ptr<llvm::IRBuilder<>> builder_;

    TypeMap types_;
private:
    std::optional<llvm::Function*> function_definition(const std::string& name, 
        const Maps::FunctionType& ast_type, llvm::FunctionType* llvm_type, 
        llvm::Function::LinkageTypes linkage = llvm::Function::ExternalLinkage);
    std::optional<llvm::Function*> function_declaration(const std::string& name, 
        const Maps::FunctionType& ast_type, llvm::FunctionType* type, 
        llvm::Function::LinkageTypes linkage = llvm::Function::ExternalLinkage);

    std::optional<llvm::Value*> global_constant(const Maps::Expression& expression);
    std::optional<llvm::Value*> convert_literal(const Maps::Expression& expression) const;
    std::optional<llvm::Value*> convert_numeric_literal(const Maps::Expression& expression) const;

    std::optional<llvm::Function*> eval_and_print_root();
    bool handle_global_functions();
    std::optional<llvm::FunctionCallee> handle_global_definition(const Maps::Callable& callable);
    llvm::Value* handle_callable(const Maps::Callable& callable);
    bool handle_statement(const Maps::Statement& statement);

    // if repl_top_level is true, wraps every expression-statement in the block into a print call for the appropriate print function
    bool handle_block(const Maps::Statement& statement, bool repl_top_level = false);
    std::optional<llvm::Value*> handle_expression_statement(const Maps::Statement& statement, bool repl_top_level = false);

    std::optional<llvm::Value*> handle_expression(const Maps::Expression& expression);
    std::optional<llvm::FunctionCallee> wrap_value_in_function(const std::string& name, const Maps::Expression& expression);
    std::optional<llvm::FunctionCallee> handle_function(const Maps::Callable& callable);

    llvm::Value* handle_call(const Maps::CallExpressionValue& call);
    llvm::GlobalVariable* handle_string_literal(const Maps::Expression& str);
    llvm::Value* handle_value(const Maps::Expression& expression);
    
    void fail(const std::string& message);

    Maps::Pragmas* pragmas_;
    const Maps::AST* ast_;
    Maps::TypeRegistry* maps_types_;
    
    bool has_failed_ = false;
    std::unique_ptr<FunctionStore> function_store_ = std::make_unique<FunctionStore>();

    friend bool insert_builtins(IR::IR_Generator& generator);
};

} // namespace IR

#endif