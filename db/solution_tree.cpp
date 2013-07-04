#include "solution_tree.h"
#include <boost/filesystem/operations.hpp>
#include <stdexcept>
#include "../extern/binary_find.h"
#include <boost/lexical_cast.hpp>
#include "../extern/object_progress.hpp"

namespace fs=boost::filesystem;

namespace Gomoku
{
	const char* solution_tree_t::first_solving_file_name="first_solving";
	const char* solution_tree_t::last_solving_file_name="last_solving";
	unsigned solution_tree_t::win_neitrals=0;

	void solution_tree_t::init(const std::string& _base_dir)
	{
		base_dir=_base_dir;

		load_solve(first_solving,first_solving_file_name);
		load_solve(last_solving,last_solving_file_name);
	}

	void solution_tree_t::load_solve(deep_solve_t& val,const std::string& _file_name) const
	{
		val=deep_solve_t();

		fs::path file_name=fs::path(base_dir)/_file_name;
		if(!fs::exists(file_name))
			return;

        
		data_t bin;
		load_file(file_name,bin);
		val.unpack(bin);
	}

	void solution_tree_t::save_solve(const deep_solve_t& val,const std::string& _file_name) const
	{
		fs::path file_name=fs::path(base_dir)/_file_name;

		data_t bin;
		val.pack(bin);
		save_file(file_name,bin);
	}

	void solution_tree_t::create_init_tree()
	{
		sol_state_t s;
		s.key.push_back(step_t(st_krestik,0,0));
		s.state=ss_solving;

		set(s);

		s.neitrals.push_back(point(1,0));
		s.neitrals.push_back(point(1,1));
		s.neitrals.push_back(point(2,0));
		s.neitrals.push_back(point(2,1));
		s.neitrals.push_back(point(2,2));

		save_job(s.key,s.neitrals,s.solved_wins,s.solved_fails);
	}

	void solution_tree_t::create_init_tree(const steps_t& key)
	{
		sol_state_t s;
		s.key=key;
		s.state=ss_not_solved;

		set(s);
	}


	bool solution_tree_t::get(sol_state_t& res) const
	{
		ibin_index_t& idx=indexes.get_index(res.key.size());

		steps_t key=res.key;
		sort_steps(key);

		data_t bin_key;
		points2bin(key,bin_key);

		data_t bin;
		if(!idx.get(bin_key,bin))return false;

		res.unpack(bin);
		return true;
	}

	void solution_tree_t::set(const sol_state_t& _val)
	{
		sol_state_t val=_val;
		sort_steps(val.key);

		ibin_index_t& idx=indexes.get_index(val.key.size());

		data_t bin_key;
		points2bin(val.key,bin_key);

		data_t bin;
		val.pack(bin);

		idx.set(bin_key,bin);
	}

	steps_t solution_tree_t::get_root_key() const
	{
		unsigned level=indexes.get_root_level();
		if(level==0)return steps_t();
		ibin_index_t& idx=indexes.get_index(level);

		data_t bin_key,bin_val;
		if(!idx.first(bin_key,bin_val))return steps_t();

		steps_t ret;
		bin2points(bin_key,ret);
		
		return ret;
	}

	bool solution_tree_t::get_root_first_deep(deep_solve_t& _val)
	{
		deep_solve_t val;
		val.key=get_root_key();
		if(val.key.empty())return false;
		
		if(!get_first_deep(val,(unsigned)-1))return false;
		_val=val;
		return true;
	}
	
	bool solution_tree_t::get_first_deep(deep_solve_t& val,unsigned max_key_size)
	{
		sol_state_t ss;
		ss.key=val.key;
		
		if(!get(ss))
        {
            steps_t sorted_key=ss.key;
            sort_steps(sorted_key);
			throw std::runtime_error("get_first_deep(): state not found: "+print_steps(ss.key)+" sorted="+print_steps(sorted_key));
        }
			
		if(ss.use_count==0)return false;
		if(ss.state!=ss_solved)return true;

		if(ss.key.size()>=max_key_size)return false;

		Step move_step=(val.key.size()%2)==0? st_krestik:st_nolik;
		
		val.key.push_back(step_t(move_step,0,0));
		val.neitrals.push_back(ss.neitrals);
		
		for(size_t i=0;i<ss.neitrals.size();i++)		
		{
			const point& p=ss.neitrals[i];
			static_cast<point&>(val.key.back())=p;
			
			if(get_first_deep(val,max_key_size))
				return true;
		}
		
		val.key.pop_back();
		val.neitrals.pop_back();
		
		return false;
	}

