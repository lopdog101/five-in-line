#include "bin_index.h"
#include <boost/filesystem/operations.hpp>
#include <stdexcept>
#include "../extern/binary_find.h"
#include <boost/lexical_cast.hpp>

namespace fs=boost::filesystem;

namespace Gomoku
{
	bin_index_t::bin_index_t(const std::string& _base_dir,size_t _key_len,size_t _dir_key_len,size_t _file_max_records) :
      base_dir(_base_dir),key_len(_key_len),dir_key_len(_dir_key_len),file_max_records(_file_max_records)
	{
		index_file_name="index";
		data_file_name="data";
		items_count_file_name="items_count";

		items_count=0;
		load_items_count();
	}

	void bin_index_t::validate_root() const
	{
		if(!root)root=create_node(base_dir,key_len);
	}

	bin_index_t::node_ptr bin_index_t::create_node(const std::string& base_dir,size_t key_len) const
	{
		bool is_any_sub_dir=false;
		fs::directory_iterator end_itr;
		for(fs::directory_iterator itr(base_dir);itr!=end_itr;++itr)
		{
			fs::path fl=*itr;
			if(!is_directory(fl))continue;
			is_any_sub_dir=true;
			break;
		}

		if(is_any_sub_dir)return node_ptr(new dir_node(*this,base_dir,key_len));
		return node_ptr(new file_node(*this,base_dir,key_len));
		
	}

	void bin_index_t::load_items_count()
	{
		fs::path file_name=fs::path(base_dir)/items_count_file_name;
		if(!fs::exists(file_name))return;
		FILE* f=fopen(file_name.string().c_str(),"rb");
		if(!f)throw std::runtime_error("could not open items count file for read: "+file_name.string());
		file_hldr hld(f);

		if(fscanf(f,"%u",&items_count)!=1)
			throw std::runtime_error("could not read items count: "+file_name.string());
	}

	void bin_index_t::save_items_count()
	{
		fs::path file_name=fs::path(base_dir)/items_count_file_name;
		FILE* f=fopen(file_name.string().c_str(),"wb");
		if(!f)throw std::runtime_error("could not open items count file for write: "+file_name.string());
		file_hldr hld(f);

		if(fprintf(f,"%u",items_count)<0)
			throw std::runtime_error("could not write items count: "+file_name.string());
	}

///////////////////////////////////////////////////////////////////////////////////
	void bin_index_t::inode::validate_key_len(const data_t& key) const
	{
		if(key.size()!=key_len)throw std::runtime_error("invalid key length");
	}

///////////////////////////////////////////////////////////////////////////////////
//  file_node
//

	bool bin_index_t::file_node::get(const data_t& key,data_t& val) const
	{
		validate_key_len(key);
		open_index_file();
		
		index_t ind;
		ind.key=key;
		if(!get_item(root_offset,ind))return false;

		val.resize(ind.data_len);
		load_data(ind.data_offset,val);
		return true;
	}

	bool bin_index_t::file_node::get_item(size_t offset,index_t& val,size_t& index_value_offset) const
	{
		if(offset==0)return false;

		index_t cr;
		load_index(offset,cr);
		if(cr.key==val.key)
		{
			val=cr;
			index_value_offset=offset;
			return true;
		}

		data_less_pr pr;

		if(pr(val.key,cr.key))
			return get_item(cr.left,val,index_value_offset);
		return get_item(cr.right,val,index_value_offset);
	}

	
	bool bin_index_t::file_node::first(data_t& key,data_t& val) const
	{
		open_index_file();
		
		index_t ind;
		if(!first_item(root_offset,ind))return false;

		key=ind.key;
		val.resize(ind.data_len);
		load_data(ind.data_offset,val);
		return true;
	}

	bool bin_index_t::file_node::first_item(size_t offset,index_t& val) const
	{
		if(offset==0)return false;

		index_t cr;
		load_index(offset,cr);
		if(cr.left!=0)return first_item(cr.left,val);

		val=cr;
		return true;
	}

	bool bin_index_t::file_node::next(data_t& key,data_t& val) const
	{
		validate_key_len(key);
		open_index_file();
		
		index_t ind;
		ind.key=key;
		if(!next_item(root_offset,ind))return false;

		key=ind.key;
		val.resize(ind.data_len);
		load_data(ind.data_offset,val);
		return true;
	}

	bool bin_index_t::file_node::next_item(size_t offset,index_t& val) const
	{
		if(offset==0)
		{
			return false;
		}

		index_t cr;
		load_index(offset,cr);

		data_less_pr pr;
		if(pr(val.key,cr.key))
		{
			if(next_item(cr.left,val))return true;

			val=cr;
			return true;
		}

		return next_item(cr.right,val);
	}

