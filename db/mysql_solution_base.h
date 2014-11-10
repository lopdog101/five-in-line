#ifndef mysql_solution_baseH
#define mysql_solution_baseH
#include <map>
#include <my_global.h>
#include <mysql.h>
#include "solution_tree.h"
#include "../extern/common_holder.h"


namespace Gomoku{ namespace Mysql
{
	class connection_t : public common_holder_t<MYSQL,void STDCALL (MYSQL *)>
	{
	public:
		connection_t(MYSQL* _val = 0) : common_holder_t<MYSQL,void STDCALL (MYSQL *)>(_val,mysql_close){}
	};

	class statement_t : public common_holder_t<MYSQL_STMT,my_bool STDCALL (MYSQL_STMT *)>
	{
	public:
		statement_t(MYSQL_STMT* _val = 0) : common_holder_t<MYSQL_STMT,my_bool STDCALL (MYSQL_STMT *)>(_val,mysql_stmt_close){}
	};

	struct get_query_t
	{
		statement_t st;
		MYSQL_BIND params[1];
		data_t key_buf;
		unsigned long key_buf_size;

		void init(MYSQL* conn,size_t key_len);
	};

	class level_t
	{
		const size_t key_len;
		MYSQL* conn;

		get_query_t qget;

		void create_set_statement();

		level_t(const level_t&);
		void operator=(const level_t&);
	public:

		level_t(size_t _key_len,MYSQL* _conn);
		
		bool get(const steps_t& key,sol_state_t& res);
	};

	typedef boost::shared_ptr<level_t> level_ptr;
	
	typedef std::map<size_t,level_ptr> levels_t;

	class base_t :  public isolution_tree_base_t
	{
		connection_t conn;
		mutable levels_t levels;

		level_ptr get_level(size_t key_len) const;
	public:
		base_t(const std::string& db_name);

		virtual bool get(sol_state_t& res) const;
		virtual void set(const sol_state_t& val);
		virtual steps_t get_root_key() const;
        virtual void width_first_search_from_bottom_to_top(sol_state_width_pr& pr);
	};

} }//namespace

#endif
