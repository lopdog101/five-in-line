#ifndef solution_tree_fix_zero_failsH
#define solution_tree_fix_zero_failsH
#include "solution_tree.h"

namespace Gomoku
{
    class fix_zero_deep_fails : public sol_state_visitor_pr
    {
        ObjectProgress::log_generator lg;
        points_t pts;
        const steps_t* cur_key;
        solution_tree_t& tree;

        fix_zero_deep_fails(const fix_zero_deep_fails&);
        void operator=(const fix_zero_deep_fails&);
    public:
        unsigned long long fixed_count;
        
        fix_zero_deep_fails(solution_tree_t& _tree) : lg(true) , tree(_tree)
        {
            fixed_count=0;
        }

        bool on_exit_node(sol_state_t& val);
    };

    class fix_not_solved_wins : public sol_state_width_pr
    {
        ObjectProgress::log_generator lg;
        solution_tree_t& tree;

        fix_not_solved_wins(const fix_not_solved_wins&);
        void operator=(const fix_not_solved_wins&);
    public:
        unsigned long long fixed_count;

        fix_not_solved_wins(solution_tree_t& _tree) : lg(true) , tree(_tree)
        {
            fixed_count=0;
        }

        bool on_enter_node(sol_state_t& val);
    };

}//namespace

#endif