	bool solution_tree_t::get_job(steps_t& key)
	{
		if(last_solving.empty())
		{
			if(!get_root_first_deep(last_solving))return false;
			first_solving=last_solving;
			key=last_solving.key;

			save_solve(first_solving,first_solving_file_name);
			save_solve(last_solving,last_solving_file_name);
			mark_solving(key);

			return true;
		}

		if(rewind_to_not_solved(true,last_solving))
		{
			key=last_solving.key;
			save_solve(last_solving,last_solving_file_name);
			mark_solving(key);
			return true;
		}

		last_solving=first_solving;

		if(rewind_to_not_solved(false,last_solving))
		{
			key=last_solving.key;
			save_solve(last_solving,last_solving_file_name);
			mark_solving(key);
			return true;
		}

		if(!get_root_first_deep(last_solving))return false;
		first_solving=last_solving;
		key=last_solving.key;

		save_solve(first_solving,first_solving_file_name);
		save_solve(last_solving,last_solving_file_name);
		mark_solving(key);

		return true;
	}
	
	bool solution_tree_t::rewind_to_not_solved(bool first_rewind,deep_solve_t& key)
	{
		if(!first_rewind)
		{
			sol_state_t st;
			st.key=key.key;
			get(st);
			if(st.state!=ss_solved&&st.use_count>0)
				return true;
		}

		steps_t p;

		for(steps_t k=key.key;k.size()>key.get_root_key_size();k=p)
		{
			p=steps_t(k.begin(),k.end()-1);

			sol_state_t pst;
			pst.key=k;
			get(pst);

			if(pst.use_count==0)
				continue;

			const points_t& n=key.get_key_neitrals(p.size());

			points_t::const_iterator it=std::find(n.begin(),n.end(),k.back());
			if(it==n.end())throw std::runtime_error("rewind_to_not_solved(): neitral does not exist: "+print_steps(k)+" of "+print_steps(key.key));

			for(++it;it!=n.end();++it)
			{
				deep_solve_t child=key;
				child.trunc_to_key_size(k.size());
				child.key=p;
				
				Step move_step=(k.size()%2)==0? st_nolik:st_krestik;
				child.key.push_back(step_t(move_step,it->x,it->y));

				if(get_first_deep(child,key.key.size()))
				{
					key=child;
					return true;
				}
			}
		}

		return false;
	}

	void solution_tree_t::mark_solving(const steps_t& key)
	{
		sol_state_t st;
		st.key=key;
		if(!get(st))throw std::runtime_error("mark_solving(): bin_key not found: "+print_steps(key));

		if(st.state==ss_solving)return;
		st.state=ss_solving;
		set(st);
	}

	void solution_tree_t::save_job(const steps_t& key,const points_t& neitrals,const npoints_t& win,const npoints_t& fails)
	{
		sol_state_t st;
		st.key=key;
		if(!get(st))return;

		if(st.state!=ss_solving)
			return;

		st.neitrals=neitrals;
		st.solved_wins=win;
		st.solved_fails=fails;
		st.state=ss_solved;

		set(st);

		if(first_solving.get_sorted_key()==st.key)
		{
			if(!rewind_to_not_solved(true,first_solving))
				first_solving=last_solving;
			save_solve(first_solving,first_solving_file_name);
		}

		if(st.neitrals.empty()||st.is_win())relax(st);
		else if(st.use_count>0)generate_new(st);
	}

	void solution_tree_t::generate_new(const sol_state_t& base_st)
	{
		Step cur_step=(base_st.key.size()%2)==0? st_nolik:st_krestik;
		Step next_step=other_step(cur_step);
		
		sol_state_t new_st;
		steps_t& key=new_st.key;
		new_st.state=ss_not_solved;

		for(size_t i=0;i<base_st.neitrals.size();i++)
		{
			const point& p=base_st.neitrals[i];
			
			key=base_st.key;
			key.push_back(step_t(next_step,p.x,p.y));

			sol_state_t exist_st;
			exist_st.key=key;
			if(!get(exist_st))set(new_st);
			else
			{
				++exist_st.use_count;
				set(exist_st);
			}
		}
	}

	void solution_tree_t::trunc_neitrals(const steps_t& key,points_t& neitrals,const npoints_t& win,const npoints_t& fails)
	{
		if(win_neitrals==0)return;
		if((key.size()%2)!=0)return;
		if(!win.empty())return;

		if(neitrals.size()<=win_neitrals)return;
		neitrals.resize(win_neitrals);
	}