	bool bin_index_t::file_node::set(const data_t& key,const data_t& val)
	{
		validate_key_len(key);
		open_index_file();

		index_t it;
		it.key=key;
		size_t it_offset=0;

		if(get_item(root_offset,it,it_offset))
		{
			index_t old_i=it;

			if(it.data_len>=val.size())save_data(it.data_offset,val);
			else it.data_offset=append_data(val);
			it.data_len=val.size();

			if(old_i.data_offset==it.data_offset&&
				old_i.data_len==it.data_len)
				return false;

			save_index(it_offset,it);
			return false;
		}

		index_t v;
		v.key=key;
		v.data_len=val.size();
		v.data_offset=append_data(val);

        bool new_level=false;
		if(add_item(root_offset,v,new_level))
			save_index_data(0,root_offset);

		++items_count;
		save_index_data(sizeof(size_t),items_count);

		if(items_count<parent.file_max_records)return true;
		if(key_len<=parent.dir_key_len)return true;
		if(disable_split)return true;

		split();
		self_valid=false;

		return true;
	}

	bool bin_index_t::file_node::add_item(size_t& offset,const index_t& val,bool& new_level)
	{
		if(offset==0)
		{
			offset=append_index(val);
			new_level=true;
			return true;
		}

		index_t cr;
		load_index(offset,cr);
		size_t orig_offset=offset;

		data_less_pr pr;
		if(pr(val.key,cr.key))
		{
			bool changed=add_item(cr.left,val,new_level);
			if(!changed&&!new_level)return false;

            if(new_level)
			{
				if(cr.balance==1)
				{
					cr.balance=0;
					new_level=false;
				}
				else if(cr.balance==0)cr.balance=-1;
				else
				{
					index_t l;
					size_t old_l_offset=cr.left;
					load_index(old_l_offset,l);
					//ll
					if(l.balance==-1)
					{
						cr.left=l.right;
						l.right=offset;
						cr.balance=0;
						offset=old_l_offset;
						l.balance=0;
					}
					//lr
					else
					{
						index_t lr;
						size_t old_lr_offset=l.right;
						load_index(old_lr_offset,lr);

						l.right=lr.left;
						lr.left=old_l_offset;
						cr.left=lr.right;
						lr.right=orig_offset;

						if(lr.balance==-1)cr.balance=1;
						else cr.balance=0;

						if(lr.balance==1)l.balance=-1;
						else l.balance=0;
						
						offset=old_lr_offset;
						lr.balance=0;
						save_index(old_lr_offset,lr);
					}

					new_level=false;
					save_index(old_l_offset,l);
				}
			}
		}
		else
		{
			bool changed=add_item(cr.right,val,new_level);
			if(!changed&&!new_level)return false;

            if(new_level)
			{
				if(cr.balance==-1)
				{
					cr.balance=0;
					new_level=false;
				}
				else if(cr.balance==0)cr.balance=1;
				else
				{
					index_t r;
					size_t old_r_offset=cr.right;
					load_index(old_r_offset,r);
					//rr
					if(r.balance==1)
					{
						cr.right=r.left;
						r.left=offset;
						cr.balance=0;
						offset=old_r_offset;
						r.balance=0;
					}
					//rl
					else
					{
						index_t rl;
						size_t old_rl_offset=r.left;
						load_index(old_rl_offset,rl);

						r.left=rl.right;
						rl.right=old_r_offset;
						cr.right=rl.left;
						rl.left=orig_offset;

						if(rl.balance==1)cr.balance=-1;
						else cr.balance=0;

						if(rl.balance==-1)r.balance=1;
						else r.balance=0;
						
						offset=old_rl_offset;
						rl.balance=0;
						save_index(old_rl_offset,rl);
					}

					new_level=false;
					save_index(old_r_offset,r);
				}
			}
		}

		save_index(orig_offset,cr);
		return offset!=orig_offset;
	}

	void bin_index_t::file_node::load_data(size_t offset,data_t& res) const
	{
		if(res.empty())return;

		open_data_file();

		if(fseek(fd,(int)offset,SEEK_SET)!=0)
			throw std::runtime_error("load_data(): seek error at "+boost::lexical_cast<std::string>(offset)+
			": "+data_file_name());

		if(fread(&res.front(),1,res.size(),fd)!=res.size())
			throw std::runtime_error(
			  "load_data(): read error at "+boost::lexical_cast<std::string>(offset)+
			  " size="+boost::lexical_cast<std::string>(res.size())+
			  ": "+data_file_name());
	}

