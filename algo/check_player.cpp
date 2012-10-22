#include "check_player.h"
#include <stdexcept>
#include "field_xml.h"

namespace Gomoku
{

void check_player_t::delegate_step()
{
	const steps_t& game_stp=game().field().get_steps();

	if(game_stp.size()>=steps.size()||
		!std::equal(game_stp.begin(),game_stp.end(),steps.begin()))
		throw std::runtime_error("Game not synchornized");

	const step_t& s=steps[game_stp.size()];

	gm->make_step(*this,s);
}

#ifdef USE_XML
void check_player_t::pack(Xpat::ipacker_t& root_node,bool process_type) const
{
	if(XML_SET_TYPE)return;
	XML_MPACK(steps);
}

void check_player_t::unpack(const Xpat::ipacker_t& root_node,bool process_type)
{
	if(XML_CHECK_TYPE)return;
	XML_MUNPACK(steps);
}
#endif

}