	void solution_tree_t::relax(const sol_state_t& base_st)
	{
		Step cur_step=(base_st.key.size()%2)==0? st_nolik:st_krestik;
		
		sol_state_t prev_st;
		steps_t& key=prev_st.key;

		unsigned n;
		if(base_st.is_win())n=base_st.min_win_chain()+1;
		else n=base_st.max_fail_chain()+1;

		for(size_t i=0;i<base_st.key.size();i++)
		{
			const step_t& st=base_st.key[i];
			if(st.step!=cur_step)continue;

			key=base_st.key;
			key.erase(key.begin()+i);

			if(!get(prev_st))continue;

			prev_st.neitrals.erase(
				std::remove(prev_st.neitrals.begin(),prev_st.neitrals.end(),st),prev_st.neitrals.end());

			npoints_t& solved=base_st.is_win()? prev_st.tree_fails:prev_st.tree_wins;

			npoint p(st);
			p.n=n;

			if(std::find(solved.begin(),solved.end(),st)==solved.end())
				solved.push_back(p);

			set(prev_st);

			if((prev_st.neitrals.empty()||prev_st.is_win())&&prev_st.key.size()>1)
				relax(prev_st);

			if(prev_st.is_win())decrment_use_count(prev_st);
		}
	}

	void solution_tree_t::decrment_use_count(const sol_state_t& base_st)
	{
		Step cur_step=(base_st.key.size()%2)==0? st_nolik:st_krestik;
		Step next_step=other_step(cur_step);
		ObjectProgress::log_generator lg(true);

		lg<<"decrment_use_count()1: "<<print_steps(base_st.key);
		
		for(size_t i=0;i<base_st.neitrals.size();i++)
		{
			const point& p=base_st.neitrals[i];
			
			sol_state_t new_st;
			steps_t& key=new_st.key;

			key=base_st.key;

			if(!get(new_st))continue;
			lg<<"decrment_use_count()2.0: i="<<i<<" use_count="<<new_st.use_count<<" "<<print_steps(key);
			if(new_st.use_count==0)continue;
			--new_st.use_count;
			lg<<"decrment_use_count()2.1: i="<<i<<" use_count="<<new_st.use_count<<" "<<print_steps(key);
			set(new_st);
			if(new_st.use_count==0)decrment_use_count(new_st);
		}
	}

/////////////////////////////////////////////////////////////////////////////////
//
	void sol_state_t::pack(data_t& bin) const
	{
		bin.resize(0);
		bin.push_back((unsigned char)state);

        const unsigned char* p=reinterpret_cast<const unsigned char*>(&use_count);
		bin.insert(bin.end(),p,p+sizeof(use_count));

		pack(key,bin);
		pack(neitrals,bin);
		pack(solved_wins,bin);
		pack(solved_fails,bin);
		pack(tree_wins,bin);
		pack(tree_fails,bin);
	}

	void sol_state_t::unpack(const data_t& bin)
	{
		size_t from=0;

		if(from+1>=bin.size())
			throw std::runtime_error("sol_state_t::unpack(): unpack state failed");
		state=(SolState)bin[from];
		++from;

		if(from+sizeof(use_count)>=bin.size())
			throw std::runtime_error("sol_state_t::unpack(): unpack use count failed");
		use_count=*(const size_t*)(&bin[from]);
		from+=sizeof(use_count);

		unpack(bin,key,from);
		unpack(bin,neitrals,from);
		unpack(bin,solved_wins,from);
		unpack(bin,solved_fails,from);
		unpack(bin,tree_wins,from);
		unpack(bin,tree_fails,from);
	}

	void sol_state_t::pack(const points_t& pts,data_t& bin)
	{
		data_t d;
		points2bin(pts,d);
		size_t sz=d.size();
		const unsigned char* psz=reinterpret_cast<const unsigned char*>(&sz);
		bin.insert(bin.end(),psz,psz+sizeof(size_t));
		bin.insert(bin.end(),d.begin(),d.end());
	}

	void sol_state_t::unpack(const data_t& bin,points_t& pts,size_t& from)
	{
		if(from+sizeof(size_t)>bin.size())
			throw std::runtime_error("sol_state_t::unpack(): unpack size failed");

		size_t sz=*reinterpret_cast<const size_t*>(&bin[from]);
		from+=sizeof(size_t);

		if(from+sz>bin.size())
			throw std::runtime_error("sol_state_t::unpack(): unpack failed");

		data_t d(bin.begin()+from,bin.begin()+from+sz);
		bin2points(d,pts);
		from+=sz;
	}

