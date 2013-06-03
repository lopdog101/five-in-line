#ifndef gomoku_wsplayer_nodeH
#define gomoku_wsplayer_nodeH
#include "wsplayer_common.h"

namespace Gomoku { namespace WsPlayer
{
	class item_t;
	typedef boost::shared_ptr<item_t> item_ptr;
	typedef std::vector<item_ptr> items_t;

	class item_t : public step_t
	{
		item_t(const item_t&);
		void operator=(const item_t&);
	protected:

        template<class Points>
		void add_neitrals(const Points& pts);

		void clear();

		Result process_predictable_move(bool need_fill_neitrals,unsigned lookup_deep);
		Result process_treat_sequence();
		Result process_neitrals(bool need_fill_neitrals,unsigned lookup_deep,unsigned from=0);
		void drop_neitrals_and_fail_child(unsigned generation=0);
		Result process_deep_stored();

        bool is_defence_five_exists() const;
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


	std::string print_chain(item_ptr root);
} }//namespace Gomoku

#endif

