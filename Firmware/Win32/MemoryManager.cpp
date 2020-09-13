/*
 * MemoryManager.cpp
 *
 *  Created on: 28.08.2014
 *      Author: Viktor Pankraz
 */

#include <MemoryManager.h>

void* allocOnce( size_t size )
{
   return new char[size];
}

void getUnusedMemory( uint16_t* freeStack, uint16_t* freeHeap )
{
   freeHeap[0] = freeStack[0] = 0;
}