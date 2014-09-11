#include <fstream>
#include "gtest/gtest.h"
#include <boost/filesystem/operations.hpp>
namespace fs=boost::filesystem;
#include "../db/solution_tree.h"
#include "../db/bin_index.h"

namespace fs=boost::filesystem;

namespace Gomoku{

class avl_tree : public testing::Test
{
	file_offset_t file_max_size;
	size_t page_max_size;
protected:
    void recreate_dirs();
    void read();
    void write();
	virtual void SetUp();
	virtual void TearDown();
public:
    static const char* index_dir;
};

const char* avl_tree::index_dir="../tmp";

void avl_tree::SetUp()
{
	file_max_size=bin_index_t::file_max_size;
	page_max_size=bin_index_t::page_max_size;
}

void avl_tree::TearDown()
{
	bin_index_t::file_max_size=file_max_size;
	bin_index_t::page_max_size=page_max_size;
}

void avl_tree::recreate_dirs()
{
    if(fs::exists(index_dir))
        fs::remove_all(index_dir);

    fs::create_directories(index_dir);
}

void avl_tree::write()
{
    recreate_dirs();

	bin_index_t ind(index_dir,4);
	for(unsigned i=0;i<65536;i++)
	{
		const char* pi=(const char*)(&i);
		data_t key(pi,pi+4);
		ind.set(key,key);
	}
}

void avl_tree::read()
{
	bin_index_t ind(index_dir,4);
	for(unsigned i=0;i<65536;i++)
	{
        SCOPED_TRACE(i);

		const char* pi=(const char*)(&i);
		data_t key(pi,pi+4);
		data_t val;
		ASSERT_TRUE(ind.get(key,val));

		ASSERT_EQ(key,val);
	}
}

TEST_F(avl_tree, DISABLED_check_write)
{
    write();
}

TEST_F(avl_tree, DISABLED_check_read_write)
{
    write();
    read();
}

TEST_F(avl_tree, DISABLED_check_first_next)
{
    write();

    bin_index_t ind(index_dir,4);
	
	data_t key;
	data_t val;
	unsigned i=0;

	const char* pi=(const char*)(&i);

	ASSERT_TRUE(ind.first(key,val));

	do
	{
        SCOPED_TRACE(i);

        data_t pkey(pi,pi+4);
		std::reverse(pkey.begin(),pkey.begin()+2);

		ASSERT_EQ(key,val);
		ASSERT_EQ(key,pkey);

		++i;
	}
	while(ind.next(key,val));

	ASSERT_EQ(i,65536);
}

TEST_F(avl_tree, check_sol_state_pack)
{
	data_t bin;
	sol_state_t st;
	st.state=ss_solving;
	st.key.push_back(step_t(st_nolik,1,2));
	st.solved_wins.push_back(point(-1,-2));
	st.solved_fails.push_back(point(-2,-3));
	st.tree_wins.push_back(point(2,3));
	st.tree_fails.push_back(point(4,5));
	st.pack(bin);

	sol_state_t nst;
	nst.unpack(bin);
	bin=bin;
}

TEST_F(avl_tree,split)
{
    char init_chain[]={0,0,0,-1,0,2,0,3,1,-1,2,-2,3,-4,3,-2,1,-2,3,-1,2,3,-2,2,3,-1,1,-2,1,-1,2,-3,4,5,3,-2,1,2,-2,4,1,-1,2,-3};
    static const size_t chain_length=sizeof(init_chain)/sizeof(char);

    static const unsigned num_iterations=3000;
	bin_index_t::file_max_size=1024;

    recreate_dirs();

    {
	    bin_index_t ind(index_dir,chain_length);
        
        data_t key(init_chain,init_chain+chain_length);

        for(unsigned i=0;i<num_iterations;i++,std::next_permutation(key.begin(),key.end()))
	    {
            SCOPED_TRACE(i);

            ind.set(key,key);
	    }
    }

    {
	    bin_index_t ind(index_dir,chain_length);
        
        data_t key(init_chain,init_chain+chain_length);

        for(unsigned i=0;i<num_iterations;i++,std::next_permutation(key.begin(),key.end()))
	    {
            SCOPED_TRACE(i);

            data_t val;

            ASSERT_TRUE(ind.get(key,val));

            ASSERT_EQ(key,val);
	    }
    }
}

void fill_key(unsigned char* _pi,unsigned long long v,int kp)
{
	unsigned long long* pi=reinterpret_cast<unsigned long long*>(_pi);
	pi[kp-1]=v;
}

TEST_F(avl_tree,DISABLED_big_split)
{
	static const unsigned num_iterations=1000000000;
	static const unsigned char key_mod=5;
	static const unsigned perf_mod=200000;
	static const unsigned kp=1;

	bin_index_t::file_max_size=9223372036854775807ll;

	recreate_dirs();
	ObjectProgress::log_generator lg(true);
	ObjectProgress::perfomance perf;

	data_t key(kp*sizeof(unsigned long long));

	{
		bin_index_t ind(index_dir,kp*sizeof(unsigned long long));

		for(unsigned long long i=0;i<num_iterations;i++)
		{
			fill_key(&key.front(),i,kp);

			ind.set(key,key);

			if((i!=0)&&(i%perf_mod)==0)
			{
				ObjectProgress::perfomance::val_t v=perf.delay();
				lg<<"write: i="<<i<<" perf="<<(v/1000.0)<<"ms rate="<<(1.0*perf_mod/v*1000000.0)<<"rps";
				perf.reset();
			}
		}
	}

	{
		bin_index_t ind(index_dir,kp*sizeof(unsigned long long));
		perf.reset();

		for(unsigned long long i=0;i<num_iterations;i++)
		{
			fill_key(&key.front(),i,kp);

			data_t val;

			ASSERT_TRUE(ind.get(key,val));
			ASSERT_EQ(key,val);

			if((i!=0)&&(i%perf_mod)==0)
			{
				ObjectProgress::perfomance::val_t v=perf.delay();
				lg<<"read: i="<<i<<" perf="<<(v/1000.0)<<"ms rate="<<(1.0*perf_mod/v*1000000.0)<<"rps";
				perf.reset();
			}
		}
	}
}

TEST_F(avl_tree, DISABLED_show)
{
    steps_t not_found=scan_steps("(-1,1:X);(0,0:X);(1,-1:X);(1,0:O);(1,1:O);(-2,2:O);(2,-2:X);(3,-3:O);(-2,0:X);(-2,3:O);(-2,1:X);(-2,4:O)");
    sort_steps(not_found);

    steps_t writed_steps=scan_steps("(-3,-2:O);(-2,0:X);(-2,2:O);(-2,4:O);(-1,1:X);(0,0:X);(0,1:X);(1,-1:X);(1,0:O);(1,1:O);(2,-2:X);(3,-3:O)");
    sort_steps(writed_steps);

    std::cout<<print_steps(not_found)<<std::endl<<print_steps(writed_steps)<<std::endl;

    data_t not_found_bin;
    points2bin(not_found,not_found_bin);

    std::string not_found_str;
    bin2hex(not_found_bin,not_found_str);

    data_t cut_bin(not_found_bin.begin()+2,not_found_bin.end());
    std::string cut_str;
    bin2hex(cut_bin,cut_str);

    std::cout<<"not_found: "<<not_found_str<<std::endl<<"cut: "<<cut_str<<std::endl;

    
/*
    bin_index_t ind("D:\\1\\w",34);
    	
	data_t key;
	data_t val;
    std::string key_str;
    std::string val_str;

    std::ofstream fs("D:\\1\\w\\key_content.txt");

    for(bool r=ind.first(key,val);r;r=ind.next(key,val))
    {
        bin2hex(key,key_str);
        bin2hex(val,val_str);

//        std::cout<<"key="<<key_str<<" val="<<val_str<<std::endl;
        fs<<key_str<<std::endl;
    }
*/
}



}//namespace
