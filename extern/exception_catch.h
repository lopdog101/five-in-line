#ifndef excepion_catchH
#define excepion_catchH
#include "object_progress.hpp"

#define APLMOD_COMMON_EXCEPTION_CATCH \
catch(::ObjectProgress::e_operation_canceled&)\
{\
}\
catch(...)\
{\
	std::string message,type_name;\
	bool handled=false;\
	FriendRtti::exceptions_filter::instance().message(message,type_name,handled);\
	if(!handled)message="unknown exception";\
	ObjectProgress::log_generator lg(true);\
	lg<<type_name<<" : "<<message;\
}

#define UNCATCHED_EXCEPTION_CATCH \
catch(std::exception& e)\
{\
    if(!std::uncaught_exception())\
        throw;\
	ObjectProgress::log_generator lg(true);\
	lg<<__FILE__<<": "<<__LINE__<<" std::exception: "<<e.what();\
}\
catch(...)\
{\
    if(!std::uncaught_exception())\
        throw;\
	ObjectProgress::log_generator lg(true);\
	lg<<__FILE__<<": "<<__LINE__<<" unknown exception";\
}

#endif
