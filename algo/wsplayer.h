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

        unsigned treat_check_count;
        unsigned treat_check_rebuild_tree_count;

        int thinking;

		void init_states();
		inline state_t& get_state(){return *states[current_state];}
		inline const state_t& get_state() const {return *states[current_state];}
		void increase_state();
		inline void decrease_state(){--current_state;}
		unsigned current_state_index() const{return current_state;}

        void increase_treat_check_count();
        void increase_treat_check_rebuild_tree_count();

	public:
		item_ptr root;

		wsplayer_t();

		virtual void delegate_step();
        virtual bool is_thinking() const{return thinking>0;}

		void solve();


        template<class Archive>
        void serialize(Archive &ar, const unsigned int version)
        {
#ifdef BOOST_SERIALIZATION_NVP
            ar & BOOST_SERIALIZATION_BASE_OBJECT_NVP(iplayer_t);
#endif
        }

        POLIMVAR_IMPLEMENT_CLONE(wsplayer_t )
	};


} }//namespace Gomoku

#endif

