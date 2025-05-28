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

#include "mapsc/llvm/type_mapping.hh"
#include "mapsc/llvm/function_store.hh"
#include "mapsc/ast/scope.hh"

namespace llvm {

class GlobalVariable;
class LLVMContext;
class Module;
class raw_ostream;

} // namespace llvm

namespace Maps {

struct Expression;
struct Statement;
class Definition;
class FunctionType;
class TypeStore;
class PragmaStore;
class CompilationState;

} // namespace Maps


namespace IR {

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
        const Maps::CompilationState* compilation_state, llvm::raw_ostream* error_stream, 
        Options options);

    // delegating contructor to make options optional
    IR_Generator(llvm::LLVMContext* context, llvm::Module* module, 
        const Maps::CompilationState* compilation_state, llvm::raw_ostream* error_stream)
    :IR_Generator(context, module, compilation_state, error_stream, Options{}) {}

    // ----- RUNNING THE GENERATOR -----
    bool run(Maps::Scopes scopes);
    bool run(Maps::Scopes, std::span<Maps::Definition* const> additional_definitions);

    // ----- PUBLIC FIELDS -----

    // TODO: move to use logging
    llvm::raw_ostream* errs_;
    
    llvm::LLVMContext* context_;
    llvm::Module* module_;
    std::unique_ptr<llvm::IRBuilder<>> builder_;

    TypeMap types_;

// private:
    void fail(const std::string& message);

    std::optional<llvm::Function*> function_definition(const std::string& name, 
        llvm::FunctionType* llvm_type, 
        llvm::Function::LinkageTypes linkage = llvm::Function::ExternalLinkage);

    std::optional<llvm::Function*> overloaded_function_definition(const std::string& name, 
        const Maps::FunctionType& maps_type, llvm::FunctionType* llvm_type, 
        llvm::Function::LinkageTypes linkage = llvm::Function::ExternalLinkage);
    
    bool close_function_definition(const llvm::Function& function, 
        llvm::Value* return_value = nullptr);

    std::optional<llvm::Function*> forward_declaration(const std::string& name, 
        llvm::FunctionType* type, 
        llvm::Function::LinkageTypes linkage = llvm::Function::ExternalLinkage);

    std::optional<llvm::Function*> overloaded_forward_declaration(const std::string& name, 
        const Maps::FunctionType& maps_type, llvm::FunctionType* type, 
        llvm::Function::LinkageTypes linkage = llvm::Function::ExternalLinkage);

    // Runs llvm::verify_module (flips the result so that true means passed)
    // If options.verify_module = false, always returns true
    bool verify_module();

    // ---- HIGH-LEVEL HANDLERS -----
    bool handle_global_functions(const Maps::RT_Scope& scope);
    std::optional<llvm::FunctionCallee> wrap_value_in_function(const std::string& name, 
        const Maps::Expression& expression);

    // ----- DEFINITION HANDLERS -----
    std::optional<llvm::FunctionCallee> handle_global_definition(const Maps::Definition& definition);
    std::optional<llvm::FunctionCallee> handle_function(const Maps::Definition& definition);

    // ----- STATEMENT HANDLERS -----
    bool handle_statement(const Maps::Statement& statement);
    // if repl_top_level is true, wraps every expression-statement in the block into a print call
    // for the appropriate print function
    bool handle_block(const Maps::Statement& statement);
    std::optional<llvm::Value*> handle_expression_statement(const Maps::Statement& statement);
    
    // ----- EXPRESSION HANDLERS -----
    std::optional<llvm::Value*> handle_expression(const Maps::Expression& expression);
    llvm::Value* handle_call(const Maps::Expression& call);

    // ----- VALUE HANDLERS -----
    llvm::Value* handle_value(const Maps::Expression& expression);
    std::optional<llvm::Value*> convert_value(const Maps::Expression& expression);
    std::optional<llvm::Value*> global_constant(const Maps::Expression& expression);
    std::optional<llvm::Value*> convert_literal(const Maps::Expression& expression);
    std::optional<llvm::Value*> convert_numeric_literal(const Maps::Expression& expression);

    // ----- PRIVATE FIELDS -----
    Options options_;

    const Maps::CompilationState* compilation_state_;
    const Maps::PragmaStore* pragmas_;
    Maps::TypeStore* maps_types_;
    
    std::unique_ptr<FunctionStore> function_store_ = std::make_unique<FunctionStore>();
    
    bool has_failed_ = false;

    // ----- FRIEND FUNCTION -----
    // friend bool insert_builtins(IR::IR_Generator& generator);
};

} // namespace IR

#endif