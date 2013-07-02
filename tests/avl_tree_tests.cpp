/*
#include <boost/filesystem/operations.hpp>
namespace fs=boost::filesystem;
#include "solution_tree.h"
#include "bin_index.h"

using namespace Gomoku;

void check_write()
{
	bin_index_t ind("c:\\1\\1",4);
	for(unsigned i=0;i<65536;i++)
	{
		const char* pi=(const char*)(&i);
		Gomoku::data_t key(pi,pi+4);
		ind.set(key,key);
	}
}

void check_read()
{
	bin_index_t ind("c:\\1\\1",4);
	for(unsigned i=0;i<65536;i++)
	{
		const char* pi=(const char*)(&i);
		Gomoku::data_t key(pi,pi+4);
		Gomoku::data_t val;
		if(!ind.get(key,val))
		{
			printf("check_read(): i=%d not found\n",i);
			continue;
		}

		if(key!=val)
		{
			printf("check_read(): i=%d not equal\n",i);
			continue;
		}
	}
}

void check_first_next()
{
	bin_index_t ind("c:\\1\\1",4);
	
	Gomoku::data_t key;
	Gomoku::data_t val;
	unsigned i=0;

	const char* pi=(const char*)(&i);

	if(!ind.first(key,val))
	{
		printf("check_first(): failed\n",i);
		return;
	}

	do
	{
		Gomoku::data_t pkey(pi,pi+4);
		std::reverse(pkey.begin(),pkey.begin()+2);

		if(key!=val)
		{
			printf("%d: key!=val\n",i);
			return;
		}

		if(key!=pkey)
		{
			printf("%d: key!=pkey\n",i);
			return;
		}

		++i;
	}
	while(ind.next(key,val));

	if(i!=65536)printf("%d: unexpected end\n",i);
}

void check_sol_state_pack()
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

void test()
{
	check_write();
//	check_read();
//	check_first_next();
//	check_sol_state_pack();
}
*/