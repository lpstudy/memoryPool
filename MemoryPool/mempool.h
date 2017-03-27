//////////////////////////////////////////////////////////////////////
// DEVELOP: ZHAOZL 2010.08.03
// MODIFY : LPSTUDY 2017.03.27
// Required: c++11 (std::mutex)
// It should work well for scenarios with memory alloc and dealloc frequently, to reduce the memory fragment
// For some cases, there will be some size are used really often (i.e. large number of objects with same size). 
// It will be better to set MEMORY_LENGTH_POPULAR and NUM_POPULAR
// If you alloc larger than MEMORT_LENGTH_MAX, the default new function will be called, where memory poll will not fuction.

// Multi-thread use: enable  MEMORY_ENABLE_MULTI_THREAD_ALLOC
// Alloc trace:      enable  MEMORY_ENABLE_TRACE
//////////////////////////////////////////////////////////////////////

#ifndef __MEMPOOL_H__
#define __MEMPOOL_H__ 

//#define MEMORY_ENABLE_MULTI_THREAD_ALLOC //whether to enable multi-thread allocation and deallocation
//#define MEMORY_ENABLE_TRACE

#define MEMORT_LENGTH_MAX	256 //size in bytes (max size that have been allocated for use)
#define MEMORY_LENGTH_STEP	8 //one unit size

#define MEMORY_LENGTH_POPULAR 32 //most common used memory size
#define NUM_POPULAR		10 //pre-alloc NUM_POPULAR with size of MEMORY_LENGTH_POPULAR



class TMemoryManager
{
public:
	virtual ~TMemoryManager();

	static TMemoryManager * GetInstance(); //not thread-safe, called first.
	static bool Delete(); //not thread-safe, called before it exists.

	void * GetMemory(unsigned int nMemorySize);
	bool ReturnMemory(char *pMemory);
	//pre-alloc the numPopular memory blocks with one block size of nPopularSize
	bool AddPopularMemory(unsigned int nPopularSize, unsigned int numPopular);
private:
	bool InitMemory();
	bool DistroyMemoryManager();
	TMemoryManager(); //No construction and copy
	TMemoryManager(const TMemoryManager&);
	void operator = (const TMemoryManager&);

private:
	char **m_pMemoryArray;
	static TMemoryManager *m_pMemoryManager;
};

#endif // __MEMPOOL_H__

