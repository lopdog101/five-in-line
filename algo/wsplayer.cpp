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

#include "wsplayer_node.h"

namespace Gomoku { namespace WsPlayer
{


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

void wsplayer_t::increase_state()
{
	++current_state;
	if(current_state+1>states.size())states.push_back(state_ptr(new state_t(*this)));
	states[current_state]->state_from(*states[current_state-1]);
}

///////////////////////////////////////////////////////////////
void wsplayer_t::init_states()
{
	current_state=0;
	if(states.empty())states.push_back(state_ptr(new state_t(*this)));
	get_state().init_zero();
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
} }//namespace Gomoku
