#ifndef __DEFINITION_HH
#define __DEFINITION_HH

#include <optional>
#include <string_view>
#include <variant>

#include "mapsc/source.hh"
#include "mapsc/types/type_defs.hh"
#include "common/std_visit_helper.hh"


namespace Maps {

class Type;
struct Expression;
struct Statement;
class AST_Store;

struct External {
    template<typename T>
    bool operator==(T&) const { return false; }
};

struct Undefined {
    template<typename T>
    bool operator==(T&) const { return false; }
};

struct Error {
    bool compiler_error = false;

    template<typename T>
    bool operator==(T&) const { return false; }
};

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

using DefinitionBody = std::variant<Undefined, External, Error, Expression*, Statement*, BTD_Binding>;
using const_DefinitionBody = 
    std::variant<Undefined, External, Error, const Expression*, const Statement*, BTD_Binding>;

/**
 * Definitions represent either expressions or statements, along with extra info like
 * holding types for statements. A bit convoluted but we'll see.
 * The source location is needed on every definition that is not a built-in
 * 
 * NOTE: if it's anonymous, it needs a source location
 */
class Definition {
public:
    virtual std::string_view name() const = 0;
    virtual std::string to_string() const { return std::string{name()}; };
    virtual const_DefinitionBody const_body() const = 0;
    virtual const SourceLocation& location() const = 0;
    virtual const Type* get_type() const = 0;
    virtual std::optional<const Type*> get_declared_type() const = 0;

    bool is_undefined() const {
        return std::holds_alternative<Undefined>(const_body());
    }

    bool is_empty() const;

    virtual bool is_operator() const { return false; }
    virtual bool is_const() const { return false; }

    virtual bool operator==(const Definition& other) const = 0;
};

class RT_Definition: public Definition {
public:
    // creates a new dummy definition suitable for unit testing
    static RT_Definition testing_definition(const Type* type = &Hole, bool is_top_level = true); 

    static RT_Definition* parameter(AST_Store& store, std::string_view name, const Type* type, 
        SourceLocation location);
    static RT_Definition* discarded_parameter(AST_Store& store, const Type* type, 
        SourceLocation location);

    RT_Definition(std::string_view name, External external, const Type* type)
    :name_(name), body_(external), location_(EXTERNAL_SOURCE_LOCATION), type_(type), 
     is_top_level_(true) {}

    RT_Definition(std::string_view name, DefinitionBody body, bool is_top_level, SourceLocation location);
    RT_Definition(DefinitionBody body, bool is_top_level, SourceLocation location); // create anonymous definition

    // anonymous definitions
    RT_Definition(std::string_view name, DefinitionBody body, const Type* type, bool is_top_level, SourceLocation location);
    RT_Definition(DefinitionBody body, const Type* type, bool is_top_level, SourceLocation location);

    RT_Definition(const RT_Definition& other) = default;
    RT_Definition& operator=(const RT_Definition& other) = default;
    virtual constexpr ~RT_Definition() = default;

    // ----- OVERRIDES ------
    virtual std::string_view name() const { return name_; }
    virtual std::string to_string() const { return name_; }
    virtual const_DefinitionBody const_body() const;
    virtual DefinitionBody& body() { return body_; }
    virtual const SourceLocation& location() const { return location_; }

    // since statements don't store types, we'll have to store them here
    // if the body is an expression, the type will just mirror it's type
    virtual const Type* get_type() const;
    virtual std::optional<const Type*> get_declared_type() const;

    void set_type(const Type* type);
    bool set_declared_type(const Type* type);

    bool is_deleted() const { return is_deleted_; }
    void mark_deleted() { is_deleted_ = true; }

    bool is_top_level_definition() const { return is_top_level_; }

    virtual bool operator==(const Definition& other) const {
        if (this == &other)
            return true;

        if (const_body() == other.const_body())
            return true;

        return false;
    }

private:
    std::string name_;
    DefinitionBody body_;
    SourceLocation location_;
    std::optional<const Type*> type_;
    std::optional<const Type*> declared_type_;
    bool is_top_level_;
    bool is_deleted_ = false;
};

} // namespace Maps

#endif