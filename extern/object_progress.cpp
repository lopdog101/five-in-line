#ifdef _WIN32
# include <windows.h>
#else
# include <sys/time.h>
#endif

#include "object_progress.hpp"
#include <boost/bind.hpp>
#include <iostream>

namespace ObjectProgress
{
    log_handler_t& get_log_handler()
    {
        static log_handler_t v;
        return v;
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
#ifdef _WIN32
	    sprintf(buf,"%I64u",sec);
#else
	    sprintf(buf,"%llu",sec);
#endif
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

    void ilogout::open()
    {
        hld=get_log_handler().connect(boost::bind(&ilogout::on_message,this,_1) );
    }
        
    std::string ilogout::current_timestamp()
    {
        time_t t=time(0);
        tm tt=*localtime(&t);

        char tmp[256];
        sprintf(tmp,"%04d-%02d-%02d %02d:%02d:%02d",
            tt.tm_year+1900,tt.tm_mon+1,tt.tm_mday,
            tt.tm_hour,tt.tm_min,tt.tm_sec);

        return std::string(tmp);
    }


    void logout_debug::on_message(const std::string& str)
    {
#ifdef _WIN32
        std::string s;
        if(print_timestamp)s=current_timestamp()+" ";
        s+=str+"\r\n";
        OutputDebugStringA(s.c_str());
#endif
    }
    
    void logout_cerr::on_message(const std::string& str)
    {
        std::string s;
        if(print_timestamp)s=current_timestamp()+" ";
        s+=str;
        std::cerr<<s<<std::endl;
    }

    void logout_file::open()
    {
        file.open(file_name.c_str(), file.binary);
        ilogout::open();
    }

    void logout_file::on_message(const std::string& str)
    {
        if(!file.is_open())
            return;
        
        std::string s;
        if(print_timestamp)s=current_timestamp()+" ";
        s+=str;

        file<<s<<std::endl;
    }

}//namespace
