#include <boost/lexical_cast.hpp>
#include <stdexcept>
#include "regular_file_access.h"

namespace Gomoku
{

regular_file_t::regular_file_t(const std::string& _file_name) : file_name(_file_name)
{
	fd=0;
}

void regular_file_t::open()
{
	if(fd)return;
	fd=fopen(file_name.c_str(),"r+b");
	if(!fd)fd=fopen(file_name.c_str(),"w+b");
	if(!fd)throw std::runtime_error("could not open file: "+file_name);
}

void regular_file_t::close()
{
	if(!fd)return;
	fclose(fd);
	fd=0;
}

size_t regular_file_t::get_size()
{
	open();

	if(fseek(fd,0,SEEK_END)!=0)
		throw std::runtime_error("get_size(): seek error at end: "+file_name);

	return ftell(fd);
}

void regular_file_t::load(size_t offset,data_t& res)
{
	if(res.empty())return;

	open();

	if(fseek(fd,(int)offset,SEEK_SET)!=0)
		throw std::runtime_error("load(): seek error at "+boost::lexical_cast<std::string>(offset)+
			": "+file_name);

	if(fread(&res.front(),1,res.size(),fd)!=res.size())
		throw std::runtime_error(
			"load(): read error at "+boost::lexical_cast<std::string>(offset)+
			" size="+boost::lexical_cast<std::string>(res.size())+
			": "+file_name);
}

void regular_file_t::save(size_t offset,const data_t& res)
{
	if(res.empty())return;

	open();

	if(fseek(fd,(int)offset,SEEK_SET)!=0)
		throw std::runtime_error("save(): seek error at "+boost::lexical_cast<std::string>(offset)+
		": "+file_name);

	if(fwrite(&res.front(),1,res.size(),fd)!=res.size())
		throw std::runtime_error(
			"save(): write error at "+boost::lexical_cast<std::string>(offset)+
			" size="+boost::lexical_cast<std::string>(res.size())+
			": "+file_name);
}

size_t regular_file_t::append(const data_t& res)
{
	if(res.empty())return 0;

	open();

	if(fseek(fd,0,SEEK_END)!=0)
		throw std::runtime_error("append(): seek error at end: "+file_name);

	size_t ret=ftell(fd);

	if(fwrite(&res.front(),1,res.size(),fd)!=res.size())
		throw std::runtime_error(
			"append_data(): write error size="+boost::lexical_cast<std::string>(res.size())+
			": "+file_name);

	return ret;
}

}//namespace
