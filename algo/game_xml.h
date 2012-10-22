#ifndef gomoku_game_xmlH
#define gomoku_game_xmlH
#include "game.h"
#ifdef USE_XML
#include "field_xml.h"

namespace Xpat
{

template <> inline std::string xml_type_name< Gomoku::iplayer_t >::get(){return "iplayer";};

template<>
struct copy_constructable< Gomoku::iplayer_t >
{
	static const bool value=false;
};

template <> inline std::string xml_type_name< Gomoku::game_t >::get(){return "game";};
void xmlpack(ipacker_t& root_node,const Gomoku::game_t& val);
void xmlunpack(const ipacker_t& root_node,Gomoku::game_t& val);

}//namespace

#endif
#endif
