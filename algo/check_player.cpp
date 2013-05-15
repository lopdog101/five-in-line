#include "check_player.h"
#include <stdexcept>
#include "field.h"

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

}
