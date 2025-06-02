#include "implementation.hh"

#include <sstream>

#include "mapsc/source.hh"
#include "mapsc/logging.hh"

#include "mapsc/pragma.hh"
#include "mapsc/parser/token.hh"

using std::optional, std::nullopt, std::make_unique;


namespace Maps {

using Log = LogInContext<LogContext::layer1>;


// NOTE: pragma.cpp does its own logging
void ParserLayer1::handle_pragma() {
    auto location = current_token().location;

    // get the first word
    std::istringstream token_value_iss{current_token().string_value()};
    std::string value_string;
    std::getline(token_value_iss, value_string, ' ');

    std::string flag_name;
    std::getline(token_value_iss, flag_name);
    
    // convert the forst word into bool
    bool value;
    if (value_string == "enable") {
        value = true;
    } else if (value_string == "disable") {
        value = false;
    } else {
        Log::error(location) << "invalid pragma declaration" << Endl;
        get_token();
        return fail();
    }

    // the rest should be the flag name
    bool succeeded = pragma_store_->set_flag(
        flag_name, value, current_token().location);
    
    if (!succeeded) {
        Log::error(location) << "handling pragma failed" << Endl;
        return fail();
    }

    get_token();
}

} // namespace Maps
