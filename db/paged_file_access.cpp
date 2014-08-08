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

file_offset_t paged_file_t::get_size()
{
	open();
	return file_size;
}

void paged_file_t::load(file_offset_t offset,data_t& res)
{
	open();

	file_offset_t from=from_index(offset);
	file_offset_t to=to_index(offset,res.size());
	
	file_offset_t res_to=offset+res.size();

	for(file_offset_t i=from;i<to;i++)
	{
		page_ptr pp=get_page(i);
		const page_t& p=*pp;

		file_offset_t p_from=p.index*page_size;
		file_offset_t p_to=p_from+p.data.size();

		file_offset_t common_from=(offset>p_from)? offset:p_from;
		file_offset_t common_to=(res_to<p_to)? res_to:p_to;

		std::copy(p.data.begin()+static_cast<ptrdiff_t>(common_from-p_from),
			p.data.begin()+static_cast<ptrdiff_t>(common_to-p_from),
			res.begin()+static_cast<ptrdiff_t>(common_from-offset));
	}
}

void paged_file_t::save(file_offset_t offset,const data_t& res)
{
	open();

	file_offset_t from=from_index(offset);
	file_offset_t to=to_index(offset,res.size());
	
	file_offset_t res_to=offset+res.size();

	for(file_offset_t i=from;i<to;i++)
	{
		page_ptr pp=get_page(i);
		page_t& p=*pp;

		file_offset_t p_from=p.index*page_size;
		file_offset_t p_to=p_from+page_size;

		file_offset_t common_from=(offset>p_from)? offset:p_from;
		file_offset_t common_to=(res_to<p_to)? res_to:p_to;

		size_t data_size=static_cast<size_t>(common_to-p_from);
		if(data_size>p.data.size())
			p.data.resize(data_size);

		std::copy(res.begin()+static_cast<ptrdiff_t>(common_from-offset),
			res.begin()+static_cast<ptrdiff_t>(common_to-offset),
			p.data.begin()+static_cast<ptrdiff_t>(common_from-p_from));
		p.dirty=true;

		if(common_to>file_size)
			file_size=common_to;
	}
}

file_offset_t paged_file_t::append(const data_t& res)
{
	file_offset_t offset=get_size();
	save(offset,res);
	return offset;
}

void paged_file_t::flush()
{
	pages_t::iterator from=pages.begin();
	pages_t::iterator to=pages.end();
	for(;from!=to;++from)
		flush_page(*from->second);
}

void paged_file_t::flush_page(page_t& p)
{
	if(!p.dirty)
		return;

	regular_file_t::save(p.index*page_size,p.data);
	p.dirty=false;
}

paged_file_t::page_ptr paged_file_t::get_page(file_offset_t idx)
{
	pages_t::iterator it=pages.find(idx);

	if(it!=pages.end())
	{
		page_ptr p=it->second;
		bring_to_front(p);
		return p;
	}

	page_ptr p(new page_t(idx,page_size));
	p->self=p;
	file_offset_t offset=idx*page_size;
	size_t sz=page_size;
	if(offset+sz>file_size)sz=static_cast<ptrdiff_t>(file_size-offset);
	p->data.resize(sz);
	regular_file_t::load(offset,p->data);

	add_page(p);

	return p;
}

void paged_file_t::bring_to_front(page_ptr& p)
{
	page_ptr left=p->left.lock();
	if(!left)
		return;

	page_ptr right=p->right.lock();

	if(left)left->right=right;

	if(right)right->left=left;
	else last_page=left;

	p->left=page_wptr();

	page_ptr pp=first_page.lock();
	p->right=first_page;
	if(pp)pp->left=p;
	first_page=p;
}


void paged_file_t::add_page(const page_ptr& p)
{
	if(pages.empty())
		last_page=p;

	pages[p->index]=p;

	page_ptr pp=first_page.lock();
	p->right=first_page;
	if(pp)pp->left=p;
	first_page=p;

	remove_oldest();
}

void paged_file_t::remove_oldest()
{
	while(pages.size()>=max_pages)
	{
		page_ptr p=last_page.lock();
		flush_page(*p);

		last_page=p->left;
		pages.erase(p->index);
	}
}

}//namespace
