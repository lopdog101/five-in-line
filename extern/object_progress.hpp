#ifndef object_progressHPP
#define object_progressHPP
#include <stdio.h>
#include <time.h>
#include <iostream>
#include <fstream>
#include <boost/lexical_cast.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/signal.hpp>

namespace ObjectProgress
{
class log_generator;

typedef boost::signal<void (const std::string&)> log_handler_t;
log_handler_t& get_log_handler();



class one_message
{
public:
	struct data_t
	{
		std::basic_stringstream<char> stream;
		data_t(log_generator&) {}
		~data_t()
        {
            std::string str = stream.str();
            get_log_handler()(str);
       }
	};
public:
	boost::shared_ptr< data_t > data;

	one_message(){}
	one_message(log_generator& mgr)
	{
		data=boost::shared_ptr< data_t >(new data_t(mgr));
	};
};

template<class T>
inline const one_message& operator<<(const one_message& lhs,const T& rhs)
{
	if(lhs.data)lhs.data->stream<<rhs;
	return lhs;
}

class log_generator
{
public:
	log_generator(bool){}

	template<class T>
	inline one_message operator<<(const T& val)
	{
		one_message mess(*this);
		mess<<val;
		return mess;
	}
};

class perfomance
{
public:
	typedef unsigned long long val_t;
	enum PrecT{pr_sec,pr_millisec,pr_microsec};
private:
	val_t start;
public:
	perfomance(){reset();}
	inline val_t delay() const{return current_time()-start;}
	std::string str() const;

	inline void reset(){start=current_time();}

	static PrecT precision;
	static std::string units;

	static val_t current_time();
};


template <class charT, class traits>
inline std::basic_ostream<charT, traits>& operator<<(std::basic_ostream<charT, traits>& os, const perfomance& t)
{
	return os<<t.str();
}



class ilogout
{
    boost::signals::scoped_connection hld;
protected:
    virtual void on_message(const std::string& str)= 0;
public:
    bool print_timestamp;

    ilogout(){print_timestamp=false;}
    virtual ~ilogout(){}
    virtual void open();
    
    static std::string current_timestamp();
};

class logout_debug : public ilogout
{
protected:
    void on_message(const std::string& str);
};

class logout_cerr : public ilogout
{
protected:
    void on_message(const std::string& str);
};

class logout_file : public ilogout
{
protected:
    std::ofstream file;

    void on_message(const std::string& str);
public:
    std::string file_name;

    void open();
};

}//namespace

#endif

