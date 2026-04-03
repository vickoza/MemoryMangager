#include "Accountant.h"
#include <cstdlib>
#include <cstddef>
#include <cstring>
#include <new>
#include <algorithm>

Accountant::Accountant() : cur(0), memoryChunksUsedSize(0)
{
	memoryChunksUsed = static_cast<std::max_align_t**>(std::malloc(sizeof(std::max_align_t*) * 16));
	memoryChunksUsedCapacity = 16;
}

void Accountant::take(std::size_t size, std::max_align_t* ptr)
{
	cur += size;
	if (memoryChunksUsedSize == memoryChunksUsedCapacity)
	{
		std::lock_guard<std::mutex> lock(mtx);
		std::max_align_t** newMemoryChunksUsed = static_cast<std::max_align_t**>(std::malloc(sizeof(std::max_align_t*) * memoryChunksUsedCapacity * 2));
		std::memcpy(newMemoryChunksUsed, memoryChunksUsed, sizeof(std::max_align_t*) * memoryChunksUsedCapacity);
		std::free(memoryChunksUsed);
		memoryChunksUsed = newMemoryChunksUsed;
		memoryChunksUsedCapacity = 2 * memoryChunksUsedCapacity;
	}
	memoryChunksUsed[memoryChunksUsedSize] = ptr;
	++memoryChunksUsedSize;

}
void Accountant::give_back(std::size_t size, std::max_align_t* ptr)
{
	{
		std::lock_guard<std::mutex> lock(mtx);
		auto it{ std::find(memoryChunksUsed, memoryChunksUsed + memoryChunksUsedSize, ptr) };
		if (it != memoryChunksUsed + memoryChunksUsedSize)
		{
			std::swap(*it, memoryChunksUsed[memoryChunksUsedSize - 1]);
			--memoryChunksUsedSize;
		}
	}
	cur -= *ptr;
}

Accountant::~Accountant() //should be called at the end of the program, so no need to lock the mutex
{
	free(memoryChunksUsed);
}

void Accountant::outputMemoryChunksUsedReport(std::ostream& os) const
{
	os << "Memory chunks used:\n";
	for (std::size_t i = 0; i < memoryChunksUsedSize; ++i)
	{
		os << "Chunk " << i + 1 << ": @" << memoryChunksUsed[i] << " " << *memoryChunksUsed[i] << " bytes\n";
	}
}

void* operator new(std::size_t n) noexcept(false)
{
    void* p = std::malloc(n + sizeof(std::max_align_t));
	if (p == nullptr)
        throw std::bad_alloc{};
	std::max_align_t* q = new (p) std::max_align_t(n);
	Accountant::get().take(n, q);
    return q + 1;
}

void* operator new[](std::size_t n) noexcept(false)
{
	void* p = std::malloc(n + sizeof(std::max_align_t));
	if (p == nullptr)
		throw std::bad_alloc{};
	std::max_align_t* q = new (p) std::max_align_t(n);
	Accountant::get().take(n, q);
	return q + 1;
}

void operator delete(void* ptr, std::size_t n) noexcept
{
	if (ptr == nullptr)
		return;
	auto q{ static_cast<std::max_align_t*>(ptr) - 1 };
	Accountant::get().give_back(n,q);
	std::free(q);
}

void operator delete[](void* ptr, std::size_t n) noexcept
{
	if (ptr == nullptr)
		return;
	auto q{ static_cast<std::max_align_t*>(ptr) - 1 };
	Accountant::get().give_back(n,q);
	std::free(q);
}
