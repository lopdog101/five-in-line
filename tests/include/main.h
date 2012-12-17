#ifndef test_mainH
#define test_mainH
#include <stdexcept>
#include <vector>
#ifdef WITHOUT_EXTERNAL_LIBS
#  include "../../extern/object_progress.hpp"
#else
#  include <object_progress/object_progress.hpp>
#  include <object_progress/perfomance.hpp>
#endif

class base_test_t
{
	base_test_t(const base_test_t&);
	void operator=(const base_test_t&);
protected:
	ObjectProgress::log_generator lg;

	virtual void process()=0;
public:
	base_test_t();

	bool pass();
};

class e_check_failed : public std::exception
{
public:
	struct item_t
	{
		std::string file;
		int line;
		std::string func;

		item_t(){line=0;}
		item_t(const std::string _file,int _line,const std::string _func): file(_file),func(_func) {line=_line;}
	};

	typedef std::vector<item_t> items_t;

private:
	std::string msg;
	items_t items;
public:
	std::string ctx;

	e_check_failed(const std::string& _msg) : msg(_msg){}
	const char* what() const{return msg.c_str();}
	void add_stack(const std::string& file,int line,const std::string& func){items.push_back(item_t(file,line,func));}
	const items_t& get_stack() const{return items;}
};

#define CHECK( EXPRESSION )\
if(!(EXPRESSION))\
{\
	e_check_failed e(#EXPRESSION);\
	e.add_stack(__FILE__,__LINE__,__FUNCTION__);\
	throw e;\
}

#define CHECK_CTX( EXPRESSION, CONTEXT )\
if(!(EXPRESSION))\
{\
	e_check_failed e(#EXPRESSION);\
	e.ctx = CONTEXT;\
	e.add_stack(__FILE__,__LINE__,__FUNCTION__);\
	throw e;\
}

#define CHECK_RETHROW \
catch(e_check_failed& e)\
{\
	e.add_stack(__FILE__,__LINE__,__FUNCTION__);\
	throw e;\
}\
catch(std::exception& se)\
{\
	e_check_failed e(se.what());\
	e.add_stack(__FILE__,__LINE__,__FUNCTION__);\
	throw e;\
}\
catch(...)\
{\
	e_check_failed e("unknown exception");\
	e.add_stack(__FILE__,__LINE__,__FUNCTION__);\
	throw e;\
}


#endif
