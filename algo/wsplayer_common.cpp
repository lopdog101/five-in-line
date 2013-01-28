#include "wsplayer_common.h"
#include <stdexcept>

#ifdef WITHOUT_EXTERNAL_LIBS
#  include "../extern/pair_comparator.h"
#  include "../extern/binary_find.h"
#  include "../extern/object_progress.hpp"
#else
#  include <object_progress/object_progress.hpp>
#  include <object_progress/perfomance.hpp>
#  include <pair_comparator.h>
#  include <binary_find.h>
#endif

#include "wsplayer.h"


//#define PRINT_PREDICT_STEPS

namespace Gomoku { namespace WsPlayer
{

unsigned nodes_count=0;
unsigned stored_deep=2;
unsigned def_lookup_deep=0;
unsigned treat_deep=14;



void increment_duplicate(npoints_t& dst,const npoints_t& src)
{
	npoints_t::iterator i=dst.begin(),endi=dst.end();
	npoints_t::const_iterator j=src.begin(),endj=src.end();

	less_point_pr pr;

	for(;i!=endi;)
	{
		j=std::lower_bound(j,endj,*i,pr);
		if(j==endj)break;
		if(i->is_same_point(*j))
		{
			i->n+=j->n;
			++i;
		}
		else i=std::lower_bound(++i,endi,*j,pr);
	}
}

void make_unique(npoints_t& pts)
{
	if(pts.empty())return;
	std::sort(pts.begin(),pts.end(),less_point_pr());

	npoints_t::iterator i=pts.begin(),endi=pts.end();
	npoints_t::iterator j=i;

	for(++i;i!=endi;++i)
	if(j->is_same_point(*i))j->n+=i->n;
	else
	{
		++j;
		*j=*i;
	}

	++j;
	pts.erase(j,endi);
}

void make_unique(points_t& pts)
{
	std::sort(pts.begin(),pts.end(),less_point_pr());
	pts.erase(
		std::unique(pts.begin(),pts.end()),
		pts.end());
}

void erase_from_sorted_points(points_t& pts,const point& p)
{
	points_t::iterator it=binary_find(pts.begin(),pts.end(),p,less_point_pr());
	if(it!=pts.end())pts.erase(it);
}


void sort_maxn_and_near_zero(npoints_t& pts)
{
	maxn_near_point_pr pr(point(0,0));
	std::sort(pts.begin(),pts.end(),pr);
}

void set_open_moves(const atacks_t& src,npoints_t& res,unsigned open_count)
{
	res.resize(0);
	res.reserve(src.size()*open_count);
	for(unsigned i=0;i<src.size();i++)
	{
		const atack_t& a=src[i];
		for(unsigned j=0;j<open_count;j++)
			res.push_back(a.open[j]);
	}
	make_unique(res);
}

void set_attack_moves(const atacks_t& src,npoints_t& res)
{
	res.resize(0);
	res.reserve(src.size());
	for(unsigned i=0;i<src.size();i++)
		res.push_back(src[i].move);
	make_unique(res);
}

temporary_state::temporary_state(wsplayer_t& _player,const step_t& st) : 
	player(_player),temporary_step(_player.field,st,st.step)
{
	player.increase_state();
}

temporary_state::~temporary_state()
{
	player.decrease_state();
}

void state_t::init_zero()
{
	player.field.get_empty_around(empty_points,2);
	find_move_to_make_five(st_krestik,player.field,make_five_krestik);
	find_move_to_make_five(st_nolik,player.field,make_five_nolik);
}

void state_t::state_from(const state_t& prev_state)
{
	state_from_empty_points(prev_state);
	state_from_make_five(prev_state);
}

void state_t::state_from_empty_points(const state_t& prev_state)
{
	step_t st=player.field.back();

	player.field.get_empty_around(st,tmp_points,2);

	empty_points.resize(prev_state.empty_points.size()+tmp_points.size());
	empty_points.erase(std::set_union(
		  prev_state.empty_points.begin(),prev_state.empty_points.end(),
		  tmp_points.begin(),tmp_points.end(),
		  empty_points.begin(),less_point_pr()),empty_points.end());

	points_t::iterator it=binary_find(empty_points.begin(),empty_points.end(),st,less_point_pr());
	if(it!=empty_points.end())empty_points.erase(it);
}

void state_t::state_from_make_five(const state_t& prev_state)
{
	step_t st=player.field.back();

	make_five_krestik=prev_state.make_five_krestik;
	make_five_nolik=prev_state.make_five_nolik;

	erase_from_sorted_points(make_five_krestik,st);
	erase_from_sorted_points(make_five_nolik,st);

	points_t& pts=get_make_five(st.step);
	add_move_to_make_five(st,player.field,pts);
	make_unique(pts);
}

////////////////////////////////////////////////////////////////////////////
void find_move_to_make_five(Step cl,const field_t& field,points_t& results)
{
	results.resize(0);

	for(unsigned i=0;i<field.size();i++)
	{
		const step_t& st=field[i];
		if(st.step!=cl)continue;

		add_move_to_make_five(st,field,results);
	}

	make_unique(results);
}

void add_move_to_make_five(const step_t& st,const field_t& field,points_t& results)
{
	check_five_line(st,field,1,0,results);
	check_five_line(st,field,0,1,results);
	check_five_line(st,field,1,1,results);
	check_five_line(st,field,1,-1,results);
}


void check_five_line(const step_t& st,const field_t& field,int dx,int dy,points_t& results)
{
	Gomoku::point p=st;
	unsigned j=0;

	for(p.x-=dx,p.y-=dy;j<3;j++,p.x-=dx,p.y-=dy)
	{
		Step s=field.at(p);
		if(s!=st.step)break;
	}

	bool left_empty=field.at(p)==st_empty;
	point left_point=p;
	unsigned left_scaned=j;

	p=st;

	for(p.x+=dx,p.y+=dy;j<3;j++,p.x+=dx,p.y+=dy)
	{
		Step s=field.at(p);
		if(s!=st.step)break;
	}

	bool right_empty=field.at(p)==st_empty;
	point right_point=p;
	unsigned right_scaned=j-left_scaned;

	if(j==3)
	{
		if(left_empty)results.push_back(left_point);
		if(right_empty)results.push_back(right_point);
		return;
	}

	if(left_empty)
	{
		j=right_scaned+left_scaned;
		p=st;
		for(p.x-=dx*(left_scaned+2),p.y-=dy*(left_scaned+2);j<3;j++,p.x-=dx,p.y-=dy)
		{
			Step s=field.at(p);
			if(s!=st.step)break;
		}

		if(j==3)results.push_back(left_point);
	}

	if(right_empty)
	{
		j=right_scaned+left_scaned;
		p=st;
		for(p.x+=dx*(right_scaned+2),p.y+=dy*(right_scaned+2);j<3;j++,p.x+=dx,p.y+=dy)
		{
			Step s=field.at(p);
			if(s!=st.step)break;
		}

		if(j==3)results.push_back(right_point);
	}
}


void find_move_to_open_four(const points_t& pts,Step cl,const field_t& field,atacks_t& results)
{
	results.resize(0);

	step_t st;
	st.step=cl;
	atack_t result;

	for(unsigned i=0;i<pts.size();i++)
	{
		static_cast<point&>(st)=pts[i];
		result.move=st;

		if(check_open_four_line(st,field,1,0,result.open[0],result.open[1]))
			results.push_back(result);

		if(check_open_four_line(st,field,0,1,result.open[0],result.open[1]))
			results.push_back(result);

		if(check_open_four_line(st,field,1,1,result.open[0],result.open[1]))
			results.push_back(result);

		if(check_open_four_line(st,field,1,-1,result.open[0],result.open[1]))
			results.push_back(result);
	}
}

bool check_open_four_line(const step_t& st,const field_t& field,int dx,int dy,point& left,point& right)
{
	Gomoku::point p=st;
	unsigned j=0;

	for(;j<4;j++)
	{
		p.x-=dx;
		p.y-=dy;

		Step s=field.at(p);

		if(s==st.step)continue;
		if(s!=st_empty)return false;

		left=p;
		break;
	}

	if(j==4)return false;

	p=st;

	for(;j<4;j++)
	{
		p.x+=dx;
		p.y+=dy;

		Step s=field.at(p);

		if(s==st.step)continue;
		if(s!=st_empty)return false;

		break;
	}

	if(j!=3)return false;
	right=p;
	return true;
}

void find_move_to_close_four(const points_t& pts,Step cl,const field_t& field,atacks_t& results)
{
	results.resize(0);

	step_t st;
	st.step=cl;
	atack_t result;

	for(unsigned i=0;i<pts.size();i++)
	{
		static_cast<point&>(st)=pts[i];
		result.move=st;

		if(check_close_four_line(st,field,1,0,result.open[0]))
			results.push_back(result);

		if(check_close_four_line(st,field,0,1,result.open[0]))
			results.push_back(result);

		if(check_close_four_line(st,field,1,1,result.open[0]))
			results.push_back(result);

		if(check_close_four_line(st,field,1,-1,result.open[0]))
			results.push_back(result);
	}
}

bool check_close_four_line(const step_t& st,const field_t& field,int dx,int dy,point& empty)
{
	Gomoku::point p=st;
	unsigned j=0;
	bool find=false;

	for(;j<4;j++)
	{
		p.x-=dx;
		p.y-=dy;

		Step s=field.at(p);

		if(s==st.step)continue;
		if(s!=st_empty)break;

		if(find)break;

		empty=p;
		find=true;
	}

	p=st;

	for(;j<4;j++)
	{
		p.x+=dx;
		p.y+=dy;

		Step s=field.at(p);

		if(s==st.step)continue;
		if(s!=st_empty)break;

		if(find)break;

		empty=p;
		find=true;
	}

	return find&&j==4;
}

void find_move_to_open_three(const points_t& pts,Step cl,const field_t& field,atacks_t& results)
{
	results.resize(0);

	step_t st;
	st.step=cl;
	atack_t result;

	for(unsigned i=0;i<pts.size();i++)
	{
		static_cast<point&>(st)=pts[i];
		result.move=st;

		if(check_open_three_line(st,field,1,0,result.open))
			results.push_back(result);

		if(check_open_three_line(st,field,0,1,result.open))
			results.push_back(result);

		if(check_open_three_line(st,field,1,1,result.open))
			results.push_back(result);

		if(check_open_three_line(st,field,1,-1,result.open))
			results.push_back(result);
	}
}

bool check_open_three_line(const step_t& st,const field_t& field,int dx,int dy,point* open)
{
	Gomoku::point p=st;

	unsigned open_count=0;
	unsigned fill_count=0;

	for(p.x-=dx,p.y-=dy;fill_count<2;p.x-=dx,p.y-=dy)
	{
		if(field.at(p)!=st.step)break;
		++fill_count;
	}

	for(;open_count<2;p.x-=dx,p.y-=dy)
	{
		if(field.at(p)!=st_empty)break;
		open[open_count++]=p;
	}

	if(open_count==0)return false;

	p=st;

	for(p.x+=dx,p.y+=dy;fill_count<2;p.x+=dx,p.y+=dy)
	{
		if(field.at(p)!=st.step)break;
		++fill_count;
	}

	if(fill_count!=2)return false;

	for(;open_count<3;p.x+=dx,p.y+=dy)
	{
		if(field.at(p)!=st_empty)break;
		open[open_count++]=p;
	}

	return open_count==3;
}

} }//namespace Gomoku

