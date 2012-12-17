#ifndef tests_find_linesH
#define tests_find_linesH
#include "../include/main.h"
#include "../../algo/wsplayer.h"

class find_lines_t : public base_test_t
{
	void check_four_empty_both_sides();
	void check_four_left_zero();

	static std::string get_ctx(const Gomoku::field_t& fl,unsigned i,unsigned j);
protected:
	void process();
public:
};

#endif