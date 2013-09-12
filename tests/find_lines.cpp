#include "gtest/gtest.h"
#include "../algo/wsplayer_treat.h"
#include <boost/lexical_cast.hpp>

class find_lines : public testing::Test
{
 protected:
    std::string get_ctx(const Gomoku::field_t& fl,unsigned i,unsigned j)
    {
	    return "i="+boost::lexical_cast<std::string>(i)+
		     " j="+boost::lexical_cast<std::string>(j)+
		     " fl:"+Gomoku::print_field(fl.get_steps());;
    }
};

TEST_F(find_lines, check_four_empty_both_sides)
{
	for(unsigned i=0;i<5;i++)
	for(unsigned j=0;j<5;j++)
	{
		if(j==i)continue;
        
        Gomoku::field_t fl;
		Gomoku::step_t p(Gomoku::st_krestik,0,0);
		for(unsigned k=0;k<5;k++)
		{
			if(k==i || k==j)continue;

			p.x=k;
			fl.add(p,p.step);
		}

        SCOPED_TRACE(get_ctx(fl,i,j));

        Gomoku::points_t empty_steps(1);
		p.x=i;
		empty_steps.front()=p;

		Gomoku::WsPlayer::treats_t res;
		Gomoku::WsPlayer::find_treats(empty_steps,res,p.step,fl,5);

		EXPECT_EQ( res.size(),1 );

		const Gomoku::WsPlayer::treat_t& tr=res.front();

		EXPECT_EQ( tr.rest_count,3 );

		for(unsigned k=0;k<5;k++)
		{
			if(k==i || k==j)continue;
			
			p.x=k;

			EXPECT_NE( std::find(tr.rest,tr.rest+tr.rest_count,p),tr.rest+tr.rest_count );
		}

        p.x=i;
		EXPECT_EQ( tr.gain , p);

		if(j==0)
		{
			EXPECT_EQ(tr.cost_count,2 );

			p.x=0;
			EXPECT_NE( std::find(tr.cost,tr.cost+tr.cost_count,p),tr.cost+tr.cost_count );

			p.x=5;
			EXPECT_NE( std::find(tr.cost,tr.cost+tr.cost_count,p),tr.cost+tr.cost_count );
		}
		else if(j==4)
		{
			EXPECT_EQ( tr.cost_count,2);

			p.x=-1;
			EXPECT_NE( std::find(tr.cost,tr.cost+tr.cost_count,p),tr.cost+tr.cost_count);

			p.x=4;
			EXPECT_NE( std::find(tr.cost,tr.cost+tr.cost_count,p),tr.cost+tr.cost_count);
		}
		else
		{
			EXPECT_EQ( tr.cost_count,1);

            p.x=j;
			EXPECT_EQ( tr.cost[0],p);
		}

	}
}

TEST_F(find_lines, check_four_left_zero)
{
	for(unsigned i=0;i<5;i++)
	for(unsigned j=0;j<5;j++)
	{
		if(j==i)continue;
		Gomoku::field_t fl;
		Gomoku::step_t p(Gomoku::st_krestik,0,0);

		p.x=-1;
		fl.add(p,Gomoku::other_color(p.step));

		for(unsigned k=0;k<5;k++)
		{
			if(k==i || k==j)continue;

			p.x=k;
			fl.add(p,p.step);
		}

        SCOPED_TRACE(get_ctx(fl,i,j));

        Gomoku::points_t empty_steps(1);
		p.x=i;
		empty_steps.front()=p;

		Gomoku::WsPlayer::treats_t res;
		Gomoku::WsPlayer::find_treats(empty_steps,res,p.step,fl,5);

		EXPECT_EQ( res.size(),1);

		const Gomoku::WsPlayer::treat_t& tr=res.front();

		EXPECT_EQ( tr.rest_count,3);

		for(unsigned k=0;k<5;k++)
		{
			if(k==i || k==j)continue;
			
			p.x=k;

			EXPECT_NE( std::find(tr.rest,tr.rest+tr.rest_count,p),tr.rest+tr.rest_count );
		}

        p.x=i;
		EXPECT_EQ( tr.gain , p);

		if(j==0)
		{
			EXPECT_EQ( tr.cost_count,2);

			p.x=0;
			EXPECT_NE( std::find(tr.cost,tr.cost+tr.cost_count,p),tr.cost+tr.cost_count );

			p.x=5;
			EXPECT_NE( std::find(tr.cost,tr.cost+tr.cost_count,p),tr.cost+tr.cost_count );
		}
		else
		{
			EXPECT_EQ( tr.cost_count,1);

            p.x=j;
			EXPECT_EQ( tr.cost[0],p);
		}
	}
}
