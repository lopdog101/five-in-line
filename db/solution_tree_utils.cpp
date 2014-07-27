#include "solution_tree_utils.h"
#include <stdexcept>
#include <boost/lexical_cast.hpp>

namespace Gomoku
{
	void points2bin(const points_t& pts,data_t& bin)
	{
		bin.resize(pts.size()*2);
		for(unsigned i=0;i<pts.size();i++)
		{
			const point& p=pts[i];
			if(p.x<=-128||p.x>127||p.y<=-128||p.y>127)
				throw std::runtime_error("points2bin(): invalid point: ("+
				boost::lexical_cast<std::string>(p.x)+","
				+boost::lexical_cast<std::string>(p.y)+")");

			char x=p.x;
			char y=p.y;

			bin[i*2]=*reinterpret_cast<const unsigned char*>(&x);
			bin[i*2+1]=*reinterpret_cast<const unsigned char*>(&y);
		}
	}

	void bin2points(const data_t& bin,points_t& pts)
	{
		if((bin.size()%2)!=0)
			throw std::runtime_error("bin2points(): (bin.size()%2)!=0");

		pts.resize(bin.size()/2);

		for(unsigned i=0;i<pts.size();i++)
		{
			point& p=pts[i];

			char x=*reinterpret_cast<const unsigned char*>(&bin[i*2]);
			char y=*reinterpret_cast<const unsigned char*>(&bin[i*2+1]);

			p.x=x;
			p.y=y;
		}
	}

	void points2bin(const steps_t& pts,data_t& bin)
	{
		bin.resize(pts.size()*3);
		for(unsigned i=0;i<pts.size();i++)
		{
			const step_t& p=pts[i];
			if(p.x<=-128||p.x>127||p.y<=-128||p.y>127)
				throw std::runtime_error("points2bin(): invalid point: ("+
				boost::lexical_cast<std::string>(p.x)+","
				+boost::lexical_cast<std::string>(p.y)+")");

			char x=p.x;
			char y=p.y;

			bin[i*3]=*reinterpret_cast<const unsigned char*>(&x);
			bin[i*3+1]=*reinterpret_cast<const unsigned char*>(&y);
			bin[i*3+2]=p.step;
		}
	}

	void bin2points(const data_t& bin,steps_t& pts)
	{
		if((bin.size()%3)!=0)
			throw std::runtime_error("bin2points(): (bin.size()%3)!=0");

		pts.resize(bin.size()/3);

		for(unsigned i=0;i<pts.size();i++)
		{
			step_t& p=pts[i];

			char x=*reinterpret_cast<const unsigned char*>(&bin[i*3]);
			char y=*reinterpret_cast<const unsigned char*>(&bin[i*3+1]);

			p.x=x;
			p.y=y;
			p.step=(Step)bin[i*3+2];
		}
	}

	void points2bin(const npoints_t& pts,data_t& bin)
	{
		bin.resize(pts.size()*3);
		for(unsigned i=0;i<pts.size();i++)
		{
			const npoint& p=pts[i];
			if(p.x<=-128||p.x>127||p.y<=-128||p.y>127)
				throw std::runtime_error("points2bin(): invalid point: ("+
				boost::lexical_cast<std::string>(p.x)+","
				+boost::lexical_cast<std::string>(p.y)+")");
			if(p.n>255)throw std::runtime_error("points2bin(npoints_t): n>255 n="+
				boost::lexical_cast<std::string>(p.n));

			char x=p.x;
			char y=p.y;

			bin[i*3]=*reinterpret_cast<const unsigned char*>(&x);
			bin[i*3+1]=*reinterpret_cast<const unsigned char*>(&y);
			bin[i*3+2]=static_cast<unsigned char>(p.n);
		}
	}

	void bin2points(const data_t& bin,npoints_t& pts)
	{
		if((bin.size()%3)!=0)
			throw std::runtime_error("bin2points(): (bin.size()%3)!=0");

		pts.resize(bin.size()/3);

		for(unsigned i=0;i<pts.size();i++)
		{
			npoint& p=pts[i];

			char x=*reinterpret_cast<const unsigned char*>(&bin[i*3]);
			char y=*reinterpret_cast<const unsigned char*>(&bin[i*3+1]);

			p.x=x;
			p.y=y;
			p.n=bin[i*3+2];
		}
	}

    void hex_or_str2points(const std::string& str,steps_t& pts)
    {
	if(str.empty())
	{
	    pts.clear();
	    return;
	}

        if(str[0]=='(')pts=scan_steps(str);
        else
        {
			Gomoku::data_t bin;
			Gomoku::hex2bin(str,bin);

			Gomoku::bin2points(bin,pts);
        }
    }

}//namespace