	void bin_index_t::file_node::save_data(size_t offset,const data_t& res)
	{
		if(res.empty())return;

		open_data_file();

		if(fseek(fd,(int)offset,SEEK_SET)!=0)
			throw std::runtime_error("save_data(): seek error at "+boost::lexical_cast<std::string>(offset)+
			": "+data_file_name());

		if(fwrite(&res.front(),1,res.size(),fd)!=res.size())
			throw std::runtime_error(
			  "save_data(): write error at "+boost::lexical_cast<std::string>(offset)+
			  " size="+boost::lexical_cast<std::string>(res.size())+
			  ": "+data_file_name());
	}

	size_t bin_index_t::file_node::append_data(const data_t& res)
	{
		if(res.empty())return 0;

		open_data_file();

		if(fseek(fd,0,SEEK_END)!=0)
			throw std::runtime_error("append_data(): seek error at end: "+data_file_name());

		size_t ret=ftell(fd);

		if(fwrite(&res.front(),1,res.size(),fd)!=res.size())
			throw std::runtime_error(
			  "append_data(): write error size="+boost::lexical_cast<std::string>(res.size())+
			  ": "+data_file_name());

		return ret;
	}

	void bin_index_t::file_node::validate_root_offset() const
	{
		root_offset=0;
		items_count=0;
		
		if(fseek(fi,0,SEEK_END)!=0)
			throw std::runtime_error("validate_root_offset(): seek error at end: "+index_file_name());
		
		size_t ret=ftell(fi);
		if(ret==0)
		{
			const_cast<file_node*>(this)->save_index_data(0,0);
			const_cast<file_node*>(this)->save_index_data(sizeof(size_t),0);
			return;
		}

		data_t d(2*sizeof(root_offset));
		load_index_data(0,d);
		root_offset=*reinterpret_cast<const size_t*>(&d[0]);
		items_count=*reinterpret_cast<const size_t*>(&d[sizeof(size_t)]);
	}

	void bin_index_t::file_node::load_index_data(size_t offset,data_t& res) const
	{
		if(res.empty())return;

		if(fseek(fi,(int)offset,SEEK_SET)!=0)
			throw std::runtime_error("load_data(): seek error at "+boost::lexical_cast<std::string>(offset)+
			": "+index_file_name());

		if(fread(&res.front(),1,res.size(),fi)!=res.size())
			throw std::runtime_error(
			  "load_data(): read error at "+boost::lexical_cast<std::string>(offset)+
			  " size="+boost::lexical_cast<std::string>(res.size())+
			  ": "+index_file_name());
	}

	void bin_index_t::file_node::save_index_data(size_t offset,const data_t& res)
	{
		if(res.empty())return;

		if(fseek(fi,(int)offset,SEEK_SET)!=0)
			throw std::runtime_error("save_data(): seek error at "+boost::lexical_cast<std::string>(offset)+
			": "+index_file_name());

		if(fwrite(&res.front(),1,res.size(),fi)!=res.size())
			throw std::runtime_error(
			  "save_data(): write error at "+boost::lexical_cast<std::string>(offset)+
			  " size="+boost::lexical_cast<std::string>(res.size())+
			  ": "+index_file_name());
	}

	size_t bin_index_t::file_node::append_index_data(const data_t& res)
	{
		if(res.empty())return 0;

		if(fseek(fi,0,SEEK_END)!=0)
			throw std::runtime_error("append_data(): seek error at end: "+index_file_name());

		size_t ret=ftell(fi);

		if(fwrite(&res.front(),1,res.size(),fi)!=res.size())
			throw std::runtime_error(
			  "append_data(): write error size="+boost::lexical_cast<std::string>(res.size())+
			  ": "+index_file_name());

		return ret;
	}

	void bin_index_t::file_node::save_index_data(size_t offset,size_t res)
	{
		const char* p=reinterpret_cast<const char*>(&res);
		data_t d(p,p+sizeof(size_t));
		save_index_data(offset,d);
	}

