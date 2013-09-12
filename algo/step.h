#ifndef gomoku_field_kindH
#define gomoku_field_kindH

namespace Gomoku
{
	enum Step{st_empty,st_krestik,st_nolik,st_undefined};

	inline Step other_color(Step v)
	{
		switch(v)
		{
		case st_krestik:return st_nolik;
		case st_nolik:return st_krestik;
		}
		return v;
	}

    inline Step last_color(unsigned field_size)
    {
        return (field_size%2)==0? st_nolik:st_krestik;
    }

    inline Step next_color(unsigned field_size)
    {
        return other_color(last_color(field_size));
    }
}

#endif

