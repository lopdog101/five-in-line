#include "find_lines.h"
#include "../../algo/wsplayer.h"

base_test_t* create_find_lines(){return new find_lines_t;}


void find_lines_t::process()
{
	try
	{
		check_four();
	}
	CHECK_RETHROW;
}

void find_lines_t::check_four()
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

		if(i==0&&j==4 || i==1&&j==0 || i==3&&j==4 && i==4&&j==0)
		{
		}
		else
		{
			CHECK_CTX( res. , Gomoku::print_field(fl.get_steps()) );
		}


//		lg<<<<" treats.size()=="<<res.size();
	}
}
