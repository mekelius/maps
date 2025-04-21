#ifndef __IR_GEN_HH
#define __IR_GEN_HH

#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/Module.h"

#include <ostream>

#include "../lang/ast.hh"
#include "../lang/pragmas.hh"

#include "ir_types.hh"

namespace IR {

// Helper class that holds the module, context, etc. for IR generation
class IR_Generator {
public:
    IR_Generator(llvm::LLVMContext* context, llvm::Module* module, llvm::raw_ostream* error_stream);

    void set_pragmas(Pragma::Pragmas* pragmas) {
        current_pragmas_ = pragmas;
    }

    bool run(const AST::AST& ast, Pragma::Pragmas* pragmas = nullptr);
    bool repl_run(const AST::AST& ast, Pragma::Pragmas* pragmas = nullptr);
    bool print_ir_to_file(const std::string& filename);
    
    // TODO: move to use logging
    llvm::raw_ostream* errs_;
    
    llvm::LLVMContext* context_;
    llvm::Module* module_;
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
    std::optional<llvm::Value*> handle_statement(const AST::Statement& statement);
    std::optional<llvm::Value*> handle_expression(const AST::Expression& expression);
    std::optional<llvm::Function*> handle_function(const AST::Callable& callable);

    llvm::Value* handle_call(const AST::Expression& call);
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