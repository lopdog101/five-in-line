#include "bin_index.h"
#include <boost/filesystem/operations.hpp>
#include <stdexcept>
#include "../extern/binary_find.h"
#include <boost/lexical_cast.hpp>

namespace fs=boost::filesystem;

namespace Gomoku
{
	const size_t bin_index_t::max_pages=16;

	bin_index_t::bin_index_t(const std::string& _base_dir,size_t _key_len,size_t _dir_key_len,file_offset_t _file_max_records,size_t _page_max_size) :
		base_dir(_base_dir),
		key_len(_key_len),
		dir_key_len(_dir_key_len),
		file_max_records(_file_max_records),
		page_max_size(_page_max_size)
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
	bin_index_t::file_node::file_node(const bin_index_t& _parent,const std::string& _base_dir,size_t _key_len) 
		: inode(_parent,_base_dir,_key_len),
		aligned_key_len((_key_len+sizeof(file_offset_t)-1)/sizeof(file_offset_t)*sizeof(file_offset_t))
	{
		self_valid=true;
		disable_split=false;
		items_count=0;
	}

	bool bin_index_t::file_node::get(const data_t& key,data_t& val) const
	{
		validate_key_len(key);
		open_index_file();

		if(!root_page)
			return false;
		
		index_t ind;
		align_key(key,ind);

		if(!get_item(ind))return false;

		val.resize(ind.data_len);
		load_data(ind.data_offset,val);
		return true;
	}

	bool bin_index_t::file_node::get_item(index_t& val) const
	{
		page_ptr page_ptr=root_page;

		while(true)
		{
			page_t& page=*page_ptr;

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
				break;

			page_ptr=get_page(r.left());
		}

		return false;

	}

	bool bin_index_t::file_node::first(data_t& key,data_t& val) const
	{
		open_index_file();

		index_t ind;
		if(!first_item(root_page,ind))return false;

		key=ind.key;
		key.resize(key_len);
		val.resize(ind.data_len);
		load_data(ind.data_offset,val);
		return true;
	}

	bool bin_index_t::file_node::first_item(page_ptr pp,index_t& val) const
	{
		while(pp!=0)
		{
			page_t& page=*pp;
			
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

			pp=get_page(r.left());
		}
		return false;
	}

	bool bin_index_t::file_node::next(data_t& key,data_t& val) const
	{
		validate_key_len(key);
		open_index_file();

		if(!root_page)
			return false;
		
		index_t ind;
		align_key(key,ind);

		if(!next_item(root_page,ind))return false;

		key=ind.key;
		key.resize(key_len);
		val.resize(ind.data_len);
		load_data(ind.data_offset,val);
		return true;
	}

