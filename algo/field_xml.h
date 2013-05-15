#ifndef gomoku_field_xmlH
#define gomoku_field_xmlH
#include "field.h"
#include <boost/serialization/string.hpp>
#include <boost/serialization/vector.hpp>
#include <boost/serialization/nvp.hpp>
#include <boost/serialization/split_free.hpp>
#include <boost/lexical_cast.hpp>

BOOST_SERIALIZATION_SPLIT_FREE(::Gomoku::field_t)
BOOST_SERIALIZATION_SPLIT_FREE(::Gomoku::Step)

namespace boost {namespace serialization {

template<class Archive>
void serialize(Archive & ar, Gomoku::point & v, const unsigned int version)
{
    ar & make_nvp(BOOST_PP_STRINGIZE(x), v.x);
    ar & make_nvp(BOOST_PP_STRINGIZE(y), v.y);
}

template<class Archive>
void serialize(Archive & ar, Gomoku::rect & v, const unsigned int version)
{
    ar & make_nvp(BOOST_PP_STRINGIZE(x1), v.x1);
    ar & make_nvp(BOOST_PP_STRINGIZE(y1), v.y1);
    ar & make_nvp(BOOST_PP_STRINGIZE(x2), v.x2);
    ar & make_nvp(BOOST_PP_STRINGIZE(y2), v.y2);
}

template<class Archive>
void save(Archive & ar,const  Gomoku::Step& v, const unsigned int version)
{
    std::string s;
    switch(v)
    {
    case ::Gomoku::st_empty: s = "empty";break;
    case ::Gomoku::st_krestik: s = "krestik";break;
    case ::Gomoku::st_nolik: s = "nolik";break;
    default:
        throw std::runtime_error("save<Step>(): invalid value: "+boost::lexical_cast<std::string>(v));
    }

    ar & s;
}

template<class Archive>
void load(Archive & ar, Gomoku::Step& v, const unsigned int version)
{
    std::string s;
    ar & s;

    if(s == "empty") v=st_empty;
    else if(s == "krestik") v=st_krestik;
    else if(s == "nolik") v=st_nolik;
    else throw std::runtime_error("load<Step>(): invalid value: "+s);
}

template<class Archive>
void serialize(Archive & ar, Gomoku::step_t & v, const unsigned int version)
{
    ar & make_nvp(BOOST_PP_STRINGIZE(point),base_object<Gomoku::point>(v));
//    ar & v.step;
}


template<class Archive>
void save(Archive & ar,const  Gomoku::field_t& v, const unsigned int version)
{
    ar & make_nvp(BOOST_PP_STRINGIZE(steps),v.get_steps());
}

template<class Archive>
void load(Archive & ar, Gomoku::field_t& v, const unsigned int version)
{
    Gomoku::steps_t steps;
    ar & make_nvp(BOOST_PP_STRINGIZE(steps),steps);
    v.set_steps(steps);
}

} } // namespace
#endif
