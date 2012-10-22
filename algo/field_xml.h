#ifndef gomoku_field_xmlH
#define gomoku_field_xmlH
#ifdef USE_XML
#include <cppexpat/cppexpat.h>
#include "field.h"

namespace Xpat
{

template <> inline std::string xml_type_name< Gomoku::point >::get(){return "point";};
void xmlpack(ipacker_t& root_node,const Gomoku::point& val);
void xmlunpack(const ipacker_t& root_node,Gomoku::point& val);


template <> inline std::string xml_type_name< Gomoku::rect>::get(){return "rect";};
void xmlpack(ipacker_t& root_node,const Gomoku::rect& val);
void xmlunpack(const ipacker_t& root_node,Gomoku::rect& val);


template <> inline std::string xml_type_name< Gomoku::Step>::get(){return "step";};
template<> enum_item const* xml_type_enum<Gomoku::Step>::get();
inline const std::string xml_T2str(const Gomoku::Step val){return xml_enum2str<Gomoku::Step>(val);}
inline void xml_str2T(const std::string& val,Gomoku::Step& var){return xml_str2enum(val,var);}

inline void xml_pack(ipacker_t& root_node,const std::string& name,Gomoku::Step val){xml_packa(root_node,name,val);}
inline void xml_unpack(const ipacker_t& root_node,const std::string& name,Gomoku::Step& val,bool must_exist=false){xml_unpacka(root_node,name,val,must_exist);}

template <> inline std::string xml_type_name< Gomoku::step_t >::get(){return "step_t";};
void xmlpack(ipacker_t& root_node,const Gomoku::step_t& val);
void xmlunpack(const ipacker_t& root_node,Gomoku::step_t& val);

template <> inline std::string xml_type_name< Gomoku::field_t >::get(){return "field";};
void xmlpack(ipacker_t& root_node,const Gomoku::field_t& val);
void xmlunpack(const ipacker_t& root_node,Gomoku::field_t& val);

}//namespace Xpat
#endif
#endif
