#ifndef gomoku_fieldH
#define gomoku_fieldH

#include "step.h"
#include <vector>
#include <string>
#include "primitives.h"
#include <algorithm>
#include <map>

namespace Gomoku
{
	struct step_t : public point
	{
		Step step;
		step_t(){step=st_empty;}
		step_t(Step _st,int _x,int _y) : point(_x,_y) {step=_st;}

		inline bool operator==(const step_t& rhs) const
		{
			return point::operator==(rhs)&&step==rhs.step;
		}
	};

	typedef std::vector<step_t> steps_t;
	typedef std::vector<point> points_t;

	struct npoint : public point
	{
		unsigned n;
		npoint(){n=1;}
		npoint(const point& rhs) : point(rhs){n=1;}
		inline void operator=(const point& rhs){static_cast<point&>(*this)=rhs;}
		inline bool is_same_point(const point& p) const{return static_cast<const point&>(*this)==p;}
	};

	typedef std::vector<npoint> npoints_t;

	struct less_point_pr
	{
		inline bool operator()(const point& a,const point& b) const
		{
			if(a.x!=b.x)return a.x<b.x;
			return a.y<b.y;
		}

		inline bool operator()(const step_t& a,const step_t& b) const
		{
			if(a.x!=b.x)return a.x<b.x;
			if(a.y!=b.y)return a.y<b.y;
			return a.step<b.step;
		}
	};

	struct near_point_pr : public less_point_pr
	{
		point c;
		near_point_pr(const point& _c) : c(_c){}
		int dist(const point& a) const
		{
			int rx=a.x-c.x;if(rx<0)rx=-rx;
			int ry=a.y-c.y;if(ry<0)ry=-ry;
            return rx+ry;
		}

		inline bool operator()(const point& a,const point& b) const
		{
		  int da=dist(a);
			int db=dist(b);
			if(da!=db)return da<db;
			return less_point_pr::operator()(a,b);
		}
	};

	class step_kind_pr
	{
        Step cl;
    public:
        step_kind_pr(Step _cl){cl=_cl;}

		inline bool operator()(const step_t& val) const
		{
            return val.step ==cl;
		}
	};

	template<class Value>
	class matrix
	{
	public:
		typedef std::map<point,Value,less_point_pr> point2val;
		typedef std::vector<Value> fld_t;
	private:
		rect max_bound;
		fld_t fld;
		point2val outbound;
		size_t width;
	public:
		matrix(const rect& _max_bound=rect(-40,-40,40,40)) : 
		  max_bound(_max_bound),
		  width(_max_bound.width()+1),
		  fld((_max_bound.width()+1)*(_max_bound.height()+1),Value())
		{
		}

        Value get(const point& pt) const
		{
			if(max_bound&pt)
				return fld[(pt.y-max_bound.y1)*width+pt.x-max_bound.x1];
			typename point2val::const_iterator it=outbound.find(pt);
			if(it==outbound.end())return Value();
			return it->second;
		}

		void set(const point& pt,Value v)
		{
			if(max_bound&pt)
				fld[(pt.y-max_bound.y1)*width+pt.x-max_bound.x1]=v;
        	else
				outbound[pt]=v;
		}

		void clear()
		{
			std::fill(fld.begin(),fld.end(),Value());
			outbound.clear();
		}
	};

	class field_t
	{
	public:
		typedef std::vector<step_t> steps_t;
	private:
		steps_t steps;
		rect bound;
		matrix<Step> fld;

		bool check_five(const step_t& st,int dx,int dy) const;
	public:
		field_t();
		virtual ~field_t(){}

        void add(const point& pt,Step st);
		inline Step at(const point& pt) const{return fld.get(pt);}

		inline size_t size() const{return steps.size();}
		inline bool empty() const{return steps.empty();}

		inline const step_t& operator[](size_t index) const{return steps[index];}
		inline const step_t& front() const{return steps.front();}
		inline const step_t& back() const{return steps.back();}
		inline const steps_t& get_steps() const{return steps;}
		void set_steps(const steps_t& _steps);


		inline void clear()
		{
			steps.clear();
			bound=rect_inverse_infinity();
			fld.clear();
		}

		inline const rect& get_bound() const{return bound;}

		void pop(const rect& old_bound);

		void get_empty_around(points_t& res,int bound_size=1) const;
		void get_empty_around(const point& p,points_t& res,int bound_size=1) const;

		bool check_five(const step_t& st) const;
	};


	class temporary_step
	{
		field_t& fd;
		rect rc;
	public:
		temporary_step(field_t& _fd,const point& pt,Step st) : fd(_fd),rc(_fd.get_bound())
		{
			fd.add(pt,st);
		}

		~temporary_step()
		{
			fd.pop(rc);
		}
	};


	inline void base_to_zero(steps_t& val)
	{
		if(val.empty())return;
		point pt=val.front();
		steps_t::iterator i=val.begin(),endi=val.end();
		for(;i!=endi;++i)*i-=pt;
	}

	std::string to_string(Step val);

	std::string print_field(const steps_t& val);
	std::string print_steps(const steps_t& val);

	std::string print_points(const points_t& vals);
	std::string print_points(const npoints_t& vals);

	steps_t scan_steps(const std::string& str);

	inline void sort_steps(steps_t& val){std::sort(val.begin(),val.end(),less_point_pr() );}
    void reorder_state_to_game_order(steps_t& steps);

}//namespace gomoku

namespace std
{
	inline bool operator==(const Gomoku::steps_t& lhs,const Gomoku::steps_t& rhs)
	{
		return lhs.size()==rhs.size()&&equal(lhs.begin(),lhs.end(),rhs.begin());
	}
}


#endif
