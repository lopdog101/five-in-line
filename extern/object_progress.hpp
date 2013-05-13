#ifndef object_progressHPP
#define object_progressHPP
#include <stdio.h>
#include <time.h>
#include <iostream>
#include <boost/lexical_cast.hpp>
#include <boost/shared_ptr.hpp>

namespace ObjectProgress
{
class log_generator;

void output_debug_message(const std::string& str);

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
            output_debug_message(str);
            std::cerr<<str<<std::endl;
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
private:
	time_t start;
public:
	perfomance(){reset();}
	inline time_t delay() const{return time(0)-start;}
	std::string str() const
	{
	  char str[256];
	  sprintf(str,"%d sec.",(int)delay());
	  return std::string(str);
	}

	inline void reset(){start=time(0);}
};


template <class charT, class traits>
inline std::basic_ostream<charT, traits>& operator<<(std::basic_ostream<charT, traits>& os, const perfomance& t)
{
	return os<<t.str();
}


}//namespace

#endif

