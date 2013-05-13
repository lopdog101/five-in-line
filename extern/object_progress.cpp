#ifdef _WIN32
#include <windows.h>
#endif
#include "object_progress.hpp"

namespace ObjectProgress
{

    void output_debug_message(const std::string& str)
    {
#ifdef _WIN32
        std::string s = "DBG: "+str+"\r\n";
        OutputDebugStringA(s.c_str());
#endif
    }

}//namespace
