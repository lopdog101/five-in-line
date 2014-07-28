#pragma once

#include "../algo/game.h"
#include "../algo/wsplayer.h"
#include "ThreadProcessor.h"
#include <boost/signals2/connection.hpp>
#include "../extern/object_progress.hpp"
#include "mfc_field.h"

namespace Gomoku
{

class mfcPlayer : public iplayer_t
{
	CMfcField* fd;
	CBitmap bmp;
	bool inited;

	boost::signals2::scoped_connection hld_mouse_down;
	boost::signals2::scoped_connection hld_mouse_up;
	boost::signals2::scoped_connection hld_mouse_move;
	boost::signals2::scoped_connection hld_after_draw;

    void reset_handlers();

	void fieldLMouseDown(point pos);
	void fieldLMouseUp(point pos);
	void fieldMouseMove(point pos);
	void afterDraw(CDC& dc);
public:
	mfcPlayer(){fd=0;inited=false;}
	mfcPlayer(const mfcPlayer& rhs){fd=0;}
	
	virtual void init(game_t& _gm,Step _cl);
	virtual void delegate_step();

    virtual void request_cancel(bool val);
    virtual bool is_thinking() const;

    template<class Archive>
    void serialize(Archive &ar, const unsigned int version)
    {
#ifdef BOOST_SERIALIZATION_NVP
        ar & BOOST_SERIALIZATION_BASE_OBJECT_NVP(iplayer_t);
#endif
    }

	POLIMVAR_IMPLEMENT_CLONE(mfcPlayer )
};

class ThreadPlayer : public iplayer_t
{
public:
private:
	CThreadProcessor processor;
	player_ptr pl;
	player_ptr null_player;
	game_t mirror_gm;
	step_t last_step;
    
    point answer_point;
    bool answer_complete;

	boost::signals2::scoped_connection hld_execute;
	boost::signals2::scoped_connection hld_complete;
	boost::signals2::scoped_connection hld_errors;
    boost::signals2::scoped_connection hld_mirror_next_step;

	void MirrorExecute();
	void TaskComplete();
	void TaskErrors(const CThreadProcessor::errors_t& vals);
    void mirrorNextStep(const iplayer_t& pl,const point& pt);

	void clear();
public:
	ThreadPlayer(){}
	ThreadPlayer(const player_ptr& _pl){pl=_pl;}
	ThreadPlayer(const ThreadPlayer& rhs){if(rhs.pl)pl=player_ptr(rhs.pl->clone());}
	~ThreadPlayer(){clear();}
	
	virtual void init(game_t& _gm,Step _cl);
	virtual void delegate_step();

    virtual void request_cancel(bool val);
    virtual bool is_thinking() const;

    player_ptr get_player() const{return pl;}

    template<class Archive>
    void serialize(Archive &ar, const unsigned int version)
    {
#ifdef BOOST_SERIALIZATION_NVP
        ar & BOOST_SERIALIZATION_BASE_OBJECT_NVP(iplayer_t);
        ar & BOOST_SERIALIZATION_NVP(pl);
#endif
    }

	POLIMVAR_IMPLEMENT_CLONE(ThreadPlayer)
};


class NullPlayer : public iplayer_t
{
public:
	NullPlayer(){}
	
	void init(game_t& _gm,Step _cl){}
	void delegate_step(){}
    bool is_thinking() const{return false;}

    template<class Archive>
    void serialize(Archive &ar, const unsigned int version)
    {
#ifdef BOOST_SERIALIZATION_NVP
        ar & BOOST_SERIALIZATION_BASE_OBJECT_NVP(iplayer_t);
#endif
    }

	POLIMVAR_IMPLEMENT_CLONE(NullPlayer)
};

}//namespace
