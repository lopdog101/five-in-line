#ifndef bin_indexH
#define bin_indexH
#include "paged_file_access.h"
#include "ibin_index.h"
#include <map>
#include "../extern/exception_catch.h"

namespace Gomoku
{

class bin_index_t : public ibin_index_t
{
public:

	struct index_t
	{
		data_t key;
		size_t left;
		size_t right;
		size_t data_offset;
		size_t data_len;
		char balance;

		index_t()
		{
			left=right=0;
			data_offset=0;
			data_len=0;
			balance=0;
		}

		inline bool operator==(const index_t& rhs) const
		{
			return key==rhs.key&&
				left==rhs.left&&
				right==rhs.right&&
				data_offset==rhs.data_offset&&
				data_len==rhs.data_len&&
				balance==rhs.balance;
		}
	};

	struct data_less_pr
	{
		inline bool cmp(const data_t& a,const data_t& b) const
		{
			return std::lexicographical_compare(a.begin(),a.end(),b.begin(),b.end());
		}

		inline bool operator()(const data_t& a,const data_t& b) const{return cmp(a,b);}
		inline bool operator()(const index_t& a,const index_t& b) const{return cmp(a.key,b.key);}
		inline bool operator()(const index_t& a,const data_t& b) const{return cmp(a.key,b);}
		inline bool operator()(const data_t& a,const index_t& b) const{return cmp(a,b.key);}
	};

	class inode
	{
		inode(const inode&);
		void operator=(const inode&);
	protected:
		const bin_index_t& parent;
		const std::string base_dir;
		const size_t key_len;

        void validate_key_len(const data_t& key) const;
	public:
		inode(const bin_index_t& _parent,const std::string& _base_dir,size_t _key_len) 
			: parent(_parent),base_dir(_base_dir),key_len(_key_len)
		{
		}

		virtual ~inode(){}

		virtual bool get(const data_t& key,data_t& val) const=0;
		virtual bool set(const data_t& key,const data_t& val)=0;
		virtual bool first(data_t& key,data_t& val) const=0;
		virtual bool next(data_t& key,data_t& val) const=0;

		virtual bool is_valid() const{return true;}
	};

	typedef boost::shared_ptr<inode> node_ptr;

	class file_node : public inode
	{
		bool self_valid;
		mutable file_access_ptr fi;
		mutable file_access_ptr fd;
		mutable size_t root_offset;
		mutable size_t items_count;

		void load_data(size_t offset,data_t& res) const;
		void save_data(size_t offset,const data_t& res);
		size_t append_data(const data_t& res);

		void load_index_data(size_t offset,data_t& res) const;
		void save_index_data(size_t offset,const data_t& res);
		void save_index_data(size_t offset,size_t res);
		size_t append_index_data(const data_t& res);

		inline size_t irec_len() const{return key_len+4*sizeof(size_t)+sizeof(char);}

		void split();
		void close_files();
		void open_index_file() const;
		void open_data_file() const;
		std::string index_file_name() const;
		std::string data_file_name() const;
		void validate_root_offset() const;

		void pack(const index_t& val,data_t& bin) const;
		void unpack(index_t& val,const data_t& bin) const;
		void load_index(size_t offset,index_t& val) const;
		void save_index(size_t offset,const index_t& val);
		size_t append_index(const index_t& val);

		bool get_item(size_t offset,index_t& val) const
		{
			size_t index_value_offset=0;
			return get_item(offset,val,index_value_offset);
		}

		bool get_item(size_t offset,index_t& val,size_t& index_value_offset) const;
		bool first_item(size_t offset,index_t& val) const;
		bool next_item(size_t offset,index_t& val) const;
		bool add_item(size_t& offset,const index_t& val,bool& new_level);
	public:
		bool disable_split;

		file_node(const bin_index_t& _parent,const std::string& _base_dir,size_t _key_len) 
			: inode(_parent,_base_dir,_key_len)
		{
			self_valid=true;
			disable_split=false;
			root_offset=items_count=0;
		}

		~file_node(){close_files();}

		bool get(const data_t& key,data_t& val) const;
		bool set(const data_t& key,const data_t& val);
		bool first(data_t& key,data_t& val) const;
		bool next(data_t& key,data_t& val) const;
		virtual bool is_valid() const{return self_valid;}
	};

	class dir_node : public inode
	{
		mutable datas_t indexes;
		mutable bool index_valid;
		
		mutable node_ptr sub_node;
		mutable data_t sub_name;


		void validate_index() const;
		void load_index() const;
		void validate_sub(const data_t& name) const;
		void create_text_child(const data_t& name) const;
	public:
		dir_node(const bin_index_t& _parent,const std::string& _base_dir,size_t _key_len) 
			: inode(_parent,_base_dir,_key_len)
		{
			index_valid=false;
		}

		bool get(const data_t& key,data_t& val) const;
		bool set(const data_t& key,const data_t& val);
		bool first(data_t& key,data_t& val) const;
		bool next(data_t& key,data_t& val) const;
	};
private:
	const size_t key_len;
	const size_t dir_key_len;
	const size_t file_max_records;
	const std::string base_dir;
	mutable node_ptr root;
	size_t items_count;
	paged_file_provider_t file_provider;

	void validate_root() const;
	node_ptr create_node(const std::string& base_dir,size_t key_len) const;

	void load_items_count();
	void save_items_count();
public:
	std::string index_file_name;
	std::string data_file_name;
	std::string items_count_file_name;

	bin_index_t(const std::string& _base_dir,size_t _key_len,size_t _dir_key_len=1,size_t _file_max_records=1048576);
	~bin_index_t()
    {
        try
        {
            save_items_count();
        }
        UNCATCHED_EXCEPTION_CATCH;
    }

	
	bool get(const data_t& key,data_t& val) const
	{
		validate_root();
		return root->get(key,val);
	}

	bool set(const data_t& key,const data_t& val)
	{
		validate_root();
		bool r=root->set(key,val);
		if(!root->is_valid())root.reset();
		
		if(r)
		{
			++items_count;
			if((items_count%1024)==0)save_items_count();
		}
		
		return r;
	}

	bool first(data_t& key,data_t& val) const
	{
		validate_root();
		return root->first(key,val);
	}

	bool next(data_t& key,data_t& val) const
	{
		validate_root();
		return root->next(key,val);
	}

	inline const ifile_access_provider_t& get_file_provider() const{return file_provider;}
};

class bin_indexes_t : public ibin_indexes_t
{
public:
  typedef boost::shared_ptr<bin_index_t> item_ptr;
  typedef std::map<unsigned,item_ptr> indexes_t;
private:
  const std::string base_dir;
  const size_t len_per_level;
  indexes_t indexes;
public:
  bin_indexes_t(const std::string& _base_dir,size_t _len_per_level) : base_dir(_base_dir),len_per_level(_len_per_level) {}

  unsigned get_root_level() const;
  bin_index_t& get_index(unsigned steps_count);
  bin_index_t* find_index(unsigned steps_count);
};



}//namespace

#endif
