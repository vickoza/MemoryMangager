#pragma once
#include <cstddef>
#include <atomic>
#include <mutex>
#include <ostream>

class Accountant
{
	std::atomic<long long> cur;
	std::max_align_t** memoryChunksUsed;
	std::atomic<std::size_t> memoryChunksUsedSize;
	std::atomic<std::size_t> memoryChunksUsedCapacity;
	std::mutex mtx;
	Accountant();
	void take(std::size_t size, std::max_align_t* ptr);
	void give_back(std::size_t size, std::max_align_t* ptr);
public:
	~Accountant();
	friend void* operator new(std::size_t m) noexcept(false);
	friend void* operator new[](std::size_t m) noexcept(false);
	friend void operator delete(void* ptr, std::size_t n) noexcept;
	friend void operator delete[](void* ptr, std::size_t n) noexcept;
	Accountant(Accountant const&) = delete;
	Accountant& operator=(Accountant const&) = delete;
	static auto& get() 
	{
		static Accountant instance;
		return instance;
	}
	std::size_t how_much() const
	{
		return cur.load();
	}
	void outputMemoryChunksUsedReport(std::ostream& os) const;
};

