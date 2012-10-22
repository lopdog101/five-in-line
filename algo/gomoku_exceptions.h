#ifndef gomoku_gomoku_exceptionsH
#define gomoku_gomoku_exceptionsH

#include <exception>
#include <string>

namespace Gomoku
{

struct e_exception : public std::exception
{
	std::string mess;
	~e_exception() throw(){}
	const char* what() const throw(){return mess.c_str();}
};

struct e_invalid_step : public e_exception
{
	e_invalid_step(unsigned val);
};

struct e_point_busy : public e_exception
{
	e_point_busy(int x,int y);
};


}//namespace Gomoku

#endif
