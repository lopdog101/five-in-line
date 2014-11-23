#include <stdio.h>
#include "../db/solution_tree_utils.h"
#include "../algo/game.h"
#include "../algo/wsplayer.h"
#include "../algo/wsplayer_node.h"

#include "../extern/object_progress.hpp"
#include "../algo/env_variables.h"


ObjectProgress::logout_cerr log_err;
ObjectProgress::logout_file log_file;

using namespace Gomoku;

void print_use()
{
	printf("USE: solver key\n");
    Gomoku::print_enviropment_variables_hint();
}

std::string print_state(const WsPlayer::wide_item_t& r,const steps_t& key,unsigned& cnt)
{
	points_t neitrals;
	items2points(r.get_neitrals(),neitrals);

	npoints_t wins;
	items2depth_npoints(r.get_wins().get_vals(),wins);

	npoints_t fails;
	items2depth_npoints(r.get_fails().get_vals(),fails);

	std::string str;
	data_t bin;

	std::string slevel=boost::lexical_cast<std::string>(cnt);
	cnt++;

	std::string ret;

	points2bin(key,bin);
	bin2hex(bin,str);
	ret+="k"+slevel+"="+str;

	if(!neitrals.empty())
	{
		points2bin(neitrals,bin);
		bin2hex(bin,str);
		ret+="&n"+slevel+"="+str;
	}

	if(!wins.empty())
	{
		points2bin(wins,bin);
		bin2hex(bin,str);
		ret+="&w"+slevel+"="+str;
	}

	if(!fails.empty())
	{
		points2bin(fails,bin);
		bin2hex(bin,str);
		ret+="&f"+slevel+"="+str;
	}

	//ObjectProgress::log_generator lg(true);

	//lg<<"k="<<print_steps(key);
	//lg<<"n="<<print_points(neitrals);
	//lg<<"w="<<print_points(wins);
	//lg<<"f="<<print_points(fails);

	return ret;
}

void print_sub_states(const WsPlayer::wide_item_t& r,steps_t& init_state,unsigned& cnt)
{
	const Gomoku::WsPlayer::items_t& neitrals=r.get_neitrals();

	init_state.push_back(step_t(next_color(init_state.size()),0,0));
	
	for(size_t i=0;i<neitrals.size();i++)
	{
		const WsPlayer::wide_item_t* sub=dynamic_cast<const WsPlayer::wide_item_t*>(&*neitrals[i]);
		if(!sub)
			continue;

		if(sub->get_neitrals().empty()&&
			sub->get_wins().empty()&&
			sub->get_fails().empty())
			continue;

		static_cast<point&>(init_state.back())=*sub;

		std::string ln=print_state(*sub,init_state,cnt);
		printf("&%s",ln.c_str());
	}
	
	for(size_t i=0;i<neitrals.size();i++)
	{
		const WsPlayer::wide_item_t* sub=dynamic_cast<const WsPlayer::wide_item_t*>(&*neitrals[i]);
		if(!sub)
			continue;

		if(sub->get_neitrals().empty()&&
			sub->get_wins().empty()&&
			sub->get_fails().empty())
			continue;

		static_cast<point&>(init_state.back())=*sub;
		print_sub_states(*sub,init_state,cnt);
	}

	init_state.pop_back();

}

void print_states(const WsPlayer::wide_item_t& r,steps_t& init_state)
{
	unsigned cnt=0;
	std::string ln=print_state(r,init_state,cnt);
	printf("%s",ln.c_str());
	print_sub_states(r,init_state,cnt);
}


int main(int argc,char** argv)
{
	if(argc!=2)
	{
		print_use();
		return 1;
	}

    log_err.open();
        
    log_file.file_name="solver.log";
    log_file.print_timestamp=true;
    log_file.open();

    scan_enviropment_variables();

	ObjectProgress::log_generator lg(true);
    print_used_enviropment_variables(lg);

	try
	{
		steps_t init_state;
		hex_or_str2points(argv[1],init_state);

		if(init_state.empty())
		{
			print_use();
			return 1;
		}

		Step last_step=last_color(init_state.size());

		steps_t::iterator i=init_state.begin();
		
		for(;i!=init_state.end();++i)
			if(i->step==last_step)
				break;

		if(i==init_state.end())
		{
			print_use();
			return 1;
		}

		std::swap(*i,*(init_state.end()-1));

		game_t gm;
		gm.field().set_steps(init_state);

		WsPlayer::wsplayer_t pl;

		pl.init(gm,other_color(last_step));
		pl.solve();

		const WsPlayer::wide_item_t& r=static_cast<const WsPlayer::wide_item_t&>(*pl.root);
		print_states(r,init_state);
	}
	catch(std::exception& e)
	{
        lg<<"std::exception: "<<e.what();
		printf("std::exception: %s\n",e.what());
		return 1;
	}
	catch(...)
	{
        lg<<"unknown exception";
		return 1;
	}

	return 0;
}
