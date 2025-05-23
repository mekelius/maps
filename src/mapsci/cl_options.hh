#ifndef __CL_OPTIONS_HH
#define __CL_OPTIONS_HH

#include "mapsci/repl.hh"

namespace Maps {

class LogOptions;

}

constexpr bool SHOULD_EXIT = true;
constexpr bool SHOULD_RUN = false;

// return values are: should exit, exitcode
std::tuple<bool, int, REPL_Options> process_cl_options(int argc, char* argv[], 
    Maps::LogOptions& log_options);

#endif