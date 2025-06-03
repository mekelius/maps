#ifndef __DEFINITION_BODY_HH
#define __DEFINITION_BODY_HH

#include <optional>
#include <string_view>
#include <variant>

#include "common/std_visit_helper.hh"
#include "common/maps_datatypes.h"

#include "mapsc/log_format.hh"
#include "mapsc/source_location.hh"
#include "mapsc/types/type_defs.hh"
#include "mapsc/ast/scope.hh"
#include "mapsc/ast/definition.hh"

namespace Maps {

class Type;
struct Expression;
struct Statement;
class AST_Store;
class CompilationState;

class DefinitionBody {
public:
    // creates a new dummy definition suitable for unit testing
    static std::pair<DefinitionHeader, DefinitionBody> 
        testing_definition(const Type* type = &Hole, bool is_top_level = true); 

    DefinitionBody(DefinitionHeader* header, LetDefinitionValue value);

    DefinitionBody(const std::string& name, DefinitionBody body, bool is_top_level, 
        const SourceLocation& location);
    DefinitionBody(DefinitionBody body, bool is_top_level, 
        const SourceLocation& location); // create anonymous definition

    // anonymous definitions
    DefinitionBody(const std::string& name, DefinitionBody body, const Type* type, bool is_top_level, 
        const SourceLocation& location);
    DefinitionBody(DefinitionBody body, const Type* type, bool is_top_level, 
        const SourceLocation& location);

    DefinitionBody(const DefinitionBody& other) = default;
    DefinitionBody& operator=(const DefinitionBody& other) = default;
    virtual constexpr ~DefinitionBody() = default;

    // ----- OVERRIDES ------
    LetDefinitionValue& body() { return value_; }
    const LetDefinitionValue& body() const { return value_; }

    // since statements don't store types, we'll have to store them here
    // if the body is an expression, the type will just mirror it's type
    std::optional<const Type*> get_declared_type() const;

    LetDefinitionValue get_value() const;
    void set_value(LetDefinitionValue);

    void set_type(const Type* type);
    bool set_declared_type(const Type* type);

    bool is_undefined()const { return header_->is_undefined(); }
    bool is_deleted() const { return header_->is_deleted_; }
    void mark_deleted() { header_->is_deleted_ = true; }

    bool is_top_level_definition() const { return header_->is_top_level_; }
    Scope* inner_scope() const {
        assert(false && "not updated");

        // return inner_scope_ ? *inner_scope_ : header_->outer_scope_;
    }

    std::string node_type_string() const { return header_->node_type_string(); }
    const Type* get_type() const { return header_->get_type(); }
    const SourceLocation& location() const { return header_->location(); }

    std::string_view log_representation() const { return header_->log_representation(); }

    DefinitionHeader* header_;
    // virtual bool operator==(const Definition& other) const {
    //     if (this == &other)
    //         return true;

    //     if (const_body() == other.const_body())
    //         return true;

    //     return false;
    // }

private:
    LetDefinitionValue value_;
    std::optional<const Type*> declared_type_;

    std::optional<Scope*> inner_scope_ = std::nullopt;
};

} // namespace Maps

#endif