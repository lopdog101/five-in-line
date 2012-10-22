#ifndef gomoku_field_kindH
#define gomoku_field_kindH

namespace Gomoku
{
	enum Step{st_empty,st_krestik,st_nolik,st_undefined};

	inline Step other_step(Step v)
	{
		switch(v)
		{
		case st_krestik:return st_nolik;
		case st_nolik:return st_krestik;
		}
		return v;
	}
}

#endif

