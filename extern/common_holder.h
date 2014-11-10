#ifndef common_holderH
#define common_holderH

template<typename T,typename Freezer>
class common_holder_t
{
	T* val;
	Freezer* f;
	
	common_holder_t(const common_holder_t&);
	void operator=(const common_holder_t&);
public:
	common_holder_t(T* _val,Freezer* _f) : val(_val),f(_f){}
	~common_holder_t(){close();}

	inline void set(T* _val)
	{
		close();
		val=_val;
	}

	inline void close()
	{
		if(!val)
			return;
		f(val);
		val=0;
	}

	inline T* get() const{return val;}
};

#endif