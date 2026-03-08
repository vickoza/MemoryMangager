// MemoryMangager.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <iostream>
#include "Accountant.h"

int main()
{
    auto pre{ Accountant::get().how_much() };
    {
        int* p = new int{ 3 };
        int* q = new int[10]{};
		delete p;
    }
    auto post{ Accountant::get().how_much() };
    if (post != pre)
    {
		std::cout << "Leaked " << post - pre << " bytes\n";
    }
	Accountant::get().outputMemoryChunksUsedReport(std::cout);
    std::cout << "Hello World!\n";
}