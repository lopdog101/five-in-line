#include "solution_tree_fix_zero_fails.h"
#include <boost/filesystem/operations.hpp>
#include "../extern/binary_find.h"

namespace Gomoku
{
    bool fix_zero_deep_fails::on_change_node(sol_state_t& val)
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
        
        fix_zero_deep_fails_point_checker pr(lg,pts,val.key);
        tree_fails.erase(std::remove_if(tree_fails.begin(),tree_fails.end(),pr),tree_fails.end());

        return tree_fails.size()!=mi;
    }
    
    bool fix_zero_deep_fails_point_checker::operator()(const npoint& p)
    {
        if(p.n!=0)return false;

        if(std::binary_search(pts.begin(),pts.end(),p,less_point_pr()) )
            return true;
        
        lg<<"fix_zero_deep_fails_point_checker::operator(): p="<<print_point(p)<<" not found in state="<<print_steps(cur_key);
        return false;
    }
}//namespace
