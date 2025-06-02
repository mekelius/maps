#include "doctest.h"

#include "mapsc/logging.hh"

using namespace Maps;


TEST_CASE("Lock should lock properly") {
    auto lock = LogStream::global.lock();
    (*lock)->options_->set_loglevel(LogLevel::debug_extra);
    CHECK((*lock)->options_->get_loglevel() == LogLevel::debug_extra);
}

TEST_CASE("Lock should lock properly when setting") {
    auto lock = LogStream::global.set_loglevel(LogLevel::debug_extra);
    CHECK((*lock)->options_->get_loglevel() == LogLevel::debug_extra);
}

TEST_CASE("Lock should prevent setting options") {
    auto lock = LogStream::global.lock();
    (*lock)->options_->set_loglevel(LogLevel::debug_extra);

    auto lock2 = LogStream::global.set_loglevel(LogLevel::error);
    
    CHECK((*lock)->options_->get_loglevel() == LogLevel::debug_extra);
}