	bool bin_index_t::file_node::next_item(page_ptr pp,index_t& val) const
	{
		bool ret=false;

		while(pp!=0)
		{
			page_t& page = *pp;

			page_pr pr(page);
			page_iter p=std::lower_bound(page.begin(),page.end(),val.key,pr);
			
			index_ref r=page[static_cast<size_t>(*p)];

			if(p==page.end())
			{
				if(r.left()==0)
					break;

				pp=get_page(r.left());
				continue;
			}

			if(pr(val.key,*p))
			{
				val.page_offset=page.page_offset;
				val.index_in_page=static_cast<size_t>(*p);
				val=r;
				ret=true;

				if(r.left()==0)
					break;

				pp=get_page(r.left());
				continue;
			}

			++p;

			r=page[static_cast<size_t>(*p)];

			if(r.left()!=0)
			{
				page_ptr pg=get_page(r.left());
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

		return ret;
	}

	bool bin_index_t::file_node::set(const data_t& key,const data_t& val)
	{
		validate_key_len(key);
		open_index_file();

		if(!root_page)
		{
			root_page=create_page();
			append_page(*root_page);
			save_index_data(0,root_page->page_offset);
			add_page(root_page);
		}
		
		index_t it;
		align_key(key,it);

		if(get_item(it))
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
		align_key(key,v);
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
		page_ptr pg=get_page(it.page_offset);

		(*pg)[it.index_in_page].copy_pointers(it);
		pg->dirty=true;
	}

	void bin_index_t::file_node::add_item(const index_t& val)
	{
		if(root_page->items_count()<root_page->page_max)
		{
			add_item(val,*root_page);
			return;
		}

		page_ptr new_root(create_page());
		index_ref r=(*new_root)[0];
		r.left()=root_page->page_offset;

		add_item(val,*new_root);

		root_page=new_root;
		append_page(*new_root);
		add_page(root_page);

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

		page_ptr child_page=get_page(r.left());

		if(child_page->items_count()<child_page->page_max)
		{
			add_item(val,*child_page);
			return;
		}

		page_ptr new_right_page(create_page());

		split_page(*child_page,page,static_cast<size_t>(*p),*new_right_page);
		
		if(pr(val.key,*p)) add_item(val,*child_page);
		else add_item(val,*new_right_page);

		add_page(new_right_page);
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

		child_page.dirty=true;
		new_right_page.dirty=true;
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
			data_t d(parent.page_max_size,0);
			const_cast<file_node*>(this)->save_index_data(0,d);
			return;
		}

		data_t d(2*sizeof(file_offset_t));
		load_index_data(0,d);
		items_count=*reinterpret_cast<const file_offset_t*>(&d[sizeof(file_offset_t)]);

		page_ptr p(create_page());
		p->page_offset=*reinterpret_cast<const file_offset_t*>(&d[0]);
		load_page(*p);
		const_cast<file_node*>(this)->add_page(p);
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
		flush();
		if(fi){fi->close();fi.reset();}
		if(fd){fd->close();fd.reset();}
	}

	void bin_index_t::file_node::open_index_file() const
	{
		if(fi)return;
		fs::path file_name=fs::path(base_dir)/parent.index_file_name;
		fi=file_access_ptr(new regular_file_t(file_name.string()));
		validate_root_offset();
	}

	void bin_index_t::file_node::open_data_file() const
	{
		if(fd)return;
		fs::path file_name=fs::path(base_dir)/parent.data_file_name;
		fd=file_access_ptr(new paged_file_t(file_name.string()));
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

	void bin_index_t::file_node::flush()
	{
		pages_t::iterator from=pages.begin();
		pages_t::iterator to=pages.end();
		for(;from!=to;++from)
			flush_page(*from->second);
	}

	bin_index_t::page_ptr bin_index_t::file_node::get_page(file_offset_t page_offset)
	{
		pages_t::iterator it=pages.find(page_offset);

		if(it!=pages.end())
		{
			page_ptr p=it->second;
			if(!p->left.expired())
			{
				remove_from_list(p);
				insert_into_list(p,first_page.lock());
			}
			return p;
		}

		page_ptr p(create_page());
		p->self=p;
		p->page_offset=page_offset;
		load_page(*p);

		add_page(p);

		return p;
	}

	void bin_index_t::file_node::remove_from_list(const page_ptr& p)
	{
		page_ptr left=p->left.lock();
		page_ptr right=p->right.lock();

		if(left)left->right=right;
		else first_page=right;

		if(right)right->left=left;
		else last_page=left;

		p->left.reset();
		p->right.reset();
	}

	void bin_index_t::file_node::insert_into_list(const page_ptr& p,page_ptr& right)
	{
		if(!right)
		{
			page_ptr l=last_page.lock();
			if(!l)
			{
				first_page=last_page=p;
				return;
			}

			l->right=p;
			p->left=l;
			last_page=p;

			return;
		}


		page_ptr l=right->left.lock();
		
		p->left=l;
		p->right=right;

		if(l)l->right=p;
		else first_page=p;

		right->left=p;
	}

	void bin_index_t::file_node::add_page(const page_ptr& p)
	{
		pages[p->page_offset]=p;
		insert_into_list(p,first_page.lock());

		remove_oldest();
	}

	void bin_index_t::file_node::remove_oldest()
	{
		for(page_wptr wp=last_page;pages.size()>=max_pages&&!wp.expired();)
		{
			page_ptr r;
			page_ptr l;

			{
				page_ptr p=wp.lock();
				r=p->right.lock();
				l=p->left.lock();
				flush_page(*p);
				pages.erase(p->page_offset);
				remove_from_list(p);
			}

			//Somebody still use this page and we can't delete it
			if(!wp.expired())
			{
				page_ptr p=wp.lock();
				pages[p->page_offset]=p;
				insert_into_list(p,r);
			}

			wp=l;
		}
	}

	bin_index_t::page_ptr bin_index_t::file_node::create_page() const
	{
		return page_ptr(new page_t(aligned_key_len,parent.page_max_size));
	}

	void bin_index_t::file_node::align_key(const data_t& key,index_t& res) const
	{
		res.key.resize(aligned_key_len);
		std::copy(key.begin(),key.end(),res.key.begin());
		std::fill(res.key.begin()+key_len,res.key.end(),0);
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
