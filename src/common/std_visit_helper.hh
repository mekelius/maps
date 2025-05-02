#ifndef __STD_VISIT_HELPER_HH
#define __STD_VISIT_HELPER_HH

template<class... Ts>
struct overloaded : Ts... { using Ts::operator()...; };

#endif