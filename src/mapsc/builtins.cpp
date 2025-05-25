#include "builtins.hh"

#include <cassert>
#include <optional>
#include <string>
#include <string_view>
#include <variant>

#include "common/maps_datatypes.h"

#include "mapsc/source.hh"
#include "mapsc/logging.hh"

#include "mapsc/types/function_type.hh"
#include "mapsc/types/type_defs.hh"

#include "mapsc/ast/definition.hh"
#include "mapsc/ast/expression.hh"
#include "mapsc/ast/operator.hh"
#include "mapsc/ast/scope.hh"
#include "mapsc/ast/statement.hh"

using std::optional, std::nullopt;
using std::monostate;


namespace Maps {

using Log = LogInContext<LogContext::compiler_init>;
class Type;

const_DefinitionBody CT_Definition::const_body() const {
    if (auto external = std::get_if<External>(&builtin_body_)) {
        return *external;
    } else if (auto expression = std::get_if<Expression>(&builtin_body_)) {
        return expression;
    }

    assert(false && "huh?");
    // return std::visit(overloaded {
    //     [](External external) { return external; },
    //     [](Expression expression) { return &expression; }
        // [this](Statement statement) { return &statement; }
    // }, builtin_body_);
}

// CT_Definition::CT_Definition(std::string_view name, const Expression&& expression)
// :CT_Definition(name, expression, *expression.type)
// {}

// CT_Definition::CT_Definition(std::string_view name, Statement&& statement, const Type& type)
// :CT_Definition(name, statement, type) {}

CT_Operator::CT_Operator(std::string_view name, const Expression&& expression, 
    Operator::Properties operator_props)
:CT_Definition(name, std::move(expression)),
    operator_props_(operator_props) {
}

// ----- BUILTIN DEFINITIONS -----

CT_Definition true_{"true", Expression::builtin(true, Boolean)};
CT_Definition false_{"false", Expression::builtin(false, Boolean)};
constinit CT_Definition print_String{"print_String", External{}, String_to_IO_Void};
constinit CT_Definition print_MutString{"print_MutString", External{}, MutString_to_IO_Void};

// ----- BUILTINS SCOPE -----

static CT_Scope builtins;
bool builtins_initialized = false;

constinit CT_Operator unary_minus_Int{"-", External{}, Int_to_Int,
    Operator::Properties{Operator::Fixity::unary_prefix}};

constinit CT_Operator plus_Int{"+", External{}, IntInt_to_Int,
    {Operator::Fixity::binary, 500}};
constinit CT_Operator binary_minus_Int{"-", External{}, IntInt_to_Int,
    {Operator::Fixity::binary,510}};
constinit CT_Operator mult_Int{"*", External{}, IntInt_to_Int,
    {Operator::Fixity::binary,520}};

constinit CT_Definition to_String_Boolean{"to_String_Boolean", External{}, Boolean_to_String};
constinit CT_Definition to_String_Int{"to_String_Int", External{}, Int_to_String};
constinit CT_Definition to_String_Float{"to_String_Float", External{}, Float_to_String};
constinit CT_Definition to_Float_Int{"to_Float_Int", External{}, Int_to_Float};
constinit CT_Definition to_String_MutString{"to_String_MutString", External{}, MutString_to_String};
constinit CT_Definition to_MutString_Int{"to_MutString_Int", External{}, Int_to_MutString};
constinit CT_Definition to_MutString_Float{"to_MutString_Float", External{}, Float_to_MutString};

constinit CT_Definition concat{"concat_MutString_MutString", External{}, MutString_MutString_to_MutString};

bool init_builtin(CT_Scope& scope, CT_Definition& definition) {
    if (!scope.create_identifier(&definition)) {
        Log::compiler_error("Creating builtin " + definition.name_string() + " failed",
            COMPILER_INIT_SOURCE_LOCATION);
        return false;
    }

    return true;
}

bool init_builtins(CT_Scope& scope) {
    builtins_initialized = true;

    return (
        init_builtin(scope, true_                       ) &&
        init_builtin(scope, false_                      ) &&
        init_builtin(scope, print_String                ) &&
        init_builtin(scope, print_MutString             ) &&
        init_builtin(scope, plus_Int                    ) &&
        init_builtin(scope, mult_Int                    ) &&
        init_builtin(scope, concat                      ) &&
        init_builtin(scope, to_String_Boolean           ) &&
        init_builtin(scope, to_Float_Int                ) &&
        init_builtin(scope, to_String_MutString         ) &&
        init_builtin(scope, to_MutString_Int            ) &&
        init_builtin(scope, to_MutString_Float          )
        // init_builtin(scope, to_String_Int               ) &&
        // init_builtin(scope, to_String_Float             ) &&
    );
}

const CT_Scope* get_builtins() {
    if (!builtins_initialized && !init_builtins(builtins)) {
        Log::compiler_error("Initializing builtins failed", COMPILER_INIT_SOURCE_LOCATION);
        assert(false && "initializing builtins failed");
    }

    return &builtins;
}

optional<Definition*> find_external_runtime_cast(const CT_Scope& scope, const Type* source_type, 
    const Type* target_type) {
    
    std::string cast_name = "to_" + target_type->name_string() + "_" + source_type->name_string();
    
    Log::debug_extra("Trying to find runtime cast " + cast_name, NO_SOURCE_LOCATION);

    auto cast = scope.get_identifier(cast_name);
    if (!cast)
        LogNoContext::debug("Could not find runtime cast " + cast_name, NO_SOURCE_LOCATION);

    return cast;
}

} // namespace Maps