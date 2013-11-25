#ifndef gomoku_gameH
#define gomoku_gameH

#include "field.h"

#include <boost/shared_ptr.hpp>

#  include "../extern/polimvar_intf.h"
#  include <boost/signal.hpp>

namespace Gomoku
{
	class game_t;

	class iplayer_t 
	{
		Step color_;
		bool cancel_requested;
		game_t* gm;

    protected:
        
        bool is_cancel_requested() const{return cancel_requested;}
		void check_cancel(){if(is_cancel_requested())throw e_cancel();}

    public:
		inline Step color() const{return color_;}
        inline const game_t& game() const{return *gm;}

		iplayer_t()
        {
            gm=0;
            color_=st_krestik;
            cancel_requested=false;
        }

		virtual ~iplayer_t(){}

		virtual void init(game_t& _gm,Step _cl)
        {
            gm=&_gm;color_=_cl;
        }

        virtual void delegate_step()=0;

		virtual void request_cancel(bool val)
		{
			cancel_requested=val;
		}

        virtual bool is_thinking() const=0;
        
        template<class Archive>
        void serialize(Archive &ar, const unsigned int version)
        {
        }

        POLIMVAR_DECLARE_CLONE(iplayer_t )
	};

	typedef boost::shared_ptr<iplayer_t> player_ptr;
	typedef boost::shared_ptr<field_t> field_ptr;

	class game_t
	{
		player_ptr krestik;
		player_ptr nolik;
	public:
		field_ptr fieldp;
		field_t& field(){return *fieldp;}
		const field_t& field() const{return *fieldp;}

		game_t();

		void reset_field();
        void init_players();
		void delegate_next_step();
		bool is_somebody_thinking() const;
		void make_step(const iplayer_t& pl,const point& pt);
		bool is_game_over() const;
		
		inline void set_krestik(const player_ptr& kr){krestik=kr;}
		inline void set_nolik(const player_ptr& nl){nolik=nl;}

		player_ptr get_krestik() const{return krestik;}
		player_ptr get_nolik() const{return nolik;}

        boost::signal< void (const iplayer_t& pl,const point& pt)> OnNextStep;
    };

}//namespace Gomoku

#endif

