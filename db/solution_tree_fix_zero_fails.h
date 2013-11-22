#ifndef solution_tree_fix_zero_failsH
#define solution_tree_fix_zero_failsH
#include "solution_tree.h"

namespace Gomoku
{
    struct fix_zero_deep_fails_point_checker
    {
        points_t& pts;
        steps_t& cur_key;
        ObjectProgress::log_generator& lg;
        
        fix_zero_deep_fails_point_checker(ObjectProgress::log_generator& _lg,points_t& _pts,steps_t& _cur_key) : lg(_lg),pts(_pts),cur_key(_cur_key)
        {
        }

        bool operator()(const npoint& p);
    };

    class fix_zero_deep_fails : public sol_state_visitor_pr
    {
        ObjectProgress::log_generator lg;
        points_t pts;
        const steps_t* cur_key;

        fix_zero_deep_fails(const fix_zero_deep_fails&);
        void operator=(const fix_zero_deep_fails&);
    public:
        unsigned long long fixed_count;
        
        fix_zero_deep_fails() : lg(true)
        {
            fixed_count=0;
        }

        bool on_change_node(sol_state_t& val);
    };
}//namespace

#endif
