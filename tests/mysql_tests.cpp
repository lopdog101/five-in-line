#include <fstream>
#include "gtest/gtest.h"
#include <boost/filesystem/operations.hpp>
namespace fs=boost::filesystem;
#include "../db/solution_tree.h"
#include "../db/bin_index.h"
#include "../algo/wsplayer.h"
#include "../algo/wsplayer_node.h"
#include <boost/algorithm/string.hpp>
#include <my_global.h>
#include <mysql.h>

namespace fs=boost::filesystem;

namespace Gomoku{

class mysql : public testing::Test
{
protected:
    void recreate_dirs();
    void read();
    void write();
	virtual void SetUp();
	virtual void TearDown();
public:
    static const char* index_dir;
    static const char* src_dir;
};

const char* mysql::index_dir="../tmp";
const char* mysql::src_dir="../src_data";

void mysql::SetUp()
{
}

void mysql::TearDown()
{
}

TEST_F(mysql, real_data_cycle)
{
	static const unsigned perf_mod=200000;

	ObjectProgress::log_generator lg(true);
	ObjectProgress::perfomance perf;

	static const size_t key_len=39;

	std::string file_name=std::string(src_dir)+"/13";
	regular_file_t fi(file_name+".idx");
	regular_file_t fd(file_name+".data");

	data_t idx_content(static_cast<size_t>(fi.get_size()));
	fi.load(0,idx_content);

	data_t data_content(static_cast<size_t>(fd.get_size()));
	fd.load(0,data_content);

	data_t key(key_len);
	data_t val,val_cp;
	steps_t steps;

	size_t mi=idx_content.size()/key_len;

	MYSQL *con = mysql_init(NULL);
	ASSERT_TRUE(con != NULL);
	
	MYSQL *ret = mysql_real_connect(con, "localhost", NULL, NULL, "test", 0, NULL, 0);
	ASSERT_TRUE(ret != NULL);

	MYSQL_STMT* stmt = mysql_stmt_init(con);
	ASSERT_TRUE(stmt != NULL);

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

	for(size_t i=0;i<mi;i++)
	{
		std::copy(idx_content.begin()+i*key_len,idx_content.begin()+(i+1)*key_len,key.begin());
		bin2points(key,steps);
		sort_steps(steps);
		points2bin(steps,key);

		std::copy(key.begin(),key.end(),idx_content.begin()+i*key_len);
	}

	{
		perf.reset();
		size_t data_pos=0;

		for(size_t i=0;i<mi;i++)
		{
			std::copy(idx_content.begin()+i*key_len,idx_content.begin()+(i+1)*key_len,key.begin());

			val.resize(*reinterpret_cast<const size_t*>(&data_content[data_pos] ));
			data_pos+=sizeof(size_t);
			std::copy(data_content.begin()+data_pos,data_content.begin()+data_pos+val.size(),val.begin());
			data_pos+=val.size();

			data_buf_size = val.size();

			query_ret = mysql_stmt_execute(stmt);
			ASSERT_EQ(query_ret,0);

			if((i!=0)&&(i%perf_mod)==0)
			{
				ObjectProgress::perfomance::val_t v=perf.delay();
				lg<<"write: i="<<i<<" perf="<<(v/1000.0)<<"ms rate="<<(1.0*perf_mod/v*1000000.0)<<"rps";
				perf.reset();
			}
		}
	}

	mysql_close(con);
}

}//namespace
