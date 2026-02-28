#pragma once
#include <cstddef>
#include <atomic>
#include <new>
class Accountant
{
	std::atomic<long long> cur;
	Accountant() : cur(0) {}
public:
	Accountant(Accountant const&) = delete;
	Accountant& operator=(Accountant const&) = delete;
	static auto& get() 
	{
		static Accountant instance;
		return instance;
	}
	void take(std::size_t size)
	{
		cur += size;
	}
	void give_back(std::size_t size)
	{
		cur -= size;
	}
	auto how_much() const
	{
		return cur.load();
	}
};

void* operator new(std::size_t m);
void* operator new[](std::size_t m);
void operator delete(void* ptr) noexcept;
void operator delete[](void* ptr) noexcept;