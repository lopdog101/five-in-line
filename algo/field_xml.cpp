#include "field_xml.h"
#ifdef USE_XML

namespace Xpat
{

void xmlpack(ipacker_t& root_node,const Gomoku::point& val)
{
	XML_PACK(x);
	XML_PACK(y);
}

void xmlunpack(const ipacker_t& root_node,Gomoku::point& val)
{
	XML_UNPACK(x);
	XML_UNPACK(y);
}

void xmlpack(ipacker_t& root_node,const Gomoku::rect& val)
{
	XML_PACK(x1);
	XML_PACK(y1);
	XML_PACK(x2);
	XML_PACK(y2);
}

void xmlunpack(const ipacker_t& root_node,Gomoku::rect& val)
{
	XML_UNPACK(x1);
	XML_UNPACK(y1);
	XML_UNPACK(x2);
	XML_UNPACK(y2);
}

template<> enum_item const* xml_type_enum<Gomoku::Step>::get()
{
	static enum_item ret[]=
	{
		enum_item(Gomoku::st_empty,"empty"),
		enum_item(Gomoku::st_krestik,"krestik"),
		enum_item(Gomoku::st_nolik,"nolik"),
		enum_item(Gomoku::st_undefined,"undefined"),
		enum_item(0,0)
	};
	return ret;
}

void xmlpack(ipacker_t& root_node,const Gomoku::step_t& val)
{
	xmlpack(root_node,static_cast<const Gomoku::point&>(val));
	XML_PACK(step);
}

void xmlunpack(const ipacker_t& root_node,Gomoku::step_t& val)
{
	xmlunpack(root_node,static_cast<Gomoku::point&>(val));
	XML_UNPACK(step);
}

void xmlpack(ipacker_t& root_node,const Gomoku::field_t& val)
{
	XML_NPACK("steps",val.get_steps());
}

void xmlunpack(const ipacker_t& root_node,Gomoku::field_t& val)
{
	Gomoku::steps_t steps;
	XML_MUNPACK(steps);
	val.set_steps(steps);
}

}//namespace Xpat
#endif