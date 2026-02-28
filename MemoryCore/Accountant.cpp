#include "Accountant.h"
#include <cstdlib>

void* operator new(std::size_t n)
{
    void* p = std::malloc(n + sizeof n);
	if (p == nullptr)
        throw std::bad_alloc{};
	auto q{ static_cast<std::size_t*>(p) };
	*q = n;
	Accountant::get().take(n);
    return q + 1;
}

void* operator new[](std::size_t n)
{
	void* p = std::malloc(n + sizeof n);
	if (p == nullptr)
		throw std::bad_alloc{};
	auto q{ static_cast<std::size_t*>(p) };
	*q = n;
	Accountant::get().take(n);
	return q + 1;
}

void operator delete(void* ptr) noexcept
{
	if (ptr == nullptr)
		return;
	auto q{ static_cast<std::size_t*>(ptr) - 1 };
	Accountant::get().give_back(*q);
	std::free(q);
}

void operator delete[](void* ptr) noexcept
{
	if (ptr == nullptr)
		return;
	auto q{ static_cast<std::size_t*>(ptr) - 1 };
	Accountant::get().give_back(*q);
	std::free(q);
}
