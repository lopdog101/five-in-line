#ifndef bin_index_utilsH
#define bin_index_utilsH
#include <stdio.h>
#include <vector>
#include <string>
#include <boost/shared_ptr.hpp>

namespace boost{namespace filesystem{
	class path;
}}//namespace

namespace Gomoku
{
typedef std::vector<unsigned char> data_t;
typedef std::vector<data_t> datas_t;

struct file_hldr
{
	FILE* f;
	file_hldr(FILE* _f) {f=_f;}
	~file_hldr(){if(f)fclose(f);}
};

void load_file(const std::string& file_name,data_t& res);
void save_file(const std::string& file_name,const data_t& res);

void load_file(const boost::filesystem::path& file_name,data_t& res);
void save_file(const boost::filesystem::path& file_name,const data_t& res);

void hex2bin(const std::string& str,data_t& bin);
void bin2hex(const data_t& bin,std::string& str);
}//namespace

#endif
