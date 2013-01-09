#include "wsplayer_treat.h"
#include <stdexcept>

#ifdef WITHOUT_EXTERNAL_LIBS
#  include "../extern/pair_comparator.h"
#  include "../extern/binary_find.h"
#  include "../extern/object_progress.hpp"
#else
#  include <object_progress/object_progress.hpp>
#  include <object_progress/perfomance.hpp>
#  include <pair_comparator.h>
#  include <binary_find.h>
#endif

#include "wsplayer.h"

namespace Gomoku { namespace WsPlayer
{

/////////////////////////////////////////////////////////////////////////////////////
void treat_node_t::build_tree(Step cl,bool only_win,const treat_filter_t& tf,unsigned deep)
{
	points_t empty_points;
	player.field.get_empty_around(empty_points,2);

	treats_t treats;
	find_treats(empty_points,treats,cl,player.field,tf.get_steps_to_win());

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

	points_t empty_points;
	player.field.get_empty_around(empty_points,2);

	treats_t treats;
	find_treats(empty_points,treats,cl,player.field,tf.get_steps_to_win());

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


item_ptr treat_node_t::check_tree(Step cl)
{
	if(childs.empty())return item_ptr();

	group_by_step();
	sort_by_min_deep();

	for(unsigned i=0;i<childs.size();i++)
	{
		player.check_cancel();
		treat_node_t& gr=*childs[i];

		item_ptr r=check_tree_one_step(gr,cl);
		if(r)return r;
	}

	return item_ptr();
}

item_ptr  treat_node_t::check_tree_one_step(treat_node_t& gr,Step cl)
{
	field_t& field=player.field;
	temporary_step hld(field,gr.gain,cl);
	return icheck_tree_one_step(gr,cl);
}

item_ptr treat_node_t::icheck_tree_one_step(treat_node_t& gr,Step cl)
{
	field_t& field=player.field;

	item_ptr max_r;
	unsigned max_depth=0;

	for(unsigned k=0;k<gr.childs.size();k++)
	{
		treat_node_t& ch=*gr.childs[k];

		if(ch.get_steps_to_win()>1)
		{
			treat_node_t revert_treat(player);
			revert_treat.build_tree(other_step(cl),false,revert_treat_filter_t(ch));
			if(revert_treat.win)return item_ptr();
			ch.remove_revertable_path(revert_treat);
		}

		if(!ch.win)return item_ptr();

		if(ch.childs.empty())
		{
			item_ptr cr;
			
			if(ch.get_steps_to_win()==1)
			{
				max_r=item_ptr(new item_t(player,ch.gain,cl ));
				max_depth=1;
				break;//альтернативы нету уже победа
			}
			else
			{
				if(ch.get_steps_to_win()!=2||ch.cost_count!=2)
					throw std::runtime_error("check_tree() open four expected");
					
				cr=item_ptr(new item_t(player,ch.gain,cl ));
				item_ptr f(new item_t(player,ch.cost[0],other_step(cl) ));
				item_ptr w(new item_t(player,ch.cost[1],cl ));

				f->win=w;
				cr->fail=f;
			}

			unsigned depth=cr->get_chain_depth();
			if(!max_r||depth>max_depth)
			{
				max_r=cr;
				max_depth=depth;
			}
		}
		else
		{
			for(unsigned j=0;j<ch.cost_count;j++)
			{
				temporary_step hld_c(field,ch.cost[j],other_step(cl));
				treat_node_ptr cch=ch.clone();
				
				item_ptr cf=cch->check_tree(cl);

				if(!cf)return item_ptr();

				unsigned depth=cf->get_chain_depth()+2;
				if(!max_r||depth>max_depth)
				{
					item_ptr f(new item_t(player,ch.cost[j],other_step(cl) ));
					f->win=cf;
					max_r=item_ptr(new item_t(player,ch.gain,cl) );
					max_r->fail=f;
					
					max_depth=depth;
				}
			}
		}
	}

	if(!max_r)return max_r;

	item_ptr ccf;
	if(contra_steps_exists(gr,cl,ccf))
	{
		if(!ccf)return item_ptr();
		unsigned depth=ccf->get_chain_depth();
		if(!max_r||depth>max_depth)
		{
			max_r=ccf;
			max_depth=depth;
		}
	}

	return max_r;
}

bool treat_node_t::contra_steps_exists(treat_node_t& attack_group,Step cl,item_ptr& res)
{
	unsigned min_steps_to_win=attack_group.get_childs_min_steps_to_win();
	if(min_steps_to_win<=1)return false;

	treat_node_ptr ctree(new treat_node_t(player));
	ctree->build_tree(other_step(cl),false,step_treat_filter_t(min_steps_to_win-1),0);
	ctree->group_by_step();

	unsigned max_depth=0;

	for(unsigned i=0;i<ctree->childs.size();i++)
	{
		player.check_cancel();
		treat_node_t& gr=*ctree->childs[i];

		item_ptr cres;
		if(!contra_steps_exists_one_step(attack_group,cl,cres,gr))
			continue;

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

bool treat_node_t::contra_steps_exists_one_step(treat_node_t& attack_group,Step cl,item_ptr& res,treat_node_t& gr)
{
	field_t& field=player.field;
	temporary_step hld(field,gr.gain,other_step(cl));

	item_ptr max_r;
	unsigned max_depth=0;

	for(unsigned k=0;k<gr.childs.size();k++)
	{
		treat_node_t& ch=*gr.childs[k];
		for(unsigned j=0;j<ch.cost_count;j++)
		{
			temporary_step hld_answer(field,ch.cost[j],cl);
			res=icheck_tree_one_step(attack_group,cl);
			if(!res)continue;
			
			item_ptr c(new item_t(player,gr.gain,other_step(cl) ));
			item_ptr ca(new item_t(player,ch.cost[j],cl ));

			c->win=ca;
			ca->fail=res->fail;
			res->fail=c;
			return true;
		}
	}

	return false;
}




void treat_node_t::group_by_step()
{
	if(childs.empty())return;

	std::sort(childs.begin(),childs.end(),treat_step_pr());
	treat_nodes_t tmp_childs;
	std::swap(tmp_childs,childs);

	treat_node_ptr cr(new treat_node_t(player));
	cr->gain=tmp_childs.front()->gain;
	cr->childs.push_back(tmp_childs.front());

	for(unsigned i=1;i<tmp_childs.size();i++)
	{
		treat_node_ptr& p=tmp_childs[i];
		if(cr->gain!=p->gain)
		{
			childs.push_back(cr);
			cr=treat_node_ptr(new treat_node_t(player));
			cr->gain=p->gain;
		}
        
		cr->childs.push_back(p);
	}

	childs.push_back(cr);
}

void treat_node_t::sort_by_min_deep()
{
	std::sort(childs.begin(),childs.end(),treat_min_deep_pr());
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
	Step ocl=other_step(cl);

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

/////////////////////////////////////////////////////////////////////
//
//
void find_treats(const points_t& empty_points,treats_t& res,Step cl,const field_t& field,unsigned steps_to_win)
{
	find_treats(empty_points,cl,field,res,check_five_line);
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

	if(s!=other_step(st.step))
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

} }//namespace Gomoku

