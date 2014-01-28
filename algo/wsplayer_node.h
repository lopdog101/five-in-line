#ifndef gomoku_wsplayer_nodeH
#define gomoku_wsplayer_nodeH
#include "wsplayer_common.h"

namespace Gomoku { namespace WsPlayer
{
	class item_t;
	typedef boost::shared_ptr<item_t> item_ptr;
	typedef std::vector<item_ptr> items_t;

	class selected_wins_childs
	{
	protected:
		unsigned current_chain_depth;
		items_t vals;
		item_ptr best_val;
	public:
		selected_wins_childs();
		virtual ~selected_wins_childs(){}
		
		virtual void add(const item_ptr& val);
		virtual void clear();
		inline const item_ptr& get_best() const{return best_val;}
		inline bool empty() const{return vals.empty();}
		inline const items_t& get_vals() const{return vals;}
		inline unsigned get_chain_depth() const{return current_chain_depth;}
		inline size_t size() const{return vals.size();}
	};

	class selected_fails_childs : public selected_wins_childs
	{
		npoints_t win_hints;

		void add_wins_hint(const item_ptr& val);
	public:
		virtual void add(const item_ptr& val);
		virtual void clear();
		inline const npoints_t& get_win_hins() const{return win_hints;}
	};


	class existing_npoints_sort_pr
	{
		const npoints_t& ref;
		less_point_pr pr;
	public:
		existing_npoints_sort_pr(const npoints_t& _ref) : ref(_ref){}
		
		bool operator()(const item_ptr& a,const item_ptr& b)const;
	};

	struct win_rate_cmp_pr
	{
		bool operator()(const item_ptr& a,const item_ptr& b)const;
	};

	class item_t : public step_t
	{
		item_t(const item_t&);
		void operator=(const item_t&);
	protected:
		items_t neitrals;
		selected_wins_childs wins;
		selected_fails_childs fails;

        template<class Points>
		void add_neitrals(const Points& pts);

		Result add_and_process_neitrals(const npoints_t& pts,unsigned lookup_deep,unsigned drop_generation);

		void clear();

		Result process_predict_treat_sequence(bool need_fill_neitrals,unsigned lookup_deep);
		Result process_predictable_move(bool need_fill_neitrals,unsigned lookup_deep);
		Result process_treat_sequence();
		virtual Result process_neitrals(bool need_fill_neitrals,unsigned lookup_deep,unsigned from=0,const item_t* parent_node=0);
		void drop_neitrals_and_fail_child(unsigned generation=0);
		Result process_deep_stored();
        bool is_defence_five_exists() const;
		void reorder_neitrals_as_neighbor_win_hint(unsigned from,const item_t* parent_node);
		size_t select_ant_neitral(const item_t* parent_node);

		Result process_deep_ant();
		void calculate_deep_wins_fails();
		Result solve_ant(const item_t* parent_node=0);
	public:
		wsplayer_t& player;
		long long deep_wins_count;
		long long deep_fails_count;

		item_t(wsplayer_t& _player,const step_t& s);
		item_t(wsplayer_t& _player,const Gomoku::point& p,Step s);
		~item_t(){--nodes_count;}

		item_ptr get_next_step() const;
		item_ptr get_win_fail_step() const;
		unsigned get_chain_depth() const;

		Result process(bool need_fill_neitrals,unsigned lookup_deep,const item_t* parent_node=0);
		Result process_deep_common();

		inline void add_win(const item_ptr& val){wins.add(val);}
		inline void add_fail(const item_ptr& val){fails.add(val);}
		inline const selected_wins_childs& get_wins() const{return wins;}
		inline const selected_fails_childs& get_fails() const{return fails;}
		inline const items_t& get_neitrals() const{return neitrals;}
        inline double get_win_rate() const{return static_cast<double>(deep_wins_count+1)/(deep_fails_count+1);}
	};

	class wide_item_t : public item_t
	{
	protected:
		void process(bool need_fill_neitrals,unsigned lookup_deep);
		void process_deep_stored();
		virtual Result process_neitrals(bool need_fill_neitrals,unsigned lookup_deep,unsigned from=0,const item_t* parent_node=0);
	public:
		wide_item_t(wsplayer_t& _player,const step_t& s) : item_t(_player,s){}
		wide_item_t(wsplayer_t& _player,const Gomoku::point& p,Step s) : item_t(_player,p,s){}

		void process_deep_common();
	};


	std::string print_chain(item_ptr root);
	void items2points(const items_t& items,points_t& res);
	void items2depth_npoints(const items_t& items,npoints_t& res);
} }//namespace Gomoku

#endif

