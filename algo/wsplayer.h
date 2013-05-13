#ifndef gomoku_wsplayerH
#define gomoku_wsplayerH
#include "wsplayer_common.h"
#include "game.h"
#include <boost/shared_ptr.hpp>

namespace Gomoku { namespace WsPlayer
{
	struct temporary_state;
	class item_t;
	class treat_node_t;
	class wide_item_t;
	class state_t;

	typedef boost::shared_ptr<item_t> item_ptr;

	//Игрок, основанный на поиске решения в ширину
	class wsplayer_t : public iplayer_t
	{
	private:
		friend struct temporary_state;
		friend class item_t;
		friend class treat_node_t;
		friend class wide_item_t;
		friend class state_t;

		field_t field;
		unsigned long long predict_processed;
		unsigned predict_deep;

        states_t states;
		unsigned current_state;

        unsigned current_treat_check_deep;

		void init_states();
		inline state_t& get_state(){return *states[current_state];}
		inline const state_t& get_state() const {return *states[current_state];}
		void increase_state();
		inline void decrease_state(){--current_state;}

	public:
		item_ptr root;

		wsplayer_t();

		void begin_game();
		void delegate_step();
		void solve();

#ifdef USE_XML
		void pack(Xpat::ipacker_t& root_node,bool process_type=true) const;
		void unpack(const Xpat::ipacker_t& root_node,bool process_type=true);
#endif

		POLIMVAR_IMPLEMENT_CLONE(wsplayer_t )
	};


} }//namespace Gomoku

#ifdef USE_XML
namespace Xpat
{
template <> inline std::string xml_type_name< Gomoku::WsPlayer::wsplayer_t>::get(){return "width_search_player";};
}//
#endif
#endif

