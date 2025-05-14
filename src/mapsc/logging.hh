#ifndef __LOGGING_HH
#define __LOGGING_HH

#include <array>
#include <iostream>
#include <string>

#include "loglevel_defs.hh"
#include "mapsc/source.hh"

namespace Maps {

class Logger {
public:
    struct Options {
        MessageTypes message_types = LogLevel::default_();
        std::ostream* ostream = &std::cout;

        constexpr bool has_message_type(MessageType message_type) const {
            return message_types.at(static_cast<int>(message_type));
        };

        constexpr void set(MessageType message_type, bool value) {
            message_types.at(static_cast<int>(message_type)) = value;
        }

        constexpr void set(const MessageTypes& message_types_new) {
            message_types = message_types_new;
        }
    };
        
    // --- STATIC METHODS ---
    static Logger get();
    static void set_global_options(const Options& options);

    // --- PUBLIC METHODS ---
    void log_error(const std::string& message, SourceLocation location = NO_SOURCE_LOCATION);
    void log_token(const std::string& message, SourceLocation location);
    void log_info(const std::string& message, MessageType message_type, 
        SourceLocation location = NO_SOURCE_LOCATION);

    // returns true if log output has happened since last time this was called
    // only call this from main as a formatting aid
    // TODO: this should be like, per stream and actually not the responsibility of the logger
    // Maybe this could be a kind of stream wrapper instead
    static bool logs_since_last_check();

private:
    Logger(Options* options = &global_options): options_(options) {}

    Options* options_;

    static Options global_options;
    static Logger global_logger;

public:
    struct Global {
        constexpr static void log_error(const std::string& message, 
            SourceLocation location = NO_SOURCE_LOCATION) {
            
            global_logger.log_error(message, location);
        }
        constexpr static void log_info(const std::string& message, MessageType message_type, 
            SourceLocation location = NO_SOURCE_LOCATION) {

            global_logger.log_info(message, message_type, location);
        }
        constexpr static void log_token(const std::string& message, SourceLocation location) {
            global_logger.log_token(message, location);
        }
    };
};

// unfortunately the default args don't get transferred correctly if we don't do this
namespace GlobalLogger {

constexpr static void log_error(const std::string& message, 
    SourceLocation location = NO_SOURCE_LOCATION) { Logger::Global::log_error(message, location); }
constexpr static void log_info(const std::string& message, MessageType message_type, 
    SourceLocation location = NO_SOURCE_LOCATION) { 
        Logger::Global::log_info(message, message_type, location);}
constexpr static void log_token(const std::string& message, SourceLocation location) { 
    Logger::Global::log_token(message, location); }

} // namespace GlobalLogger

} // namespace Maps

#endif