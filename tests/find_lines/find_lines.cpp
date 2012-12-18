#include "find_lines.h"
#include "../../algo/wsplayer.h"
#include <boost/lexical_cast.hpp>

base_test_t* create_find_lines(){return new find_lines_t;}


void find_lines_t::process()
{
	try
	{
		check_four_empty_both_sides();
		check_four_left_zero();
	}
	CHECK_RETHROW;
}

void find_lines_t::check_four_empty_both_sides()
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

		Gomoku::points_t empty_steps(1);
		p.x=i;
		empty_steps.front()=p;

		Gomoku::wsplayer_t::treats_t res;
		Gomoku::wsplayer_t::find_treats(empty_steps,res,p.step,fl,5);

		CHECK( res.size()==1 );

		const Gomoku::wsplayer_t::treat_t& tr=res.front();

		CHECK( tr.rest_count==3 );

		for(unsigned k=0;k<5;k++)
		{
			if(k==i || k==j)continue;
			
			p.x=k;

			CHECK_CTX( std::find(tr.rest,tr.rest+tr.rest_count,p)!=tr.rest+tr.rest_count , get_ctx(fl,i,j) );
		}

        p.x=i;
		CHECK_CTX( tr.gain == p , get_ctx(fl,i,j));

		if(j==0)
		{
			CHECK_CTX( tr.cost_count==2 , get_ctx(fl,i,j) );

			p.x=0;
			CHECK_CTX( std::find(tr.cost,tr.cost+tr.cost_count,p)!=tr.cost+tr.cost_count , get_ctx(fl,i,j) );

			p.x=5;
			CHECK_CTX( std::find(tr.cost,tr.cost+tr.cost_count,p)!=tr.cost+tr.cost_count , get_ctx(fl,i,j) );
		}
		else if(j==4)
		{
			CHECK_CTX( tr.cost_count==2 , get_ctx(fl,i,j) );

			p.x=-1;
			CHECK_CTX( std::find(tr.cost,tr.cost+tr.cost_count,p)!=tr.cost+tr.cost_count , get_ctx(fl,i,j) );

			p.x=4;
			CHECK_CTX( std::find(tr.cost,tr.cost+tr.cost_count,p)!=tr.cost+tr.cost_count , get_ctx(fl,i,j) );
		}
		else
		{
			CHECK_CTX( tr.cost_count==1 , get_ctx(fl,i,j) );

            p.x=j;
			CHECK_CTX( tr.cost[0]==p , get_ctx(fl,i,j) );
		}

	}
}

void find_lines_t::check_four_left_zero()
{
	for(unsigned i=0;i<5;i++)
	for(unsigned j=0;j<5;j++)
	{
		if(j==i)continue;
		Gomoku::field_t fl;
		Gomoku::step_t p(Gomoku::st_krestik,0,0);

		p.x=-1;
		fl.add(p,Gomoku::other_step(p.step));

		for(unsigned k=0;k<5;k++)
		{
			if(k==i || k==j)continue;

			p.x=k;
			fl.add(p,p.step);
		}

		Gomoku::points_t empty_steps(1);
		p.x=i;
		empty_steps.front()=p;

		Gomoku::wsplayer_t::treats_t res;
		Gomoku::wsplayer_t::find_treats(empty_steps,res,p.step,fl,5);

		if(res.size()==2)
		{
			lg<<get_ctx(fl,i,j);
			
			for(unsigned k=0;k<res.size();k++)
			{
				Gomoku::wsplayer_t::treat_t tr=res[k];
				tr.sort();
				lg<<Gomoku::wsplayer_t::print_treat(tr);
			}
		}
		
		CHECK_CTX( res.size()==1, get_ctx(fl,i,j) );

		const Gomoku::wsplayer_t::treat_t& tr=res.front();

		CHECK_CTX( tr.rest_count==3, get_ctx(fl,i,j) );

		for(unsigned k=0;k<5;k++)
		{
			if(k==i || k==j)continue;
			
			p.x=k;

			CHECK_CTX( std::find(tr.rest,tr.rest+tr.rest_count,p)!=tr.rest+tr.rest_count , get_ctx(fl,i,j) );
		}

        p.x=i;
		CHECK_CTX( tr.gain == p , get_ctx(fl,i,j));

		if(j==0)
		{
			CHECK_CTX( tr.cost_count==2 , get_ctx(fl,i,j) );

			p.x=0;
			CHECK_CTX( std::find(tr.cost,tr.cost+tr.cost_count,p)!=tr.cost+tr.cost_count , get_ctx(fl,i,j) );

			p.x=5;
			CHECK_CTX( std::find(tr.cost,tr.cost+tr.cost_count,p)!=tr.cost+tr.cost_count , get_ctx(fl,i,j) );
		}
		else
		{
			CHECK_CTX( tr.cost_count==1 , get_ctx(fl,i,j) );

            p.x=j;
			CHECK_CTX( tr.cost[0]==p , get_ctx(fl,i,j) );
		}
	}
}

std::string find_lines_t::get_ctx(const Gomoku::field_t& fl,unsigned i,unsigned j)
{
	return "i="+boost::lexical_cast<std::string>(i)+
		 " j="+boost::lexical_cast<std::string>(j)+
		 " fl:"+Gomoku::print_field(fl.get_steps());;
}