	void bin_index_t::file_node::split()
	{
		bool r;
		data_t key;
		data_t val;

        ObjectProgress::log_generator lg(true);

		for(r=first(key,val);r;)
		{
			data_t child_base(key.begin(),key.begin()+parent.dir_key_len);
			std::string sub_dir;
			bin2hex(child_base,sub_dir);
			fs::path full_path=fs::path(base_dir)/sub_dir;
			fs::create_directory(full_path);

			file_node ch(parent,full_path.string(),key_len-parent.dir_key_len);
			ch.disable_split=true;

			for(;r;r=next(key,val))
			{
				if(!std::equal(child_base.begin(),child_base.end(),key.begin()))
					break;

				data_t sub_key(key.begin()+parent.dir_key_len,key.end());
				ch.set(sub_key,val);
			}
		}

        close_files();
		fs::remove(fs::path(base_dir)/parent.index_file_name);
		fs::remove(fs::path(base_dir)/parent.data_file_name);
	}

	void bin_index_t::file_node::close_files()
	{
		if(fi)fclose(fi);
		if(fd)fclose(fd);
	}

	void bin_index_t::file_node::open_index_file() const
	{
		if(fi)return;
		fs::path file_name=fs::path(base_dir)/parent.index_file_name;
		fi=fopen(file_name.string().c_str(),"r+b");
		if(!fi)fi=fopen(file_name.string().c_str(),"w+b");
		if(!fi)throw std::runtime_error("could not open index file: "+file_name.string());
		validate_root_offset();
	}

	void bin_index_t::file_node::open_data_file() const
	{
		if(fd)return;
		fs::path file_name=fs::path(base_dir)/parent.data_file_name;
		fd=fopen(file_name.string().c_str(),"r+b");
		if(!fd)fd=fopen(file_name.string().c_str(),"w+b");
		if(!fd)throw std::runtime_error("could not open index file: "+file_name.string());
	}

	std::string bin_index_t::file_node::index_file_name() const
	{
		fs::path file_name=fs::path(base_dir)/parent.index_file_name;
		return file_name.string();
	}

	std::string bin_index_t::file_node::data_file_name() const
	{
		fs::path file_name=fs::path(base_dir)/parent.data_file_name;
		return file_name.string();
	}

	void bin_index_t::file_node::pack(const index_t& val,data_t& bin) const
	{
		bin.resize(irec_len());
		std::copy(val.key.begin(),val.key.end(),bin.begin());

		*reinterpret_cast<size_t*>(&bin[key_len])=val.left;
		*reinterpret_cast<size_t*>(&bin[key_len+1*sizeof(size_t)])=val.right;
		*reinterpret_cast<size_t*>(&bin[key_len+2*sizeof(size_t)])=val.data_offset;
		*reinterpret_cast<size_t*>(&bin[key_len+3*sizeof(size_t)])=val.data_len;
		*reinterpret_cast<char*>(&bin[key_len+4*sizeof(size_t)])=val.balance;
	}

	void bin_index_t::file_node::unpack(index_t& val,const data_t& bin) const
	{
		if(bin.size()!=irec_len())throw std::runtime_error(index_file_name()+": unpack() invalid bin.size()");
		val.key.resize(0);
		val.key.insert(val.key.end(),bin.begin(),bin.begin()+key_len);
		val.left=*reinterpret_cast<const size_t*>(&bin[key_len]);
		val.right=*reinterpret_cast<const size_t*>(&bin[key_len+1*sizeof(size_t)]);
		val.data_offset=*reinterpret_cast<const size_t*>(&bin[key_len+2*sizeof(size_t)]);
		val.data_len=*reinterpret_cast<const size_t*>(&bin[key_len+3*sizeof(size_t)]);
		val.balance=*reinterpret_cast<const char*>(&bin[key_len+4*sizeof(size_t)]);
	}

	void bin_index_t::file_node::load_index(size_t offset,index_t& val) const
	{
		data_t d(irec_len());
		load_index_data(offset,d);
		unpack(val,d);
	}

	void bin_index_t::file_node::save_index(size_t offset,const index_t& val)
	{
		data_t d;
		pack(val,d);
		save_index_data(offset,d);
	}

	size_t bin_index_t::file_node::append_index(const index_t& val)
	{
		data_t d;
		pack(val,d);
		return append_index_data(d);
	}

///////////////////////////////////////////////////////////////////////////////////
//  dir_node
//
	bool bin_index_t::dir_node::get(const data_t& key,data_t& val) const
	{
		validate_key_len(key);
		validate_index();
		
		data_t head(key.begin(),key.begin()+parent.dir_key_len);
		data_t tail(key.begin()+parent.dir_key_len,key.end());

		if(!std::binary_search(indexes.begin(),indexes.end(),head,data_less_pr()))
			return false;

		validate_sub(head);

		return sub_node->get(tail,val);
	}
	
