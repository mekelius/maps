#ifndef __IR_GEN_HH
#define __IR_GEN_HH

#include <memory>
#include <optional>
#include <string>
#include <string_view>

#include "llvm/IR/DerivedTypes.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/Value.h"

#include "mapsc/ast/scope.hh"
#include "mapsc/llvm_ir_gen/type_mapping.hh"
#include "mapsc/llvm_ir_gen/function_store.hh"

namespace llvm {

class GlobalVariable;
class LLVMContext;
class Module;
class raw_ostream;

} // namespace llvm

namespace Maps {

struct Expression;
struct Statement;
class FunctionType;
class TypeStore;
class PragmaStore;
class CompilationState;

namespace LLVM_IR {

constexpr std::string_view REPL_WRAPPER_NAME = "repl_wrapper";

// Helper class that holds the module, context, etc. for IR generation
class IR_Generator {
public:
    struct Options {
        // one of these kinda needs to be on
        // if true, every function is verified individually
        bool verify_functions = true;
        // if true, the module as a whole is verified in the end
        bool verify_module = true;
    };

    // ----- CONSTRUCTORS -----
    IR_Generator(llvm::LLVMContext* context, llvm::Module* module, 
        const CompilationState* compilation_state, llvm::raw_ostream* error_stream, 
        Options options);

    // delegating contructor to make options optional
    IR_Generator(llvm::LLVMContext* context, llvm::Module* module, 
        const CompilationState* compilation_state, llvm::raw_ostream* error_stream)
    :IR_Generator(context, module, compilation_state, error_stream, Options{}) {}

    // ----- RUNNING THE GENERATOR -----
    bool run(const Scope& scope);
    bool run(const Scope& scope, std::span<DefinitionHeader* const> additional_definitions);

    // ----- PUBLIC FIELDS -----

    // TODO: move to use logging
    llvm::raw_ostream* errs_;
    
    llvm::LLVMContext* context_;
    llvm::Module* module_;
    std::unique_ptr<llvm::IRBuilder<>> builder_;

    TypeMap types_;

// private:
    void fail();
    std::nullopt_t fail_optional();

    std::optional<llvm::Function*> function_definition(const std::string& name, 
        llvm::FunctionType* llvm_type, 
        llvm::Function::LinkageTypes linkage = llvm::Function::ExternalLinkage);

    std::optional<llvm::Function*> overloaded_function_definition(const std::string& name, 
        const FunctionType& maps_type, llvm::FunctionType* llvm_type, 
        llvm::Function::LinkageTypes linkage = llvm::Function::ExternalLinkage);
    
    bool close_function_definition(const llvm::Function& function, 
        llvm::Value* return_value = nullptr);

    std::optional<llvm::Function*> forward_declaration(const std::string& name, 
        llvm::FunctionType* type, 
        llvm::Function::LinkageTypes linkage = llvm::Function::ExternalLinkage);

    std::optional<llvm::Function*> overloaded_forward_declaration(const std::string& name, 
        const FunctionType& maps_type, llvm::FunctionType* type, 
        llvm::Function::LinkageTypes linkage = llvm::Function::ExternalLinkage);

    bool block_has_terminated() const;

    // Runs llvm::verify_module (flips the result so that true means passed)
    // If options.verify_module = false, always returns true
    bool verify_module();

    // ---- HIGH-LEVEL HANDLERS -----
    bool handle_global_functions(const Scope& scope);
    std::optional<llvm::FunctionCallee> wrap_value_in_function(const std::string& name, 
        const Expression& expression);

    // ----- DEFINITION HANDLERS -----
    std::optional<llvm::FunctionCallee> handle_global_definition(const DefinitionHeader& definition);
    std::optional<llvm::FunctionCallee> handle_function(const DefinitionHeader& definition);

    // ----- STATEMENT HANDLERS -----
    bool handle_statement(const Statement& statement);
    // if repl_top_level is true, wraps every expression-statement in the block into a print call
    // for the appropriate print function
    bool handle_block(const Statement& statement);
    std::optional<llvm::Value*> handle_expression_statement(const Statement& statement);

    bool handle_conditional(const Statement& statement);
    bool handle_switch(const Statement& statement);
    bool handle_loop(const Statement& statement);
    bool handle_guard(const Statement& statement);

    // ----- EXPRESSION HANDLERS -----
    std::optional<llvm::Value*> handle_expression(const Expression& expression);
    llvm::Value* handle_call(const Expression& call);

    // ----- VALUE HANDLERS -----
    llvm::Value* handle_value(const Expression& expression);
    std::optional<llvm::Value*> convert_value(const Expression& expression);
    std::optional<llvm::Value*> global_constant(const Expression& expression);
    std::optional<llvm::Value*> convert_literal(const Expression& expression);
    std::optional<llvm::Value*> convert_numeric_literal(const Expression& expression);

    // ----- PRIVATE FIELDS -----
    Options options_;

    const CompilationState* compilation_state_;
    const PragmaStore* pragmas_;
    TypeStore* maps_types_;
    
    std::unique_ptr<FunctionStore> function_store_ = std::make_unique<FunctionStore>();
    
    bool has_failed_ = false;

    // ----- FRIEND FUNCTION -----
    // friend bool insert_builtins(IR::IR_Generator& generator);
};

} // namespace LLVM_IR
} // nameespace Maps

#endif