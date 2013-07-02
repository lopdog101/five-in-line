#include "gtest/gtest.h"
#include <boost/filesystem/operations.hpp>
namespace fs=boost::filesystem;
#include "../db/solution_tree.h"
#include "../db/bin_index.h"

namespace fs=boost::filesystem;

namespace Gomoku{

class avl_tree : public testing::Test
{
protected:
     void read();
     void write();
public:
    static const char* index_dir;
};

const char* avl_tree::index_dir="../tmp";

void avl_tree::write()
{
    if(fs::exists(index_dir))
        fs::remove_all(index_dir);

    fs::create_directories(index_dir);

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

}//namespace