	bool bin_index_t::dir_node::set(const data_t& key,const data_t& val)
	{
		validate_key_len(key);
		validate_index();

		data_t head(key.begin(),key.begin()+parent.dir_key_len);
		data_t tail(key.begin()+parent.dir_key_len,key.end());

		bool already_exists=std::binary_search(indexes.begin(),indexes.end(),head,data_less_pr());

        if(!already_exists)create_text_child(head);
		validate_sub(head);

		bool r=sub_node->set(tail,val);
		if(!sub_node->is_valid())sub_node.reset();
		return r;
	}
	
	bool bin_index_t::dir_node::first(data_t& key,data_t& val) const
	{
		validate_index();

		if(indexes.empty())return false;

		const data_t& head=indexes.front();
		validate_sub(head);

		data_t tail;
		if(!sub_node->first(tail,val))return false;
		
		key=head;
		key.insert(key.end(),tail.begin(),tail.end());

		return true;
	}

	bool bin_index_t::dir_node::next(data_t& key,data_t& val) const
	{
		validate_key_len(key);
		validate_index();
		
		data_t head(key.begin(),key.begin()+parent.dir_key_len);
		data_t tail(key.begin()+parent.dir_key_len,key.end());

		datas_t::const_iterator it=std::lower_bound(indexes.begin(),indexes.end(),head,data_less_pr());
		if(it==indexes.end())return false;

		if(*it==head)
		{
			validate_sub(head);
			if(sub_node->next(tail,val))
			{
				key=head;
				key.insert(key.end(),tail.begin(),tail.end());
				return true;
			}
		}

		++it;
		if(it==indexes.end())return false;
		head=*it;

		validate_sub(head);
		if(!sub_node->first(tail,val))return false;

		key=head;
		key.insert(key.end(),tail.begin(),tail.end());
		return true;
	}

	void bin_index_t::dir_node::validate_index() const
	{
		if(index_valid)return;
		load_index();
		index_valid=true;

	}

	void bin_index_t::dir_node::load_index() const
	{
		indexes.clear();

		fs::path file_name=fs::path(base_dir)/parent.index_file_name;

		fs::directory_iterator end_itr;
		for(fs::directory_iterator itr(base_dir);itr!=end_itr;++itr)
		{
			fs::path fl=*itr;
			if(!is_directory(fl))continue;
            std::string h=fl.leaf().string();
			data_t d;
			hex2bin(h,d);
			if(d.size()!=parent.dir_key_len)
				continue;
			indexes.push_back(d);
		}

		std::sort(indexes.begin(),indexes.end(),data_less_pr());
	}

	void bin_index_t::dir_node::validate_sub(const data_t& name) const
	{
		if(sub_name==name&&sub_node!=0)return;
		std::string h;
		bin2hex(name,h);
		fs::path sub_path=fs::path(base_dir)/h;
		sub_node=parent.create_node(sub_path.string(),key_len-parent.dir_key_len);
		sub_name=name;
	}

	void bin_index_t::dir_node::create_text_child(const data_t& name) const
	{
		std::string h;
		bin2hex(name,h);
		fs::path sub_path=fs::path(base_dir)/h;
		fs::create_directory(sub_path);
	}



	bin_index_t& bin_indexes_t::get_index(unsigned steps_count)
	{
		indexes_t::const_iterator it=indexes.find(steps_count);
		if(it!=indexes.end())return *it->second;

		fs::path dir_name=fs::path(base_dir)/("S"+boost::lexical_cast<std::string>(steps_count));
		if(!fs::exists(dir_name))
			fs::create_directory(dir_name);

		item_ptr ind(new bin_index_t(dir_name.string(),steps_count*len_per_level));
		indexes[steps_count]=ind;
		return *ind;
	}
    
    bin_index_t* bin_indexes_t::find_index(unsigned steps_count)
    {
		indexes_t::const_iterator it=indexes.find(steps_count);
		if(it!=indexes.end())return &*it->second;

		fs::path dir_name=fs::path(base_dir)/("S"+boost::lexical_cast<std::string>(steps_count));
		if(!fs::exists(dir_name))
            return nullptr;

		item_ptr ind(new bin_index_t(dir_name.string(),steps_count*len_per_level));
		indexes[steps_count]=ind;
		return &*ind;
    }

	unsigned bin_indexes_t::get_root_level() const
	{
		for(unsigned i=1;i<100;i++)
		{
			fs::path dir_name=fs::path(base_dir)/("S"+boost::lexical_cast<std::string>(i));
			if(fs::exists(dir_name))return i;
		}
		return 0;
	}
}//namespace
