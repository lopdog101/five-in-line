#ifndef gomoku_gameH
#define gomoku_gameH

#include "field.h"

#include <boost/shared_ptr.hpp>

#ifdef WITHOUT_EXTERNAL_LIBS
#  include "../extern/polimvar_intf.h"
#else
#  include <notification/notification.hpp>
#  include <polimvar_intf.h>
#endif

#ifdef USE_XML
#  include <cppexpat/cppexpat.h>
#endif


namespace Gomoku
{
	class game_t;

	struct e_cancel{};

	class iplayer_t 
#ifdef USE_XML
		: public Xpat::packable_t
#endif
	{
		Step color_;
	protected:
		game_t* gm;
	private:
		bool canceled;
	public:
		inline Step color() const{return color_;}

		iplayer_t() {gm=0;color_=st_krestik;canceled=false;}
		virtual ~iplayer_t(){}

		virtual void begin_game(){canceled=false;}
		virtual void delegate_step()=0;

		virtual void init(game_t& _gm,Step _cl){gm=&_gm;color_=_cl;}
		inline const game_t& game() const{return *gm;}

		inline bool is_canceled() const{return canceled;}
		virtual void cancel()
		{
			canceled=true;
		}
		void check_cancel(){if(is_canceled())throw e_cancel();}

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

		void begin_play();
		void continue_play();
		void end_play();
		bool is_play() const;
		void make_step(const iplayer_t& pl,const point& pt);
		bool is_game_over() const;
		
		inline void set_krestik(const player_ptr& kr){krestik=kr;}
		inline void set_nolik(const player_ptr& nl){nolik=nl;}

		player_ptr get_krestik() const{return krestik;}
		player_ptr get_nolik() const{return nolik;}

#ifndef WITHOUT_EXTERNAL_LIBS
		Notification::notify< void (*)(const game_t&)> OnNextStep;
#endif
	};

}//namespace Gomoku

#endif

