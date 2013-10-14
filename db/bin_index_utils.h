#ifndef bin_index_utilsH
#define bin_index_utilsH
#include <stdio.h>
#include <vector>
#include <string>
#include <boost/shared_ptr.hpp>
#include <boost/filesystem/path.hpp>


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

template<typename T>
inline void pack_raw(const T& val,data_t& bin)
{
    const unsigned char* p=reinterpret_cast<const unsigned char*>(&val);
	bin.insert(bin.end(),p,p+sizeof(T));
}

template<typename T>
inline void unpack_raw(const data_t& bin,T& val,size_t& from,const char* throw_message)
{
	if(from+sizeof(T)>=bin.size())
		throw std::runtime_error(throw_message);
	
    val=*(const T*)(&bin[from]);
	from+=sizeof(T);
}

}//namespace

#endif
