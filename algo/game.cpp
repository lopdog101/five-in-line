
#include "game_xml.h"
#include "gomoku_exceptions.h"

namespace Gomoku
{

game_t::game_t() : fieldp(new field_t)
{
    reset_field();
}


void game_t::reset_field()
{
	field().clear();
	field().add(point(0,0),st_krestik);
}

void game_t::init_players()
{
	krestik->init(*this,st_krestik);
	nolik->init(*this,st_nolik);
}

void game_t::delegate_next_step()
{
    if(is_game_over()) return;

	if(field().size()%2) nolik->delegate_step();
	else krestik->delegate_step();
}

void game_t::make_step(const iplayer_t& pl,const point& pt)
{
	if(&pl!=krestik.get()&&&pl!=nolik.get())throw e_invalid_step(pl.color());
	if((field().size()%2==0) != (&pl==krestik.get()))throw e_invalid_step(pl.color());

	field().add(pt,pl.color());
}

bool game_t::is_game_over() const
{
	return !field().empty()&&field().check_five(field().back());
}

bool game_t::is_somebody_thinking() const
{
	return 
        !is_game_over()&&
        (krestik!=nullptr&&krestik->is_thinking() ||
         nolik!=nullptr&&nolik->is_thinking() );
}

}//namespace Gomoku
