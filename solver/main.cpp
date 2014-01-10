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
        data_t bin;
		hex2bin(argv[1],bin);
		steps_t init_state;
		bin2points(bin,init_state);

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

		points_t neitrals;
		items2points(r.get_neitrals(),neitrals);

		npoints_t wins;
		items2depth_npoints(r.get_wins().get_vals(),wins);

		npoints_t fails;
		items2depth_npoints(r.get_fails().get_vals(),fails);

		std::string str;

		std::string ret=argv[1];

		points2bin(neitrals,bin);
		bin2hex(bin,str);
		ret+=" ";
		if(str.empty())ret+="empty";
		else ret+=str;

		points2bin(wins,bin);
		bin2hex(bin,str);
		ret+=" ";
		if(str.empty())ret+="empty";
		else ret+=str;

		points2bin(fails,bin);
		bin2hex(bin,str);
		ret+=" ";
		if(str.empty())ret+="empty";
		else ret+=str;

		printf("%s",ret.c_str());

		ObjectProgress::log_generator lg(true);

		lg<<"n="<<print_points(neitrals);
		lg<<"w="<<print_points(wins);
		lg<<"f="<<print_points(fails);
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
