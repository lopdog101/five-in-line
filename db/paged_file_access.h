#ifndef paged_file_accessH
#define paged_file_accessH
#include "regular_file_access.h"
#include <list>
#include <stdio.h>
#include <unordered_map>
#include <boost/weak_ptr.hpp>

#ifdef _WIN32
namespace std
{
	using namespace std::tr1;
}
#endif

namespace Gomoku
{

	class paged_file_t : public regular_file_t
	{
	public:
		struct page_t;

		typedef boost::shared_ptr<page_t> page_ptr;
		typedef boost::weak_ptr<page_t> page_wptr;
		typedef std::unordered_map<file_offset_t,page_ptr> pages_t;

		struct page_t
		{
			const file_offset_t index;
			bool dirty;
			data_t data;
			page_wptr left;
			page_wptr right;
			page_wptr self;

			page_t(file_offset_t _index,size_t max_size) : index(_index)
			{
				dirty=false;
				data.reserve(max_size);
			}
		private:
			page_t(const page_t&);
			void operator=(const page_t&);
		};

	protected:
		const size_t page_size;
		const unsigned max_pages;
		pages_t pages;
		file_offset_t file_size;
		page_wptr first_page;
		page_wptr last_page;

		virtual void open();

		void flush();
		void flush_page(page_t& p);
		void remove_oldest();
		page_ptr get_page(file_offset_t idx);
		void add_page(const page_ptr& p);
		void bring_to_front(page_ptr& p);

		inline file_offset_t from_index(file_offset_t offset) const{return offset/page_size;}
		inline file_offset_t to_index(file_offset_t offset,file_offset_t size) const{return (offset+size+page_size-1)/page_size;}
	public:
		paged_file_t(const std::string& file_name,size_t page_size=512,unsigned max_pages=512);
		~paged_file_t(){close();}
		void close();
		file_offset_t get_size();
		virtual void load(file_offset_t offset,data_t& res);
		virtual void save(file_offset_t offset,const data_t& res);
		virtual file_offset_t append(const data_t& res);
	};

	class paged_file_provider_t : public regular_file_provider_t
	{
	public:
		virtual ~paged_file_provider_t(){}

		virtual file_access_ptr create(const std::string& file_name) const{return file_access_ptr(new paged_file_t(file_name));}
	};

}//namespace

#endif
