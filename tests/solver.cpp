#include "gtest/gtest.h"
#include "../algo/wsplayer.h"
#include "../algo/wsplayer_node.h"
#include <boost/lexical_cast.hpp>
#include "../db/solution_tree_utils.h"

namespace Gomoku
{

class solver : public testing::Test
{
	unsigned backup_stored_deep;
	unsigned backup_def_lookup_deep;
	unsigned backup_treat_deep;
	unsigned backup_max_treat_check;
	unsigned backup_max_treat_check_rebuild_tree;
	unsigned ant_count;
 protected:
	game_t gm;
	WsPlayer::wsplayer_t player;
	points_t neitrals;
	npoints_t wins;
	npoints_t fails;

	virtual void SetUp();
	virtual void TearDown();
	void solve(const std::string& state_str);

	bool fails_contains(const point &p){return std::find(fails.begin(),fails.end(),p)!=fails.end();}
	bool wins_contains(const point &p){return std::find(wins.begin(),wins.end(),p)!=wins.end();}

	void sort_results();
	void print_results();
};

void solver::SetUp()
{
	backup_stored_deep=WsPlayer::stored_deep;
	backup_def_lookup_deep=WsPlayer::def_lookup_deep;
	backup_treat_deep=WsPlayer::treat_deep;
    backup_max_treat_check=WsPlayer::max_treat_check;
    backup_max_treat_check_rebuild_tree=WsPlayer::max_treat_check_rebuild_tree;
    ant_count=WsPlayer::ant_count;
	srand(0);
}

void solver::TearDown()
{
	WsPlayer::stored_deep=backup_stored_deep;
	WsPlayer::def_lookup_deep=backup_def_lookup_deep;
	WsPlayer::treat_deep=backup_treat_deep;
    WsPlayer::max_treat_check=backup_max_treat_check;
    WsPlayer::max_treat_check_rebuild_tree=backup_max_treat_check_rebuild_tree;
    WsPlayer::ant_count=ant_count;
}

void solver::solve(const std::string& state_str)
{
    SCOPED_TRACE(__FUNCTION__);

	steps_t init_state;
	hex_or_str2points(state_str,init_state);
	reorder_state_to_game_order(init_state);

	game_t gm;
	gm.field().set_steps(init_state);

	WsPlayer::wsplayer_t pl;

	pl.init(gm,next_color(init_state.size()));
	pl.solve();

	const WsPlayer::wide_item_t& r=static_cast<const WsPlayer::wide_item_t&>(*pl.root);

	WsPlayer::items2points(r.get_neitrals(),neitrals);

	WsPlayer::items2depth_npoints(r.get_wins().get_vals(),wins);

	items2depth_npoints(r.get_fails().get_vals(),fails);
}

void solver::sort_results()
{
    std::sort(neitrals.begin(),neitrals.end(),less_point_pr());
    std::sort(wins.begin(),wins.end(),less_point_pr());
    std::sort(fails.begin(),fails.end(),less_point_pr());
}

void solver::print_results()
{
	std::string sneitrals=print_points(neitrals);
	std::string swins=print_points(wins);
	std::string sfails=print_points(fails);

	printf("neitrals=%s\nwins=%s\nfails=%s\n",sneitrals.c_str(),swins.c_str(),sfails.c_str());
}

// Bug description. In solve result empty points inside play field is absent in neitrals,wins,fails output.
// This points should be there because they pass "2 points around" condition and this state is not restricted answer to the treat
TEST_F(solver, skiped_neitrals_inside_field)
{
	WsPlayer::stored_deep=1;
	WsPlayer::def_lookup_deep=6;

	solve("fd0201fd0302fe0102fe0201ff0002ff0101ff020200fe0200ff0200000100010101ff0101010102fe02020001020102020202030101");

//	sort_results();
//	print_results();

	EXPECT_TRUE(fails_contains(point(1,0)));
	EXPECT_TRUE(fails_contains(point(1,-3)));
	EXPECT_TRUE(fails_contains(point(1,2)));
	EXPECT_TRUE(fails_contains(point(-1,3)));
	EXPECT_TRUE(fails_contains(point(-2,4)));
}

// Bug description. Close four based treat is hidded by opponent open four treat.
// Wrong assumptation is close four is "stronger" than open four. But in a fact both of them reach win on 2 steps.
TEST_F(solver, lost_close_four_treat_cause_opponent_open_four_exists)
{
	WsPlayer::stored_deep=1;
	WsPlayer::def_lookup_deep=0;
	WsPlayer::ant_count=0;

	solve("(0,0:X);(1,0:O);(-1,0:X);(8,-4:O);(-2,0:X);(8,-3:O);(-3,-1:X);(7,-5:O);(-3,-2:X);(6,-5:O);(-3,-3:X);(-3,-4:O);(2,0:X);(8,-5:O)");

//	sort_results();
//	print_results();
	
	EXPECT_TRUE(wins_contains(point(-3,0)));
}

// Bug description. If we use ant search and disable treat search but lookup enabled, we found wrong 5 step win
TEST_F(solver, wrong_ant_win_lookup_enabled)
{
	WsPlayer::stored_deep=1;
	WsPlayer::def_lookup_deep=5;
	WsPlayer::treat_deep=0;
	WsPlayer::ant_count=1000;

	solve("(0,0:X);(0,1:O);(-1,1:X);(-2,2:O);(1,-1:X);(-1,0:O);(1,0:X);(1,2:O);(-2,-1:X);(-1,-1:O);(0,-1:X);(0,2:O);(-1,2:X);(2,3:O);(3,4:X);(-1,-2:O);(1,-3:X);(1,-2:O);(2,-2:X);(3,-3:O);(3,-1:X);(4,0:O);(2,-1:X);(4,-1:O);(2,0:X);(-1,-3:O);(-1,-4:X);(2,-3:O)");

	EXPECT_FALSE(wins_contains(point(0,-2)));
}


TEST_F(solver, regress_before_duplicated_states)
{
	WsPlayer::stored_deep=3;
	WsPlayer::def_lookup_deep=5;
	WsPlayer::ant_count=1000;

	solve("(0,0:X);(2,1:O);(-1,1:X);(-2,-2:O);(-1,0:X);(-2,2:O)");

	sort_results();
	print_results();

	EXPECT_TRUE(wins_contains(point(0,1)) );

	//517963
	//523690
	//5558
}

TEST_F(solver, solver_crash)
{
	WsPlayer::stored_deep=2;
	WsPlayer::def_lookup_deep=5;
	WsPlayer::ant_count=5000;

	solve("000001020102ff0101fe000201ff0102fe02ff0201000202000101ff0302020001fd0502");

	sort_results();
	print_results();
}


}//namespace