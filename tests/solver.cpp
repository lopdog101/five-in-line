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
}

void solver::TearDown()
{
	WsPlayer::stored_deep=backup_stored_deep;
	WsPlayer::def_lookup_deep=backup_def_lookup_deep;
	WsPlayer::treat_deep=backup_treat_deep;
    WsPlayer::max_treat_check=backup_max_treat_check;
    WsPlayer::max_treat_check_rebuild_tree=backup_max_treat_check_rebuild_tree;
}

void solver::solve(const std::string& state_str)
{
    SCOPED_TRACE(__FUNCTION__);

	data_t bin;
	hex2bin(state_str,bin);

	steps_t init_state;
	bin2points(bin,init_state);
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

TEST_F(solver, skiped_neitrals_inside_field)
{
	WsPlayer::stored_deep=1;
	WsPlayer::def_lookup_deep=5;

	solve("fd0201fd0302fe0102fe0201ff0002ff0101ff020200fe0200ff0200000100010101ff0101010102fe02020001020102020202030101");

	sort_results();
	print_results();

	EXPECT_TRUE(fails_contains(point(1,0)));
	EXPECT_TRUE(fails_contains(point(1,-3)));
	EXPECT_TRUE(fails_contains(point(1,2)));
	EXPECT_TRUE(fails_contains(point(-1,3)));
	EXPECT_TRUE(fails_contains(point(-2,4)));
}

}//namespace