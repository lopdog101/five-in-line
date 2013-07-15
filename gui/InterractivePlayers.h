#pragma once

#include "../algo/game.h"
#include "../algo/wsplayer.h"
#include "ThreadProcessor.h"
#include <boost/signals/connection.hpp>
#include "../extern/object_progress.hpp"
#include "mfc_field.h"


class mfcPlayer : public Gomoku::iplayer_t
{
	CMfcField* fd;
	CBitmap bmp;
	bool inited;

	boost::signals::scoped_connection hld_mouse_down;
	boost::signals::scoped_connection hld_mouse_up;
	boost::signals::scoped_connection hld_mouse_move;
	boost::signals::scoped_connection hld_after_draw;

	void fieldLMouseDown(Gomoku::point pos);
	void fieldLMouseUp(Gomoku::point pos);
	void fieldMouseMove(Gomoku::point pos);
	void afterDraw(CDC& dc);
public:
	mfcPlayer(){fd=0;inited=false;}
	mfcPlayer(const mfcPlayer& rhs){fd=0;}
	
	void init(Gomoku::game_t& _gm,Gomoku::Step _cl);
	void delegate_step();

    template<class Archive>
    void serialize(Archive &ar, const unsigned int version)
    {
        ar & BOOST_SERIALIZATION_BASE_OBJECT_NVP(iplayer_t);
    }

	POLIMVAR_IMPLEMENT_CLONE(mfcPlayer )
};

class ThreadPlayer : public Gomoku::iplayer_t
{
public:
private:
	CThreadProcessor processor;
	Gomoku::player_ptr pl;
	Gomoku::player_ptr null_player;
	Gomoku::game_t mirror_gm;
	Gomoku::step_t last_step;

	boost::signals::scoped_connection hld_execute;
	boost::signals::scoped_connection hld_complete;
	boost::signals::scoped_connection hld_errors;

	void MirrorExecute();
	void TaskComplete();
	void TaskErrors(const CThreadProcessor::errors_t& vals);

	void clear();
public:
	ThreadPlayer(){}
	~ThreadPlayer(){clear();}
	ThreadPlayer(const Gomoku::player_ptr& _pl){pl=_pl;}
	ThreadPlayer(const ThreadPlayer& rhs){if(rhs.pl)pl=Gomoku::player_ptr(rhs.pl->clone());}
	
	void init(Gomoku::game_t& _gm,Gomoku::Step _cl);
	void delegate_step();
	Gomoku::player_ptr get_player() const{return pl;}

    template<class Archive>
    void serialize(Archive &ar, const unsigned int version)
    {
        ar & BOOST_SERIALIZATION_BASE_OBJECT_NVP(iplayer_t);
        ar & BOOST_SERIALIZATION_NVP(pl);
    }

	POLIMVAR_IMPLEMENT_CLONE(ThreadPlayer)
};


class NullPlayer : public Gomoku::iplayer_t
{
public:
	NullPlayer(){}
	
	void init(Gomoku::game_t& _gm,Gomoku::Step _cl){}
	void delegate_step(){}

    template<class Archive>
    void serialize(Archive &ar, const unsigned int version)
    {
        ar & BOOST_SERIALIZATION_BASE_OBJECT_NVP(iplayer_t);
    }

	POLIMVAR_IMPLEMENT_CLONE(NullPlayer)
};
