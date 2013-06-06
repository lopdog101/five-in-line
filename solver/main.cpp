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

		Step last_step=(init_state.size()%2)==0? st_nolik:st_krestik;

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

		pl.init(gm,other_step(last_step));
		pl.begin_game();
		pl.solve();

		const WsPlayer::wide_item_t& r=static_cast<const WsPlayer::wide_item_t&>(*pl.root);

		points_t neitrals(r.neitrals.size());
		for(unsigned i=0;i<r.neitrals.size();i++)
			neitrals[i]=*r.neitrals[i];

		npoints_t wins(r.wins.size());
		for(unsigned i=0;i<r.wins.size();i++)
		{
			npoint& p=wins[i];
			WsPlayer::item_t& it=*r.wins[i];
			p=it;
			p.n=it.get_chain_depth();
		}

		npoints_t fails(r.fails.size());
		for(unsigned i=0;i<r.fails.size();i++)
		{
			npoint& p=fails[i];
			WsPlayer::item_t& it=*r.fails[i];
			p=it;
			p.n=it.get_chain_depth();
		}

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
