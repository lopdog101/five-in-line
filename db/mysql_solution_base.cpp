#include "mysql_solution_base.h"

namespace Gomoku{ namespace Mysql
{

base_t::base_t(const std::string& db_name)
{
	static MYSQL *c = 0;
	
	if(c == 0)
	{
		c=mysql_init(NULL);
		if(c==0)
			throw std::runtime_error("mysql_init() has failed");
	}

	conn.set(mysql_real_connect(c, "localhost", NULL, NULL, db_name.c_str(), 0, NULL, 0));
	if(!conn.get())
		throw std::runtime_error("mysql_real_connect() has failed");
}

level_ptr base_t::get_level(size_t key_len) const
{
	levels_t::const_iterator it=levels.find(key_len);
	if(it!=levels.end())
		return it->second;

	level_ptr p(new level_t(key_len,conn.get()));
	levels[key_len]=p;
	return p;
}


bool base_t::get(sol_state_t& res) const
{
	steps_t key=res.key;
	sort_steps(key);

	level_ptr l=get_level(key.size());
	return l->get(key,res);
}

void base_t::set(const sol_state_t& _val)
{
	sol_state_t val=_val;
	sort_steps(val.key);

	level_ptr l=get_level(val.key.size());
	return l->set(val);
}

steps_t base_t::get_root_key() const
{
	for(unsigned i=1;i<=max_level;i++)
	{
		steps_t steps;
		level_ptr l=get_level(i);
		if(l->first(steps))
			return steps;
	}

	return steps_t();
}


void base_t::width_first_search_from_bottom_to_top(sol_state_width_pr& pr)
{
	for(unsigned i=max_level;i>0;i--)
	{
		level_ptr l=get_level(i);
		sol_state_t st;
		
		for(bool j=l->first(st.key);j;j=l->next(st.key))
		{
			if(pr.is_canceled())
				throw e_cancel();
			
			if(!l->get(st.key,st))
				throw std::runtime_error(__FUNCTION__ ": state doesn't exists");

			if(pr.on_enter_node(st))
			{
				set(st);
			}
		}
	}
}

level_t::level_t(size_t _key_len,MYSQL* _conn) :
	key_len(_key_len),conn(_conn)
{
	qget.init(conn,key_len);
	qset.init(conn,key_len);
	qfirst.init(conn,key_len);
	qnext.init(conn,key_len);
}

void get_query_t::init(MYSQL* conn,size_t key_len)
{
	MYSQL_STMT* stmt = mysql_stmt_init(conn);
	if(stmt == NULL)
		throw std::runtime_error(__FUNCTION__ ": mysql_stmt_init() failed");

	st.set(stmt);

	std::string query="select state,wins_count,fails_count,neitrals,solved_wins,solved_fails,tree_wins,tree_fails from s"
		+boost::lexical_cast<std::string>(key_len)+" where k=?";

	int query_ret = mysql_stmt_prepare(stmt, query.c_str(), query.size());
	if(query_ret!=0)
		throw std::runtime_error(__FUNCTION__ ": mysql_stmt_prepare() failed");


	memset(params, 0, sizeof(params));

	key_buf_size=key_len*3;
	key_buf.resize(key_buf_size);

	MYSQL_BIND* p=&params[0];
	p->buffer_type=MYSQL_TYPE_BLOB;
	p->buffer=(char *)&key_buf.front();
	p->buffer_length=key_buf_size;
	p->is_null=0;
	p->length=&key_buf_size;

	query_ret = mysql_stmt_bind_param(stmt, params);
	if(query_ret!=0)
		throw std::runtime_error(__FUNCTION__ ": mysql_stmt_bind_param() failed");


	neitrals_buf.resize(def_buf_size);
	solved_wins_buf.resize(def_buf_size);
	solved_fails_buf.resize(def_buf_size);
	tree_wins_buf.resize(def_buf_size);
	tree_fails_buf.resize(def_buf_size);

	neitrals_buf_size=def_buf_size;
	solved_wins_buf_size=def_buf_size;
	solved_fails_buf_size=def_buf_size;
	tree_wins_buf_size=def_buf_size;
	tree_fails_buf_size=def_buf_size;
	
	memset(results, 0, sizeof(results));

	p=&results[0];
	p->buffer_type=MYSQL_TYPE_LONG;
	p->buffer=(char *)&st_state;

	p=&results[1];
	p->buffer_type=MYSQL_TYPE_LONGLONG;
	p->buffer=(char *)&st_wins_count;

	p=&results[2];
	p->buffer_type=MYSQL_TYPE_LONGLONG;
	p->buffer=(char *)&st_fails_count;
	
	p=&results[3];
	p->buffer_type=MYSQL_TYPE_BLOB;
	p->buffer=(char *)&neitrals_buf.front();
	p->buffer_length=def_buf_size;
	p->is_null=0;
	p->length=&neitrals_buf_size;
	
	p=&results[4];
	p->buffer_type=MYSQL_TYPE_BLOB;
	p->buffer=(char *)&solved_wins_buf.front();
	p->buffer_length=def_buf_size;
	p->is_null=0;
	p->length=&solved_wins_buf_size;
	
	p=&results[5];
	p->buffer_type=MYSQL_TYPE_BLOB;
	p->buffer=(char *)&solved_fails_buf.front();
	p->buffer_length=def_buf_size;
	p->is_null=0;
	p->length=&solved_fails_buf_size;
	
	p=&results[6];
	p->buffer_type=MYSQL_TYPE_BLOB;
	p->buffer=(char *)&tree_wins_buf.front();
	p->buffer_length=def_buf_size;
	p->is_null=0;
	p->length=&tree_wins_buf_size;
	
	p=&results[7];
	p->buffer_type=MYSQL_TYPE_BLOB;
	p->buffer=(char *)&tree_fails_buf.front();
	p->buffer_length=def_buf_size;
	p->is_null=0;
	p->length=&tree_fails_buf_size;

	query_ret = mysql_stmt_bind_result(stmt, results);
	if(query_ret!=0)
		throw std::runtime_error(__FUNCTION__ ": mysql_stmt_bind_result() failed");
}

bool get_query_t::execute(const steps_t& key,sol_state_t& res)
{
	MYSQL_STMT* stmt=st.get();

	neitrals_buf.resize(def_buf_size);
	solved_wins_buf.resize(def_buf_size);
	solved_fails_buf.resize(def_buf_size);
	tree_wins_buf.resize(def_buf_size);
	tree_fails_buf.resize(def_buf_size);

	points2bin(key,key_buf);
	int query_ret=mysql_stmt_execute(stmt);
	if(query_ret!=0)
		throw std::runtime_error(__FUNCTION__ ": mysql_stmt_execute() failed");
	
	query_ret=mysql_stmt_store_result(stmt);
	if(query_ret!=0)
		throw std::runtime_error(__FUNCTION__ ": mysql_stmt_store_result() failed");

	statement_result_t hld_result(stmt);

	if(mysql_stmt_num_rows(stmt)==0)
		return false;
	
	query_ret=mysql_stmt_fetch(stmt);
	if(query_ret!=0)
		throw std::runtime_error(__FUNCTION__ ": mysql_stmt_fetch() failed");

	res.state=static_cast<SolState>(st_state);
	res.wins_count=st_wins_count;
	res.fails_count=st_fails_count;

	neitrals_buf.resize(neitrals_buf_size);
	solved_wins_buf.resize(solved_wins_buf_size);
	solved_fails_buf.resize(solved_fails_buf_size);
	tree_wins_buf.resize(tree_wins_buf_size);
	tree_fails_buf.resize(tree_fails_buf_size);

	bin2points(neitrals_buf,res.neitrals);
	bin2points(solved_wins_buf,res.solved_wins);
	bin2points(solved_fails_buf,res.solved_fails);
	bin2points(tree_wins_buf,res.tree_wins);
	bin2points(tree_fails_buf,res.tree_fails);
	
	return true;
}

void set_query_t::init(MYSQL* conn,size_t key_len)
{
	MYSQL_STMT* stmt = mysql_stmt_init(conn);
	if(stmt == NULL)
		throw std::runtime_error(__FUNCTION__ ": mysql_stmt_init() failed");

	st.set(stmt);

	std::string query="insert into s"+boost::lexical_cast<std::string>(key_len)
		+"(k,state,wins_count,fails_count,neitrals,solved_wins,solved_fails,tree_wins,tree_fails) values(?,?,?,?,?,?,?,?,?)"
		 " ON DUPLICATE KEY UPDATE state=?,wins_count=?,fails_count=?,neitrals=?,solved_wins=?,solved_fails=?,tree_wins=?,tree_fails=?";

	int query_ret = mysql_stmt_prepare(stmt, query.c_str(), query.size());
	if(query_ret!=0)
		throw std::runtime_error(__FUNCTION__ ": mysql_stmt_prepare() failed");

	neitrals_buf.resize(def_buf_size);
	solved_wins_buf.resize(def_buf_size);
	solved_fails_buf.resize(def_buf_size);
	tree_wins_buf.resize(def_buf_size);
	tree_fails_buf.resize(def_buf_size);

	neitrals_buf_size=def_buf_size;
	solved_wins_buf_size=def_buf_size;
	solved_fails_buf_size=def_buf_size;
	tree_wins_buf_size=def_buf_size;
	tree_fails_buf_size=def_buf_size;

	memset(params, 0, sizeof(params));

	key_buf_size=key_len*3;
	key_buf.resize(key_buf_size);

	MYSQL_BIND* p=&params[0];
	p->buffer_type=MYSQL_TYPE_BLOB;
	p->buffer=(char *)&key_buf.front();
	p->buffer_length=key_buf_size;
	p->is_null=0;
	p->length=&key_buf_size;

	p=&params[1];
	p->buffer_type=MYSQL_TYPE_LONG;
	p->buffer=(char *)&st_state;

	p=&params[2];
	p->buffer_type=MYSQL_TYPE_LONGLONG;
	p->buffer=(char *)&st_wins_count;

	p=&params[3];
	p->buffer_type=MYSQL_TYPE_LONGLONG;
	p->buffer=(char *)&st_fails_count;
	
	p=&params[4];
	p->buffer_type=MYSQL_TYPE_BLOB;
	p->buffer=(char *)&neitrals_buf.front();
	p->buffer_length=def_buf_size;
	p->is_null=0;
	p->length=&neitrals_buf_size;
	
	p=&params[5];
	p->buffer_type=MYSQL_TYPE_BLOB;
	p->buffer=(char *)&solved_wins_buf.front();
	p->buffer_length=def_buf_size;
	p->is_null=0;
	p->length=&solved_wins_buf_size;
	
	p=&params[6];
	p->buffer_type=MYSQL_TYPE_BLOB;
	p->buffer=(char *)&solved_fails_buf.front();
	p->buffer_length=def_buf_size;
	p->is_null=0;
	p->length=&solved_fails_buf_size;
	
	p=&params[7];
	p->buffer_type=MYSQL_TYPE_BLOB;
	p->buffer=(char *)&tree_wins_buf.front();
	p->buffer_length=def_buf_size;
	p->is_null=0;
	p->length=&tree_wins_buf_size;
	
	p=&params[8];
	p->buffer_type=MYSQL_TYPE_BLOB;
	p->buffer=(char *)&tree_fails_buf.front();
	p->buffer_length=def_buf_size;
	p->is_null=0;
	p->length=&tree_fails_buf_size;

	p=&params[9];
	p->buffer_type=MYSQL_TYPE_LONG;
	p->buffer=(char *)&st_state;

	p=&params[10];
	p->buffer_type=MYSQL_TYPE_LONGLONG;
	p->buffer=(char *)&st_wins_count;

	p=&params[11];
	p->buffer_type=MYSQL_TYPE_LONGLONG;
	p->buffer=(char *)&st_fails_count;
	
	p=&params[12];
	p->buffer_type=MYSQL_TYPE_BLOB;
	p->buffer=(char *)&neitrals_buf.front();
	p->buffer_length=def_buf_size;
	p->is_null=0;
	p->length=&neitrals_buf_size;
	
	p=&params[13];
	p->buffer_type=MYSQL_TYPE_BLOB;
	p->buffer=(char *)&solved_wins_buf.front();
	p->buffer_length=def_buf_size;
	p->is_null=0;
	p->length=&solved_wins_buf_size;
	
	p=&params[14];
	p->buffer_type=MYSQL_TYPE_BLOB;
	p->buffer=(char *)&solved_fails_buf.front();
	p->buffer_length=def_buf_size;
	p->is_null=0;
	p->length=&solved_fails_buf_size;
	
	p=&params[15];
	p->buffer_type=MYSQL_TYPE_BLOB;
	p->buffer=(char *)&tree_wins_buf.front();
	p->buffer_length=def_buf_size;
	p->is_null=0;
	p->length=&tree_wins_buf_size;
	
	p=&params[16];
	p->buffer_type=MYSQL_TYPE_BLOB;
	p->buffer=(char *)&tree_fails_buf.front();
	p->buffer_length=def_buf_size;
	p->is_null=0;
	p->length=&tree_fails_buf_size;

	query_ret = mysql_stmt_bind_param(stmt, params);
	if(query_ret!=0)
		throw std::runtime_error(__FUNCTION__ ": mysql_stmt_bind_param() failed");
}

void set_query_t::execute(const sol_state_t& res)
{
	MYSQL_STMT* stmt=st.get();

	points2bin(res.key,key_buf);
	points2bin(res.neitrals,neitrals_buf);
	points2bin(res.solved_wins,solved_wins_buf);
	points2bin(res.solved_fails,solved_fails_buf);
	points2bin(res.tree_wins,tree_wins_buf);
	points2bin(res.tree_fails,tree_fails_buf);

	neitrals_buf_size=neitrals_buf.size();
	solved_wins_buf_size=solved_wins_buf.size();
	solved_fails_buf_size=solved_fails_buf.size();
	tree_wins_buf_size=tree_wins_buf.size();
	tree_fails_buf_size=tree_fails_buf.size();

	st_state=res.state;
	st_wins_count=res.wins_count;
	st_fails_count=res.fails_count;

	int query_ret=mysql_stmt_execute(stmt);
	if(query_ret!=0)
		throw std::runtime_error(std::string(__FUNCTION__ ": mysql_stmt_execute() failed: ")+mysql_stmt_error(stmt));
}

void first_query_t::init(MYSQL* conn,size_t key_len)
{
	MYSQL_STMT* stmt = mysql_stmt_init(conn);
	if(stmt == NULL)
		throw std::runtime_error(__FUNCTION__ ": mysql_stmt_init() failed");

	st.set(stmt);

	std::string query="select k from s"
		+boost::lexical_cast<std::string>(key_len)+" order by k limit 1";

	int query_ret = mysql_stmt_prepare(stmt, query.c_str(), query.size());
	if(query_ret!=0)
		throw std::runtime_error(__FUNCTION__ ": mysql_stmt_prepare() failed");


	key_buf_size=key_len*3;
	key_buf.resize(key_buf_size);

	memset(results, 0, sizeof(results));

	MYSQL_BIND* p=&results[0];
	p->buffer_type=MYSQL_TYPE_BLOB;
	p->buffer=(char *)&key_buf.front();
	p->buffer_length=key_buf_size;
	p->is_null=0;
	p->length=&key_buf_size;

	query_ret = mysql_stmt_bind_result(stmt, results);
	if(query_ret!=0)
		throw std::runtime_error(__FUNCTION__ ": mysql_stmt_bind_result() failed");
}

bool first_query_t::execute(steps_t& res)
{
	MYSQL_STMT* stmt=st.get();

	int query_ret=mysql_stmt_execute(stmt);
	if(query_ret!=0)
		throw std::runtime_error(__FUNCTION__ ": mysql_stmt_execute() failed");
	
	query_ret=mysql_stmt_store_result(stmt);
	if(query_ret!=0)
		throw std::runtime_error(__FUNCTION__ ": mysql_stmt_store_result() failed");

	statement_result_t hld_result(stmt);

	if(mysql_stmt_num_rows(stmt)==0)
		return false;
	
	query_ret=mysql_stmt_fetch(stmt);
	if(query_ret!=0)
		throw std::runtime_error(__FUNCTION__ ": mysql_stmt_fetch() failed");

	bin2points(key_buf,res);

	return true;
}

void next_query_t::init(MYSQL* conn,size_t key_len)
{
	MYSQL_STMT* stmt = mysql_stmt_init(conn);
	if(stmt == NULL)
		throw std::runtime_error(__FUNCTION__ ": mysql_stmt_init() failed");

	st.set(stmt);

	std::string query="select k from s"
		+boost::lexical_cast<std::string>(key_len)+" where k>? order by k limit 1";

	int query_ret = mysql_stmt_prepare(stmt, query.c_str(), query.size());
	if(query_ret!=0)
		throw std::runtime_error(__FUNCTION__ ": mysql_stmt_prepare() failed");


	key_buf_size=key_len*3;
	key_buf.resize(key_buf_size);

	MYSQL_BIND* p=&params[0];
	p->buffer_type=MYSQL_TYPE_BLOB;
	p->buffer=(char *)&key_buf.front();
	p->buffer_length=key_buf_size;
	p->is_null=0;
	p->length=&key_buf_size;

	query_ret = mysql_stmt_bind_param(stmt, params);
	if(query_ret!=0)
		throw std::runtime_error(__FUNCTION__ ": mysql_stmt_bind_param() failed");

	res_key_size=key_len*3;
	res_key.resize(res_key_size);

	memset(results, 0, sizeof(results));

	p=&results[0];
	p->buffer_type=MYSQL_TYPE_BLOB;
	p->buffer=(char *)&res_key.front();
	p->buffer_length=res_key_size;
	p->is_null=0;
	p->length=&res_key_size;

	query_ret = mysql_stmt_bind_result(stmt, results);
	if(query_ret!=0)
		throw std::runtime_error(__FUNCTION__ ": mysql_stmt_bind_result() failed");
}

bool next_query_t::execute(steps_t& res)
{
	MYSQL_STMT* stmt=st.get();

	points2bin(res,key_buf);

	int query_ret=mysql_stmt_execute(stmt);
	if(query_ret!=0)
		throw std::runtime_error(__FUNCTION__ ": mysql_stmt_execute() failed");
	
	query_ret=mysql_stmt_store_result(stmt);
	if(query_ret!=0)
		throw std::runtime_error(__FUNCTION__ ": mysql_stmt_store_result() failed");

	statement_result_t hld_result(stmt);

	if(mysql_stmt_num_rows(stmt)==0)
		return false;
	
	query_ret=mysql_stmt_fetch(stmt);
	if(query_ret!=0)
		throw std::runtime_error(__FUNCTION__ ": mysql_stmt_fetch() failed");

	bin2points(res_key,res);

	return true;
}



}}//namespace
