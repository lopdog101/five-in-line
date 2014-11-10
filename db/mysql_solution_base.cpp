#include "mysql_solution_base.h"

namespace Gomoku{ namespace Mysql
{

base_t::base_t(const std::string& db_name)
{
	static MYSQL *c = 0;
	
	if(c == 0)
	{
		mysql_init(NULL);
		if(c==0)
			throw std::runtime_error("mysql_init() has failed");
	}

	conn.set(mysql_real_connect(c, "localhost", NULL, NULL, db_name.c_str(), 0, NULL, 0));
	if(conn.get())
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
/*
	ibin_index_t& idx=indexes.get_index(val.key.size());

	data_t bin_key;
	points2bin(val.key,bin_key);

	data_t bin;
	val.pack(bin);

	idx.set(bin_key,bin);
*/
}

steps_t base_t::get_root_key() const
{
	return steps_t();
/*
	unsigned level=indexes.get_root_level();
	if(level==0)return steps_t();
	ibin_index_t& idx=indexes.get_index(level);

	data_t bin_key,bin_val;
	if(!idx.first(bin_key,bin_val))return steps_t();

	steps_t ret;
	bin2points(bin_key,ret);
	
	return ret;
*/
}

void base_t::width_first_search_from_bottom_to_top(sol_state_width_pr& pr)
{
/*
    for(size_t l=retreive_max_level();l>0;l--)
    {
        ibin_index_t& idx=indexes.get_index(l);
        width_first_search_from_bottom_to_top(pr,idx);
    }
*/
}

level_t::level_t(size_t _key_len,MYSQL* _conn) :
	key_len(_key_len),conn(_conn)
{
	qget.init(conn,key_len);
	create_set_statement();
}

bool level_t::get(const steps_t& key,sol_state_t& res)
{
	points2bin(key,qget.key_buf);
	int query_ret = mysql_stmt_execute(qget.st.get());
	if(query_ret!=0)
		throw std::runtime_error("level_t::get(): mysql_stmt_execute() failed");

	return false;
}

void get_query_t::init(MYSQL* conn,size_t key_len)
{
	MYSQL_STMT* stmt = mysql_stmt_init(conn);
	if(stmt == NULL)
		throw std::runtime_error("get_query_t::init(): mysql_stmt_init() failed");

	st.set(stmt);

	std::string query="select state,wins_count,fails_count,neitrals,solved_wins,solved_fails,tree_wins,tree_fails from s"
		+boost::lexical_cast<std::string>(key_len)+" where k=?";

	int query_ret = mysql_stmt_prepare(stmt, query.c_str(), query.size());
	if(query_ret!=0)
		throw std::runtime_error("get_query_t::init(): mysql_stmt_prepare() failed");

	memset(params, 0, sizeof(params));

	unsigned long key_buf_size=key_len*3;
	key_buf.resize(key_buf_size);


	MYSQL_BIND* p=&params[0];
	p->buffer_type=MYSQL_TYPE_BLOB;
	p->buffer=(char *)&key_buf.front();
	p->buffer_length=key_buf_size;
	p->is_null=0;
	p->length=&key_buf_size;

	query_ret = mysql_stmt_bind_param(stmt, params);
	if(query_ret!=0)
		throw std::runtime_error("get_query_t::init(): mysql_stmt_bind_param() failed");
}

void level_t::create_set_statement()
{
/*
INSERT INTO table (a,b,c) VALUES (1,2,3)
  ON DUPLICATE KEY UPDATE c=c+1;
  static const char* query="insert into f5(k,d) values(?,?)";
	int query_ret = mysql_stmt_prepare(stmt, query, strlen(query));
	ASSERT_EQ(query_ret,0);

	ASSERT_EQ(mysql_stmt_param_count(stmt),2);

	MYSQL_BIND    bind[2];
	memset(bind, 0, sizeof(bind));

	unsigned long key_buf_size=key_len;
	unsigned long data_buf_size=0;
	val.resize(2048);

	bind[0].buffer_type= MYSQL_TYPE_BLOB;
	bind[0].buffer= (char *)&key.front();
	bind[0].buffer_length= key_len;
	bind[0].is_null= 0;
	bind[0].length= &key_buf_size;

	bind[1].buffer_type= MYSQL_TYPE_BLOB;
	bind[1].buffer= (char *)&val.front();
	bind[1].buffer_length= val.capacity();
	bind[1].is_null= 0;
	bind[1].length= &data_buf_size;

	query_ret = mysql_stmt_bind_param(stmt, bind);
	ASSERT_EQ(query_ret,0);
*/
}


}}//namespace
