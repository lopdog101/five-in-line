#include "wsplayer_node.h"
#include <stdexcept>

#  include "../extern/pair_comparator.h"
#  include "../extern/binary_find.h"
#  include "../extern/object_progress.hpp"

#include "wsplayer.h"
#include "wsplayer_treat.h"

namespace Gomoku { namespace WsPlayer
{

Result item_t::process_deep_common()
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

Result item_t::process_deep_stored()
{
	for(unsigned i=0;i<stored_deep;i++)
	{
		Result r=process(i+1<stored_deep||stored_deep==1,(i+1<stored_deep? 0:def_lookup_deep));
		if(r!=r_neitral)return r;
		if(neitrals.size()==1)return r;
	}
	return r_neitral;
}

void item_t::clear()
{
	win=item_ptr();
	neitrals.clear();
	fail=item_ptr();
}


item_ptr item_t::get_next_step() const
{
	if(win)return win;
	if(!neitrals.empty())return neitrals.front();
	if(!fail)throw std::runtime_error("item_t::get_next_step(): invalid state");
	return fail;
}

Result item_t::process(bool need_fill_neitrals,unsigned lookup_deep)
{
	if(win)return r_fail;
	if(neitrals.empty())
	{
		if(fail)return r_win;
		Result r=process_predictable_move(false,0);
		if(r!=r_neitral)return r;

		neitrals.clear();

//		if(lookup_deep==0)
//		{
			r=process_treat_sequence();
			if(r!=r_neitral)return r;
//		}

		return process_predictable_move(need_fill_neitrals,lookup_deep);
	}

	return process_neitrals(need_fill_neitrals,lookup_deep);
}

Result item_t::process_predictable_move(bool need_fill_neitrals,unsigned lookup_deep)
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

	find_move_to_open_four(empty_points,other_step(step),player.field,open_four);
	if(!open_four.empty())
	{
		win=item_ptr(new item_t(player,open_four.front().move,other_step(step) ));
		win->fail=item_ptr(new item_t(player,open_four.front().open[0],step ));
		win->fail->win=item_ptr(new item_t(player,open_four.front().open[1],other_step(step) ));
		return r_fail;
	}

	find_move_to_open_four(empty_points,step,player.field,open_four);

	atacks_t close_four;
	find_move_to_close_four(empty_points,other_step(step),player.field,close_four);
	npoints_t ac4_pts;
	set_attack_moves(close_four,ac4_pts);
	npoints_t ac4_open_pts;
	set_open_moves(close_four,ac4_open_pts,1);

	atacks_t open_three;
	find_move_to_open_three(empty_points,other_step(step),player.field,open_three);
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
		//if fail exists all node would be mean as fail
		//But it is not known yet
		fail.reset();
		return r_neitral;
	}

	exclude_exists(ac4_pts,empty_points);
	exclude_exists(ao3_pts,empty_points);
	exclude_exists(ao3_open_pts,empty_points);

	find_move_to_open_three(empty_points,step,player.field,open_three);
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

Result item_t::process_neitrals(bool need_fill_neitrals,unsigned lookup_deep,unsigned from)
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

void item_t::drop_neitrals_and_fail_child(unsigned generation)
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

Result item_t::process_treat_sequence()
{
#ifdef PRINT_TREAT_PERFOMANCE
	ObjectProgress::log_generator lg(true);
#endif

    unsigned const start_deep=4;
    
    int last_treat_check_count=0;
    int last_treat_check_rebuild_tree_count=0;

    for(unsigned cur_deep=start_deep;cur_deep<treat_deep;cur_deep+=4)
	{
#ifdef PRINT_TREAT_PERFOMANCE
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

#ifdef PRINT_TREAT_PERFOMANCE
		lg<<"process_treat_sequence()1 build_tree(): "<<to_string(other_step(step))<<": deep="<<deep<<" cur_deep="<<cur_deep<<" win="<<tr->win<<" time="<<perf;
		perf.reset();
#endif
        item_ptr r;

        player.treat_check_count=0;
        player.treat_check_rebuild_tree_count=0;

        bool max_check_reached=true;
        
        try
        {
            r=tr->check_tree(other_step(step),false);

            max_check_reached=false;
        }
        catch(e_max_treat_check_rebuild_tree&)
        {
#ifdef PRINT_TREAT_PERFOMANCE
		lg<<"process_treat_sequence()1.1 check_tree(): "<<to_string(other_step(step))<<": e_max_treat_check_rebuild_tree";
#endif
        }
        catch(e_max_treat_check_reached&)
        {
#ifdef PRINT_TREAT_PERFOMANCE
		lg<<"process_treat_sequence()1.2 check_tree(): "<<to_string(other_step(step))<<": e_max_treat_check_reached";
#endif
        }

#ifdef PRINT_TREAT_PERFOMANCE
		lg<<"process_treat_sequence()2 check_tree(): "<<to_string(other_step(step))<<": deep="<<deep<<" win="<<tr->win<<" childs.size()="<<tr->childs.size()
            <<" treat_check_count="<<player.treat_check_count<<" treat_check_rebuild_tree_count="<<player.treat_check_rebuild_tree_count<<" time="<<perf;
		perf.reset();
#endif

        if(max_check_reached)
        {
            return r_neitral;
        }

#ifdef PRINT_TREAT_PERFOMANCE
		if(r)
		{
			lg<<"process_treat_sequence()3 check_tree(): "<<to_string(other_step(step))<<": chain_depth="<<r->get_chain_depth()<<": "<<print_chain(r);
			lg<<"process_treat_sequence()3.1 field: "<<print_steps(player.field.get_steps());
			
			lg<<"process_treat_sequence()3.2\r\n";
			lg<<print_treat_tree(*tr);

		}
#endif

        if(r)
        {
            win=r;
            return r_fail;
        }
        
        if(deep<cur_deep)return r_neitral;

        if(last_treat_check_count==player.treat_check_count && 
           last_treat_check_rebuild_tree_count==player.treat_check_rebuild_tree_count)
        {
            return r_neitral;
        }

        last_treat_check_count=player.treat_check_count;
        last_treat_check_rebuild_tree_count=player.treat_check_rebuild_tree_count;
	}
	
#ifdef PRINT_TREAT_PERFOMANCE
	lg<<"process_treat_sequence()4: "<<to_string(other_step(step))<<": max deep reached";
#endif
	return r_neitral;

}

unsigned item_t::get_chain_depth() const
{
	if(win)return win->get_chain_depth()+1;
	if(fail)return fail->get_chain_depth()+1;
	return 1;
}

///////////////////////////////////////////////////////////////
void wide_item_t::process_deep_common()
{
	process_deep_stored();
}

void wide_item_t::process_deep_stored()
{
	for(unsigned i=0;i<stored_deep;i++)
	{
		process(i+1<stored_deep||stored_deep==1,(i+1<stored_deep? 0:def_lookup_deep));
		if(neitrals.empty())break;
	}
}

void wide_item_t::process(bool need_fill_neitrals,unsigned lookup_deep)
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

template<class Points>
void item_t::add_neitrals(const Points& pts)
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

/////////////////////////////////////////////////////////////////////////////////////
std::string print_chain(item_ptr root)
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

} }//namespace Gomoku

