#include "mapsc/logging.hh"

using std::optional, std::nullopt, std::unique_ptr, std::make_unique;

namespace Maps {

namespace {

// at line 1000 the token stream is gonna shift right, but that's ok
constexpr unsigned int LINE_COL_FORMAT_PADDING = 8;

bool log_check_flag = false;

std::string line_col_padding(unsigned int width) {
    return width < LINE_COL_FORMAT_PADDING ? 
        std::string(LINE_COL_FORMAT_PADDING - width, ' ') : " ";
}

} // namespace

Logger::Options Logger::global_options = Logger::Options{};
Logger Logger::global_logger = Logger{};

Logger Logger::get() {
    return Logger{};
}
void Logger::set_global_options(const Options& options) {
    global_options = options;
}

void Logger::log_error(const std::string& message, SourceLocation location) {
    if (!options_->has_message_type(MessageType::error))
        return;
    
    std::string location_string = location.to_string();
    
    log_check_flag = true;

    *options_->ostream
        << location_string << line_col_padding(location_string.size())
        << "error: " << message << "\n";
}

void Logger::log_info(const std::string& message, MessageType message_type, SourceLocation location) {
    if (!options_->has_message_type(message_type))
        return;

    std::string location_string = location.to_string();

    log_check_flag = true;

    *options_->ostream
        << location_string << line_col_padding(location_string.size())
        << "info:  " << message << '\n';
}

void Logger::log_token(const std::string& message, SourceLocation location) {
    if (!options_->has_message_type(MessageType::lexer_debug_token))
        return;
    log_check_flag = true;

    *options_->ostream
        << location.to_string() << line_col_padding(location.to_string().size()) 
        << "token: " << message << '\n';
}

bool Logger::logs_since_last_check() {
    bool value = log_check_flag;
    log_check_flag = false;
    return value;
}

} //namespace Maps