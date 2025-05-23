#include "cl_options.hh"

#include "mapsc/logging.hh"
#include "mapsc/logging_options.hh"

using Maps::LogInContext, Maps::LogOptions, Maps::LogLevel, Maps::LogContext;
using Maps::CompilationState;


constexpr std::string_view USAGE = "\
!!! NOT UP TO DATE !!!!\n\
\n\
USAGE: mapsci [options]\n\
\n\
options:\n\
  --parser-debug | -q | --quiet | -e | --everything\n\
  --ir | --print-ir\n\
  --no-eval\n\
  --parsed | --print-parsed\n\
  -h | --help\n\
";

const std::string DATA_SUBDIRECTORY = "mapsc";
const std::string HISTORY_FILENAME = "mapsci_history";

// first see if $XDG_DATA_DIR is set
// if not, try $HOME/.local/share
std::optional<std::filesystem::path> get_data_directory() {
    char* c_str;

    if ((c_str = getenv("XDG_DATA_DIR")))
        return std::filesystem::path{c_str};

    std::filesystem::path home_dir;
    if ((c_str = getenv("HOME"))) {
        home_dir = std::filesystem::path{c_str};
    } else {
        std::cerr << "ERROR: finding user data directory failed\n";
        return std::nullopt;
    }

    return home_dir /= ".local/share";
}

// gets the default history file path, creating the subdirectory if it doesn't exist
std::optional<std::filesystem::path> get_default_history_file_path() {
    auto data_directory = get_data_directory();

    if (!data_directory)
        return std::nullopt;
    
    *data_directory /= DATA_SUBDIRECTORY;

    // if the directory doesn't exist, just create it
    if (!std::filesystem::exists(*data_directory))
        std::filesystem::create_directory(*data_directory);

    // check if the directory existed but was a file
    if (!std::filesystem::is_directory(*data_directory)) {
        std::cerr << "ERROR: " << *data_directory << " is not a directory";
        return std::nullopt;
    }

    return *data_directory /= HISTORY_FILENAME;
}

std::pair<bool, int> process_cl_options(int argc, char* argv[], REPL::Options& repl_options, 
    Maps::LogOptions& log_options) {

    std::vector<std::string> args = {argv + 1, argc + argv};
    std::vector<std::string> source_filenames{};

    for (std::string arg: args) {
        // split the arg on '='
        std::stringstream arg_s{arg};
        std::string key;
        std::string value;

        std::getline(arg_s, key, '=');
        std::getline(arg_s, value);

        if (key == "-t" || key == "--tokens") {
            log_options.set_loglevel(LogContext::lexer, LogLevel::debug_extra);

        } else if (key == "--debug" || key == "--parser-debug") {
            log_options.set_loglevel(LogLevel::debug);
            
        } else if (key == "-q" || key == "--quiet") {
            log_options.set_loglevel(LogLevel::compiler_error);

        } else if (key == "-e" || key == "--everything") {
            log_options.set_loglevel(LogLevel::debug_extra);

        } else if (key == "--ir" || key == "--print-ir" || key == "--dump-ir") {
            repl_options.print_ir = true;
            
        } else if (key == "--no-ir" || key == "--no-codegen") {
            repl_options.stop_after = std::min(REPL::Stage::layer3, repl_options.stop_after);

        } else if (key == "--no-eval") {
            repl_options.eval = false;

        } else if (key == "--layer3" || key == "--print-layer3" || key == "--pre-ir" || key == "--print-pre-ir" || key == "--reverse-parse") {
            repl_options.print_layer3 = true;

        } else if (key == "--quit-on-error" || key == "--exit-on-error" || key == "--quit-on-fail" || key == "--exit-on-fail") {
            repl_options.quit_on_error = true;

        } else if (key == "--no-prompt") {
            repl_options.prompt = "";

        } else if (key == "--e2e-tests-mode") {
            repl_options.save_history = false;
            repl_options.quit_on_error = true;
            log_options.set_loglevel(LogLevel::error);

        } else if (key == "--layer1") {
            repl_options.print_layer1 = true;

        } else if (key == "--layer2") {
            repl_options.print_layer2 = true;

        } else if (key == "--stop-after") {
            if (value == "layer1") {
                repl_options.stop_after = REPL::Stage::layer1;

            } else if (value == "layer2") {
                repl_options.stop_after = REPL::Stage::layer2;

            } else if (value == "layer3") {
                repl_options.stop_after = REPL::Stage::layer3;

            } else if(value == "ir") {
                repl_options.stop_after = REPL::Stage::ir;
            }
        } else if (key == "--ignore-errors" || key == "--ignore-error") {
            repl_options.ignore_errors = false;

        } else if (key == "--types" || key == "--print-all-types" || key == "--print-types" || key == "--include-all-types" || key == "--include-types") {
            repl_options.reverse_parse.include_all_types = true;

        } else if (key == "--no-history") {
            repl_options.save_history = false;

        } else if (key == "--history-file") {
            repl_options.history_file_path = value;

        } else if (key == "-h" || key == "--help" || key == "--usage") {
            std::cout << USAGE << std::endl;
            return {SHOULD_EXIT, EXIT_SUCCESS};

        } else if (key.at(0) == '-') {
            std::cout << "ERROR: unknown option: " << key << std::endl;
            return {SHOULD_EXIT, EXIT_FAILURE};

        } else {
            std::cerr << "ERROR: interpreting source files not implemented, exiting" << std::endl;
            return {SHOULD_EXIT, EXIT_FAILURE};
            source_filenames.push_back(key);
        }
    }

    if (repl_options.save_history && repl_options.history_file_path.empty()) {
        auto history_file_path = get_default_history_file_path();
        if (!history_file_path) {
            repl_options.save_history = false;
            repl_options.history_file_path = "";
        } else {
            repl_options.history_file_path = *history_file_path;
        }
    }

    return {SHOULD_RUN, EXIT_SUCCESS};
}