#ifndef gomoku_wsplayer_commonH
#define gomoku_wsplayer_commonH
#include "game.h"
#include <boost/shared_ptr.hpp>

namespace Gomoku { namespace WsPlayer
{
	extern unsigned stored_deep;
	extern unsigned def_lookup_deep;
	extern unsigned treat_deep;
    extern unsigned max_treat_check;
    extern unsigned max_treat_check_rebuild_tree;

    struct e_max_treat_check_reached : public e_cancel{};
    struct e_max_treat_check_rebuild_tree : public e_max_treat_check_reached{};

	extern unsigned nodes_count;

	enum Result{r_fail=-1,r_neitral=0,r_win=1};

	class wsplayer_t;

	struct atack_t
	{
		point open[3];
		point move;
    };

	typedef std::vector<atack_t> atacks_t;

	struct inc_t
	{
		unsigned & val;
		inc_t(unsigned & _val):val(_val) {++val;}
		~inc_t(){--val;}
	};

	struct maxn_near_point_pr : public near_point_pr
	{
		maxn_near_point_pr(const point& _c) : near_point_pr(_c){}
		inline bool operator()(const npoint& a,const npoint& b) const
		{
			if(a.n!=b.n)return a.n>b.n;
			return near_point_pr::operator()(a,b);
		}

	};

	
	void set_attack_moves(const atacks_t& src,npoints_t& res);
	void set_open_moves(const atacks_t& src,npoints_t& res,unsigned open_count);
	void make_unique(npoints_t& pts);
	void make_unique(points_t& pts);
	void erase_from_sorted_points(points_t& pts,const point& p);
	void sort_maxn_and_near_zero(npoints_t& pts);

	void increment_duplicate(npoints_t& dst,const npoints_t& src);
	
	template<class Points>
	void exclude_exists(const npoints_t& src,Points& dst);
	
	template<class DstPoints,class SrcPoints>
		inline void add(DstPoints& dst,const SrcPoints& src){dst.insert(dst.end(),src.begin(),src.end());}


	struct temporary_state : public temporary_step
	{
		wsplayer_t& player;
		temporary_state(wsplayer_t& _player,const step_t& st);
		~temporary_state();
	};

	class state_t
	{
		state_t(const state_t&);
		void operator=(const state_t&);
	public:
		wsplayer_t& player;
		points_t empty_points;
		points_t tmp_points;

		points_t make_five_krestik;
		points_t make_five_nolik;
		
		state_t(wsplayer_t& _player) : player(_player){}
		void init_zero();
		void state_from(const state_t& prev_state);
		void state_from_empty_points(const state_t& prev_state);
		void state_from_make_five(const state_t& prev_state);

		inline points_t& get_make_five(Step cl){return (cl==st_krestik)? make_five_krestik:make_five_nolik;}
		inline const points_t& get_make_five(Step cl) const{return (cl==st_krestik)? make_five_krestik:make_five_nolik;}
	};

	typedef boost::shared_ptr<state_t> state_ptr;
	typedef std::vector<state_ptr> states_t;

	void find_move_to_make_five(Step cl,const field_t& field,points_t& results);
	void add_move_to_make_five(const step_t& st,const field_t& field,points_t& results);
	void check_five_line(const step_t& st,const field_t& field,int dx,int dy,points_t& results);

	void find_move_to_open_four(const points_t& pts,Step cl,const field_t& field,atacks_t& results);
	bool check_open_four_line(const step_t& st,const field_t& field,int dx,int dy,point& left,point& right);

	void find_move_to_close_four(const points_t& pts,Step cl,const field_t& field,atacks_t& results);
	bool check_close_four_line(const step_t& st,const field_t& field,int dx,int dy,point& empty);

	void find_move_to_open_three(const points_t& pts,Step cl,const field_t& field,atacks_t& results);
	bool check_open_three_line(const step_t& st,const field_t& field,int dx,int dy,point* open);
} }//namespace Gomoku

#include "wsplayer_common.inl"

#endif

