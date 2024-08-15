#include <iostream>
#include "../unittest.h"

const bool UnitTest::BaseTest::Test(void) const noexcept
{
	const size_t testCount = _tests.size();
	std::cout << "Test Count: " << testCount << std::endl;
	for ( size_t i = 0; i < testCount; i++ )
	{
		std::cout << "\tTest[#" << i << "] `" << _names[i] << "`: ";
		if ( _tests[i]() == false )
		{
			std::cout << " Failed" << std::endl;
			return false;
		}
		std::cout << " Passed" << std::endl;
	}
	return true;
}