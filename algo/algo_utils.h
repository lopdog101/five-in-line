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

    template<class T>
	struct restore_t
	{
		T& val;
		T copied; 
        bool active;

		restore_t(T & _val):val(_val),copied(_val) {active=true;}
		~restore_t(){reset();}

        inline void reset()
        {
            if(!active)return;
            val = copied;
            active=false;
        }
	};

	typedef restore_t<unsigned> unsigned_restore_t;

}

#endif
