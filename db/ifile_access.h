#ifndef ifile_accessH
#define ifile_accessH
#include <boost/shared_ptr.hpp>
#include <vector>
#include <string>

namespace Gomoku
{
	typedef std::vector<unsigned char> data_t;

	class ifile_access_t
	{
		ifile_access_t(const ifile_access_t&);
		void operator=(const ifile_access_t&);
	public:
		ifile_access_t(){}
		virtual ~ifile_access_t(){}
		virtual void close()=0;
		virtual size_t get_size()=0;
		virtual void load(size_t offset,data_t& res)=0;
		virtual void save(size_t offset,const data_t& res)=0;
		virtual size_t append(const data_t& res)=0;
	};

	typedef boost::shared_ptr<ifile_access_t> file_access_ptr;

	class ifile_access_provider_t
	{
		ifile_access_provider_t(const ifile_access_provider_t&);
		void operator=(const ifile_access_provider_t&);
	public:
		ifile_access_provider_t(){}
		virtual ~ifile_access_provider_t(){}

		virtual file_access_ptr create(const std::string& file_name) const=0;
	};

}//namespace

#endif
