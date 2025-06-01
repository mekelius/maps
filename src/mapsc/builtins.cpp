#include "builtins.hh"

#include <cassert>
#include <optional>
#include <string>
#include <string_view>
#include <variant>

#include "mapsc/source.hh"
#include "mapsc/logging.hh"

#include "mapsc/types/function_type.hh"
#include "mapsc/types/type_defs.hh"

#include "mapsc/ast/definition.hh"
#include "mapsc/ast/builtin.hh"
#include "mapsc/ast/expression.hh"
#include "mapsc/ast/operator.hh"
#include "mapsc/ast/scope.hh"

using std::optional, std::nullopt;
using std::monostate;


namespace Maps {

using Log = LogInContext<LogContext::compiler_init>;

// ----- BUILTIN DEFINITIONS -----

Builtin true_ = create_builtin("true", BuiltinValue{true}, &Boolean);
Builtin false_ = create_builtin("false", BuiltinValue{false}, &Boolean);
BuiltinExternal print_String{"prints", &String_to_IO_Void};
BuiltinExternal print_MutString{"printms", &MutString_to_IO_Void};

// ----- BUILTINS SCOPE -----

static Scope builtins;
bool builtins_initialized = false;

BuiltinExternal unary_minus_Int_{"unary_minus_Int_", &Int_to_Int};
BuiltinExternal plus_Int_{"plus_Int_", &IntInt_to_Int};
BuiltinExternal minus_Int_{"minus_Int_", &IntInt_to_Int};
BuiltinExternal mult_Int_{"mult_Int_", &IntInt_to_Int};

Operator unary_minus_Int{"-", &unary_minus_Int_, Operator::Properties{Operator::Fixity::unary_prefix}, BUILTIN_SOURCE_LOCATION};
Operator plus_Int{"+", &plus_Int_, {Operator::Fixity::binary, 500}, BUILTIN_SOURCE_LOCATION};
Operator binary_minus_Int{"-", &minus_Int_, {Operator::Fixity::binary, 510}, BUILTIN_SOURCE_LOCATION};
Operator mult_Int{"*", &mult_Int_, {Operator::Fixity::binary, 520}, BUILTIN_SOURCE_LOCATION};

BuiltinExternal to_String_Boolean{"to_String_Boolean", &Boolean_to_String};
BuiltinExternal to_String_Int{"to_String_Int", &Int_to_String};
BuiltinExternal to_String_Float{"to_String_Float", &Float_to_String};
BuiltinExternal to_Float_Int{"to_Float_Int", &Int_to_Float};
BuiltinExternal to_String_MutString{"to_String_MutString", &MutString_to_String};
BuiltinExternal to_MutString_Int{"to_MutString_Int", &Int_to_MutString};
BuiltinExternal to_MutString_Float{"to_MutString_Float", &Float_to_MutString};

BuiltinExternal concat{"concat", &MutString_MutString_to_MutString};

bool init_builtin(Scope& scope, DefinitionHeader& node) {
    if (!scope.create_identifier(&node)) {
        Log::compiler_error(COMPILER_INIT_SOURCE_LOCATION) << "Creating builtin " << node << " failed";
        return false;
    }

    return true;
}

bool init_builtins(Scope& scope) {
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

const Scope* get_builtins() {
    if (!builtins_initialized && !init_builtins(builtins)) {
        Log::compiler_error(COMPILER_INIT_SOURCE_LOCATION) << "Initializing builtins failed";
        assert(false && "initializing builtins failed");
    }

    return &builtins;
}

optional<DefinitionHeader*> find_external_runtime_cast(const Scope& scope, const Type* source_type, 
    const Type* target_type) {
    
    std::string cast_name = "to_" + target_type->name_string() + "_" + source_type->name_string();
    
    Log::debug_extra(NO_SOURCE_LOCATION) << "Trying to find runtime cast " << cast_name;

    auto cast = scope.get_identifier(cast_name);
    if (!cast)
        LogNoContext::debug(NO_SOURCE_LOCATION) << "Could not find runtime cast " << cast_name;

    return cast;
}

} // namespace Maps