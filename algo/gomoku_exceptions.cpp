
#include "gomoku_exceptions.h"
#include <boost/lexical_cast.hpp>

namespace Gomoku
{

e_invalid_step::e_invalid_step(unsigned val)
{
	mess="Invalid step "+boost::lexical_cast<std::string>(val);
}

e_point_busy::e_point_busy(int x,int y)
{
	mess="Point ("+boost::lexical_cast<std::string>(x)+";"+boost::lexical_cast<std::string>(y)+")";
}

}//namespace Gomoku
