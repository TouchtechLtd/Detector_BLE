/*
 * test_vector.h
 *
 *  Created on: 15/09/2017
 *      Author: michaelmcadam
 */

#ifndef TEST_VECTOR_H_
#define TEST_VECTOR_H_

#include <cstdlib>
#include <new>

void* operator new(size_t size) noexcept
{
    return std::malloc(size);
}

void operator delete(void *p) noexcept
{
    std::free(p);
}

void* operator new[](size_t size) noexcept
{
    return operator new(size); // Same as regular new
}

void operator delete[](void *p) noexcept
{
    operator delete(p); // Same as regular delete
}

void* operator new(size_t size, std::nothrow_t) noexcept
{
    return operator new(size); // Same as regular new
}

void operator delete(void *p,  std::nothrow_t) noexcept
{
    operator delete(p); // Same as regular delete
}

void* operator new[](size_t size, std::nothrow_t) noexcept
{
    return operator new(size); // Same as regular new
}

void operator delete[](void *p,  std::nothrow_t) noexcept
{
    operator delete(p); // Same as regular delete
}



#endif /* TEST_VECTOR_H_ */
