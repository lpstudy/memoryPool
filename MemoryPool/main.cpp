#include <iostream>
#include "mempool.h"
using namespace std;

#define MEMORY_SIZE 4096*2
int main()
{
	/* for test
	#define MEMORY_ENABLE_MULTI_THREAD_ALLOC //whether to enable multi-thread allocation and deallocation
	#define MEMORY_ENABLE_TRACE

	#define MEMORT_LENGTH_MAX	256 //size in bytes (max size that have been allocated for use)
	#define MEMORY_LENGTH_STEP	8 //one unit size

	#define MEMORY_LENGTH_POPULAR 32 //most common used memory size
	#define NUM_POPULAR		10 //pre-alloc NUM_POPULAR with size of MEMORY_LENGTH_POPULAR
	*/
	cout << "Test Init:" << endl;
	TMemoryManager* m_pMemPool = TMemoryManager::GetInstance();
	cout << "Test Basic Get and Return:" << endl;
	for (int i = 0; i < MEMORT_LENGTH_MAX+10; ++i)
	{
		//printf("GetMemory(%d):\n", i);
		char*p = (char*)m_pMemPool->GetMemory(i);
		m_pMemPool->ReturnMemory(p);
	}
	cout << "Test Next Get and Return:" << endl;
	for (int i = 0; i < MEMORT_LENGTH_MAX + 10; ++i)
	{
		//printf("GetMemory(%d):\n", i);
		char*p = (char*)m_pMemPool->GetMemory(i);
		m_pMemPool->ReturnMemory(p);
	}

	cout << "Test popular:" << endl;
	for (int i = 0; i < NUM_POPULAR+1; ++i)
	{
		//should only alloc once
		char*p = (char*)m_pMemPool->GetMemory(MEMORY_LENGTH_POPULAR);
	}

	cout << "Test Add popular:" << endl;
	m_pMemPool->AddPopularMemory(40, NUM_POPULAR);
	cout << "Test Add popular by GetMemory:" << endl;
	for (int i = 0; i < NUM_POPULAR + 1; ++i)
	{
		//should have no alloc
		char*p = (char*)m_pMemPool->GetMemory(40);
	}
	cout << "Test Delete:" << endl;
	TMemoryManager::Delete();
}