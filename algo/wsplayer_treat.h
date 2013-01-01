#ifndef gomoku_wsplayer_treatH
#define gomoku_wsplayer_treatH
#include "wsplayer_node.h"

namespace Gomoku { namespace WsPlayer
{
	struct treat_t
	{
		point gain;
		point cost[5];
		unsigned cost_count;
		point rest[5];
		unsigned rest_count;

		bool win;

		treat_t(){cost_count=0;rest_count=0;win=false;}
		inline unsigned get_steps_to_win() const{return 5-rest_count;}
		inline void add_cost(const point& pt){cost[cost_count++]=pt;}
		inline void add_rest(const point& pt){rest[rest_count++]=pt;}

		bool is_one_of_rest(const point& p) const;
		bool is_one_of_cost(const point& p) const;
		bool is_conflict(const treat_t& b) const;
		bool is_gain_conflict_with_rhs_gain_and_cost(const treat_t& rhs) const;

		void sort();
	};

	class temporary_treat_state
	{
		field_t& fd;
		rect rc;
		treat_t tr;
	public:
		temporary_treat_state(field_t& _fd,const treat_t& _tr,Step st) : fd(_fd),tr(_tr),rc(_fd.get_bound())
		{
			fd.add(tr.gain,st);
			for(unsigned i=0;i<tr.cost_count;i++)
				fd.add(tr.cost[i],other_step(st));

		}

		~temporary_treat_state()
		{
			fd.pop(rc);
			for(unsigned i=0;i<tr.cost_count;i++)
				fd.pop(rc);
		}
	};

	typedef std::vector<treat_t> treats_t;

	class treat_node_t;
	typedef boost::shared_ptr<treat_node_t> treat_node_ptr;
	typedef std::vector<treat_node_ptr> treat_nodes_t;

	struct treat_filter_t
	{
		virtual ~treat_filter_t(){}
		virtual unsigned get_steps_to_win() const{return 5;}
		virtual bool operator()(const treat_t&tr) const{return true;}
	};
	
	class step_treat_filter_t : public treat_filter_t
	{
	protected:
		unsigned steps_to_win;
	public:
		step_treat_filter_t(unsigned _steps_to_win){steps_to_win=_steps_to_win;}
		unsigned get_steps_to_win() const{return steps_to_win;}
		bool operator()(const treat_t&tr) const{return tr.get_steps_to_win()<=steps_to_win;}
	};
	
	class revert_treat_filter_t : public step_treat_filter_t
	{
		const treat_t& treat_that_could_be_interrupted;
	public:
		revert_treat_filter_t(const treat_t& _treat_that_could_be_interrupted) : 
			step_treat_filter_t(_treat_that_could_be_interrupted.get_steps_to_win()),
			treat_that_could_be_interrupted(_treat_that_could_be_interrupted){}
			
		bool operator()(const treat_t&tr) const
		{
			if(tr.get_steps_to_win()<steps_to_win)return true;
			return tr.is_gain_conflict_with_rhs_gain_and_cost(treat_that_could_be_interrupted);
		}
	};

	class treat_node_t : public treat_t
	{
		wsplayer_t& player;

		void build_tree_same_line(const treat_t& b,Step cl,bool only_win,const treat_filter_t& tf,unsigned deep);
	public:
		treat_nodes_t childs;

		treat_node_t(wsplayer_t& _player) : player(_player){}
		void build_tree(Step cl,bool only_win,const treat_filter_t& tf,unsigned deep=(unsigned)-1);

		item_ptr check_tree(Step cl);
		item_ptr check_tree_one_step(treat_node_t& gr,Step cl);
		item_ptr icheck_tree_one_step(treat_node_t& gr,Step cl);
		bool contra_steps_exists(treat_node_t& attack_group,Step cl,item_ptr& res);
		bool contra_steps_exists_one_step(treat_node_t& attack_group,Step cl,item_ptr& res,treat_node_t& gr);
		void remove_revertable_path(const treat_node_t& revert_treat);
		bool is_tree_conflict(const treat_t& tr) const;
		bool is_tree_gain_conflict_with_rhs_gain_and_cost(const treat_t& tr) const;
		void group_by_step();
		void sort_by_min_deep();

		treat_node_ptr clone() const;
		unsigned max_deep() const;
		unsigned min_deep() const;
		unsigned get_childs_min_steps_to_win() const;
	};

	struct treat_step_pr : public less_point_pr
	{
		inline bool operator()(const treat_node_ptr& a,const treat_node_ptr& b) const
		{
			return less_point_pr::operator ()(a->gain,b->gain);
		}
	};

	struct treat_min_deep_pr
	{
		inline bool operator()(const treat_node_ptr& a,const treat_node_ptr& b) const
		{
			return a->min_deep()<b->min_deep();
		}
	};

	bool is_on_same_line(const point& a,const point& b);
	void make_step_dx_dy(const point& a,const point& b,int& dx,int& dy);
	bool is_on_same_line_and_can_make_five(const field_t& field,const point& a,const point& b,Step cl);

	std::string print_treat(treat_t& tr);


	void find_treats(const points_t& empty_points,treats_t& res,Step cl,const field_t& field,unsigned steps_to_win);
	typedef bool (* treat_f)(const step_t& st,const field_t& field,int dx,int dy,treat_t& tr);
	void find_treats(const points_t& pts,Step cl,const field_t& field,treats_t& results,treat_f f);
	void find_two_way_treats(const points_t& pts,Step cl,const field_t& field,treats_t& results,treat_f f);
	bool check_five_line(const step_t& st,const field_t& field,int dx,int dy,treat_t& tr);
	bool check_open_four_line(const step_t& st,const field_t& field,int dx,int dy,treat_t& tr);
	bool check_four_line_hole_inside(const step_t& st,const field_t& field,int dx,int dy,treat_t& tr);
	bool check_four_line_zero_left_hole_right(const step_t& st,const field_t& field,int dx,int dy,treat_t& tr);
	bool check_open_three_line_two_cost(const step_t& st,const field_t& field,int dx,int dy,treat_t& tr);
	bool check_open_three_line_three_cost(const step_t& st,const field_t& field,int dx,int dy,treat_t& tr);

} }//namespace Gomoku

#endif

