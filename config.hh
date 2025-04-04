#ifndef __CONFIG_HH
#define __CONFIG_HH

#include <string>
#include <vector>

// TODO: handle multiple inputfiles
const std::string USAGE = "USAGE: testc inputfile [-o filename] [-ir filename] [-tokens filename] \n                        [-dump ir|tokens]";

const std::string DEFAULT_OBJ_FILE_PATH = "out.o";
const std::string DEFAULT_IR_FILE_PATH = "out.ll";
const std::string DEFAULT_TOKENS_FILE_PATH = "out.tokens";

// at line 1000 the token stream is gonna shift right, but that's ok
constexpr unsigned int LINE_COL_FORMAT_PADDING = 8;

inline std::string line_col_padding(unsigned int width) {
    return ( width < LINE_COL_FORMAT_PADDING ? 
            std::string(LINE_COL_FORMAT_PADDING - width, ' ') : " ");
}

enum class OutputSink {
    stderr,
    file,
    none
};

struct CL_Options {
    bool bad_args = false;
    std::vector<std::string> input_file_paths = {};

    OutputSink output_ir_to = OutputSink::none;
    OutputSink output_object_file_to = OutputSink::none; // NOTE: object file can't currectly be dumped to stderr
    OutputSink output_token_stream_to = OutputSink::none;
    
    std::string object_file_path = DEFAULT_OBJ_FILE_PATH;
    std::string ir_file_path = DEFAULT_IR_FILE_PATH;  
    std::string tokens_file_path = DEFAULT_TOKENS_FILE_PATH;
};

CL_Options parse_cl_args(int argc, char** argv);

#endif