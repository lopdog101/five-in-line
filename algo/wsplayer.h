#ifndef gomoku_wsplayerH
#define gomoku_wsplayerH
#include "game.h"
#include <boost/shared_ptr.hpp>

namespace Gomoku
{
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

	//Игрок, основанный на поиске решения в ширину
	class wsplayer_t : public iplayer_t
	{
    public:
		static unsigned stored_deep;
		static unsigned def_lookup_deep;
		static unsigned treat_deep;

		static unsigned nodes_count;
	public:
		enum Result{r_fail=-1,r_neitral=0,r_win=1};

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

		class item_t;
		typedef boost::shared_ptr<item_t> item_ptr;
		typedef std::vector<item_ptr> items_t;

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
		
		class item_t : public step_t
		{
			item_t(const item_t&);
			void operator=(const item_t&);
		protected:

            template<class Points>
			void add_neitrals(const Points& pts);

			void clear();

			Result process_predictable_move(bool need_fill_neitrals,unsigned lookup_deep);
			Result process_treat_sequence(bool need_fill_neitrals,unsigned lookup_deep);
			Result process_neitrals(bool need_fill_neitrals,unsigned lookup_deep,unsigned from=0);
			void drop_neitrals_and_fail_child(unsigned generation=0);
			Result process_deep_stored();
		public:
			wsplayer_t& player;

			items_t neitrals;
			item_ptr win;
			item_ptr fail;

			item_t(wsplayer_t& _player,const step_t& s) : step_t(s),player(_player){++nodes_count;}
			item_t(wsplayer_t& _player,const Gomoku::point& p,Step s) : step_t(s,p.x,p.y),player(_player){++nodes_count;}
			~item_t(){--nodes_count;}

			item_ptr get_next_step() const;
			unsigned get_chain_depth() const;

			Result process(bool need_fill_neitrals,unsigned lookup_deep);
			Result process_deep_common();
		};

		class wide_item_t : public item_t
		{
		protected:
			void process(bool need_fill_neitrals,unsigned lookup_deep);
			void process_deep_stored();
		public:
			wide_item_t(wsplayer_t& _player,const step_t& s) : item_t(_player,s){}
			wide_item_t(wsplayer_t& _player,const Gomoku::point& p,Step s) : item_t(_player,p,s){}

			items_t wins;
			items_t fails;

			void process_deep_common();
		};

		struct temporary_state : public temporary_step
		{
			wsplayer_t& player;
			temporary_state(wsplayer_t& _player,const step_t& st) : 
			  player(_player),temporary_step(_player.field,st,st.step){player.increase_state();}
			  ~temporary_state(){player.decrease_state();}
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

	private:
		field_t field;
		unsigned long long predict_processed;
		unsigned predict_deep;

        states_t states;
		unsigned current_state;

		void init_states();
		inline state_t& get_state(){return *states[current_state];}
		inline const state_t& get_state() const {return *states[current_state];}
		void increase_state();
		inline void decrease_state(){--current_state;}

		void find_move_to_make_five(Step cl,points_t& results) const;
		void add_move_to_make_five(const step_t& st,points_t& results) const;
		static void check_five_line(const step_t& st,const field_t& field,int dx,int dy,points_t& results);
		void find_move_to_open_four(const points_t& pts,Step cl,atacks_t& results) const;
		static bool check_open_four_line(const step_t& st,const field_t& field,int dx,int dy,point& left,point& right);
		void find_move_to_close_four(const points_t& pts,Step cl,atacks_t& results) const;
		static bool check_close_four_line(const step_t& st,const field_t& field,int dx,int dy,point& empty);
		void find_move_to_open_three(const points_t& pts,Step cl,atacks_t& results) const;
		static bool check_open_three_line(const step_t& st,const field_t& field,int dx,int dy,point* open);

        void find_treats(const points_t& empty_points,treats_t& res,Step cl,unsigned steps_to_win);
		typedef bool (* treat_f)(const step_t& st,const field_t& field,int dx,int dy,treat_t& tr);
		void find_treats(const points_t& pts,Step cl,treats_t& results,treat_f f) const;
		void find_two_way_treats(const points_t& pts,Step cl,treats_t& results,treat_f f) const;
		static bool check_five_line(const step_t& st,const field_t& field,int dx,int dy,treat_t& tr);
		static bool check_open_four_line(const step_t& st,const field_t& field,int dx,int dy,treat_t& tr);
		static bool check_close_four_line(const step_t& st,const field_t& field,int dx,int dy,treat_t& tr);
		static bool check_four_line_hole_inside(const step_t& st,const field_t& field,int dx,int dy,treat_t& tr);
		static bool check_open_three_line_two_cost(const step_t& st,const field_t& field,int dx,int dy,treat_t& tr);
		static bool check_open_three_line_three_cost(const step_t& st,const field_t& field,int dx,int dy,treat_t& tr);

		static bool is_on_same_line(const point& a,const point& b);
		static void make_step_dx_dy(const point& a,const point& b,int& dx,int& dy);
		static bool is_on_same_line_and_can_make_five(const field_t& field,const point& a,const point& b,Step cl);

		static std::string print_chain(item_ptr root);
	public:
		item_ptr root;

		wsplayer_t();

		void begin_game();
		void delegate_step();
		void solve();

#ifdef USE_XML
		void pack(Xpat::ipacker_t& root_node,bool process_type=true) const;
		void unpack(const Xpat::ipacker_t& root_node,bool process_type=true);
#endif

		POLIMVAR_IMPLEMENT_CLONE(wsplayer_t )
	};


}//namespace Gomoku

#ifdef USE_XML
namespace Xpat
{
template <> inline std::string xml_type_name< Gomoku::wsplayer_t>::get(){return "width_search_player";};

}//
#endif
#endif

