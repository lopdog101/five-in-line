#include <memory>
#include "../include/main.h"

base_test_t* create_find_lines();

bool process(base_test_t* t)
{
	std::auto_ptr<base_test_t> thld(t);
	return t->pass();
}

int main(int argc,char** argv)
{
	process(create_find_lines());

	return 0;
}

base_test_t::base_test_t()
: lg(true)
{
}

bool base_test_t::pass()
{
	try
	{
		process();
		return true;
	}
	catch(e_check_failed& e)
	{
		lg<<"failed: "<<e.what();
		
		const e_check_failed::items_t& stack=e.get_stack();
		
		if(!stack.empty())lg<<"at:";
		for(size_t i=0;i<stack.size();i++)
		{
			const e_check_failed::item_t& s=stack[i];
			lg<<s.file<<": "<<s.line;
		}
	}
	catch(std::exception& e)
	{
		lg<<"std::exception: "<<e.what();
	}
	catch(...)
	{
		lg<<"unknown exception";
	}

	return false;
}
