#include "config.hh"

#include <iostream>

CL_Options parse_cl_args(int argc, char** argv) {
    CL_Options failed = { true };
    if (argc < 2)
        return failed;

    std::vector<std::string> args{ argv + 1, argv + argc };
    CL_Options options = { false };

    for (auto it = args.begin(); it < args.end(); it++) {
        std::string arg = *it;

        if (arg == "-o" || arg == "-ir" || arg == "-tokens") {
            it++;

            if (it >= args.end())
                return failed;

            if (options.output_object_file_to != OutputSink::none) {
                std::cerr << "Conflicting command line arguments" << std::endl;
                return failed;
            }

            if (arg == "-o") {
                options.object_file_path = *it;
                options.output_object_file_to = OutputSink::file;
            } else if (arg == "-ir") {
                options.ir_file_path = *it;
                options.output_ir_to = OutputSink::file;
            } else if (arg == "-tokens") {
                options.tokens_file_path = *it;
                options.output_token_stream_to = OutputSink::file;
            }

            continue;            
        }
        
        if (arg == "-dump") {
            OutputSink sink = OutputSink::stderr;

            it++;
            if (it >= args.end())
                return failed;
            
            if (*it == "ir") {
                options.output_ir_to = sink;

            } else if (*it == "tokens") {
                options.output_token_stream_to = sink;

            } else {
                return failed;
            }

            continue;
        } 
        
        // args without '-' are input files
        if (options.input_file_paths.size() >= 1) {
            std::cerr << "Multiple input files not yet supported" << std::endl;
            return failed;
        }

        options.input_file_paths.push_back(arg);
    }

    return options;
}