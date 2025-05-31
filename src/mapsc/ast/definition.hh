#ifndef __DEFINITION_HH
#define __DEFINITION_HH

#include <optional>
#include <string_view>
#include <variant>

#include "common/std_visit_helper.hh"
#include "common/maps_datatypes.h"

#include "mapsc/log_format.hh"
#include "mapsc/source.hh"
#include "mapsc/types/type_defs.hh"
#include "mapsc/ast/scope.hh"

namespace Maps {

class Type;
struct Expression;
struct Statement;
class AST_Store;
class CompilationState;

struct Undefined {
    bool operator==(auto) const { return false; }
};

struct Error {
    bool compiler_error = false;

    template<typename T>
    bool operator==(T&) const { return false; }
};

class DefinitionBody;
using LetDefinitionValue = std::variant<Undefined, Error, Expression*, Statement*>;

class DefinitionHeader {
public:
    DefinitionHeader(const std::string& name, const Type* type, Scope* outer_scope,
        bool is_top_level, SourceLocation location);
    DefinitionHeader(const std::string& name, Scope* outer_scope,
        bool is_top_level, SourceLocation location);
    DefinitionHeader(const std::string& name, const Type* type, SourceLocation location);
    DefinitionHeader(const std::string& name, SourceLocation location);

    const SourceLocation& location() const { return location_; }

    virtual ~DefinitionHeader() = default;
    DefinitionHeader(const DefinitionHeader&) = default;
    DefinitionHeader& operator=(const DefinitionHeader&) = default;

    virtual bool is_operator() const { return false; }
    bool is_const() const { return false; }
    bool is_known_scalar_value() const;
    bool has_body() const { return body_.has_value(); }
    bool is_empty() const { return !body_.has_value(); }
    bool is_undefined() const { return !body_.has_value(); }
    
    std::string node_type_string() const { return "not implemented"; }
    std::string name_string() const { return name_; }
    std::string_view name_view() const { return name_; }
    const Type* get_type() const { return type_; }

    std::optional<LetDefinitionValue> get_body_value() const;

    bool operator==(const DefinitionHeader& other) const {
        return this == &other;
    };

    std::string name_;
    const Type* type_;
    bool is_top_level_;
    SourceLocation location_;
    std::optional<Scope*> outer_scope_;
    std::optional<DefinitionBody*> body_ = std::nullopt;
    bool is_deleted_ = false;
};

DefinitionHeader* create_definition(AST_Store& ast_store, const Type* type, 
    const SourceLocation& location);

struct BTD_Binding {
    enum class Type {
        parameter,
        discarded_parameter
    };

    Type slot_type;
    const Maps::Type* type;

    template<typename T>
    bool operator==(T& t) const { return this == &t; }
};

class DS_Binding {
public:

};

using BuiltinValue = std::variant<maps_Boolean, maps_String, maps_Int, maps_Float>;

class Builtin: public DefinitionHeader {
public:
    // Builtin(const std::string& name, Statement&& statement, const Type& type);
    Builtin(const std::string& name, BuiltinValue value, const Type* type)
    :DefinitionHeader(name, type, BUILTIN_SOURCE_LOCATION), value_(value) {}

private:
    BuiltinValue value_;
};

class BuiltinExternal: public DefinitionHeader {
public:
    virtual std::string node_type_string() const { return "builtin external"; };
    virtual std::string name_string() const { return std::string{name_}; };
    virtual const Type* get_type() const {return type_; };

    BuiltinExternal(const std::string& name, const Type* type)
    :DefinitionHeader(name, type, BUILTIN_SOURCE_LOCATION) {}
};

} // namespace Maps

#endif