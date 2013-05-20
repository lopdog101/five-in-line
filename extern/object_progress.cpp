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

    perfomance::PrecT perfomance::precision=pr_millisec;
    std::string perfomance::units=" Sec.";

    perfomance::val_t perfomance::current_time()
    {
    #ifdef _WIN32
	    static double resolution=0;
	    if(!resolution)
	    {
		    val_t res;
		    if(QueryPerformanceFrequency((LARGE_INTEGER*)&res))resolution=res/1000000.0;
		    else resolution=1;
	    }

	    val_t ret=0;
	    if(QueryPerformanceCounter((LARGE_INTEGER*)&ret)) 
		    ret=static_cast<val_t>(ret/resolution);
	    else ret=GetTickCount()*1000;
	    return ret;
    #else
	    timeval tv;
	    gettimeofday(&tv,0);
	    val_t ret=val_t(tv.tv_sec)*1000000+tv.tv_usec;
	    return ret;
    #endif
    }

    std::string perfomance::str() const
    {
	    val_t v=delay();
	    val_t sec=v/1000000;
	    unsigned micr=static_cast<unsigned>(v%1000000);

	    char buf[128];
	    sprintf(buf,"%I64u",sec);
	    std::string ret=buf;

	    switch(precision)
	    {
	    case pr_millisec:
		    micr/=1000;
		    if(micr!=0)
		    {
			    sprintf(buf,".%03u",micr);
			    ret+=buf;
		    }
		    break;
	    case pr_microsec:
		    if(micr!=0)
		    {
			    sprintf(buf,".%06u",micr);
			    ret+=buf;
		    }
		    break;
	    }
    
	    ret+=units;
	    return ret;
    }

}//namespace
