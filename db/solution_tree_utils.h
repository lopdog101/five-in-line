#ifndef solution_tree_utilsH
#define solution_tree_utilsH
#include "../algo/field.h"
#include "bin_index_utils.h"

namespace Gomoku
{
	void points2bin(const points_t& pts,data_t& bin);
	void bin2points(const data_t& bin,points_t& pts);
	void points2bin(const steps_t& pts,data_t& bin);
	void bin2points(const data_t& bin,steps_t& pts);
	void points2bin(const npoints_t& pts,data_t& bin);
	void bin2points(const data_t& bin,npoints_t& pts);

    void hex_or_str2points(const std::string& str,steps_t& pts);

}//namespace

#endif
