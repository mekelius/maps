#include "doctest.h"

#include "mapsc/types/function_type.hh"
#include "mapsc/types/type_defs.hh"

using namespace std;
using namespace Maps;

TEST_CASE("Should be able to create functiontype as CTFunctionType") {
    CTFunctionType<2> test_ct_funtion_type{"qwrteyutyttcy", &TestingType, {&TestingType, &Int}, true};

    FunctionType* ftp = &test_ct_funtion_type;
    CHECK(ftp->arity() == 2);
    auto params = ftp->param_types();
    CHECK(params.size() == 2);

    auto params_it = params.begin();
    CHECK(**params_it == TestingType);
    params_it++;
    CHECK(**params_it == Int);
}