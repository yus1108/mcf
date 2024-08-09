#include <iostream>
#include "test/unittest/unittest.h"

const bool UnitTest::BaseTest::Test(void) const noexcept
{
	for (size_t i = 0; i < _tests.size(); i++)
	{
		if (_tests[i]() == false)
		{
			std::cout << "\tTest[#" << i << "] `" << _names[i] << "` Failed" << std::endl;
			return false;
		}
		std::cout << "\tTest[#" << i << "] `" << _names[i] << "` Passed" << std::endl;
	}
	return true;
}