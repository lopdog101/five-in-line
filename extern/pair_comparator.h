#ifndef shared_pair_comparatoH
#define shared_pair_comparatoH
#include <utility>

namespace stdext
{
	template<class U,class V>
	struct first_less_pr
	{
		inline bool operator()(const std::pair<U,V>& a,const std::pair<U,V>& b) const{return a.first<b.first;}
		inline bool operator()(const U& a,const std::pair<U,V>& b) const{return a<b.first;}
		inline bool operator()(const std::pair<U,V>& a,const U& b) const{return a.first<b;}
	};

	template<class U,class V>
	struct first_equal_pr
	{
		inline bool operator()(const std::pair<U,V>& a,const std::pair<U,V>& b) const{return a.first==b.first;}
		inline bool operator()(const U& a,const std::pair<U,V>& b) const{return a==b.first;}
		inline bool operator()(const std::pair<U,V>& a,const U& b) const{return a.first==b;}
	};
}

#endif
