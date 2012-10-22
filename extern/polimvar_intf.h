#ifndef polimvar_intfH
#define polimvar_intfH

#define POLIMVAR_DECLARE_CLONE( _Type )\
	virtual _Type* clone() const=0;\
	virtual _Type* clone(void* raw) const=0;\
	virtual size_t own_size() const=0;

#define POLIMVAR_IMPLEMENT_CLONE( _Type )\
	_Type* clone() const{return new _Type(*this);}\
	_Type* clone(void* raw) const{return new(raw) _Type(*this);}\
	size_t own_size() const{return sizeof(_Type);}

#endif

