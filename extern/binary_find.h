#ifndef binary_findH
#define binary_findH
#include <algorithm>

template<class _FwdIt,
	class _Ty> inline
	_FwdIt binary_find(_FwdIt _First, _FwdIt _Last, const _Ty& _Val)
	{	// test if _Val equivalent to some element, using operator<
	_First = std::lower_bound(_First, _Last, _Val);
	if(_First != _Last && !(_Val < *_First)) return _First;
	return _Last;
	}

		// TEMPLATE FUNCTION binary_search WITH PRED
template<class _FwdIt,
	class _Ty,
	class _Pr> inline
	_FwdIt binary_find(_FwdIt _First, _FwdIt _Last,
		const _Ty& _Val, _Pr _Pred)
	{	// test if _Val equivalent to some element, using _Pred
	_First = std::lower_bound(_First, _Last, _Val, _Pred);
	if(_First != _Last && !_Pred(_Val, *_First)) return _First;
	return _Last;
	}

#endif
