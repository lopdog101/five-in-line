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
	if(fd)return;

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
	open();
	return file_size;
}

void paged_file_t::load(size_t offset,data_t& res)
{
	open();

	size_t from=from_index(offset);
	size_t to=to_index(offset,res.size());
	
	size_t res_to=offset+res.size();

	for(size_t i=from;i<to;i++)
	{
		page_ptr pp=get_page(i);
		const page_t& p=*pp;

		size_t p_from=p.index*page_size;
		size_t p_to=p_from+p.data.size();

		size_t common_from=(offset>p_from)? offset:p_from;
		size_t common_to=(res_to<p_to)? res_to:p_to;

		std::copy(p.data.begin()+(common_from-p_from),p.data.begin()+(common_to-p_from),res.begin()+(common_from-offset));
	}
}

void paged_file_t::save(size_t offset,const data_t& res)
{
	open();

	size_t from=from_index(offset);
	size_t to=to_index(offset,res.size());
	
	size_t res_to=offset+res.size();

	for(size_t i=from;i<to;i++)
	{
		page_ptr pp=get_page(i);
		page_t& p=*pp;

		size_t p_from=p.index*page_size;
		size_t p_to=p_from+page_size;

		size_t common_from=(offset>p_from)? offset:p_from;
		size_t common_to=(res_to<p_to)? res_to:p_to;

		size_t data_size=common_to-p_from;
		if(data_size>p.data.size())
			p.data.resize(data_size);

		std::copy(res.begin()+(common_from-offset),res.begin()+(common_to-offset),p.data.begin()+(common_from-p_from));
		p.dirty=true;

		if(common_to>file_size)
			file_size=common_to;
	}
}

size_t paged_file_t::append(const data_t& res)
{
	size_t offset=get_size();
	save(offset,res);
	return offset;
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

	page_ptr p(new page_t(idx,page_size));
	size_t offset=idx*page_size;
	size_t sz=page_size;
	if(offset+sz>file_size)sz=file_size-offset;
	p->data.resize(sz);
	regular_file_t::load(offset,p->data);

	add_page(p);

	return p;
}

void paged_file_t::add_page(const page_ptr& p)
{
	pages.insert(pages.begin(),p);
	remove_oldest();
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
