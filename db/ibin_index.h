#ifndef ibin_indexH
#define ibin_indexH
#include <vector>
#include <string>
#include <boost/shared_ptr.hpp>
#include "bin_index_utils.h"

namespace Gomoku
{

class ibin_index_t
{
public:
	virtual ~ibin_index_t(){}
	virtual bool get(const data_t& key,data_t& val) const=0;
	virtual bool set(const data_t& key,const data_t& val)=0;
	virtual bool first(data_t& key,data_t& val) const=0;
	virtual bool next(data_t& key,data_t& val) const=0;
};

class ibin_indexes_t
{
public:
	virtual ~ibin_indexes_t(){}
	virtual unsigned get_root_level() const=0;
	virtual ibin_index_t& get_index(unsigned steps_count)=0;
	virtual ibin_index_t* find_index(unsigned steps_count)=0;
};

}//namespace

#endif
