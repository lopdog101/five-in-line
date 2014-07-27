#include "bin_index_utils.h"
#include <stdexcept>
#include <boost/lexical_cast.hpp>

#  include <boost/filesystem/operations.hpp>
namespace fs=boost::filesystem;


namespace Gomoku
{
	void hex2bin(const std::string& str,data_t& bin)
	{
		bin.resize(str.size()/2);
		for(size_t i=0;i<bin.size();i++)
		{
			int val;
			if(sscanf(str.c_str()+i*2,"%2x",&val)!=1)
				throw std::runtime_error("hex2bin(): format error: "+str);
			bin[i]=(unsigned char)val;
		}
	}

	void bin2hex(const data_t& bin,std::string& str)
	{
#ifdef _WIN32
		str.resize(bin.size()*2);
		for(size_t i=0;i<bin.size();i++)
			_snprintf(&str[0]+i*2,2,"%02x",bin[i]);
#else
		str.clear();
		for(size_t i=0;i<bin.size();i++)
		{
			char tmp[256];
            snprintf(tmp,sizeof(tmp),"%02x",(int)bin[i]);
			str+=tmp;
		}
#endif
	}

	void load_file(const std::string& file_name,data_t& res)
	{
		FILE* f=fopen(file_name.c_str(),"rb");
		if(!f)throw std::runtime_error("Could not open for read "+file_name);
		file_hldr hld(f);

		if(fseek(f,0,SEEK_END)!=0)
			throw std::runtime_error(file_name+": seek at end error");

		res.resize(ftell(f));

		if(res.empty())return;

		if(fseek(f,0,SEEK_SET)!=0)
			throw std::runtime_error(file_name+": seek at begin error");

		if(fread(&res.front(),1,res.size(),f)!=res.size())
			throw std::runtime_error(file_name+": read error");
	}

	void save_file(const std::string& file_name,const data_t& res)
	{
		FILE* f=fopen(file_name.c_str(),"wb");
		if(!f)throw std::runtime_error("Could not open for write "+file_name);
		file_hldr hld(f);

		if(fwrite(&res.front(),1,res.size(),f)!=res.size())
			throw std::runtime_error(file_name+": write error");
	}

	void load_file(const boost::filesystem::path& file_name,data_t& res)
	{
		load_file(file_name.string(),res);
	}

	void save_file(const boost::filesystem::path& file_name,const data_t& res)
	{
		save_file(file_name.string(),res);
	}
}//namespace
