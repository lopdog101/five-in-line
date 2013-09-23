#ifndef gomoku_algo_utilsH
#define gomoku_algo_utilsH

namespace Gomoku
{
    template<class T>
	struct incer_t
	{
		T & val;
        bool active;

		incer_t(T & _val):val(_val) {++val; active=true;}
		~incer_t(){reset();}

        inline void reset()
        {
            if(!active)return;
            --val;
            active=false;
        }
	};

}

#endif
