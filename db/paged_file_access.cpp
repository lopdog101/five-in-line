#include <boost/lexical_cast.hpp>
#include <stdexcept>
#include "paged_file_access.h"

namespace Gomoku
{

paged_file_t::paged_file_t(const std::string& _file_name,size_t _page_size,unsigned _max_pages)
	: regular_file_t(_file_name),
	page_size(_page_size),
	max_pages(_max_pages)
{
	file_size=0;
}

void paged_file_t::open()
{
	regular_file_t::open();
	file_size=regular_file_t::get_size();
}

void paged_file_t::close()
{
	flush();
	regular_file_t::close();
}

size_t paged_file_t::get_size()
{
	return file_size;
}

void paged_file_t::flush()
{
	pages_t::iterator from=pages.begin();
	pages_t::iterator to=pages.end();
	for(;from!=to;++from)
		flush_page(**from);
}

void paged_file_t::flush_page(page_t& p)
{
	if(!p.dirty)
		return;

	regular_file_t::save(p.index*page_size,p.data);
	p.dirty=false;
}

void paged_file_t::load(size_t offset,data_t& res)
{
	size_t from=offset/page_size;
	size_t to=(offset+res.size()+page_size-1)/page_size;
	for(size_t i=from;i<to;i++)
	{
		page_ptr pp=get_page(i);
		page_t& p=*pp;
	}
}

void paged_file_t::save(size_t offset,const data_t& res)
{
	return regular_file_t::save(offset,res);
}

size_t paged_file_t::append(const data_t& res)
{
	return regular_file_t::append(res);
}

paged_file_t::page_ptr paged_file_t::get_page(size_t idx)
{
	pages_t::iterator it=pages.begin();

	for(;it!=pages.end();++it)
	{
		page_t& p=**it;
		if(p.index==idx)
			break;
	}

	if(it!=pages.end())
	{
		if(it==pages.begin())
			return *it;

		page_ptr p=*it;
		pages.erase(it);
		pages.insert(pages.begin(),p);

		return p;
	}

	remove_oldest();

	page_ptr p(new page_t(idx,page_size));
	
	size_t offset=idx*page_size;
	size_t sz=page_size;
	if(offset+sz>file_size)sz=file_size-offset;
	p->data.resize(sz);
	regular_file_t::load(offset,p->data);

	pages.insert(pages.begin(),p);

	return p;
}

void paged_file_t::remove_oldest()
{
	while(pages.size()>=max_pages)
	{
		flush_page(*pages.back());
		pages.erase(--pages.end());
	}
}


}//namespace
