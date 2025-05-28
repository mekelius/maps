#ifndef __REPL_HH
#define __REPL_HH

#include <filesystem>
#include <string>

#include "mapsc/compilation_state.hh"
#include "mapsc/procedures/reverse_parse.hh"

class JIT_Manager;

namespace llvm { class LLVMContext; class raw_ostream; }

static constexpr std::string_view REPL_DEFAULT_MODULE_NAME = "ir_module";
static constexpr std::string_view REPL_DEFAULT_PROMPT = "mapsci> ";
static constexpr std::string_view REPL_DEFAULT_REPL_WRAPPER_NAME = "mapsci_repl_wrapper";

constexpr static auto REPL_STAGES_START_LINE = __LINE__;
enum class REPL_Stage {
    layer1                  = 0,
    type_name_resolution    = 1,
    name_resolution         = 2,
    layer2                  = 3,
    transform_stage         = 4,
    pre_ir                  = 5,
    ir                      = 6,
    done                    = 7
};
constexpr static auto REPL_STAGE_COUNT = __LINE__ - REPL_STAGES_START_LINE - 3;

struct REPL_Options {
    std::array<bool, REPL_STAGE_COUNT> debug_prints;
    
    bool eval = true;

    bool ignore_errors = false;
    REPL_Stage stop_after = REPL_Stage::done;
    bool quit_on_error = false;

    bool save_history = true;
    std::filesystem::path history_file_path;

    Maps::CompilationState::Options compiler_options{};
    Maps::ReverseParser::Options reverse_parse{};

    std::string module_name = std::string{REPL_DEFAULT_MODULE_NAME};
    std::string prompt = std::string{REPL_DEFAULT_PROMPT};
    std::string repl_wrapper_name = std::string{REPL_DEFAULT_REPL_WRAPPER_NAME};

    bool has_debug_print(REPL_Stage stage) {
        return debug_prints.at(static_cast<size_t>(stage));
    }

    void set_debug_print(REPL_Stage stage, bool set_to = true) {
        debug_prints.at(static_cast<size_t>(stage)) = set_to;
    }
};

bool run_repl(JIT_Manager& jit, llvm::LLVMContext& context, llvm::raw_ostream& error_stream, 
    const REPL_Options& options);
    
#endif