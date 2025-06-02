#ifndef __CL_OPTIONS_HH
#define __CL_OPTIONS_HH

#include "mapsci/repl.hh"
#include "mapsc/logging.hh"

namespace Maps {

constexpr bool SHOULD_EXIT = true;
constexpr bool SHOULD_RUN = false;

// return values are: should exit, exitcode
std::tuple<bool, int, REPL_Options> process_cl_options(int argc, char* argv[], 
    Maps::LogStream::Options& log_options);

}

#endif