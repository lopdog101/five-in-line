#include <stdio.h>
#include "field.h"
#include <algorithm>
#include <stdexcept>
#include "gomoku_exceptions.h"

# include "../extern/binary_find.h"


namespace Gomoku
{
	field_t::field_t()
	{
		bound=rect_inverse_infinity();
	}

	void field_t::add(const point& pt,Step st)
	{
		if(st!=st_krestik&&st!=st_nolik)throw e_invalid_step(st);

		if(fld.get(pt)!=st_empty)throw e_point_busy(pt.x,pt.y);
		fld.set(pt,st);

		step_t val(st,pt.x,pt.y);
		steps.push_back(val);

		bound+=pt;
	}

	void field_t::pop(const rect& old_bound)
	{
		steps_t::iterator it=steps.end()-1;
		fld.set(*it,st_empty);
		steps.erase(it);
		bound=old_bound;
	}

	void field_t::set_steps(const steps_t& _steps)
	{
		clear();
		steps_t::const_iterator i=_steps.begin(),endi=_steps.end();
		for(;i!=endi;++i)
			add(*i,i->step);
	}

	void field_t::get_empty_around(points_t& res,int bound_size) const
	{
		res.resize(0);
		for(unsigned i=0;i<steps.size();i++)
		{
			const step_t& st=steps[i];
			for(int y=st.y-bound_size;y<=st.y+bound_size;y++)
			for(int x=st.x-bound_size;x<=st.x+bound_size;x++)
			{
				point p(x,y);
				if(at(p)==st_empty)
					res.push_back(p);
			}
		}

		std::sort(res.begin(),res.end(),less_point_pr());
		res.erase(std::unique(res.begin(),res.end()),res.end());
	}

	void field_t::get_empty_around(const point& c,points_t& res,int bound_size) const
	{
		res.resize(0);
		for(int y=c.y-bound_size;y<=c.y+bound_size;y++)
		for(int x=c.x-bound_size;x<=c.x+bound_size;x++)
		{
			point p(x,y);
			if(at(p)==st_empty)
				res.push_back(p);
		}

		std::sort(res.begin(),res.end(),less_point_pr());
		res.erase(std::unique(res.begin(),res.end()),res.end());
	}


	bool field_t::check_five(const step_t& st) const
	{
		return 
			check_five(st,1,0)||
			check_five(st,0,1)||
			check_five(st,1,1)||
			check_five(st,1,-1);
	}

	bool field_t::check_five(const step_t& st,int dx,int dy) const
	{
		if(at(st)!=st.step)return false;

		point p=st;
		unsigned j=0;

		for(;j<4;j++)
		{
			p.x-=dx;
			p.y-=dy;

			if(at(p)!=st.step)break;
		}

		p=st;
		for(;j<4;j++)
		{
			p.x+=dx;
			p.y+=dy;

			if(at(p)!=st.step)break;
		}

		return j==4;
	}


std::string to_string(Step val)
{
    if(val==st_krestik)return std::string("x");
    if(val==st_nolik)return std::string("0");
    return std::string();
}

std::string print_field(const steps_t& val)
	{
		if(val.empty())return std::string();
		rect rc=rect_inverse_infinity();
		for(steps_t::const_iterator i=val.begin();i!=val.end();++i)
			rc+=*i;

		unsigned width=rc.x2-rc.x1+1;
		unsigned height=rc.y2-rc.y1+1;

		std::vector<char> data(width*height,'+');
		for(unsigned i=0;i<val.size();i++)
		{
			const step_t& v=val[i];
			char sym='¤';
			switch(v.step)
			{
			case st_krestik:
				if(i==0||i+1==val.size())sym='x';
				else sym='X';
				break;
			case st_nolik:
				if(i==0||i+1==val.size())sym='o';
				else sym='O';
				break;
			}
			data[width*(v.y-rc.y1)+(v.x-rc.x1)]=sym;
		}

		std::string ret;
		std::vector<char>::iterator it=data.begin();
		for(unsigned y=0;y<height;y++,it+=width)
			ret+=std::string(it,it+width)+"\r\n";
		return ret;
	}

	std::string print_points(const points_t& vals)
	{
		std::string ret;

		for(unsigned i=0;i<vals.size();i++)
		{
			char tmp[256];
			const npoint& p=vals[i];
			sprintf(tmp,"(%d,%d)",p.x,p.y);
			ret+=tmp;
			if(i+1!=vals.size())ret+=";";
		}
		return ret;
	}

	std::string print_points(const npoints_t& vals)
	{
		std::string ret;

		for(unsigned i=0;i<vals.size();i++)
		{
			char tmp[256];
			const npoint& p=vals[i];
			sprintf(tmp,"(%d,%d:%d)",p.x,p.y,p.n);
			ret+=tmp;
			if(i+1!=vals.size())ret+=";";
		}
		return ret;
	}

	std::string print_steps(const steps_t& vals)
	{
		std::string ret;

		for(unsigned i=0;i<vals.size();i++)
		{
			char tmp[256];
			const step_t& p=vals[i];
			sprintf(tmp,"(%d,%d:%s)",p.x,p.y,(p.step==st_krestik? "X":"O"));
			ret+=tmp;
			if(i+1!=vals.size())ret+=";";
		}
		return ret;
	}

	steps_t scan_steps(const std::string& str)
	{
		steps_t ret;
		if(str.empty())return ret;
		std::string::const_iterator i=str.begin();
		do
		{
			step_t st;
			char sym[2];
			sym[1]=0;
			if(sscanf(&*i,"%*1c %d %*1c %d %*1c %1c",&st.x,&st.y,sym)!=3)
				throw std::runtime_error("scan_steps(): invalid format: "+str);

			if(sym[0]=='X'||sym[0]=='x')st.step=st_krestik;
			else if(sym[0]=='O'||sym[0]=='o')st.step=st_nolik;
			else throw std::runtime_error("scan_steps(): invalid symbol format: "+str);

			ret.push_back(st);
			i=std::find(i,str.end(),';');
			if(i==str.end())break;
			++i;
		}
		while(true);

		return ret;
	}

    void reorder_state_to_game_order(steps_t& steps)
    {
        steps_t krestik_steps(steps.size());
        krestik_steps.erase(
            std::copy_if(steps.begin(),steps.end(),krestik_steps.begin(),step_kind_pr(st_krestik)),
            krestik_steps.end());

        steps_t nolik_steps(steps.size());
        nolik_steps.erase(
            std::copy_if(steps.begin(),steps.end(),nolik_steps.begin(),step_kind_pr(st_nolik)),
            nolik_steps.end());

        if(krestik_steps.empty()&&nolik_steps.empty())
            throw std::runtime_error("String field does not contain valid steps");

        if(krestik_steps.size()!=nolik_steps.size() &&
            krestik_steps.size()!=nolik_steps.size()+1)
        {
            throw std::runtime_error("Game not synced");
        }

        steps_t::iterator it=std::find(krestik_steps.begin(),krestik_steps.end(),step_t(st_krestik,0,0));
        if(it!=krestik_steps.end())
        {
            std::swap(krestik_steps.front(),*it);
        }

        steps.resize(0);
        for(unsigned i=0;i<krestik_steps.size();i++)
        {
            steps.push_back(krestik_steps[i]);
            if(i<nolik_steps.size())
                steps.push_back(nolik_steps[i]);
        }
    }
}
