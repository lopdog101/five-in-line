#include "bin_index.h"
#include <boost/filesystem/operations.hpp>
#include <stdexcept>
#include "../extern/binary_find.h"
#include <boost/lexical_cast.hpp>

namespace fs=boost::filesystem;

namespace Gomoku
{
	bin_index_t::bin_index_t(const std::string& _base_dir,size_t _key_len,size_t _dir_key_len,file_offset_t _file_max_records,size_t _page_max) :
		base_dir(_base_dir),
		key_len(_key_len),
		dir_key_len(_dir_key_len),
		file_max_records(_file_max_records),
		page_max(_page_max)
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

		if(!root_page)
			return false;
		
		index_t ind;
		ind.key=key;

		if(!get_item(*root_page,ind))return false;

		val.resize(ind.data_len);
		load_data(ind.data_offset,val);
		return true;
	}

	bool bin_index_t::file_node::get_item(page_t& page,index_t& val) const
	{
		page_pr pr(page);
		page_iter p=std::lower_bound(page.begin(),page.end(),val.key,pr);
		
		index_ref r=page[static_cast<size_t>(*p)];

		if(p!=page.end() && !pr(val.key,*p))
		{
			val.page_offset=page.page_offset;
			val.index_in_page=static_cast<size_t>(*p);
			val.copy_pointers(r);

			return true;
		}

		if(r.left()==0)
			return false;

		page_t pg(key_len,parent.page_max);
		pg.page_offset=r.left();
		load_page(pg);
		return get_item(pg,val);
	}

	bool bin_index_t::file_node::first(data_t& key,data_t& val) const
	{
		open_index_file();

		if(!root_page)
			return false;
		
		index_t ind;

		if(!first_item(*root_page,ind))return false;

		key=ind.key;
		val.resize(ind.data_len);
		load_data(ind.data_offset,val);
		return true;
	}

	bool bin_index_t::file_node::first_item(page_t& page,index_t& val) const
	{
		if(page.items_count()==0)
			return false;

		index_ref r=page[0];
		if(r.left()==0)
		{
			val.page_offset=page.page_offset;
			val.index_in_page=0;
			val=r;
			return true;
		}

		page_t pg(key_len,parent.page_max);
		pg.page_offset=r.left();
		load_page(pg);
		return first_item(pg,val);
	}

	bool bin_index_t::file_node::next(data_t& key,data_t& val) const
	{
		validate_key_len(key);
		open_index_file();

		if(!root_page)
			return false;
		
		index_t ind;
		ind.key=key;

		if(!next_item(*root_page,ind))return false;

		key=ind.key;
		val.resize(ind.data_len);
		load_data(ind.data_offset,val);
		return true;
	}

	bool bin_index_t::file_node::next_item(page_t& page,index_t& val) const
	{
		page_pr pr(page);
		page_iter p=std::lower_bound(page.begin(),page.end(),val.key,pr);
		
		if(p==page.end())
			return next_item(page,p,val);

		if(pr(val.key,*p))
		{
			if(next_item(page,p,val))
				return true;

			index_ref r=page[static_cast<size_t>(*p)];
			val.page_offset=page.page_offset;
			val.index_in_page=static_cast<size_t>(*p);
			val=r;
			return true;
		}

		++p;

		index_ref r=page[static_cast<size_t>(*p)];

		if(r.left()!=0)
		{
			page_t pg(key_len,parent.page_max);
			pg.page_offset=r.left();
			load_page(pg);
			if(first_item(pg,val))
				return true;
		}

		if(p==page.end())
			return false;

		val.page_offset=page.page_offset;
		val.index_in_page=static_cast<size_t>(*p);
		val=r;
		return true;
	}

	bool bin_index_t::file_node::next_item(page_t& page,const page_iter& p,index_t& val) const
	{
		index_ref r=page[static_cast<size_t>(*p)];

		if(r.left()==0)
			return false;

		page_t pg(key_len,parent.page_max);
		pg.page_offset=r.left();
		load_page(pg);
		return next_item(pg,val);
	}

	bool bin_index_t::file_node::set(const data_t& key,const data_t& val)
	{
		validate_key_len(key);
		open_index_file();

		if(!root_page)
		{
			root_page=page_ptr(new page_t(key_len,parent.page_max));
			append_page(*root_page);
			save_index_data(0,root_page->page_offset);
		}
		
		index_t it;
		it.key=key;

		if(get_item(*root_page,it))
		{
			index_t old_i=it;

			if(it.data_len>=val.size())save_data(it.data_offset,val);
			else it.data_offset=append_data(val);
			it.data_len=val.size();

			if(old_i.data_offset==it.data_offset&&
				old_i.data_len==it.data_len)
				return false;

			update_page(it);
			return false;
		}

		index_t v;
		v.key=key;
		v.data_len=val.size();
		v.data_offset=append_data(val);

		add_item(v);

		++items_count;
		save_index_data(sizeof(file_offset_t),items_count);

		if(items_count<parent.file_max_records)return true;
		if(key_len<=parent.dir_key_len)return true;
		if(disable_split)return true;

		split();
		self_valid=false;

		return true;
	}

	void bin_index_t::file_node::update_page(const index_t& it)
	{
		page_ptr pg;

		if(root_page->page_offset==it.page_offset)
			pg=root_page;
		else
		{
			pg=page_ptr(new page_t(key_len,parent.page_max));
			pg->page_offset=it.page_offset;
			load_page(*pg);
		}

		(*pg)[it.index_in_page].copy_pointers(it);
		save_page(*pg);
	}

	void bin_index_t::file_node::add_item(const index_t& val)
	{
		if(root_page->items_count()<parent.page_max)
		{
			add_item(val,*root_page);
			flush_page(*root_page);
			return;
		}

		page_ptr new_root(new page_t(key_len,parent.page_max));
		index_ref r=(*new_root)[0];
		r.left()=root_page->page_offset;

		add_item(val,*new_root);

		root_page=new_root;
		append_page(*new_root);

		save_index_data(0,new_root->page_offset);
	}

	void bin_index_t::file_node::add_item(const index_t& val,page_t& page)
	{
		page_pr pr(page);
		page_iter p=std::lower_bound(page.begin(),page.end(),val.key,pr);
		
		index_ref r=page[static_cast<size_t>(*p)];

		if(r.left()==0)
		{
			index_t cp(val);
			cp.index_in_page=static_cast<size_t>(*p);
			page.insert_item(cp);
			return;
		}

		page_t child_page(key_len,parent.page_max);
		child_page.page_offset=r.left();
		load_page(child_page);

		if(child_page.items_count()<parent.page_max)
		{
			add_item(val,child_page);
			flush_page(child_page);
			return;
		}

		page_t new_right_page(key_len,parent.page_max);

		split_page(child_page,page,static_cast<size_t>(*p),new_right_page);
		
		if(pr(val.key,*p)) add_item(val,child_page);
		else add_item(val,new_right_page);

		save_page(child_page);
		save_page(new_right_page);
	}

	void bin_index_t::file_node::split_page(page_t& child_page,page_t& parent_page,size_t parent_index,page_t& new_right_page)
	{
		size_t left_count=(child_page.items_count()-1)/2;

		std::copy(
			child_page.buffer.begin()+idx_rec_len(key_len)*(left_count+1),
			child_page.buffer.begin()+idx_rec_len(key_len)*(child_page.items_count()+1),
			new_right_page.buffer.begin());
		new_right_page.items_count()=child_page.items_count()-left_count-1;
		append_page(new_right_page);
		

		index_t val;
		val=child_page[left_count];
		val.index_in_page=parent_index;
		val.left=child_page.page_offset;
		parent_page.insert_item(val);

		child_page.items_count()=left_count;
		parent_page[parent_index+1].left()=new_right_page.page_offset;
	}

	void bin_index_t::file_node::load_data(file_offset_t offset,data_t& res) const
	{
		open_data_file();
		fd->load(offset,res);
	}

	void bin_index_t::file_node::save_data(file_offset_t offset,const data_t& res)
	{
		open_data_file();
		fd->save(offset,res);
	}

	file_offset_t bin_index_t::file_node::append_data(const data_t& res)
	{
		open_data_file();
		return fd->append(res);
	}

	void bin_index_t::file_node::validate_root_offset() const
	{
		root_page.reset();
		items_count=0;

		file_offset_t sz=fi->get_size();		
		if(sz==0)
		{
			data_t d(idx_page_len(key_len,parent.page_max),0);
			const_cast<file_node*>(this)->save_index_data(0,d);
			return;
		}

		data_t d(2*sizeof(file_offset_t));
		load_index_data(0,d);
		items_count=*reinterpret_cast<const file_offset_t*>(&d[sizeof(file_offset_t)]);

		page_ptr p(new page_t(key_len,parent.page_max));
		p->page_offset=*reinterpret_cast<const file_offset_t*>(&d[0]);
		load_page(*p);
		root_page=p;
	}

	void bin_index_t::file_node::load_index_data(file_offset_t offset,data_t& res) const
	{
		fi->load(offset,res);
	}

	void bin_index_t::file_node::save_index_data(file_offset_t offset,const data_t& res)
	{
		fi->save(offset,res);
	}

	file_offset_t bin_index_t::file_node::append_index_data(const data_t& res)
	{
		return fi->append(res);
	}

	void bin_index_t::file_node::save_index_data(file_offset_t offset,file_offset_t res)
	{
		const char* p=reinterpret_cast<const char*>(&res);
		data_t d(p,p+sizeof(file_offset_t));
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
		if(fi){fi->close();fi.reset();}
		if(fd){fd->close();fd.reset();}
	}

	void bin_index_t::file_node::open_index_file() const
	{
		if(fi)return;
		fs::path file_name=fs::path(base_dir)/parent.index_file_name;
		fi=parent.get_file_provider().create(file_name.string());
		validate_root_offset();
	}

	void bin_index_t::file_node::open_data_file() const
	{
		if(fd)return;
		fs::path file_name=fs::path(base_dir)/parent.data_file_name;
		fd=parent.get_file_provider().create(file_name.string());
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

	void bin_index_t::page_t::dump()
	{
		ObjectProgress::log_generator lg(true);
		lg<<"page_offset="<<page_offset<<" items_count()="<<items_count();

		size_t mi=items_count();
		for(size_t i=0;i<mi;i++)
		{
			index_ref r=operator[](i);
			data_t key(r.key_begin(),r.key_end());
			std::string str;
			bin2hex(key,str);
			lg<<"  left="<<r.left()<<" data_offset="<<r.data_offset()<<" data_len="<<r.data_len()<<" key="<<str;
		}

		index_ref r=operator[](mi);
		lg<<"  right="<<r.left();
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
            return 0;

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
