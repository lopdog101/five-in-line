#include <fstream>
#include "gtest/gtest.h"
#include <boost/filesystem/operations.hpp>
namespace fs=boost::filesystem;
#include "../db/solution_tree.h"
#include "../db/bin_index.h"
#include "../db/mysql_solution_base.h"
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

void mysql::recreate_dirs()
{
    if(fs::exists(index_dir))
        fs::remove_all(index_dir);

    fs::create_directories(index_dir);
}

TEST_F(mysql, DISABLED_real_data_cycle)
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

TEST_F(mysql, DISABLED_generate_index_data)
{
	recreate_dirs();

	Mysql::base_t mysql_db("f5");
	solution_tree_t tr(mysql_db);
	tr.init(index_dir);

	std::string str("(-1,0:O);(-1,1:X);(-1,2:X);(0,0:X);(0,1:X);(0,2:O);(1,1:O);(1,2:X);(2,2:O)");
    steps_t steps;
    hex_or_str2points(str,steps);
	tr.create_init_tree(steps);

	WsPlayer::stored_deep=1;
	WsPlayer::def_lookup_deep=0;
	WsPlayer::treat_deep=0;
	WsPlayer::ant_count=0;

	size_t key_size=0;

	std::string file_name;
	file_access_ptr fi;
	file_access_ptr fd;

	ObjectProgress::log_generator lg(true);
	ObjectProgress::perfomance perf;

	for(int i=0;i<1000000;i++)
	{
		steps_t key;
        if(!tr.get_job(key))
		{
			printf("no job anymore\n");
			break;
		}

		reorder_state_to_game_order(key);

		game_t gm;
		gm.field().set_steps(key);

		WsPlayer::wsplayer_t pl;

		pl.init(gm,other_color(key.back().step));
		pl.solve();

		const WsPlayer::wide_item_t& r=static_cast<const WsPlayer::wide_item_t&>(*pl.root);

		points_t neitrals;
		items2points(r.get_neitrals(),neitrals);

		npoints_t wins;
		items2depth_npoints(r.get_wins().get_vals(),wins);

		npoints_t fails;
		items2depth_npoints(r.get_fails().get_vals(),fails);

		tr.save_job(key,neitrals,wins,fails);


		if(key_size!=key.size())
		{
			file_name=std::string(index_dir)+"/"+boost::lexical_cast<std::string>(key.size());
			fi=file_access_ptr(new paged_file_t(file_name+".idx") );
			fd=file_access_ptr(new paged_file_t(file_name+".data") );
			key_size=key.size();
			lg<<i<<": key_size="<<key.size();
		}

		sol_state_t st;
		st.key=key;
		st.neitrals=neitrals;
		st.solved_wins=wins;
		st.solved_fails=fails;
		
		steps_t sorted_key=key;
		sort_steps(sorted_key);
		data_t bin_key;
		Gomoku::points2bin(sorted_key,bin_key);
		
		data_t st_bin;
		st.pack(st_bin);

		fi->append(bin_key);

		data_t st_bin_size(sizeof(size_t));
		*reinterpret_cast<size_t*>(&st_bin_size.front())=st_bin.size();
		fd->append(st_bin_size);
		fd->append(st_bin);

		static const int perf_mod=10000;
		if((i!=0)&&(i%perf_mod)==0)
		{
			ObjectProgress::perfomance::val_t v=perf.delay();
			lg<<i<<": perf="<<(v/1000.0)<<"ms   rate="<<(1.0*perf_mod/v*1000000.0)<<" kps";
			perf.reset();
		}
	}

}


}//namespace
