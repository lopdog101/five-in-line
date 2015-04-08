#include "wsplayer.h"
#include <stdexcept>

#include "../extern/pair_comparator.h"
#include "../extern/binary_find.h"
#include "../extern/object_progress.hpp"

#include "wsplayer_node.h"

namespace Gomoku { namespace WsPlayer
{


wsplayer_t::wsplayer_t()
{
	predict_deep=0;
	predict_processed=0;
    treat_check_count=0;
    treat_check_rebuild_tree_count=0;
    thinking=0;
	lookup_deep=def_lookup_deep;
}

void wsplayer_t::delegate_step()
{
    incer_t<int> hld_thinking(thinking);

	field=game().field();
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
        hld_thinking.reset();
        throw;
	}

	if(!root->get_wins().empty())
	{
		unsigned int depth=root->get_chain_depth()-1;
		std::string chain=print_chain(root->get_wins().get_best());
		lg<<"wsplayer_t::delegate_step(): find win chain_depth="<<depth<<": "<<chain;
	}
	if(!root->get_fails().empty() && root->get_neitrals().empty())lg<<"wsplayer_t::delegate_step(): find fail chain_depth="<<(root->get_chain_depth()-1)
		<<": "<<print_chain(root->get_fails().get_best());

	point p=*root->get_next_step();
    game().OnNextStep(*this,p);
}

void wsplayer_t::solve()
{
	field=game().field();
	wide_item_t* wr=new wide_item_t(*this,field.back());
	root=item_ptr(wr);

	init_states();
	wr->process_deep_common();
}

void wsplayer_t::increase_state()
{
	++current_state;
	if(current_state+1>states.size())states.push_back(state_ptr(new state_t(*this)));
	states[current_state]->state_from(*states[current_state-1]);
}

void wsplayer_t::init_states()
{
	current_state=0;
	if(states.empty())states.push_back(state_ptr(new state_t(*this)));
	get_state().init_zero();
}

void wsplayer_t::increase_treat_check_count()
{
    ++treat_check_count;
    if(treat_check_count>max_treat_check)
        throw e_max_treat_check_reached();
}

void wsplayer_t::increase_treat_check_rebuild_tree_count()
{
    ++treat_check_rebuild_tree_count;
    if(treat_check_rebuild_tree_count>max_treat_check_rebuild_tree)
        throw e_max_treat_check_rebuild_tree();
}


} }//namespace Gomoku
