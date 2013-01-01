#include <stdio.h>
#include "../db/solution_tree_utils.h"
#include "../algo/game.h"
#include "../algo/wsplayer.h"
#include "../algo/wsplayer_node.h"

#ifdef WITHOUT_EXTERNAL_LIBS
#  include "../extern/object_progress.hpp"
#else
# include <boost/filesystem/operations.hpp>
namespace fs=boost::filesystem;
# include <object_progress/log_to_file.hpp>
# include <object_progress/object_progress.hpp>
#endif

using namespace Gomoku;

void print_use()
{
	printf("USE: solver key\n");
	printf("Enviropment variables\n");
	printf("stored_deep (default: %u)\n",WsPlayer::stored_deep);
	printf("lookup_deep (default: %u)\n",WsPlayer::def_lookup_deep);
	printf("treat_deep  (default: %u)\n",WsPlayer::treat_deep);
}


int main(int argc,char** argv)
{
#ifndef WITHOUT_EXTERNAL_LIBS
	fs::path::default_name_check(boost::filesystem::native);

	ObjectProgress::log_to_file lf;
	
	lf.file_name="solver.log";
	lf.create_empty=true;
	lf.set_manager(ObjectProgress::log_manager_singleton::instance());
	lf.open();
#endif

	const char* sval=getenv("stored_deep");
	if(sval!=0&&(*sval)!=0)
		WsPlayer::stored_deep=atol(sval);

	sval=getenv("lookup_deep");
	if(sval!=0&&(*sval)!=0)
		WsPlayer::def_lookup_deep=atol(sval);

	sval=getenv("treat_deep");
	if(sval!=0&&(*sval)!=0)
		WsPlayer::treat_deep=atol(sval);

	ObjectProgress::log_generator lg(true);
	lg<<"stored_deep="<<WsPlayer::stored_deep<<" lookup_deep="<<WsPlayer::def_lookup_deep<<" treat_deep="<<WsPlayer::treat_deep;

	if(argc!=2)
	{
		print_use();
		return 1;
	}

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
		printf("std::exception: %s\n",e.what());
		return 1;
	}
	catch(...)
	{
		printf("unknown exception\n");
		return 1;
	}

	return 0;
}
