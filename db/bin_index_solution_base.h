#ifndef bin_index_solution_baseH
#define bin_index_solution_baseH
#include "solution_tree.h"
#include "bin_index.h"

namespace Gomoku
{
	class bin_index_solution_base_t :  public isolution_tree_base_t
	{
		mutable bin_indexes_t indexes;

        void width_first_search_from_bottom_to_top(sol_state_width_pr& pr,ibin_index_t& idx);
        size_t retreive_max_level() const;
	public:
		bin_index_solution_base_t(const std::string& base_dir) : indexes(base_dir,3){}

		virtual bool get(sol_state_t& res) const;
		virtual void set(const sol_state_t& val);
		virtual steps_t get_root_key() const;
        virtual void width_first_search_from_bottom_to_top(sol_state_width_pr& pr);
	};

}//namespace

#endif
