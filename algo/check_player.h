#ifndef gomoku_check_playerH
#define gomoku_check_playerH
#include "game.h"

namespace Gomoku
{
	class check_player_t : public iplayer_t
	{
		steps_t steps;
    public:
		void begin_game(){}
		void delegate_step();

		void set_steps(const steps_t& val){steps=val;}

#ifdef USE_XML
		void pack(Xpat::ipacker_t& root_node,bool process_type=true) const;
		void unpack(const Xpat::ipacker_t& root_node,bool process_type=true);
#endif
		POLIMVAR_IMPLEMENT_CLONE( check_player_t )
	};
}

#ifdef USE_XML
namespace Xpat
{
template <> inline std::string xml_type_name< Gomoku::check_player_t>::get(){return "check_player";};

}//
#endif

#endif
