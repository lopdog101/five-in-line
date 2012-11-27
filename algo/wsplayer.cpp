#include "wsplayer.h"
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

//#define PRINT_PREDICT_STEPS

namespace Gomoku
{


unsigned wsplayer_t::nodes_count=0;
unsigned wsplayer_t::stored_deep=2;
unsigned wsplayer_t::def_lookup_deep=0;
unsigned wsplayer_t::treat_deep=20;

wsplayer_t::wsplayer_t()
{
	predict_deep=0;
	predict_processed=0;
}

void wsplayer_t::begin_game()
{
	iplayer_t::begin_game();
	root=item_ptr();
}

void wsplayer_t::delegate_step()
{
	field=gm->field();
	root=item_ptr(new item_t(*this,field.back()));

	ObjectProgress::log_generator lg(true);

	init_states();

	try
	{
		root->process_deep_common();
	}
	catch(e_cancel&)
	{
		lg<<"canceled";
	}

	if(root->win)lg<<"wsplayer_t::delegate_step(): find win chain_depth="<<root->win->get_chain_depth()<<": "<<print_chain(root->win);
	if(root->fail!=0&&root->neitrals.empty())lg<<"wsplayer_t::delegate_step(): find fail chain_depth="<<root->fail->get_chain_depth()
		<<": "<<print_chain(root->fail);

	point p=*root->get_next_step();
	gm->make_step(*this,p);
}

void wsplayer_t::solve()
{
	field=gm->field();
	wide_item_t* wr=new wide_item_t(*this,field.back());
	root=item_ptr(wr);

	init_states();
	wr->process_deep_common();
}


wsplayer_t::Result wsplayer_t::item_t::process_deep_common()
{
	ObjectProgress::log_generator lg(true);
	ObjectProgress::perfomance perf;

	player.predict_processed=0;

	Result r=process_deep_stored();

	point pt=*get_next_step();

	lg<<"process_deep_common():"
		<<" step #"<<(player.field.size()+1)<<"=("<<pt.x<<","<<pt.y<<")"
		<<" neitrals="<<neitrals.size()
		<<" processed="<<player.predict_processed
		<<" perf="<<perf;
	
	return r;
}

wsplayer_t::Result wsplayer_t::item_t::process_deep_stored()
{
	for(unsigned i=0;i<stored_deep;i++)
	{
		Result r=process(i+1<stored_deep||stored_deep==1,(i+1<stored_deep? 0:def_lookup_deep));
		if(r!=r_neitral)return r;
		if(neitrals.size()==1)return r;
	}
	return r_neitral;
}

void wsplayer_t::item_t::clear()
{
	win=item_ptr();
	neitrals.clear();
	fail=item_ptr();
}


wsplayer_t::item_ptr wsplayer_t::item_t::get_next_step() const
{
	if(win)return win;
	if(!neitrals.empty())return neitrals.front();
	if(!fail)throw std::runtime_error("wsplayer_t::item_t::get_next_step(): invalid state");
	return fail;
}

wsplayer_t::Result wsplayer_t::item_t::process(bool need_fill_neitrals,unsigned lookup_deep)
{
	if(win)return r_fail;
	if(neitrals.empty())
	{
		if(fail)return r_win;
		Result r=process_predictable_move(false,0);
		if(r!=r_neitral)return r;

		if(neitrals.empty())
		{
			r=process_treat_sequence(need_fill_neitrals,lookup_deep);
			if(r!=r_neitral)return r;
		}

		neitrals.clear();
		return process_predictable_move(need_fill_neitrals,lookup_deep);
	}

	return process_neitrals(need_fill_neitrals,lookup_deep);
}

wsplayer_t::Result wsplayer_t::item_t::process_predictable_move(bool need_fill_neitrals,unsigned lookup_deep)
{
	unsigned& recursive_deep=player.predict_deep;
	inc_t r(recursive_deep);
	++player.predict_processed;

#ifdef PRINT_PREDICT_STEPS
	ObjectProgress::log_generator lg(true);
	lg<<"process_predictable_move()1: recursive_deep="<<recursive_deep
		<<" processed="<<player.predict_processed
		<<" nodes_count="<<nodes_count;
#endif

	const state_t& state=player.get_state();

	const points_t& a5_pts=state.get_make_five(other_step(step));
	if(!a5_pts.empty())
	{
		win=item_ptr(new item_t(player,a5_pts.front(),other_step(step) ));
		return r_fail;
	}

	const points_t& d5_pts=state.get_make_five(step);
	if(!d5_pts.empty())
	{
		if(d5_pts.size()>1)
		{
			fail=item_ptr(new item_t(player,d5_pts.front(),step ));
			fail->win=item_ptr(new item_t(player,d5_pts[1],other_step(step) ));
			return r_win;
		}

		neitrals.push_back(item_ptr(new item_t(player,d5_pts.front(),other_step(step))) );

		if(lookup_deep==0)return r_neitral;

#ifdef PRINT_PREDICT_STEPS
		lg<<"five defence: recursive_deep="<<recursive_deep
			<<" d5_pts="<<print_points(d5_pts)
			<<"\r\n"<<print_field(player.field.get_steps());
#endif

		return process_neitrals(need_fill_neitrals,lookup_deep-1);
	}

	points_t empty_points=state.empty_points;

	atacks_t open_four;

	player.find_move_to_open_four(empty_points,other_step(step),open_four);
	if(!open_four.empty())
	{
		win=item_ptr(new item_t(player,open_four.front().move,other_step(step) ));
		win->fail=item_ptr(new item_t(player,open_four.front().open[0],step ));
		win->fail->win=item_ptr(new item_t(player,open_four.front().open[1],other_step(step) ));
		return r_fail;
	}

	player.find_move_to_open_four(empty_points,step,open_four);

	atacks_t close_four;
	player.find_move_to_close_four(empty_points,other_step(step),close_four);
	npoints_t ac4_pts;
	set_attack_moves(close_four,ac4_pts);
	npoints_t ac4_open_pts;
	set_open_moves(close_four,ac4_open_pts,1);

	atacks_t open_three;
	player.find_move_to_open_three(empty_points,other_step(step),open_three);
	npoints_t ao3_pts;
	set_attack_moves(open_three,ao3_pts);
	npoints_t ao3_open_pts;
	set_open_moves(open_three,ao3_open_pts,3);

	if(!open_four.empty())
	{
		npoints_t do4_pts;
		set_attack_moves(open_four,do4_pts);

		increment_duplicate(do4_pts,ac4_pts);
		exclude_exists(do4_pts,ac4_pts);

		npoints_t do4_open_pts;
		set_open_moves(open_four,do4_open_pts,2);
		increment_duplicate(do4_pts,do4_open_pts);
		increment_duplicate(ac4_pts,do4_open_pts);
		exclude_exists(do4_pts,do4_open_pts);
		exclude_exists(ac4_pts,do4_open_pts);

		increment_duplicate(do4_pts,ac4_open_pts);
		increment_duplicate(ac4_pts,ac4_open_pts);
		increment_duplicate(do4_open_pts,ac4_open_pts);
		exclude_exists(do4_pts,ac4_open_pts);
		exclude_exists(ac4_pts,ac4_open_pts);
		exclude_exists(do4_open_pts,ac4_open_pts);

		increment_duplicate(do4_pts,ao3_pts);
		increment_duplicate(ac4_pts,ao3_pts);
		increment_duplicate(do4_open_pts,ao3_pts);

		sort_maxn_and_near_zero(do4_pts);
		sort_maxn_and_near_zero(ac4_pts);
		sort_maxn_and_near_zero(do4_open_pts);
		sort_maxn_and_near_zero(ac4_open_pts);

		add(do4_pts,ac4_pts);
		add(do4_pts,do4_open_pts);
		add(do4_pts,ac4_open_pts);


		add_neitrals(do4_pts);

		if(lookup_deep==0)return r_neitral;

#ifdef PRINT_PREDICT_STEPS
		lg<<"open_four defence: do4_pts.size()="<<do4_pts.size()<<" recursive_deep="<<recursive_deep
			<<" do4_pts="<<print_points(do4_pts)
			<<"\r\n"<<print_field(player.field.get_steps());
#endif

		return process_neitrals(false,lookup_deep-1);
	}

	if(!need_fill_neitrals&&lookup_deep==0)return r_neitral;

	if(!ac4_pts.empty())
	{
		increment_duplicate(ac4_pts,ao3_pts);
		increment_duplicate(ac4_pts,ao3_open_pts);

		npoints_t pts(ac4_pts);
		sort_maxn_and_near_zero(pts);
		add_neitrals(pts);

		if(lookup_deep>0)
		{
#ifdef PRINT_PREDICT_STEPS
			lg<<"close_four attack: ac4_pts.size()="<<ac4_pts.size()<<" recursive_deep="<<recursive_deep
				<<" pts="<<print_points(pts)
				<<"\r\n"<<print_field(player.field.get_steps());
#endif

			Result r=process_neitrals(false,lookup_deep-1);
			if(r==r_fail)
				return r;
			drop_neitrals_and_fail_child(2);
		}
	}

	exclude_exists(ac4_pts,ao3_pts);
	exclude_exists(ac4_pts,ao3_open_pts);

	if(!ao3_pts.empty())
	{
		increment_duplicate(ao3_pts,ao3_open_pts);

		npoints_t pts(ao3_pts);
		sort_maxn_and_near_zero(pts);
		add_neitrals(pts);

		if(lookup_deep>0)
		{
#ifdef PRINT_PREDICT_STEPS
			lg<<"open three attack: ao3_pts.size()="<<ao3_pts.size()<<" recursive_deep="<<recursive_deep
				<<" pts="<<print_points(pts)
				<<"\r\n"<<print_field(player.field.get_steps());
#endif

			Result r=process_neitrals(false,lookup_deep-1,ac4_pts.size());
			if(r==r_fail)
				return r;
			drop_neitrals_and_fail_child(1);
		}
	}

	exclude_exists(ao3_pts,ao3_open_pts);

	if(!ao3_open_pts.empty())
	{
		npoints_t pts(ao3_open_pts);
		sort_maxn_and_near_zero(pts);
		add_neitrals(pts);

		if(lookup_deep>0)
		{
#ifdef PRINT_PREDICT_STEPS
			lg<<"hole three attack: ao3_open_pts.size()="<<ao3_open_pts.size()<<" recursive_deep="<<recursive_deep
				<<" pts="<<print_points(pts)
				<<"\r\n"<<print_field(player.field.get_steps());
#endif

			Result r=process_neitrals(false,lookup_deep-1,ac4_pts.size()+ao3_pts.size());
			if(r==r_fail)
				return r;
			drop_neitrals_and_fail_child(1);
		}
	}

	if(!need_fill_neitrals)
	{
		//Если есть fail весь узел будет считаться фэйлом, хотя это ещё неизвестно
		fail.reset();
		return r_neitral;
	}

	exclude_exists(ac4_pts,empty_points);
	exclude_exists(ao3_pts,empty_points);
	exclude_exists(ao3_open_pts,empty_points);

	player.find_move_to_open_three(empty_points,step,open_three);
	npoints_t do3_pts;
	set_attack_moves(open_three,do3_pts);
	npoints_t do3_open_pts;
	set_open_moves(open_three,do3_open_pts,3);

	increment_duplicate(do3_pts,do3_open_pts);
	exclude_exists(do3_pts,do3_open_pts);

	exclude_exists(do3_pts,empty_points);
	exclude_exists(do3_open_pts,empty_points);

	sort_maxn_and_near_zero(do3_pts);
	sort_maxn_and_near_zero(do3_open_pts);
	std::sort(empty_points.begin(),empty_points.end(),near_point_pr(point(0,0)));

	add(do3_pts,do3_open_pts);
	add(do3_pts,empty_points);

	add_neitrals(do3_pts);
	return r_neitral;
}

wsplayer_t::Result wsplayer_t::item_t::process_neitrals(bool need_fill_neitrals,unsigned lookup_deep,unsigned from)
{
	field_t& field=player.field;

	unsigned max_depth=0;
	if(fail)max_depth=fail->get_chain_depth();

	for(unsigned i=0;i<neitrals.size();i++)
	{
		player.check_cancel();
		item_ptr& pch=neitrals[i];
		item_t& ch=*pch;

		temporary_state ts(player,ch);
		Result r=ch.process(need_fill_neitrals,lookup_deep);
		if(r==r_neitral)continue;
		if(r==r_win)
		{
			if(!win||pch->get_chain_depth()<win->get_chain_depth())
				win=pch;
			if(!lookup_deep)break;
			--lookup_deep;
			continue;
		}

		unsigned fail_depth=pch->get_chain_depth();
        if(fail_depth>max_depth)
		{
			fail=pch;
			max_depth=fail_depth;
		}
		pch.reset();
	}

	if(win)
	{
		neitrals.clear();
		fail.reset();
		return r_fail;
	}

	neitrals.erase(std::remove(neitrals.begin(),neitrals.end(),item_ptr()),neitrals.end());

	if(neitrals.empty())return r_win;
	return r_neitral;
}

void wsplayer_t::item_t::drop_neitrals_and_fail_child(unsigned generation)
{
	if(generation==0)
	{
		neitrals.resize(0);
		fail.reset();
		return;
	}

	for(unsigned i=0;i<neitrals.size();i++)
		neitrals[i]->drop_neitrals_and_fail_child(generation-1);
}

wsplayer_t::Result wsplayer_t::item_t::process_treat_sequence(bool need_fill_neitrals,unsigned lookup_deep)
{
#ifdef PRINT_PREDICT_STEPS
	ObjectProgress::log_generator lg(true);
#endif

    for(unsigned cur_deep=4;cur_deep<treat_deep;cur_deep+=4)
	{
#ifdef PRINT_PREDICT_STEPS
		ObjectProgress::perfomance perf;
#endif

		treat_node_ptr tr(new treat_node_t(player));
		tr->build_tree(other_step(step),true,treat_filter_t(),cur_deep);

		unsigned deep=tr->max_deep();

		if(!tr->win)
		{
			if(deep<cur_deep)return r_neitral;
			continue;
		}

#ifdef PRINT_PREDICT_STEPS
		lg<<"process_treat_sequence()1 build_tree(): deep="<<deep<<" cur_deep="<<cur_deep<<" win="<<tr->win<<" time="<<perf;
		perf.reset();
#endif

		item_ptr r=tr->check_tree(other_step(step));

#ifdef PRINT_PREDICT_STEPS
		lg<<"process_treat_sequence()2 check_tree(): deep="<<deep<<" win="<<tr->win<<" childs.size()="<<tr->childs.size()<<" time="<<perf;
		perf.reset();

		if(r)
		{
			lg<<"process_treat_sequence()3 check_tree(): chain_depth="<<r->get_chain_depth()<<": "<<print_chain(r);
			lg<<"process_treat_sequence()3.1 field: "<<print_steps(player.field.get_steps());
		}
#endif

		if(!r)
		{
			if(deep<cur_deep)return r_neitral;
			continue;
		}
		
		win=r;
		return r_fail;
	}
	
#ifdef PRINT_PREDICT_STEPS
	lg<<"process_treat_sequence()4: max deep reached";
#endif
	return r_neitral;

}

////////////////////////////////////////////////////////////////////////////
void wsplayer_t::find_move_to_make_five(Step cl,points_t& results) const
{
	results.resize(0);

	for(unsigned i=0;i<field.size();i++)
	{
		const step_t& st=field[i];
		if(st.step!=cl)continue;

		add_move_to_make_five(st,results);
	}

	make_unique(results);
}

void wsplayer_t::add_move_to_make_five(const step_t& st,points_t& results) const
{
	check_five_line(st,field,1,0,results);
	check_five_line(st,field,0,1,results);
	check_five_line(st,field,1,1,results);
	check_five_line(st,field,1,-1,results);
}


void wsplayer_t::check_five_line(const step_t& st,const field_t& field,int dx,int dy,points_t& results)
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


void wsplayer_t::find_move_to_open_four(const points_t& pts,Step cl,atacks_t& results) const
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

bool wsplayer_t::check_open_four_line(const step_t& st,const field_t& field,int dx,int dy,point& left,point& right)
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

void wsplayer_t::find_move_to_close_four(const points_t& pts,Step cl,atacks_t& results) const
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

bool wsplayer_t::check_close_four_line(const step_t& st,const field_t& field,int dx,int dy,point& empty)
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

void wsplayer_t::find_move_to_open_three(const points_t& pts,Step cl,atacks_t& results) const
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

bool wsplayer_t::check_open_three_line(const step_t& st,const field_t& field,int dx,int dy,point* open)
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

/////////////////////////////////////////////////////////////////////////////////////
void wsplayer_t::treat_node_t::build_tree(Step cl,bool only_win,const treat_filter_t& tf,unsigned deep)
{
	points_t empty_points;
	player.field.get_empty_around(empty_points,2);

	treats_t treats;
	player.find_treats(empty_points,treats,cl,tf.get_steps_to_win());

	for(unsigned i=0;i<treats.size();i++)
	{
		player.check_cancel();

		const treat_t& t=treats[i];
		if(!tf(t))continue;

		if(rest_count==0||t.is_one_of_rest(gain))
		{
			treat_node_ptr ch(new treat_node_t(player));
			static_cast<treat_t&>(*ch)=t;
			if(!ch->win&&deep>0)
			{
				temporary_treat_state hld_tr(player.field,t,cl);
				ch->build_tree(cl,only_win,tf,deep-1);
			}

			if(!only_win||ch->win)childs.push_back(ch);
			if(ch->win)win=true;
		}

		if(deep>1&&rest_count!=0&&!t.win&&!t.is_one_of_rest(gain)&&
			is_on_same_line_and_can_make_five(player.field,gain,t.gain,cl))
		{
			build_tree_same_line(t,cl,only_win,tf,deep-1);
		}
	}
}

void wsplayer_t::treat_node_t::build_tree_same_line(const treat_t& b,Step cl,bool only_win,const treat_filter_t& tf,unsigned deep)
{
	temporary_treat_state hld_b(player.field,b,cl);

	points_t empty_points;
	player.field.get_empty_around(empty_points,2);

	treats_t treats;
	player.find_treats(empty_points,treats,cl,tf.get_steps_to_win());

	for(unsigned i=0;i<treats.size();i++)
	{
		player.check_cancel();

		const treat_t& t=treats[i];
		if(!tf(t))continue;

		if(!t.is_one_of_rest(gain)||!t.is_one_of_rest(b.gain))
			continue;

		treat_node_ptr ch(new treat_node_t(player));
		static_cast<treat_t&>(*ch)=t;
		if(!ch->win)
		{
			temporary_treat_state hld_tr(player.field,t,cl);
			ch->build_tree(cl,only_win,tf,deep-1);
		}

		if(!only_win||ch->win)
		{
			treat_node_ptr chb(new treat_node_t(player));
			static_cast<treat_t&>(*chb)=b;
			chb->childs.push_back(ch);
			if(ch->win)
			{
				chb->win=true;
			}

			childs.push_back(chb);
		}

		if(ch->win)
		{
			win=true;
		}
	}
}


wsplayer_t::item_ptr wsplayer_t::treat_node_t::check_tree(Step cl)
{
	if(childs.empty())return item_ptr();

	group_by_step();
	sort_by_min_deep();

	for(unsigned i=0;i<childs.size();i++)
	{
		player.check_cancel();
		treat_node_t& gr=*childs[i];

		item_ptr r=check_tree_one_step(gr,cl);
		if(r)return r;
	}

	return item_ptr();
}

wsplayer_t::item_ptr  wsplayer_t::treat_node_t::check_tree_one_step(treat_node_t& gr,Step cl)
{
	field_t& field=player.field;
	temporary_step hld(field,gr.gain,cl);
	return icheck_tree_one_step(gr,cl);
}

wsplayer_t::item_ptr wsplayer_t::treat_node_t::icheck_tree_one_step(treat_node_t& gr,Step cl)
{
	field_t& field=player.field;

	item_ptr max_r;
	unsigned max_depth=0;

	for(unsigned k=0;k<gr.childs.size();k++)
	{
		treat_node_t& ch=*gr.childs[k];

		if(ch.get_steps_to_win()>1)
		{
			treat_node_t revert_treat(player);
			revert_treat.build_tree(other_step(cl),false,revert_treat_filter_t(ch));
			if(revert_treat.win)return item_ptr();
			ch.remove_revertable_path(revert_treat);
		}

		if(!ch.win)return item_ptr();

		if(ch.childs.empty())
		{
			item_ptr cr;
			
			if(ch.get_steps_to_win()==1)
			{
				max_r=item_ptr(new item_t(player,ch.gain,cl ));
				max_depth=1;
				break;//альтернативы нету уже победа
			}
			else
			{
				if(ch.get_steps_to_win()!=2||ch.cost_count!=2)
					throw std::runtime_error("check_tree() open four expected");
					
				cr=item_ptr(new item_t(player,ch.gain,cl ));
				item_ptr f(new item_t(player,ch.cost[0],other_step(cl) ));
				item_ptr w(new item_t(player,ch.cost[1],cl ));

				f->win=w;
				cr->fail=f;
			}

			unsigned depth=cr->get_chain_depth();
			if(!max_r||depth>max_depth)
			{
				max_r=cr;
				max_depth=depth;
			}
		}
		else
		{
			for(unsigned j=0;j<ch.cost_count;j++)
			{
				temporary_step hld_c(field,ch.cost[j],other_step(cl));
				treat_node_ptr cch=ch.clone();
				
				item_ptr cf=cch->check_tree(cl);

				if(!cf)return item_ptr();

				unsigned depth=cf->get_chain_depth()+2;
				if(!max_r||depth>max_depth)
				{
					item_ptr f(new item_t(player,ch.cost[j],other_step(cl) ));
					f->win=cf;
					max_r=item_ptr(new item_t(player,ch.gain,cl) );
					max_r->fail=f;
					
					max_depth=depth;
				}
			}
		}
	}
/*
	item_ptr ccf;
	if(contra_steps_exists(gr,cl,ccf))
	{
		if(!ccf)return item_ptr();
		unsigned depth=ccf->get_chain_depth();
		if(!max_r||depth>max_depth)
		{
			max_r=ccf;
			max_depth=depth;
		}
	}
*/
	return max_r;
}

bool wsplayer_t::treat_node_t::contra_steps_exists(treat_node_t& attack_group,Step cl,item_ptr& res)
{
	unsigned min_steps_to_win=attack_group.get_childs_min_steps_to_win();
	if(min_steps_to_win<=1)return false;

	treat_node_ptr ctree(new treat_node_t(player));
	ctree->build_tree(other_step(cl),false,step_treat_filter_t(min_steps_to_win-1),0);
	ctree->group_by_step();

	unsigned max_depth=0;

	for(unsigned i=0;i<ctree->childs.size();i++)
	{
		player.check_cancel();
		treat_node_t& gr=*ctree->childs[i];

		item_ptr cres;
		if(!contra_steps_exists_one_step(attack_group,cl,cres,gr))
			continue;

		if(!cres)
		{
			res.reset();
			return true;
		}

		unsigned depth=cres->get_chain_depth();

		if(!res||depth>max_depth)
		{
			res=cres;
			max_depth=depth;
		}
	}

	return res!=0;
}

bool wsplayer_t::treat_node_t::contra_steps_exists_one_step(treat_node_t& attack_group,Step cl,item_ptr& res,treat_node_t& gr)
{
	field_t& field=player.field;
	temporary_step hld(field,gr.gain,other_step(cl));

	item_ptr max_r;
	unsigned max_depth=0;

	for(unsigned k=0;k<gr.childs.size();k++)
	{
		treat_node_t& ch=*gr.childs[k];
		for(unsigned j=0;j<ch.cost_count;j++)
		{
			temporary_step hld_answer(field,ch.cost[j],cl);
			res=attack_group.icheck_tree_one_step(attack_group,cl);
			if(!res)continue;
			
			item_ptr c(new item_t(player,gr.gain,other_step(cl) ));
			item_ptr ca(new item_t(player,ch.cost[j],cl ));
			c->win=ca;
			ca->fail=res->fail;
			res->fail=c;
			return true;
		}
	}

	return false;
}




void wsplayer_t::treat_node_t::group_by_step()
{
	if(childs.empty())return;

	std::sort(childs.begin(),childs.end(),treat_step_pr());
	treat_nodes_t tmp_childs;
	std::swap(tmp_childs,childs);

	treat_node_ptr cr(new treat_node_t(player));
	cr->gain=tmp_childs.front()->gain;
	cr->childs.push_back(tmp_childs.front());

	for(unsigned i=1;i<tmp_childs.size();i++)
	{
		treat_node_ptr& p=tmp_childs[i];
		if(cr->gain!=p->gain)
		{
			childs.push_back(cr);
			cr=treat_node_ptr(new treat_node_t(player));
			cr->gain=p->gain;
		}
        
		cr->childs.push_back(p);
	}

	childs.push_back(cr);
}

void wsplayer_t::treat_node_t::sort_by_min_deep()
{
	std::sort(childs.begin(),childs.end(),treat_min_deep_pr());
}


wsplayer_t::treat_node_ptr wsplayer_t::treat_node_t::clone() const
{
	treat_node_ptr res(new treat_node_t(player));
	static_cast<treat_t&>(*res)=*this;
	res->childs.resize(childs.size());

	for(unsigned i=0;i<childs.size();i++)
		res->childs[i]=childs[i]->clone();

    return res;
}

unsigned wsplayer_t::treat_node_t::max_deep() const
{
	unsigned ret=0;
	for(unsigned i=0;i<childs.size();i++)
	{
		unsigned d=childs[i]->max_deep();
		if(d>ret)ret=d;
	}

	return ret+1;
}

unsigned wsplayer_t::treat_node_t::min_deep() const
{
	unsigned ret=0;
	for(unsigned i=0;i<childs.size();i++)
	{
		unsigned d=childs[i]->min_deep();
		if(i==0||d<ret)ret=d;
	}

	return ret+1;
}

unsigned wsplayer_t::treat_node_t::get_childs_min_steps_to_win() const
{
	unsigned ret=0;
	for(unsigned i=0;i<childs.size();i++)
	{
		unsigned d=childs[i]->get_steps_to_win();
		if(i==0||d<ret)ret=d;
	}

	return ret;
}



void wsplayer_t::treat_node_t::remove_revertable_path(const treat_node_t& revert_treat)
{
	if(revert_treat.is_tree_gain_conflict_with_rhs_gain_and_cost(*this))
	{
		childs.clear();
		win=false;
		return;
	}

	if(childs.empty())return;

	for(unsigned i=0;i<childs.size();i++)
	{
		treat_node_ptr& t=childs[i];
		t->remove_revertable_path(revert_treat);
		if(!t->win)t.reset();
	}

	childs.erase(std::remove(childs.begin(),childs.end(),treat_node_ptr()),childs.end());
	if(childs.empty())win=false;
}

bool wsplayer_t::treat_node_t::is_tree_conflict(const treat_t& tr) const
{
	for(unsigned i=0;i<childs.size();i++)
	{
		const treat_node_t& t=*childs[i];
        if(t.is_conflict(tr))
			return true;

		if(t.is_tree_conflict(tr))
			return true;
	}
	return false;
}

bool wsplayer_t::treat_node_t::is_tree_gain_conflict_with_rhs_gain_and_cost(const treat_t& tr) const
{
	for(unsigned i=0;i<childs.size();i++)
	{
		const treat_node_t& t=*childs[i];
        if(t.is_conflict(tr))
			return true;

		if(t.is_tree_conflict(tr))
			return true;
	}
	return false;
}

bool wsplayer_t::treat_t::is_conflict(const treat_t& b) const
{
	if(gain==b.gain)
		return true;

	if(is_one_of_cost(b.gain))
		return true;

	if(b.is_one_of_cost(gain))
		return true;

	for(unsigned j=0;j<cost_count;j++)
		if(b.is_one_of_cost(cost[j]))
			return true;

	return false;
}

bool wsplayer_t::treat_t::is_gain_conflict_with_rhs_gain_and_cost(const treat_t& rhs) const
{
	if(gain==rhs.gain)
		return true;

	if(rhs.is_one_of_cost(gain))
		return true;

	return false;
}

bool wsplayer_t::treat_t::is_one_of_rest(const point& p) const
{
	for(unsigned i=0;i<rest_count;i++)
		if(rest[i]==p)return true;
	return false;
}

bool wsplayer_t::treat_t::is_one_of_cost(const point& p) const
{
	for(unsigned i=0;i<cost_count;i++)
		if(cost[i]==p)return true;
	return false;
}


void wsplayer_t::find_treats(const points_t& empty_points,treats_t& res,Step cl,unsigned steps_to_win)
{
	find_treats(empty_points,cl,res,check_five_line);
	if(steps_to_win>=2)find_treats(empty_points,cl,res,check_open_four_line);
	if(steps_to_win>=2)find_two_way_treats(empty_points,cl,res,check_close_four_line);
	if(steps_to_win>=2)find_two_way_treats(empty_points,cl,res,check_four_line_hole_inside);
	if(steps_to_win>=3)find_treats(empty_points,cl,res,check_open_three_line_two_cost);
	if(steps_to_win>=3)find_two_way_treats(empty_points,cl,res,check_open_three_line_three_cost);
}


void wsplayer_t::find_treats(const points_t& pts,Step cl,treats_t& results,treat_f f) const
{
	step_t st;
	st.step=cl;
	treat_t tr;

	for(unsigned i=0;i<pts.size();i++)
	{
		static_cast<point&>(st)=pts[i];

		if(f(st,field,1,0,tr))
			results.push_back(tr);

		if(f(st,field,0,1,tr))
			results.push_back(tr);

		if(f(st,field,1,1,tr))
			results.push_back(tr);

		if(f(st,field,1,-1,tr))
			results.push_back(tr);
	}
}

void wsplayer_t::find_two_way_treats(const points_t& pts,Step cl,treats_t& results,treat_f f) const
{
	step_t st;
	st.step=cl;
	treat_t tr;

	for(unsigned i=0;i<pts.size();i++)
	{
		static_cast<point&>(st)=pts[i];

		if(f(st,field,1,0,tr))results.push_back(tr);
		if(f(st,field,-1,0,tr))results.push_back(tr);

		if(f(st,field,0,1,tr))results.push_back(tr);
		if(f(st,field,0,-1,tr))results.push_back(tr);

		if(f(st,field,1,1,tr))results.push_back(tr);
		if(f(st,field,-1,-1,tr))results.push_back(tr);

		if(f(st,field,1,-1,tr))results.push_back(tr);
		if(f(st,field,-1,1,tr))results.push_back(tr);
	}
}


bool wsplayer_t::check_five_line(const step_t& st,const field_t& field,int dx,int dy,treat_t& tr)
{
	tr=treat_t();
	tr.gain=st;
	tr.win=true;
	Gomoku::point p=st;
	unsigned j=0;

	for(;j<4;j++)
	{
		p.x-=dx;
		p.y-=dy;

		Step s=field.at(p);
		if(s!=st.step)break;
		tr.add_rest(p);
	}

	if(j==4)return true;

	p=st;

	for(;j<4;j++)
	{
		p.x+=dx;
		p.y+=dy;

		Step s=field.at(p);
		if(s!=st.step)break;
		tr.add_rest(p);
	}

	return j==4;
}

bool wsplayer_t::check_open_four_line(const step_t& st,const field_t& field,int dx,int dy,treat_t& tr)
{
	tr=treat_t();
	tr.gain=st;
	tr.win=true;
	Gomoku::point p=st;
	unsigned j=0;

	for(;j<4;j++)
	{
		p.x-=dx;
		p.y-=dy;

		Step s=field.at(p);

		if(s==st.step)
		{
			tr.add_rest(p);
			continue;
		}

		if(s!=st_empty)return false;

		tr.add_cost(p);
		break;
	}

	if(j==4)return false;

	p=st;

	for(;j<4;j++)
	{
		p.x+=dx;
		p.y+=dy;

		Step s=field.at(p);

		if(s==st.step)
		{
			tr.add_rest(p);
			continue;
		}

		if(s!=st_empty)return false;
		break;
	}

	if(j!=3)return false;
	tr.add_cost(p);
	return true;
}

bool wsplayer_t::check_close_four_line(const step_t& st,const field_t& field,int dx,int dy,treat_t& tr)
{
	tr=treat_t();
	tr.gain=st;
	Gomoku::point p=st;
	unsigned j=0;
	Step s;

    //Мы анализируем пять клеток влючая завершающий левый ноль.
	//Правый ноль находится повторным вызовом фнукции с симметричными dx,dy в find_two_way_treats()
	//Возможно даже одновременное наличие правой и левой закрытой четвёрки
	for(;j<4;j++)
	{
		p.x-=dx;
		p.y-=dy;
		s=field.at(p);

		if(s==st.step)
		{
			tr.add_rest(p);
			continue;
		}

		if(s!=st_empty)break;

		if(tr.cost_count!=0)return false;
		tr.add_cost(p);
	}

    if(j==4)
	{
		p.x-=dx;
		p.y-=dy;
		s=field.at(p);
	}

	//Должна быть левая клетка противоположного цвета
	if(s==st_empty||s==st.step)return false;

	p=st;
	for(;j<4;j++)
	{
		p.x+=dx;
		p.y+=dy;
		s=field.at(p);

		if(s==st.step)
		{
			tr.add_rest(p);
			continue;
		}

		if(s!=st_empty)return false;

		if(tr.cost_count!=0)return false;
		tr.add_cost(p);
	}

	if(tr.cost_count==0)return false;

	//Существует случай, когда закрытая четвёрка с пустой клеткой перед нулём может оказаться открытой чётвёркой
	treat_t ctr;
	return !check_open_four_line(st,field,dx,dy,ctr);
}

bool wsplayer_t::check_four_line_hole_inside(const step_t& st,const field_t& field,int dx,int dy,treat_t& tr)
{
	tr=treat_t();
	tr.gain=st;
	Gomoku::point p=st;
	unsigned j=0;
	Step s;

    //Ищем дырку слева
	for(;j<3;j++)
	{
		p.x-=dx;
		p.y-=dy;
		s=field.at(p);

		if(s==st.step)
		{
			tr.add_rest(p);
			continue;
		}

		if(s!=st_empty)return false;
		tr.add_cost(p);
		break;
	}

	if(tr.cost_count==0)
		return false;

	//След. за дыркой должен быть 0
	{
		j++;
		p.x-=dx;
		p.y-=dy;
		s=field.at(p);

		if(s!=st.step)return false;
		tr.add_rest(p);
	}

	if(j==4)
	{
		return true;
	}

	for(j++;j<4;j++)
	{
		p.x-=dx;
		p.y-=dy;
		s=field.at(p);

		if(s!=st.step)break;
		tr.add_rest(p);
	}

	if(j==4)
	{
		return true;
	}

	p=st;
	for(;j<4;j++)
	{
		p.x+=dx;
		p.y+=dy;
		s=field.at(p);

		if(s!=st.step)break;
		tr.add_rest(p);
	}

	if(j!=4)return false;
	
	return true;
}

bool wsplayer_t::check_open_three_line_two_cost(const step_t& st,const field_t& field,int dx,int dy,treat_t& tr)
{
	tr=treat_t();
	tr.gain=st;
	Gomoku::point p=st;
	Step s;
	unsigned j=0;

	for(;j<2;j++)
	{
		p.x-=dx;
		p.y-=dy;
		s=field.at(p);

		if(s!=st.step)break;
		tr.add_rest(p);
	}

    if(j==2)
	{
		p.x-=dx;
		p.y-=dy;
		s=field.at(p);
	}

	if(s!=st_empty)return false;
	tr.add_cost(p);

	p.x-=dx;
	p.y-=dy;
	s=field.at(p);
	if(s!=st_empty)return false;

	p=st;
	for(;j<2;j++)
	{
		p.x+=dx;
		p.y+=dy;
		s=field.at(p);

		if(s!=st.step)break;
		tr.add_rest(p);
	}

	if(j!=2)return false;

	p.x+=dx;
	p.y+=dy;
	s=field.at(p);

	if(s!=st_empty)return false;
	tr.add_cost(p);

	p.x+=dx;
	p.y+=dy;
	s=field.at(p);
	return s==st_empty;
}

bool wsplayer_t::check_open_three_line_three_cost(const step_t& st,const field_t& field,int dx,int dy,treat_t& tr)
{
	tr=treat_t();
	tr.gain=st;
	Gomoku::point p=st;
	Step s;
	unsigned j=0;

	for(;j<2;j++)
	{
		p.x-=dx;
		p.y-=dy;
		s=field.at(p);

		if(s!=st.step)break;
		tr.add_rest(p);
	}

    if(j==2)
	{
		p.x-=dx;
		p.y-=dy;
		s=field.at(p);
	}

	if(s!=st_empty)return false;
	tr.add_cost(p);

	p.x-=dx;
	p.y-=dy;
	s=field.at(p);
	//Должна быть левая клетка противоположного цвета
	if(s==st_empty||s==st.step)return false;

	p=st;
	for(;j<2;j++)
	{
		p.x+=dx;
		p.y+=dy;
		s=field.at(p);

		if(s!=st.step)break;
		tr.add_rest(p);
	}

	if(j!=2)return false;

	p.x+=dx;
	p.y+=dy;
	s=field.at(p);

	if(s!=st_empty)return false;
	tr.add_cost(p);

	p.x+=dx;
	p.y+=dy;
	s=field.at(p);
	return s==st_empty;
}

bool wsplayer_t::is_on_same_line(const point& a,const point& b)
{
	int dx=b.x-a.x;
	int dy=b.y-a.y;

	if(dx!=0&&std::abs(dx)>4)return false;
	if(dy!=0&&std::abs(dy)>4)return false;
	if(dx!=0&&dy!=0&&std::abs(dx)!=std::abs(dy))return false;

	return true;
}

void wsplayer_t::make_step_dx_dy(const point& a,const point& b,int& dx,int& dy)
{
	dx=b.x-a.x;
	dy=b.y-a.y;

	if(dx>0)dx=1;
	else if(dx<0)dx=-1;

	if(dy>0)dy=1;
	else if(dy<0)dy=-1;
}


bool wsplayer_t::is_on_same_line_and_can_make_five(const field_t& field,const point& a,const point& b,Step cl)
{
	if(!is_on_same_line(a,b))return false;

	int dx=0;
	int dy=0;

	make_step_dx_dy(a,b,dx,dy);

	Step s;
	unsigned j=0;
	Step ocl=other_step(cl);

	Gomoku::point p=a;
	for(;j<3;j++)
	{
		p.x+=dx;
		p.y+=dy;
		if(p==b)break;

		s=field.at(p);
		if(s==ocl)return false;
	}
	
	if(j==3)return true;

	p=a;
	for(;j<3;j++)
	{
		p.x-=dx;
		p.y-=dy;

		s=field.at(p);
		if(s==ocl)break;
	}

	if(j==3)return true;

	p=b;
	for(;j<3;j++)
	{
		p.x+=dx;
		p.y+=dy;

		s=field.at(p);
		if(s==ocl)break;
	}

	return j==3;
}


/////////////////////////////////////////////////////////////////////////////////////
std::string wsplayer_t::print_chain(item_ptr root)
{
	steps_t sts;
	while(root)
	{
		sts.push_back(*root);
		if(root->win)root=root->win;
		else root=root->fail;
	}
	return print_steps(sts);
}


template<class Points>
void wsplayer_t::item_t::add_neitrals(const Points& pts)
{
	step_t s;
	s.step=other_step(step);

	size_t exist_count=neitrals.size();

	neitrals.resize(exist_count+pts.size());

	typename Points::const_iterator i=pts.begin(),ei=pts.end();
	items_t::iterator j=neitrals.begin()+exist_count;
    
	for(;i!=ei;++i,++j)
	{
		static_cast<point&>(s)=*i;
		*j=item_ptr(new item_t(player,s));
	}
}

unsigned wsplayer_t::item_t::get_chain_depth() const
{
	if(win)return win->get_chain_depth()+1;
	if(fail)return fail->get_chain_depth()+1;
	return 1;
}

///////////////////////////////////////////////////////////////
void wsplayer_t::wide_item_t::process_deep_common()
{
	process_deep_stored();
}

void wsplayer_t::wide_item_t::process_deep_stored()
{
	for(unsigned i=0;i<stored_deep;i++)
	{
		process(i+1<stored_deep||stored_deep==1,(i+1<stored_deep? 0:def_lookup_deep));
		if(neitrals.empty())break;
	}
}

void wsplayer_t::wide_item_t::process(bool need_fill_neitrals,unsigned lookup_deep)
{
	if(neitrals.empty()&&wins.empty()&&fails.empty())
	{
		process_predictable_move(need_fill_neitrals,lookup_deep);
		if(win)
		{
			wins.push_back(win);
			win.reset();
		}

		if(fail)
		{
			fails.push_back(fail);
			fail.reset();
		}
		return;
	}

	field_t& field=player.field;

	for(unsigned i=0;i<neitrals.size();i++)
	{
		item_ptr& pch=neitrals[i];
		item_t& ch=*pch;

		temporary_state ts(player,ch);
		Result r=ch.process(need_fill_neitrals,lookup_deep);
		if(r==r_neitral)continue;
		if(r==r_win)wins.push_back(pch);
		else fails.push_back(pch);
		pch.reset();
	}

	neitrals.erase(std::remove(neitrals.begin(),neitrals.end(),item_ptr()),neitrals.end());
}

///////////////////////////////////////////////////////////////
void wsplayer_t::init_states()
{
	current_state=0;
	if(states.empty())states.push_back(state_ptr(new state_t(*this)));
	get_state().init_zero();
}

void wsplayer_t::state_t::init_zero()
{
	player.field.get_empty_around(empty_points,2);
	player.find_move_to_make_five(st_krestik,make_five_krestik);
	player.find_move_to_make_five(st_nolik,make_five_nolik);
}

void wsplayer_t::increase_state()
{
	++current_state;
	if(current_state+1>states.size())states.push_back(state_ptr(new state_t(*this)));
	states[current_state]->state_from(*states[current_state-1]);
}

void wsplayer_t::state_t::state_from(const state_t& prev_state)
{
	state_from_empty_points(prev_state);
	state_from_make_five(prev_state);
}

void wsplayer_t::state_t::state_from_empty_points(const state_t& prev_state)
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

void wsplayer_t::state_t::state_from_make_five(const state_t& prev_state)
{
	step_t st=player.field.back();

	make_five_krestik=prev_state.make_five_krestik;
	make_five_nolik=prev_state.make_five_nolik;

	erase_from_sorted_points(make_five_krestik,st);
	erase_from_sorted_points(make_five_nolik,st);

	points_t& pts=get_make_five(st.step);
	player.add_move_to_make_five(st,pts);
	make_unique(pts);
}

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

template<class Points>
void exclude_exists(const npoints_t& src,Points& dst)
{
	if(src.empty()||dst.empty())return;

	Points res(dst.size());
	res.erase(
	  std::set_difference(dst.begin(),dst.end(),src.begin(),src.end(),res.begin(),less_point_pr()),
	  res.end());
	std::swap(dst,res);
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

#ifdef USE_XML
void wsplayer_t::pack(Xpat::ipacker_t& root_node,bool process_type) const
{
	if(XML_SET_TYPE)return;
}

void wsplayer_t::unpack(const Xpat::ipacker_t& root_node,bool process_type)
{
	if(XML_CHECK_TYPE)return;
}
#endif
}//namespace Gomoku
