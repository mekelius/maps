#ifndef __BUILTINS_HH
#define __BUILTINS_HH

#include <cassert>
#include <optional>
#include <string>
#include <string_view>
#include <variant>

#include "common/maps_datatypes.h"

#include "mapsc/source_location.hh"

#include "mapsc/types/function_type.hh"
#include "mapsc/types/type_defs.hh"

#include "mapsc/ast/definition.hh"
#include "mapsc/ast/builtin.hh"
#include "mapsc/ast/expression.hh"
#include "mapsc/ast/operator.hh"
#include "mapsc/ast/scope.hh"

namespace Maps {

constexpr DefinitionHeader unary_minus_Int_{"unary_minus_Int_", &Int_to_Int};
constexpr DefinitionHeader plus_Int_{"plus_Int_", &IntInt_to_Int};
constexpr DefinitionHeader minus_Int_{"minus_Int_", &IntInt_to_Int};
constexpr DefinitionHeader mult_Int_{"mult_Int_", &IntInt_to_Int};

constexpr Operator unary_minus_Int{"-", &unary_minus_Int_, Operator::Properties{Operator::Fixity::unary_prefix}, BUILTIN_SOURCE_LOCATION};
constexpr Operator plus_Int{"+", &plus_Int_, {Operator::Fixity::binary, 500}, BUILTIN_SOURCE_LOCATION};
constexpr Operator binary_minus_Int{"-", &minus_Int_, {Operator::Fixity::binary, 510}, BUILTIN_SOURCE_LOCATION};
constexpr Operator mult_Int{"*", &mult_Int_, {Operator::Fixity::binary, 520}, BUILTIN_SOURCE_LOCATION};

constexpr DefinitionHeader prints("prints", &String_to_IO_Void);
constexpr DefinitionHeader printms{"printms", &MutString_to_IO_Void};
constexpr DefinitionHeader to_String_Boolean{"to_String_Boolean", &Boolean_to_String};
constexpr DefinitionHeader to_String_Int{"to_String_Int", &Int_to_String};
constexpr DefinitionHeader to_String_Float{"to_String_Float", &Float_to_String};
constexpr DefinitionHeader to_Float_Int{"to_Float_Int", &Int_to_Float};
constexpr DefinitionHeader to_String_MutString{"to_String_MutString", &MutString_to_String};
constexpr DefinitionHeader to_MutString_Int{"to_MutString_Int", &Int_to_MutString};
constexpr DefinitionHeader to_MutString_Float{"to_MutString_Float", &Float_to_MutString};
constexpr DefinitionHeader concat{"concat", &MutString_MutString_to_MutString};

constexpr BuiltinExternalScope builtin_externals {
    &prints,
    &printms,
    &to_String_Boolean,
    // &to_String_Int,
    // &to_String_Float,
    &to_Float_Int,
    &to_String_MutString,
    &to_MutString_Int,
    &to_MutString_Float,
    &concat,
    &unary_minus_Int,
    &plus_Int,
    &binary_minus_Int,
    &mult_Int,
};

constexpr BuiltinValue true_{"true", true, &Boolean};
constexpr BuiltinValue false_{"false", false, &Boolean};

constexpr BuiltinValueScope builtin_values {
    &true_,
    &false_,
};

inline std::optional<const DefinitionHeader*> find_external_runtime_cast(
    const Type* source_type, const Type* target_type) {
    
    using Log = LogInContext<LogContext::type_casts>;

    std::string cast_name = "to_" + target_type->name_string() + "_" + source_type->name_string();
    
    Log::debug_extra(NO_SOURCE_LOCATION) << "Trying to find runtime cast " << cast_name << Endl;

    auto cast = builtin_externals.get_identifier(cast_name);
    if (!cast)
        LogNoContext::debug(NO_SOURCE_LOCATION) << 
            "Could not find runtime cast " << cast_name << Endl;

    return cast;
}

constexpr auto builtins = std::tie(builtin_externals, builtin_values);

} // namespace Maps

#endif