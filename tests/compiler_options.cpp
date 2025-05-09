#include "doctest.h"

#include <thread>
#include <future>

#include "mapsc/compiler_options.hh"

using namespace Maps;
using namespace std;

TEST_CASE("Should be able to read and set globally") {
    REQUIRE(CompilerOptions::get(CompilerOption::dummyvalue_1) == "1");
    REQUIRE(CompilerOptions::get(CompilerOption::dummyvalue_2) == "2");
    REQUIRE(CompilerOptions::get(CompilerOption::dummyvalue_3) == "3");

    SUBCASE("Set 1 read another") {
        auto options = CompilerOptions::lock();

        CHECK(options);
        (*options)->set(CompilerOption::dummyvalue_2, "test");

        CHECK(CompilerOptions::get(CompilerOption::dummyvalue_2) == "test");
        CHECK(CompilerOptions::get(CompilerOption::dummyvalue_1) == "1");
    }

    SUBCASE("Set and reset the lock should reset the value") {
        auto options = CompilerOptions::lock();
        CHECK(options);

        (*options)->set(CompilerOption::dummyvalue_2, "test");
        CHECK(CompilerOptions::get(CompilerOption::dummyvalue_2) == "test");

        options.reset();
        CHECK(CompilerOptions::get(CompilerOption::dummyvalue_2) == "2");
    }

    SUBCASE("Setting the value should affect globally in the same thread") {
        auto options = CompilerOptions::lock();

        (*options)->set(CompilerOption::dummyvalue_2, "test");
        CHECK(CompilerOptions::get(CompilerOption::dummyvalue_2) == "test");

        options.reset();
        CHECK(CompilerOptions::get(CompilerOption::dummyvalue_2) == "2");
    }

    SUBCASE("Setting the value globally should affect other threads") {
        auto get_val_2 = [](){
            return CompilerOptions::get(CompilerOption::dummyvalue_2);
        };
        auto options = CompilerOptions::lock();

        CHECK(async(get_val_2).get() == "2");

        (*options)->set(CompilerOption::dummyvalue_2, "test");
        CHECK(async(get_val_2).get() == "test");
    }
}

TEST_CASE("Should be able to set for single thread") {
    REQUIRE(CompilerOptions::get(CompilerOption::dummyvalue_1) == "1");
    REQUIRE(CompilerOptions::get(CompilerOption::dummyvalue_2) == "2");
    REQUIRE(CompilerOptions::get(CompilerOption::dummyvalue_3) == "3");

    auto get_val_1 = [](){
        return CompilerOptions::get(CompilerOption::dummyvalue_1);
    };

    SUBCASE("Set in parent shouldn;t affect child") {
        auto options = CompilerOptions::lock_for_this_thread({
            {CompilerOption::dummyvalue_1, "parent"}
        });

        CHECK(async(get_val_1).get() == "1");
        CHECK(get_val_1() == "parent");
    }
}

TEST_CASE("Shouldn't be able to get multiple locks") {
    REQUIRE(CompilerOptions::get(CompilerOption::dummyvalue_1) == "1");
    REQUIRE(CompilerOptions::get(CompilerOption::dummyvalue_2) == "2");
    REQUIRE(CompilerOptions::get(CompilerOption::dummyvalue_3) == "3");

    auto options = CompilerOptions::lock_for_this_thread();
    CHECK(options);

    CHECK(!CompilerOptions::lock_for_this_thread());
    CHECK(!CompilerOptions::lock());

    CHECK(async([](){
        return static_cast<bool>(CompilerOptions::lock_for_this_thread());
    }).get());
}