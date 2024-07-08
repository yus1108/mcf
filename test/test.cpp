#include <iostream>
#include <lexer.h>

#if defined(_DEBUG)
#define fatal_assert(PREDICATE, FORMAT, ...) if (PREDICATE == false) { printf(FORMAT, ...); __debugbreak(); return false; } 
#else
#define fatal_assert(PREDICATE, FORMAT, ...) if (PREDICATE == false) { printf(FORMAT, ...); return false; } 
#endif

bool test_next_token()
{
    return true;
}

int main()
{
    if (test_next_token() == false)
    {
        std::cout << "Test `test_next_token()` Failed" << std::endl;
        return 1;
    }
    std::cout << "All Test Passed" << std::endl;
    return 0;
}