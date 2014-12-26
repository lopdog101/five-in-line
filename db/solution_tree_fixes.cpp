#include "solution_tree_fixes.h"
#include <boost/filesystem/operations.hpp>
#include "../extern/binary_find.h"

namespace Gomoku
{
    bool fix_zero_deep_fails::on_exit_node(sol_state_t& val)
    {
        npoints_t& tree_fails=val.tree_fails;
        cur_key=&val.key;

        if(tree_fails.empty())
            return false;

        size_t mi=tree_fails.size();

        pts.resize(0);
        pts.reserve(mi);

        for(unsigned i=0;i<mi;i++)
        {
            const npoint& p=tree_fails[i];
            if(p.n>0)pts.push_back(p);
        }

        if(pts.size()==mi)
            return false;
        
        lg<<"fix_zero_deep_fails::on_change_node(): key="<<print_steps(val.key);
        
        make_unique(pts);
        bool ret=false;

        for(size_t ii=mi;ii>0;ii--)
        {
            size_t i=ii-1;
            npoint& p=tree_fails[i];

            if(p.n!=0)
                continue;

            if(std::binary_search(pts.begin(),pts.end(),p,less_point_pr()) )
            {
                tree_fails.erase(tree_fails.begin()+i);
                ret=true;
                continue;
            }

            sol_state_t ch;
            ch.key=val.key;
            ch.key.push_back(step_t(next_color(ch.key.size()),p.x,p.y));

            if(!tree.get(ch))
            {
                lg<<"fix_zero_deep_fails_point_checker::operator(): p="<<print_point(p)<<" not found in state="<<print_steps(val.key);
                continue;
            }

            p.n=ch.max_fail_chain()+1;
            ret=true;
        }

        if(ret)fixed_count++;
        
        return ret;
    }

}//namespace
