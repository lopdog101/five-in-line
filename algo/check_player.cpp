#include "check_player.h"
#include <stdexcept>
#include "field.h"

namespace Gomoku
{

void check_player_t::delegate_step()
{
    if(is_cancel_requested())
        return;

	const steps_t& game_stp=game().field().get_steps();

	if(game_stp.size()>=steps.size()||
		!std::equal(game_stp.begin(),game_stp.end(),steps.begin()))
		throw std::runtime_error("Game not synchornized");

	const step_t& s=steps[game_stp.size()];

	game().OnNextStep(*this,s);
}

bool check_player_t::is_thinking() const
{
    return false;
}


}
