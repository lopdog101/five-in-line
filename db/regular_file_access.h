#ifndef regular_file_accessH
#define regular_file_accessH
#include "ifile_access.h"
#include <stdio.h>

namespace Gomoku
{

	class regular_file_t : public ifile_access_t
	{
	protected:
		std::string file_name;
		FILE* fd;

		virtual void open();
	public:
		regular_file_t(const std::string& file_name);
		~regular_file_t(){close();}
		virtual void close();
		virtual size_t get_size();
		virtual void load(size_t offset,data_t& res);
		virtual void save(size_t offset,const data_t& res);
		virtual size_t append(const data_t& res);
	};

	class regular_file_provider_t : public ifile_access_provider_t
	{
	public:
		virtual ~regular_file_provider_t(){}

		virtual file_access_ptr create(const std::string& file_name) const{return file_access_ptr(new regular_file_t(file_name));}
	};

}//namespace

#endif
