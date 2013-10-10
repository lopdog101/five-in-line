#include "wsplayer_treat.h"
#include <stdexcept>

#  include "../extern/pair_comparator.h"
#  include "../extern/binary_find.h"
#  include "../extern/object_progress.hpp"

#include "wsplayer.h"

namespace Gomoku { namespace WsPlayer
{

/////////////////////////////////////////////////////////////////////////////////////
void treat_node_t::build_tree(Step cl,bool only_win,const treat_filter_t& tf,unsigned deep)
{
	treats_t treats;
    find_treats_for_build_tree(cl,tf,treats);

	for(unsigned i=0;i<treats.size();i++)
	{
		player.check_cancel();

		const treat_t& t=treats[i];
		if(!tf(t))continue;

		if(rest_count==0||t.is_one_of_rest(gain))
		{
			treat_node_ptr ch(new treat_node_t(player));
			static_cast<treat_t&>(*ch)=t;
			if(!ch->win&&deep>0)
			{
				temporary_treat_state hld_tr(player.field,t,cl);
				ch->build_tree(cl,only_win,tf,deep-1);
			}

			if(!only_win||ch->win)childs.push_back(ch);
			if(ch->win)win=true;
		}

		if(deep>1&&rest_count!=0&&!t.win&&!t.is_one_of_rest(gain)&&
			is_on_same_line_and_can_make_five(player.field,gain,t.gain,cl))
		{
			build_tree_same_line(t,cl,only_win,tf,deep-1);
		}
	}
}

void treat_node_t::build_tree_same_line(const treat_t& b,Step cl,bool only_win,const treat_filter_t& tf,unsigned deep)
{
	temporary_treat_state hld_b(player.field,b,cl);

	treats_t treats;
    find_treats_for_build_tree(cl,tf,treats);

	for(unsigned i=0;i<treats.size();i++)
	{
		player.check_cancel();

		const treat_t& t=treats[i];
		if(!tf(t))continue;

		if(!t.is_one_of_rest(gain)||!t.is_one_of_rest(b.gain))
			continue;

		treat_node_ptr ch(new treat_node_t(player));
		static_cast<treat_t&>(*ch)=t;
		if(!ch->win)
		{
			temporary_treat_state hld_tr(player.field,t,cl);
			ch->build_tree(cl,only_win,tf,deep-1);
		}

		if(!only_win||ch->win)
		{
			treat_node_ptr chb(new treat_node_t(player));
			static_cast<treat_t&>(*chb)=b;
			chb->childs.push_back(ch);
			if(ch->win)
			{
				chb->win=true;
			}

			childs.push_back(chb);
		}

		if(ch->win)
		{
			win=true;
		}
	}
}

void treat_node_t::find_treats_for_build_tree(Step cl,const treat_filter_t& tf,treats_t& treats)
{
	static points_t empty_points;
	player.field.get_empty_around(empty_points,2);

	find_one_step_win_treats(empty_points,treats,cl,player.field);

    treats_t oponent_make_five_treats;
    find_one_step_win_treats(empty_points,oponent_make_five_treats,other_color(cl),player.field);

    points_t oponent_make_five_points;
    treats_gains_and_costs_to_points(oponent_make_five_treats,oponent_make_five_points);
    
    if(oponent_make_five_points.size()>2)
        return;

    if(!oponent_make_five_points.empty())
        empty_points=oponent_make_five_points;

	find_more_than_one_steps_win_treats(empty_points,treats,cl,player.field,tf.get_steps_to_win());
}



item_ptr treat_node_t::check_tree(Step cl,bool refind_one_step)
{
	if(childs.empty())return item_ptr();

    group_by_step();

	for(unsigned i=0;i<groups.size();i++)
	{
		player.check_cancel();
		treat_node_t& gr=*groups[i];

		item_ptr r=gr.check_tree_one_group_step(cl,refind_one_step);
		if(r)return r;
	}

	return item_ptr();
}

item_ptr treat_node_t::check_tree_one_group_step(Step cl,bool refind_one_step)
{
	field_t& field=player.field;

    if(refind_one_step)
	{
		Step cur_cl=field.at(gain);

		if(cur_cl==other_color(cl)) return item_ptr();

		if(cur_cl==cl)
		{
            player.increase_treat_check_rebuild_tree_count();

            unsigned cur_deep=max_deep();

#ifdef PRINT_TREAT_PERFOMANCE
			ObjectProgress::log_generator lg(true);
			ObjectProgress::perfomance perf;
#endif
			treat_node_ptr tr(new treat_node_t(player));
			tr->build_tree(cl,true,treat_filter_t(),cur_deep);

#ifdef PRINT_TREAT_PERFOMANCE
			lg<<"check_tree_one_group_step()1 build_tree():  cur_deep="<<cur_deep
				<<" res_deep="<<tr->max_deep()<<" win="<<tr->win<<" time="<<perf;
			perf.reset();
#endif

			if(!tr->win)return item_ptr();

			return tr->check_tree(cl,false);
		}
	}

	if(get_childs_min_steps_to_win()==1)
	{
		return item_ptr(new item_t(player,gain,cl ));
	}

	if(refind_one_step&&is_one_step_treat_exists(gain,player.field,cl))
	{
		return item_ptr(new item_t(player,gain,cl ));
	}

	temporary_step hld(field,gain,cl);
	
	item_ptr c=icheck_tree_one_group_step(cl,refind_one_step);
	if(!c)return item_ptr();

	item_ptr ret(new item_t(player,gain,cl ));
	ret->fail=c;

	return ret;
}

item_ptr treat_node_t::icheck_tree_one_group_step(Step cl,bool refind_one_step)
{
	field_t& field=player.field;

	item_ptr max_r;
	unsigned max_depth=0;

    player.increase_treat_check_count();

	if(contra_steps_exists(cl,max_r))
	{
		if(!max_r)return item_ptr();
		max_depth=max_r->get_chain_depth();
	}

	for(unsigned k=0;k<childs.size();k++)
	{
		treat_node_t& ch=*childs[k];

        if(refind_one_step)
		{
			if(ch.one_of_cost_have_color(field,other_color(cl)) )
				continue;

			if(ch.one_of_cost_have_color(field,cl) )
			{
                player.increase_treat_check_rebuild_tree_count();

				unsigned cur_deep=max_deep();
                if(cur_deep>3)cur_deep-=3;

#ifdef PRINT_TREAT_PERFOMANCE
				ObjectProgress::log_generator lg(true);
				ObjectProgress::perfomance perf;
#endif
				treat_node_ptr tr(new treat_node_t(player));
				tr->build_tree(cl,true,treat_filter_t(),cur_deep);

#ifdef PRINT_TREAT_PERFOMANCE
				lg<<"icheck_tree_one_group_step()1 build_tree():  cur_deep="<<cur_deep
					<<" res_deep="<<tr->max_deep()<<" win="<<tr->win<<" time="<<perf;
				perf.reset();
#endif
				if(!tr->win)return item_ptr();

				return tr->check_tree(cl,false);
			}
		}

		if(ch.childs.empty())
		{
			item_ptr cr;
			
			if(ch.get_steps_to_win()!=2||ch.cost_count!=2)
				throw std::runtime_error("check_tree() open four expected");
				
			item_ptr f(new item_t(player,ch.cost[0],other_color(cl) ));
			item_ptr w(new item_t(player,ch.cost[1],cl ));
			f->win=w;

			unsigned depth=f->get_chain_depth();
			if(!max_r||depth>max_depth)
			{
				max_r=f;
				max_depth=depth;
			}
		}
		else
		{
			for(unsigned j=0;j<ch.cost_count;j++)
			{
				temporary_step hld_c(field,ch.cost[j],other_color(cl));

				item_ptr cf=ch.check_tree(cl,refind_one_step);
				if(!cf)return item_ptr();

				unsigned depth=cf->get_chain_depth()+1;
				if(!max_r||depth>max_depth)
				{
					max_r=item_ptr(new item_t(player,ch.cost[j],other_color(cl) ));
					max_r->win=cf;
		
					max_depth=depth;
				}
			}
		}
	}

	return max_r;
}

bool treat_node_t::contra_steps_exists(Step cl,item_ptr& res)
{
	unsigned min_steps_to_win=get_childs_min_steps_to_win();

	treat_node_ptr ctree(new treat_node_t(player));
	ctree->build_tree(other_color(cl),false,step_treat_filter_t(min_steps_to_win-1),0);
	ctree->group_by_step();

	unsigned max_depth=0;

	for(unsigned i=0;i<ctree->groups.size();i++)
	{
		player.check_cancel();
		treat_node_t& gr=*ctree->groups[i];

		item_ptr cres=contra_steps_exists_one_step(cl,gr);

		if(!cres)
		{
			res.reset();
			return true;
		}

		unsigned depth=cres->get_chain_depth();

		if(!res||depth>max_depth)
		{
			res=cres;
			max_depth=depth;
		}
	}

	return res!=item_ptr();
}

item_ptr treat_node_t::contra_steps_exists_one_step(Step cl,treat_node_t& gr)
{
	if(gr.get_childs_min_steps_to_win()==1)
	{
		return item_ptr();
	}

	field_t& field=player.field;
	temporary_step hld(field,gr.gain,other_color(cl));

	item_ptr max_r;
	unsigned max_depth=0;

	for(unsigned k=0;k<gr.childs.size();k++)
	{
		treat_node_t& ch=*gr.childs[k];

        //check if one of anwer to contra step make five
		for(unsigned j=0;j<ch.cost_count;j++)
		{
			if(!is_one_step_treat_exists(ch.cost[j],player.field,cl))
				continue;

			item_ptr c(new item_t(player,gr.gain,other_color(cl) ));
			item_ptr ca(new item_t(player,ch.cost[j],cl ));

			c->win=ca;
			return c;
		}

		for(unsigned j=0;j<ch.cost_count;j++)
		{
			temporary_step hld_answer(field,ch.cost[j],cl);

			item_ptr res=icheck_tree_one_group_step(cl,true);
			if(!res)continue;
			
			item_ptr c(new item_t(player,gr.gain,other_color(cl) ));
			item_ptr ca(new item_t(player,ch.cost[j],cl ));

			c->win=ca;
			ca->fail=res;
			return c;
		}
	}

	return item_ptr();
}




void treat_node_t::group_by_step()
{
	if(!groups.empty()) return;
	if(childs.empty())return;

	std::sort(childs.begin(),childs.end(),treat_step_pr());

	treat_node_ptr cr(new treat_node_t(player));
	cr->gain=childs.front()->gain;
	cr->childs.push_back(childs.front());

	for(unsigned i=1;i<childs.size();i++)
	{
		treat_node_ptr& p=childs[i];
		if(cr->gain!=p->gain)
		{
			groups.push_back(cr);
			cr=treat_node_ptr(new treat_node_t(player));
			cr->gain=p->gain;
		}
        
		cr->childs.push_back(p);
	}

	groups.push_back(cr);
	std::sort(groups.begin(),groups.end(),treat_min_deep_pr());
}

treat_node_ptr treat_node_t::clone() const
{
	treat_node_ptr res(new treat_node_t(player));
	static_cast<treat_t&>(*res)=*this;
	res->childs.resize(childs.size());

	for(unsigned i=0;i<childs.size();i++)
		res->childs[i]=childs[i]->clone();

    return res;
}

unsigned treat_node_t::max_deep() const
{
	unsigned ret=0;
	for(unsigned i=0;i<childs.size();i++)
	{
		unsigned d=childs[i]->max_deep();
		if(d>ret)ret=d;
	}

	return ret+1;
}

unsigned treat_node_t::min_deep() const
{
	unsigned ret=0;
	for(unsigned i=0;i<childs.size();i++)
	{
		unsigned d=childs[i]->min_deep();
		if(i==0||d<ret)ret=d;
	}

	return ret+1;
}

unsigned treat_node_t::get_childs_min_steps_to_win() const
{
	unsigned ret=0;
	for(unsigned i=0;i<childs.size();i++)
	{
		unsigned d=childs[i]->get_steps_to_win();
		if(i==0||d<ret)ret=d;
	}

	return ret;
}



void treat_node_t::remove_revertable_path(const treat_node_t& revert_treat)
{
	if(revert_treat.is_tree_gain_conflict_with_rhs_gain_and_cost(*this))
	{
		childs.clear();
		win=false;
		return;
	}

	if(childs.empty())return;

	for(unsigned i=0;i<childs.size();i++)
	{
		treat_node_ptr& t=childs[i];
		t->remove_revertable_path(revert_treat);
		if(!t->win)t.reset();
	}

	childs.erase(std::remove(childs.begin(),childs.end(),treat_node_ptr()),childs.end());
	if(childs.empty())win=false;
}

bool treat_node_t::is_tree_conflict(const treat_t& tr) const
{
	for(unsigned i=0;i<childs.size();i++)
	{
		const treat_node_t& t=*childs[i];
        if(t.is_conflict(tr))
			return true;

		if(t.is_tree_conflict(tr))
			return true;
	}
	return false;
}

bool treat_node_t::is_tree_gain_conflict_with_rhs_gain_and_cost(const treat_t& tr) const
{
	for(unsigned i=0;i<childs.size();i++)
	{
		const treat_node_t& t=*childs[i];
        if(t.is_conflict(tr))
			return true;

		if(t.is_tree_conflict(tr))
			return true;
	}
	return false;
}

////////////////////////////////////////////////////////////////////////////////////

bool treat_t::is_conflict(const treat_t& b) const
{
	if(gain==b.gain)
		return true;

	if(is_one_of_cost(b.gain))
		return true;

	if(b.is_one_of_cost(gain))
		return true;

	for(unsigned j=0;j<cost_count;j++)
		if(b.is_one_of_cost(cost[j]))
			return true;

	return false;
}

bool treat_t::is_gain_conflict_with_rhs_gain_and_cost(const treat_t& rhs) const
{
	if(gain==rhs.gain)
		return true;

	if(rhs.is_one_of_cost(gain))
		return true;

	return false;
}

bool treat_t::is_one_of_rest(const point& p) const
{
	for(unsigned i=0;i<rest_count;i++)
		if(rest[i]==p)return true;
	return false;
}

bool treat_t::is_one_of_cost(const point& p) const
{
	for(unsigned i=0;i<cost_count;i++)
		if(cost[i]==p)return true;
	return false;
}

void treat_t::sort()
{
	std::sort(cost,cost+cost_count,less_point_pr());
	std::sort(rest,rest+rest_count,less_point_pr());
}

bool treat_t::one_of_cost_have_color(const field_t& fl,Step cl) const
{
	for(unsigned i=0;i<cost_count;i++)
	if(fl.at(cost[i])==cl)
		return true;

	return false;
}

/////////////////////////////////////////////////////////////////////
//
//

bool is_on_same_line(const point& a,const point& b)
{
	int dx=b.x-a.x;
	int dy=b.y-a.y;

	if(dx!=0&&std::abs(dx)>4)return false;
	if(dy!=0&&std::abs(dy)>4)return false;
	if(dx!=0&&dy!=0&&std::abs(dx)!=std::abs(dy))return false;

	return true;
}

void make_step_dx_dy(const point& a,const point& b,int& dx,int& dy)
{
	dx=b.x-a.x;
	dy=b.y-a.y;

	if(dx>0)dx=1;
	else if(dx<0)dx=-1;

	if(dy>0)dy=1;
	else if(dy<0)dy=-1;
}


bool is_on_same_line_and_can_make_five(const field_t& field,const point& a,const point& b,Step cl)
{
	if(!is_on_same_line(a,b))return false;

	int dx=0;
	int dy=0;

	make_step_dx_dy(a,b,dx,dy);

	Step s;
	unsigned j=0;
	Step ocl=other_color(cl);

	Gomoku::point p=a;
	for(;j<3;j++)
	{
		p.x+=dx;
		p.y+=dy;
		if(p==b)break;

		s=field.at(p);
		if(s==ocl)return false;
	}
	
	if(j==3)return true;

	p=a;
	for(;j<3;j++)
	{
		p.x-=dx;
		p.y-=dy;

		s=field.at(p);
		if(s==ocl)break;
	}

	if(j==3)return true;

	p=b;
	for(;j<3;j++)
	{
		p.x+=dx;
		p.y+=dy;

		s=field.at(p);
		if(s==ocl)break;
	}

	return j==3;
}

std::string print_treat(treat_t& tr)
{
	points_t rest(tr.rest,tr.rest+tr.rest_count);
	points_t cost(tr.cost,tr.cost+tr.cost_count);
	points_t gain;
	gain.push_back(tr.gain);

	std::string ret="gain: "+print_points(gain)+" cost: "+print_points(cost)+" rest: "+print_points(rest);
	return ret;
}

std::string print_treat_tree(treat_node_t& tr)
{
	std::string res;

	if(tr.childs.empty())
	{
		return res;
	}

	res="\r\n";
	
	std::string offset_str;

	print_treat_tree(*tr.childs.front(),res,offset_str);
	for(size_t i=1;i<tr.childs.size();i++)
	{
		print_treat_tree(*tr.childs[i],res,offset_str);
	}

	return res;
}

void print_treat_tree(treat_node_t& tr,std::string& res,std::string& offset_str)
{
	char tmp[256];
	int len=sprintf(tmp,"(%d,%d)",tr.gain.x,tr.gain.y);
	res+=tmp;

	if(tr.childs.empty())
	{
		res+="\r\n";
		return;
	}

	static const int offset_delta=10;

	if(offset_delta>len)
		res.resize(res.size()+offset_delta-len,' ');

	offset_str.resize(offset_str.size()+offset_delta,' ');

	print_treat_tree(*tr.childs.front(),res,offset_str);
	for(size_t i=1;i<tr.childs.size();i++)
	{
		res+=offset_str;
		print_treat_tree(*tr.childs[i],res,offset_str);
	}

	offset_str.resize(offset_str.size()-offset_delta);
}


/////////////////////////////////////////////////////////////////////
//
//
void find_one_step_win_treats(const points_t& empty_points,treats_t& res,Step cl,const field_t& field)
{
	find_treats(empty_points,cl,field,res,check_five_line);
}

void find_more_than_one_steps_win_treats(const points_t& empty_points,treats_t& res,Step cl,const field_t& field,unsigned steps_to_win)
{
	if(steps_to_win>=2)find_treats(empty_points,cl,field,res,check_open_four_line);
	if(steps_to_win>=2)find_two_way_treats(empty_points,cl,field,res,check_four_line_hole_inside);
	if(steps_to_win>=2)find_two_way_treats(empty_points,cl,field,res,check_four_line_zero_left_hole_right);
	if(steps_to_win>=3)find_treats(empty_points,cl,field,res,check_open_three_line_two_cost);
	if(steps_to_win>=3)find_two_way_treats(empty_points,cl,field,res,check_open_three_line_three_cost);
}



void find_treats(const points_t& pts,Step cl,const field_t& field,treats_t& results,treat_f f)
{
	step_t st;
	st.step=cl;
	treat_t tr;

	for(unsigned i=0;i<pts.size();i++)
	{
		static_cast<point&>(st)=pts[i];

		if(f(st,field,1,0,tr))
			results.push_back(tr);

		if(f(st,field,0,1,tr))
			results.push_back(tr);

		if(f(st,field,1,1,tr))
			results.push_back(tr);

		if(f(st,field,1,-1,tr))
			results.push_back(tr);
	}
}

void find_two_way_treats(const points_t& pts,Step cl,const field_t& field,treats_t& results,treat_f f)
{
	step_t st;
	st.step=cl;
	treat_t tr;

	for(unsigned i=0;i<pts.size();i++)
	{
		static_cast<point&>(st)=pts[i];

		if(f(st,field,1,0,tr))results.push_back(tr);
		if(f(st,field,-1,0,tr))results.push_back(tr);

		if(f(st,field,0,1,tr))results.push_back(tr);
		if(f(st,field,0,-1,tr))results.push_back(tr);

		if(f(st,field,1,1,tr))results.push_back(tr);
		if(f(st,field,-1,-1,tr))results.push_back(tr);

		if(f(st,field,1,-1,tr))results.push_back(tr);
		if(f(st,field,-1,1,tr))results.push_back(tr);
	}
}


bool check_five_line(const step_t& st,const field_t& field,int dx,int dy,treat_t& tr)
{
	tr=treat_t();
	tr.gain=st;
	tr.win=true;
	Gomoku::point p=st;
	unsigned j=0;

	for(;j<4;j++)
	{
		p.x-=dx;
		p.y-=dy;

		Step s=field.at(p);
		if(s!=st.step)break;
		tr.add_rest(p);
	}

	if(j==4)return true;

	p=st;

	for(;j<4;j++)
	{
		p.x+=dx;
		p.y+=dy;

		Step s=field.at(p);
		if(s!=st.step)break;
		tr.add_rest(p);
	}

	return j==4;
}

bool check_open_four_line(const step_t& st,const field_t& field,int dx,int dy,treat_t& tr)
{
	tr=treat_t();
	tr.gain=st;
	tr.win=true;
	Gomoku::point p=st;
	unsigned j=0;

	for(;j<4;j++)
	{
		p.x-=dx;
		p.y-=dy;

		Step s=field.at(p);

		if(s==st.step)
		{
			tr.add_rest(p);
			continue;
		}

		if(s!=st_empty)return false;

		tr.add_cost(p);
		break;
	}

	if(j==4)return false;

	p=st;

	for(;j<4;j++)
	{
		p.x+=dx;
		p.y+=dy;

		Step s=field.at(p);

		if(s==st.step)
		{
			tr.add_rest(p);
			continue;
		}

		if(s!=st_empty)return false;
		break;
	}

	if(j!=3)return false;
	tr.add_cost(p);
	return true;
}

bool check_four_line_hole_inside(const step_t& st,const field_t& field,int dx,int dy,treat_t& tr)
{
	tr=treat_t();
	tr.gain=st;
	Gomoku::point p=st;
	unsigned j=0;
	Step s;

    //Ищем дырку слева
	for(;j<3;j++)
	{
		p.x-=dx;
		p.y-=dy;
		s=field.at(p);

		if(s==st.step)
		{
			tr.add_rest(p);
			continue;
		}

		if(s!=st_empty)return false;
		tr.add_cost(p);
		break;
	}

	if(tr.cost_count==0)
		return false;

	//След. за дыркой должен быть 0
	{
		j++;
		p.x-=dx;
		p.y-=dy;
		s=field.at(p);

		if(s!=st.step)return false;
		tr.add_rest(p);
	}

	if(j==4)
	{
		return true;
	}

	for(j++;j<4;j++)
	{
		p.x-=dx;
		p.y-=dy;
		s=field.at(p);

		if(s!=st.step)break;
		tr.add_rest(p);
	}

	if(j==4)
	{
		return true;
	}

	p=st;
	for(;j<4;j++)
	{
		p.x+=dx;
		p.y+=dy;
		s=field.at(p);

		if(s!=st.step)break;
		tr.add_rest(p);
	}

	if(j!=4)return false;
	
	return true;
}

bool check_four_line_zero_left_hole_right(const step_t& st,const field_t& field,int dx,int dy,treat_t& tr)
{
	tr=treat_t();
	tr.gain=st;
	Gomoku::point p=st;
	unsigned j=0;
	Step s;

	for(;j<3;j++)
	{
		p.x-=dx;
		p.y-=dy;
		s=field.at(p);

		if(s!=st.step)break;

		tr.add_rest(p);
	}

	if(j==3)
	{
		p.x-=dx;
		p.y-=dy;
		s=field.at(p);
	}

	if(s!=other_color(st.step))
		return false;

	p=st;
	for(;j<3;j++)
	{
		p.x+=dx;
		p.y+=dy;
		s=field.at(p);

		if(s!=st.step)break;

		tr.add_rest(p);
	}

	if(j!=3)return false;

	p.x+=dx;
	p.y+=dy;
	s=field.at(p);

	if(s!=st_empty)
		return false;

	tr.add_cost(p);

	return true;
}


bool check_open_three_line_two_cost(const step_t& st,const field_t& field,int dx,int dy,treat_t& tr)
{
	tr=treat_t();
	tr.gain=st;
	Gomoku::point p=st;
	Step s;
	unsigned j=0;

	for(;j<2;j++)
	{
		p.x-=dx;
		p.y-=dy;
		s=field.at(p);

		if(s!=st.step)break;
		tr.add_rest(p);
	}

    if(j==2)
	{
		p.x-=dx;
		p.y-=dy;
		s=field.at(p);
	}

	if(s!=st_empty)return false;
	tr.add_cost(p);

	p.x-=dx;
	p.y-=dy;
	s=field.at(p);
	if(s!=st_empty)return false;

	p=st;
	for(;j<2;j++)
	{
		p.x+=dx;
		p.y+=dy;
		s=field.at(p);

		if(s!=st.step)break;
		tr.add_rest(p);
	}

	if(j!=2)return false;

	p.x+=dx;
	p.y+=dy;
	s=field.at(p);

	if(s!=st_empty)return false;
	tr.add_cost(p);

	p.x+=dx;
	p.y+=dy;
	s=field.at(p);
	return s==st_empty;
}

bool check_open_three_line_three_cost(const step_t& st,const field_t& field,int dx,int dy,treat_t& tr)
{
	tr=treat_t();
	tr.gain=st;
	Gomoku::point p=st;
	Step s;
	unsigned j=0;

	for(;j<2;j++)
	{
		p.x-=dx;
		p.y-=dy;
		s=field.at(p);

		if(s!=st.step)break;
		tr.add_rest(p);
	}

    if(j==2)
	{
		p.x-=dx;
		p.y-=dy;
		s=field.at(p);
	}

	if(s!=st_empty)return false;
	tr.add_cost(p);

	p.x-=dx;
	p.y-=dy;
	s=field.at(p);
	//Должна быть левая клетка противоположного цвета
	if(s==st_empty||s==st.step)return false;

	p=st;
	for(;j<2;j++)
	{
		p.x+=dx;
		p.y+=dy;
		s=field.at(p);

		if(s!=st.step)break;
		tr.add_rest(p);
	}

	if(j!=2)return false;

	p.x+=dx;
	p.y+=dy;
	s=field.at(p);

	if(s!=st_empty)return false;
	tr.add_cost(p);

	p.x+=dx;
	p.y+=dy;
	s=field.at(p);
	return s==st_empty;
}

/////////////////////////////////////////////////////////////////

bool is_one_step_treat_exists(const point& pt,const field_t& field,Step cl)
{
	points_t empty_points(1);
	empty_points.front()=pt;

	treats_t treats;
	find_one_step_win_treats(empty_points,treats,cl,field);

	return !treats.empty();
}

void treats_gains_and_costs_to_points(const treats_t& treats,points_t& res)
{
    res.resize(0);
    res.reserve(treats.size()*4);

    for(unsigned i=0;i<treats.size();i++)
    {
        const treat_t& t=treats[i];
        res.push_back(t.gain);

        for(unsigned j=0;j<t.cost_count;j++)
            res.push_back(t.cost[j]);
    }
}

} }//namespace Gomoku

