#include "Accountant.h"
#include <cstdlib>
#include <cstring>
#include <new>
#include <algorithm>

Accountant::Accountant() : cur(0), memoryChunksUsedSize(0)
{
	memoryChunksUsed = static_cast<std::size_t**>(std::malloc(sizeof(std::size_t*) * 16));
	memoryChunksUsedCapacity = 16;
}

void Accountant::take(std::size_t size, std::size_t* ptr)
{
	cur += size;
	if (memoryChunksUsedSize == memoryChunksUsedCapacity)
	{
		std::lock_guard<std::mutex> lock(mtx);
		std::size_t** newMemoryChunksUsed = static_cast<std::size_t**>(std::malloc(sizeof(std::size_t*) * memoryChunksUsedCapacity * 2));
		std::memcpy(newMemoryChunksUsed, memoryChunksUsed, sizeof(std::size_t*) * memoryChunksUsedCapacity);
		std::free(memoryChunksUsed);
		memoryChunksUsed = newMemoryChunksUsed;
		memoryChunksUsedCapacity = 2 * memoryChunksUsedCapacity;
	}
	memoryChunksUsed[memoryChunksUsedSize] = ptr;
	++memoryChunksUsedSize;

}
void Accountant::give_back(std::size_t* ptr)
{
	auto it{ std::find(memoryChunksUsed, memoryChunksUsed + memoryChunksUsedSize, ptr) };
	{
		std::lock_guard<std::mutex> lock(mtx);
		if (it != memoryChunksUsed + memoryChunksUsedSize)
		{
			std::swap(*it, memoryChunksUsed[memoryChunksUsedSize - 1]);
			--memoryChunksUsedSize;
		}
	}
	cur -= *ptr;
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
    void* p = std::malloc(n + sizeof n);
	if (p == nullptr)
        throw std::bad_alloc{};
	std::size_t* q = new (p) std::size_t(n);
	Accountant::get().take(n, q);
    return q + 1;
}

void* operator new[](std::size_t n) noexcept(false)
{
	void* p = std::malloc(n + sizeof n);
	if (p == nullptr)
		throw std::bad_alloc{};
	std::size_t* q = new (p) std::size_t(n);
	Accountant::get().take(n, q);
	return q + 1;
}

void operator delete(void* ptr) noexcept
{
	if (ptr == nullptr)
		return;
	auto q{ static_cast<std::size_t*>(ptr) - 1 };
	Accountant::get().give_back(q);
	std::free(q);
}

void operator delete[](void* ptr) noexcept
{
	if (ptr == nullptr)
		return;
	auto q{ static_cast<std::size_t*>(ptr) - 1 };
	Accountant::get().give_back(q);
	std::free(q);
}
