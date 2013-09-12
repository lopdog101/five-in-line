
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

	if(next_color(field().size()) == st_krestik) krestik->delegate_step();
	else nolik->delegate_step();
}

void game_t::make_step(const iplayer_t& pl,const point& pt)
{
	if(&pl!=krestik.get()&&&pl!=nolik.get())throw e_invalid_step(pl.color());
	if( (next_color(field().size())==st_krestik) != (&pl==krestik.get()))throw e_invalid_step(pl.color());

	field().add(pt,pl.color());
}

bool game_t::is_game_over() const
{
	return !field().empty()&&field().check_five(field().back());
}

bool game_t::is_somebody_thinking() const
{
    if(is_game_over())
        return false;

    if(krestik&&krestik->is_thinking())
        return true;

    if(nolik&&nolik->is_thinking())
        return true;

    return false;
}

}//namespace Gomoku
