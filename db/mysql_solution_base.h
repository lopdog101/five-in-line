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

	class statement_result_t : public common_holder_t<MYSQL_STMT,my_bool STDCALL (MYSQL_STMT *)>
	{
	public:
		statement_result_t(MYSQL_STMT* _val = 0) : common_holder_t<MYSQL_STMT,my_bool STDCALL (MYSQL_STMT *)>(_val,mysql_stmt_free_result){}
	};

	static const unsigned def_buf_size = 800;
	static const unsigned max_level = 150;


	struct base_query_t
	{
		statement_t st;

		data_t key_buf;
		unsigned long key_buf_size;

		unsigned st_state;
		unsigned long long st_wins_count;
		unsigned long long st_fails_count;

		data_t neitrals_buf;
		unsigned long neitrals_buf_size;

		data_t solved_wins_buf;
		unsigned long solved_wins_buf_size;

		data_t solved_fails_buf;
		unsigned long solved_fails_buf_size;

		data_t tree_wins_buf;
		unsigned long tree_wins_buf_size;

		data_t tree_fails_buf;
		unsigned long tree_fails_buf_size;
	};

	struct get_query_t : public base_query_t
	{
		MYSQL_BIND params[1];
		MYSQL_BIND results[8];

		void init(MYSQL* conn,size_t key_len);
		bool execute(const steps_t& key,sol_state_t& res);
	};

	struct set_query_t : public base_query_t
	{
		MYSQL_BIND params[17];

		void init(MYSQL* conn,size_t key_len);
		void execute(const sol_state_t& res);
	};

	struct first_query_t : public base_query_t
	{
		MYSQL_BIND results[1];

		void init(MYSQL* conn,size_t key_len);
		bool execute(steps_t& res);
	};

	struct next_query_t : public base_query_t
	{
		MYSQL_BIND params[1];
		MYSQL_BIND results[1];

		data_t res_key;
		unsigned long res_key_size;

		void init(MYSQL* conn,size_t key_len);
		bool execute(steps_t& res);
	};

	class level_t
	{
		const size_t key_len;
		MYSQL* conn;

		get_query_t qget;
		set_query_t qset;
		first_query_t qfirst;
		next_query_t qnext;

		level_t(const level_t&);
		void operator=(const level_t&);
	public:

		level_t(size_t _key_len,MYSQL* _conn);
		
		bool get(const steps_t& key,sol_state_t& res){return qget.execute(key,res);}
		void set(const sol_state_t& res){return qset.execute(res);}
		bool first(steps_t& res){return qfirst.execute(res);}
		bool next(steps_t& res){return qnext.execute(res);}
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
