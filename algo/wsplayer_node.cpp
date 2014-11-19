#include "wsplayer_node.h"
#include <stdexcept>
#include <numeric>

#  include "../extern/pair_comparator.h"
#  include "../extern/binary_find.h"
#  include "../extern/object_progress.hpp"

#include "wsplayer.h"
#include "wsplayer_treat.h"

namespace Gomoku { namespace WsPlayer
{

item_t::item_t(wsplayer_t& _player,const step_t& s) : 
	step_t(s),
	player(_player)
{
	++nodes_count;
	deep_wins_count=0;
	deep_fails_count=0;
}

item_t::item_t(wsplayer_t& _player,const Gomoku::point& p,Step s) : 
	step_t(s,p.x,p.y),
	player(_player)
{
	++nodes_count;
	deep_wins_count=0;
	deep_fails_count=0;
}

Result item_t::process_deep_common()
{
	ObjectProgress::log_generator lg(true);
	ObjectProgress::perfomance perf;

	player.predict_processed=0;

	Result r=process_deep_stored();

	if(r==r_neitral && neitrals.size()>1)
		r=process_deep_ant();

	point pt=*get_next_step();

	lg<<"process_deep_common():"
		<<" step #"<<(player.field.size()+1)<<"=("<<pt.x<<","<<pt.y<<")"
		<<" neitrals="<<neitrals.size()
		<<" processed="<<player.predict_processed
		<<" perf="<<perf;
	
	return r;
}

Result item_t::process_deep_ant()
{
	calculate_deep_wins_fails();
	for(unsigned i=0;i<ant_count;i++)
	{
		player.check_cancel();
		Result r=solve_ant();
		if(r!=r_neitral)
			return r;
	}

	if(!neitrals.empty())
	{
		items_t::iterator it=std::min_element(neitrals.begin(),neitrals.end(),win_rate_cmp_pr());
		std::iter_swap(neitrals.begin(),it);
	}

	return r_neitral;
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
	neitrals.clear();
	wins.clear();
	fails.clear();
}

item_ptr item_t::get_next_step() const
{
	if(!wins.empty())return wins.get_best();
	if(!neitrals.empty())return neitrals.front();
	if(fails.empty())throw std::runtime_error("item_t::get_next_step(): invalid state");
	return fails.get_best();
}

item_ptr item_t::get_win_fail_step() const
{
	if(!wins.empty())return wins.get_best();
	if(!fails.empty())return fails.get_best();
	return item_ptr();
}


Result item_t::process(bool need_fill_neitrals,unsigned lookup_deep,const item_t* parent_node)
{
	if(!wins.empty())
		return r_fail;
	
	if(neitrals.empty())
	{
		if(!fails.empty())
			return r_win;
		
		return process_predict_treat_sequence(need_fill_neitrals,lookup_deep);
	}

	return process_neitrals(need_fill_neitrals,lookup_deep,0,parent_node);
}

Result item_t::process_predict_treat_sequence(bool need_fill_neitrals,unsigned lookup_deep)
{
	Result r=process_predictable_move(false,0);
	if(r!=r_neitral)return r;

	clear();

	r=process_treat_sequence();
	if(r!=r_neitral)return r;

	return process_predictable_move(need_fill_neitrals,lookup_deep);
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

	const points_t& a5_pts=state.get_make_five(other_color(step));
	if(!a5_pts.empty())
	{
		add_win(item_ptr(new item_t(player,a5_pts.front(),other_color(step) )) );
		return r_fail;
	}

	const points_t& d5_pts=state.get_make_five(step);
	if(!d5_pts.empty())
	{
		if(d5_pts.size()>1)
		{
			item_ptr fail=item_ptr(new item_t(player,d5_pts.front(),step ));
			fail->add_win(item_ptr(new item_t(player,d5_pts[1],other_color(step) )) );
			add_fail(fail);
			return r_win;
		}

		neitrals.push_back(create_neitral_item(d5_pts.front(),other_color(step)) );

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

	find_move_to_open_four(empty_points,other_color(step),player.field,open_four);
	if(!open_four.empty())
	{
		item_ptr win=item_ptr(new item_t(player,open_four.front().move,other_color(step) ));
		item_ptr win_fail(new item_t(player,open_four.front().open[0],step ));
		win_fail->add_win(item_ptr(new item_t(player,open_four.front().open[1],other_color(step) )) );
		win->add_fail(win_fail);
		add_win(win);
		return r_fail;
	}

	find_move_to_open_four(empty_points,step,player.field,open_four);

	atacks_t close_four;
	find_move_to_close_four(empty_points,other_color(step),player.field,close_four);
	npoints_t ac4_pts;
	set_attack_moves(close_four,ac4_pts);
	npoints_t ac4_open_pts;
	set_open_moves(close_four,ac4_open_pts,1);

	atacks_t open_three;
	find_move_to_open_three(empty_points,other_color(step),player.field,open_three);
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

#ifdef PRINT_PREDICT_STEPS
		if(lookup_deep>0)
			lg<<"close_four attack: ac4_pts.size()="<<ac4_pts.size()<<" recursive_deep="<<recursive_deep
				<<" pts="<<print_points(pts)
				<<"\r\n"<<print_field(player.field.get_steps());
#endif
		Result r=add_and_process_neitrals(pts,lookup_deep,2);
		if(r==r_fail)
			return r;
	}

	exclude_exists(ac4_pts,ao3_pts);
	exclude_exists(ac4_pts,ao3_open_pts);

	if(!ao3_pts.empty())
	{
		increment_duplicate(ao3_pts,ao3_open_pts);

		npoints_t pts(ao3_pts);
		sort_maxn_and_near_zero(pts);

#ifdef PRINT_PREDICT_STEPS
		if(lookup_deep>0)
			lg<<"open three attack: ao3_pts.size()="<<ao3_pts.size()<<" recursive_deep="<<recursive_deep
				<<" pts="<<print_points(pts)
				<<"\r\n"<<print_field(player.field.get_steps());
#endif
		Result r=add_and_process_neitrals(pts,lookup_deep,1);
		if(r==r_fail)
			return r;
	}

	exclude_exists(ao3_pts,ao3_open_pts);

	if(!ao3_open_pts.empty())
	{
		npoints_t pts(ao3_open_pts);
		sort_maxn_and_near_zero(pts);

#ifdef PRINT_PREDICT_STEPS
		if(lookup_deep>0)
			lg<<"hole three attack: ao3_open_pts.size()="<<ao3_open_pts.size()<<" recursive_deep="<<recursive_deep
				<<" pts="<<print_points(pts)
				<<"\r\n"<<print_field(player.field.get_steps());
#endif
		Result r=add_and_process_neitrals(pts,lookup_deep,1);
		if(r==r_fail)
			return r;
	}

	if(!need_fill_neitrals)
	{
		//if fail exists all node would be mean as fail
		//But it is not known yet
		neitrals.resize(0);
		fails.clear();
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

	//Open points could be other then do3_open_pts
    exclude_exists(ac4_pts,do3_open_pts);
	exclude_exists(ao3_pts,do3_open_pts);
	exclude_exists(ao3_open_pts,do3_open_pts);

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

Result item_t::process_neitrals(bool need_fill_neitrals,unsigned lookup_deep,unsigned from,const item_t* parent_node)
{
//	reorder_neitrals_as_neighbor_win_hint(from,parent_node);

	for(unsigned i=from;i<neitrals.size();i++)
	{
		player.check_cancel();
		item_ptr& pch=neitrals[i];
		item_t& ch=*pch;

		temporary_state ts(player,ch);
		Result r=ch.process(need_fill_neitrals,lookup_deep,this);

		if(r==r_win)
		{
			add_win(pch);
			pch.reset();
			if(!lookup_deep)break;
			--lookup_deep;
			continue;
		}
		else if(r==r_fail)
		{
			add_fail(pch);
			pch.reset();
		}
	}

	neitrals.erase(std::remove(neitrals.begin(),neitrals.end(),item_ptr()),neitrals.end());

	if(!wins.empty())
		return r_fail;

	if(neitrals.empty())return r_win;
	return r_neitral;
}

void item_t::reorder_neitrals_as_neighbor_win_hint(unsigned from,const item_t* parent_node)
{
	if(!parent_node)
		return;

	const npoints_t& hint=parent_node->get_fails().get_win_hins();
	
	if(hint.empty())
		return;

	existing_npoints_sort_pr pr(hint);
	std::stable_sort(neitrals.begin()+from,neitrals.end(),pr);
}


Result item_t::add_and_process_neitrals(const npoints_t& pts,unsigned lookup_deep,unsigned drop_generation)
{
	unsigned from=neitrals.size();
	add_neitrals(pts);

	if(lookup_deep==0)
		return r_neitral;

	Result r=process_neitrals(false,lookup_deep-1,from);

	if(r==r_fail)
		return r;

	drop_neitrals_and_fail_child(drop_generation);

	return r_neitral;
}

void item_t::drop_neitrals_and_fail_child(unsigned generation)
{
	if(generation!=0)
	{
		--generation;

		for(unsigned i=0;i<neitrals.size();i++)
			neitrals[i]->drop_neitrals_and_fail_child(generation);

		return;
	}

	//neitrals.empty() && !fails.empty() means all fails
	//but goal of this function just reset uncompleted branches to be in the same level of depth with others
	if(neitrals.empty())
		return;

	neitrals.resize(0);
	fails.clear();
	return;
}

Result item_t::process_treat_sequence()
{
    if(treat_deep==0)
        return r_neitral;

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
		tr->build_tree(other_color(step),true,treat_filter_t(),cur_deep);

		unsigned deep=tr->max_deep();

		if(!tr->win)
		{
			if(deep<cur_deep)return r_neitral;
			continue;
		}

#ifdef PRINT_TREAT_PERFOMANCE
		lg<<"process_treat_sequence()1 build_tree(): "<<to_string(other_color(step))<<": deep="<<deep<<" cur_deep="<<cur_deep<<" win="<<tr->win<<" time="<<perf;
		perf.reset();
#endif
        item_ptr r;

        player.treat_check_count=0;
        player.treat_check_rebuild_tree_count=0;

        bool max_check_reached=true;
        
        try
        {
            r=tr->check_tree(other_color(step),false);

            max_check_reached=false;
        }
        catch(e_max_treat_check_rebuild_tree&)
        {
#ifdef PRINT_TREAT_PERFOMANCE
		lg<<"process_treat_sequence()1.1 check_tree(): "<<to_string(other_color(step))<<": e_max_treat_check_rebuild_tree";
#endif
        }
        catch(e_max_treat_check_reached&)
        {
#ifdef PRINT_TREAT_PERFOMANCE
		lg<<"process_treat_sequence()1.2 check_tree(): "<<to_string(other_color(step))<<": e_max_treat_check_reached";
#endif
        }

#ifdef PRINT_TREAT_PERFOMANCE
		lg<<"process_treat_sequence()2 check_tree(): "<<to_string(other_color(step))<<": deep="<<deep<<" win="<<tr->win<<" childs.size()="<<tr->childs.size()
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
			lg<<"process_treat_sequence()3 check_tree(): "<<to_string(other_color(step))<<": chain_depth="<<r->get_chain_depth()<<": "<<print_chain(r);
			lg<<"process_treat_sequence()3.1 field: "<<print_steps(player.field.get_steps());
			
			lg<<"process_treat_sequence()3.2\r\n";
			lg<<print_treat_tree(*tr);

		}
#endif

        if(r)
        {
            add_win(r);
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
	lg<<"process_treat_sequence()4: "<<to_string(other_color(step))<<": max deep reached";
#endif
	return r_neitral;

}

unsigned item_t::get_chain_depth() const
{
	if(!wins.empty())return wins.get_chain_depth()+1;
	if(!fails.empty())return fails.get_chain_depth()+1;
	return 1;
}

bool item_t::is_defence_five_exists() const
{
	const state_t& state=player.get_state();

	const points_t& d5_pts=state.get_make_five(step);
    return !d5_pts.empty();
}

template<class Points>
void item_t::add_neitrals(const Points& pts)
{
	step_t s;
	s.step=other_color(step);

	size_t exist_count=neitrals.size();

	neitrals.resize(exist_count+pts.size());

	typename Points::const_iterator i=pts.begin(),ei=pts.end();
	items_t::iterator j=neitrals.begin()+exist_count;
    
	for(;i!=ei;++i,++j)
	{
		static_cast<point&>(s)=*i;
		*j=create_neitral_item(s);
	}
}

item_ptr item_t::create_neitral_item(const step_t& s)
{
	return item_ptr(new item_t(player,s));
}


void item_t::calculate_deep_wins_fails()
{
	deep_wins_count=wins.size();
	deep_fails_count=fails.size();
	
	for(size_t i=0;i<neitrals.size();i++)
	{
		item_t& v=*neitrals[i];
		v.calculate_deep_wins_fails();
		deep_wins_count+=v.deep_fails_count;
		deep_fails_count+=v.deep_wins_count;
	}
}

Result item_t::solve_ant(const item_t* parent_node)
{
	if(!wins.empty())
		return r_fail;
	
	if(neitrals.empty())
	{
		if(!fails.empty())
			return r_win;
		
		Result r=process_predict_treat_sequence(true,0);

		if(r!=r_neitral)
		{
			deep_wins_count=wins.size();
			deep_fails_count=fails.size();
		}
		return r;
	}

	size_t shift=select_ant_neitral(parent_node);
	item_ptr pch=neitrals[shift];
	item_t& ch=*pch;

	long long dw=ch.deep_wins_count;
	long long df=ch.deep_fails_count;

	temporary_state ts(player,ch);
	Result r=ch.solve_ant(this);

	deep_wins_count-=df;
	deep_fails_count-=dw;

	if(r==r_neitral)
	{
		deep_wins_count+=ch.deep_fails_count;
		deep_fails_count+=ch.deep_wins_count;
		return r;
	}

	neitrals.erase(neitrals.begin()+shift);

	if(r==r_win)
	{
		add_win(pch);
		++deep_wins_count;
		return r_fail;
	}
	
	add_fail(pch);
	++deep_fails_count;
	
	if(neitrals.empty())
		return r_win;

	return r_neitral;
}

size_t item_t::select_ant_neitral(const item_t* parent_node)
{
    std::vector<double> marks(neitrals.size());

    for(size_t i=0;i<marks.size();i++)
    {
		const item_t& v=*neitrals[i];
        marks[i]=1.0/v.get_win_rate();
    }

	if(parent_node&&!parent_node->get_fails().empty())
	{
		const npoints_t& wins_hint=parent_node->get_fails().get_win_hins();
		for(size_t i=0;i<marks.size();i++)
		{
			const item_t& v=*neitrals[i];

			npoints_t::const_iterator it=binary_find(wins_hint.begin(),wins_hint.end(),v,less_point_pr());
			if(it!=wins_hint.end())
			{
				marks[i]*=it->n;
			}
		}
	}
	
	double max_rate=std::accumulate(marks.begin(),marks.end(),0.0);

    double adj=0;
    for(size_t i=0;i<marks.size();i++)
    {
        double v=marks[i]/max_rate;
        marks[i]=adj;
        adj+=v;
    }

    double r=static_cast<double>(rand())/RAND_MAX;

    std::vector<double>::const_iterator it=std::lower_bound(marks.begin(),marks.end(),r);
    if(it==marks.end() || (*it!=r && it!=marks.begin()))
        --it;

    return it-marks.begin();
}


///////////////////////////////////////////////////////////////
void wide_item_t::process_deep_common()
{
	process_deep_stored();
	
	if(wins.empty() && !neitrals.empty())
		process_deep_ant();
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
		Result r=process_predictable_move(false,0);
		if(r!=r_neitral)return;

		clear();

		r=process_treat_sequence();
		if(r!=r_neitral)return;

		process_predictable_move(need_fill_neitrals,lookup_deep);
		return;
	}

	process_neitrals(need_fill_neitrals,lookup_deep);
}

Result wide_item_t::process_neitrals(bool need_fill_neitrals,unsigned lookup_deep,unsigned from,const item_t* parent_node)
{
	for(unsigned i=from;i<neitrals.size();i++)
	{
		player.check_cancel();
		item_ptr& pch=neitrals[i];
		item_t& ch=*pch;

		temporary_state ts(player,ch);
		Result r=ch.process(need_fill_neitrals,lookup_deep);
		if(r==r_neitral)continue;
		if(r==r_win)add_win(pch);
		else add_fail(pch);
		pch.reset();
	}

	neitrals.erase(std::remove(neitrals.begin(),neitrals.end(),item_ptr()),neitrals.end());

	if(!wins.empty())
		return r_fail;

	if(neitrals.empty())return r_win;
	return r_neitral;
}

item_ptr wide_item_t::create_neitral_item(const step_t& s)
{
	unsigned st_idx=player.current_state_index();
	if(st_idx+1<stored_deep)
		return item_ptr(new wide_item_t(player,s));

	return item_ptr(new item_t(player,s));
}

///////////////////////////////////////////////////////////////
selected_wins_childs::selected_wins_childs()
{
	current_chain_depth=0;
}
		
void selected_wins_childs::add(const item_ptr& val)
{
	vals.push_back(val);

	unsigned depth=val->get_chain_depth();

	if(!best_val)
	{
		best_val=val;
		current_chain_depth=depth;
		return;
	}

	if(depth<current_chain_depth)
	{
		best_val=val;
		current_chain_depth=depth;
	}
}

void selected_wins_childs::clear()
{
	vals.clear();
	best_val.reset();
	current_chain_depth=0;
}

void selected_fails_childs::add(const item_ptr& val)
{
	add_wins_hint(val);

	vals.push_back(val);

	unsigned depth=val->get_chain_depth();

	if(!best_val)
	{
		best_val=val;
		current_chain_depth=depth;
		return;
	}

	if(depth>current_chain_depth)
	{
		best_val=val;
		current_chain_depth=depth;
	}
}

void selected_fails_childs::clear()
{
	vals.clear();
	best_val.reset();
	current_chain_depth=0;
	win_hints.clear();
}

void selected_fails_childs::add_wins_hint(const item_ptr& val)
{
	const items_t& child_wins=val->get_wins().get_vals();
	
	if(child_wins.empty())
		return;
	
	win_hints.reserve(win_hints.size()+child_wins.size());
	for(size_t i=0;i<child_wins.size();i++)
	{
		const item_t& v=*child_wins[i];
		win_hints.push_back(npoint(v));
	}

	make_unique(win_hints);
}

/////////////////////////////////////////////////////////////////////////////////////

bool existing_npoints_sort_pr::operator()(const item_ptr& a,const item_ptr& b)const
{
	npoints_t::const_iterator ia=binary_find(ref.begin(),ref.end(),*a,pr);
	npoints_t::const_iterator ib=binary_find(ref.begin(),ref.end(),*b,pr);

	if(ia==ref.end())
		return false;

	if(ib==ref.end())
		return true;
	
	return ia->n > ib->n;
}

bool win_rate_cmp_pr::operator()(const item_ptr& a,const item_ptr& b)const
{
	return a->get_win_rate()<b->get_win_rate();
}

/////////////////////////////////////////////////////////////////////////////////////
std::string print_chain(item_ptr root)
{
	steps_t sts;
	while(root)
	{
		sts.push_back(*root);
		root=root->get_win_fail_step();
	}
	return print_steps(sts);
}

void items2points(const items_t& items,points_t& res)
{
	res.resize(items.size());
	for(unsigned i=0;i<items.size();i++)
		res[i]=*items[i];
}

void items2depth_npoints(const items_t& items,npoints_t& res)
{
	res.resize(items.size());
	for(unsigned i=0;i<items.size();i++)
	{
		npoint& p=res[i];
		const WsPlayer::item_t& it=*items[i];
		p=it;
		p.n=it.get_chain_depth();
	}
}

} }//namespace Gomoku