	void sol_state_t::pack(const steps_t& pts,data_t& bin)
	{
		data_t d;
		points2bin(pts,d);
		size_t sz=d.size();
		const unsigned char* psz=reinterpret_cast<const unsigned char*>(&sz);
		bin.insert(bin.end(),psz,psz+sizeof(size_t));
		bin.insert(bin.end(),d.begin(),d.end());
	}

	void sol_state_t::unpack(const data_t& bin,steps_t& pts,size_t& from)
	{
		if(from+sizeof(size_t)>bin.size())
			throw std::runtime_error("sol_state_t::unpack(): unpack size failed");

		size_t sz=*reinterpret_cast<const size_t*>(&bin[from]);
		from+=sizeof(size_t);

		if(from+sz>bin.size())
			throw std::runtime_error("sol_state_t::unpack(): unpack failed");

		data_t d(bin.begin()+from,bin.begin()+from+sz);
		bin2points(d,pts);
		from+=sz;
	}

	void sol_state_t::pack(const npoints_t& pts,data_t& bin)
	{
		data_t d;
		points2bin(pts,d);
		size_t sz=d.size();
		const unsigned char* psz=reinterpret_cast<const unsigned char*>(&sz);
		bin.insert(bin.end(),psz,psz+sizeof(size_t));
		bin.insert(bin.end(),d.begin(),d.end());
	}

	void sol_state_t::unpack(const data_t& bin,npoints_t& pts,size_t& from)
	{
		if(from+sizeof(size_t)>bin.size())
			throw std::runtime_error("sol_state_t::unpack(): unpack size failed");

		size_t sz=*reinterpret_cast<const size_t*>(&bin[from]);
		from+=sizeof(size_t);

		if(from+sz>bin.size())
			throw std::runtime_error("sol_state_t::unpack(): unpack failed");

		data_t d(bin.begin()+from,bin.begin()+from+sz);
		bin2points(d,pts);
		from+=sz;
	}

	unsigned get_min_n(const npoints_t& vals)
	{
		unsigned ret=(unsigned)-1;
		for(unsigned i=0;i<vals.size();i++)
			if(vals[i].n<ret)ret=vals[i].n;
		return ret;
	}

	unsigned get_max_n(const npoints_t& vals)
	{
		unsigned ret=0;
		for(unsigned i=0;i<vals.size();i++)
			if(vals[i].n>ret)ret=vals[i].n;
		return ret;
	}

	unsigned sol_state_t::min_win_chain() const
	{
		unsigned a=get_min_n(solved_wins);
		unsigned b=get_min_n(tree_wins);
		if(a<b)return a;
		return b;
	}

	unsigned sol_state_t::max_fail_chain() const
	{
		unsigned a=get_max_n(solved_fails);
		unsigned b=get_max_n(tree_fails);
		if(a>b)return a;
		return b;
	}

	void deep_solve_t::pack(data_t& bin) const
	{
		bin.resize(0);

		sol_state_t::pack(key,bin);

		size_t neitrals_counts=neitrals.size();
        const unsigned char* p=reinterpret_cast<const unsigned char*>(&neitrals_counts);
		bin.insert(bin.end(),p,p+sizeof(neitrals_counts));

		for(size_t i=0;i<neitrals_counts;i++)
			sol_state_t::pack(neitrals[i],bin);
	}

	void deep_solve_t::unpack(const data_t& bin)
	{
		size_t from=0;
		sol_state_t::unpack(bin,key,from);

		size_t neitrals_counts=0;
		if(from+sizeof(neitrals_counts)>=bin.size())
			throw std::runtime_error("deep_solve_t::unpack(): unpack neitrals_counts failed");
		neitrals_counts=*(const size_t*)(&bin[from]);
		from+=sizeof(neitrals_counts);

		neitrals.resize(neitrals_counts);

		for(size_t i=0;i<neitrals_counts;i++)
			sol_state_t::unpack(bin,neitrals[i],from);
	}

	const points_t& deep_solve_t::get_key_neitrals(size_t cur_key_size) const
	{
		unsigned idx=neitrals.size()-(key.size()-cur_key_size);
		if(idx>=neitrals.size())throw std::runtime_error("get_key_neitrals(): invalid neitrals index");
		return neitrals[idx];
	}

	void deep_solve_t::trunc_to_key_size(size_t cur_key_size)
	{
		unsigned idx=neitrals.size()-(key.size()-cur_key_size);
		if(idx>neitrals.size())throw std::runtime_error("get_key_neitrals(): invalid neitrals index");
		neitrals.erase(neitrals.begin()+idx,neitrals.end());
	}

}//namespace
