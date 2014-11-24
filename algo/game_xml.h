#ifndef gomoku_game_xmlH
#define gomoku_game_xmlH
#include "game.h"
#include "field_xml.h"
#include <boost/serialization/shared_ptr.hpp>
#include <boost/serialization/nvp.hpp>

BOOST_SERIALIZATION_SPLIT_FREE(::Gomoku::game_t)
//BOOST_SERIALIZATION_ASSUME_ABSTRACT(Gomoku::iplayer_t)

namespace boost {namespace serialization {

template<class Archive>
void save(Archive & ar,const  Gomoku::game_t& v, const unsigned int version)
{
    ar & make_nvp(BOOST_PP_STRINGIZE(field),v.field());
	::Gomoku::player_ptr krestik;
    ar & make_nvp(BOOST_PP_STRINGIZE(krestik),krestik);
	::Gomoku::player_ptr nolik;
    ar & make_nvp(BOOST_PP_STRINGIZE(nolik),nolik);
}

template<class Archive>
void load(Archive & ar, Gomoku::game_t& v, const unsigned int version)
{
    ar & make_nvp(BOOST_PP_STRINGIZE(field),v.field());
    
    Gomoku::player_ptr krestik,nolik;
    ar & BOOST_SERIALIZATION_NVP(krestik);
    ar & BOOST_SERIALIZATION_NVP(nolik);

    v.set_krestik(krestik);
    v.set_nolik(nolik);
}

} } // namespace

#endif
