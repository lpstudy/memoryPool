#include <memory>
#include <mutex>
#include "mempool.h"

#ifdef MEMORY_ENABLE_TRACE
#include <stdio.h>
#endif


#define NUM_ZERO	0
#define NUM_ONE		1


#define SIZE_INT	(sizeof(int))
#define SIZE_LPVOID	(sizeof(void *))

#define MEMORY_COUNT		(MEMORT_LENGTH_MAX / MEMORY_LENGTH_STEP)
#define MEMORY_POPULAR_SLOT ((MEMORY_LENGTH_POPULAR-1)/MEMORY_LENGTH_STEP)

#define GetNextMemory(pMemory)			((char *)(*(char **)((pMemory) - SIZE_LPVOID)))
#define SetNextMemory(pMemory, pMemoryNext)	((*((char **)((pMemory) - SIZE_LPVOID))) = (char *)pMemoryNext)
#define GetMemorySize(pMemory)			(*(int *)((pMemory) - SIZE_LPVOID - SIZE_INT))
#define SetMemorySize(pMemory, number)		(*(int *)((pMemory) - SIZE_LPVOID - SIZE_INT) = number)

/////////////////////////////////////////////////////
// CLASS TMemoryManager
/////////////////////////////////////////////////////

//multi-thread allocation
#ifdef MEMORY_ENABLE_MULTI_THREAD_ALLOC
static std::mutex mtx;
#define _TMemoryManagerLock() mtx.lock()
#define _TMemoryManagerUnLock() mtx.unlock()
#else
#define _TMemoryManagerLock() 
#define _TMemoryManagerUnLock() 
#endif // MEMORY_ENABLE_MULTI_THREAD_ALLOC

#define MEMORY_ENABLE_TRACE
#ifdef MEMORY_ENABLE_TRACE
char* _internal_new(unsigned int size){
	char* ptr = new char[size]; 
	printf("Alloc %p,%d\n", ptr, size-SIZE_INT-SIZE_LPVOID);
	return ptr;
}
void _internal_delete(void* ptr){
	delete[]((char*)ptr); 
	printf("Delete %p\n", ptr);
}
#define new_trace(size) (_internal_new(size))
#define delete_trace(ptr) (_internal_delete(ptr))
#else
#define new_trace(size) new char[size]
#define delete_trace(ptr) delete[] ((char*)ptr)
#endif


TMemoryManager *TMemoryManager::m_pMemoryManager = NULL;

TMemoryManager::TMemoryManager()
{
	m_pMemoryArray = new char*[MEMORY_COUNT]; //do not use trace
	memset(m_pMemoryArray,NUM_ZERO,sizeof(char*)*MEMORY_COUNT);
	InitMemory();
}

TMemoryManager::~TMemoryManager()
{
	if( NULL != m_pMemoryManager )
	{
		DistroyMemoryManager();
		m_pMemoryManager = NULL;
	}
}

bool TMemoryManager::DistroyMemoryManager()
{
	char *pMemory = NULL;

	for(unsigned int i = NUM_ZERO; i < MEMORY_COUNT; i++)
	{
		if( NULL == m_pMemoryArray[i] )
		{
			continue;
		}

		pMemory = GetNextMemory(m_pMemoryArray[i]);
		delete_trace(m_pMemoryArray[i] - SIZE_LPVOID - SIZE_INT);

		while( NULL != pMemory )
		{
			char *pMemoryTemp = pMemory;
			pMemory	= GetNextMemory(pMemory);
			delete_trace(pMemoryTemp - SIZE_LPVOID - SIZE_INT);
		}
	}
	delete [] m_pMemoryArray; //do not use trace
	m_pMemoryArray = NULL;

	return true;
}

TMemoryManager * TMemoryManager::GetInstance()
{
	if( NULL == m_pMemoryManager )
	{
		m_pMemoryManager = new TMemoryManager();
	}
	return m_pMemoryManager;
}

bool TMemoryManager::Delete()
{
	if( NULL != m_pMemoryManager )
	{
		delete m_pMemoryManager;
	}
	return true;
}

/*
Construct a memory block linklist with the block size of MEMORY_LENGTH_POPULAR and linklist size of NUM_POPULAR
Block:  [SIZE_INT, SIZE_LPVOID, memory]
m_pMemoryArray[i] = Block->Block->Block->NULL (Block size: (i+1)*MEMORY_LENGTH_STEP )
i >= 0 && i < MEMORT_LENGTH_MAX/MEMORY_LENGTH_STEP which means the max block size is MEMORT_LENGTH_MAX
*/
bool TMemoryManager::InitMemory()
{
	char *pMemory = NULL;

	for (int i = NUM_ZERO; i < NUM_POPULAR; i++)
	{
		char *p = new_trace((MEMORY_POPULAR_SLOT + NUM_ONE) * MEMORY_LENGTH_STEP + SIZE_LPVOID + SIZE_INT);
		*(int*)p = (MEMORY_POPULAR_SLOT + NUM_ONE) * MEMORY_LENGTH_STEP;
		pMemory = p +  SIZE_LPVOID + SIZE_INT;
		char* pTemp = m_pMemoryArray[MEMORY_POPULAR_SLOT];
		m_pMemoryArray[MEMORY_POPULAR_SLOT] = pMemory;
		SetNextMemory(pMemory, pTemp);
	}

	return true;
}
//Add popular size in runtime.
bool TMemoryManager::AddPopularMemory(unsigned int nPopularSize, unsigned int numPopular)
{
	if (nPopularSize > MEMORT_LENGTH_MAX) return false;

	char *pMemory = NULL;
	nPopularSize = (nPopularSize - 1) / MEMORY_LENGTH_STEP;
	for (int i = NUM_ZERO; i < numPopular; i++)
	{
		char *p = new_trace((nPopularSize + NUM_ONE) * MEMORY_LENGTH_STEP + SIZE_LPVOID + SIZE_INT);
		*(int*)p = (nPopularSize + NUM_ONE) * MEMORY_LENGTH_STEP;
		pMemory = p + SIZE_LPVOID + SIZE_INT;

		_TMemoryManagerLock();
		char* pTemp = m_pMemoryArray[nPopularSize];
		m_pMemoryArray[nPopularSize] = pMemory;
		SetNextMemory(pMemory, pTemp);
		_TMemoryManagerUnLock();
	}
	return true;
}

void * TMemoryManager::GetMemory(unsigned int nMemorySize)
{
	int nIdle = 0;
	char *pMemory = NULL;

	if( NUM_ZERO == nMemorySize )
	{
		return NULL;
	}

	nIdle = (nMemorySize - NUM_ONE) / MEMORY_LENGTH_STEP; //Alloc times of MEMORY_LENGTH_STEP

	if( MEMORT_LENGTH_MAX < nMemorySize  )
	{
		char *p = new_trace(nMemorySize + SIZE_LPVOID + SIZE_INT);
		*(int*)p = nMemorySize;
		pMemory = p + SIZE_LPVOID + SIZE_INT;
		return pMemory;
	}

	_TMemoryManagerLock();

	//need to get (nIdle+1) units
	if( NULL != m_pMemoryArray[nIdle] )
	{
		pMemory = m_pMemoryArray[nIdle];
		m_pMemoryArray[nIdle] = GetNextMemory(m_pMemoryArray[nIdle]);
	}
	else
	{
		char* p = new_trace((nIdle + NUM_ONE) * MEMORY_LENGTH_STEP + SIZE_LPVOID + SIZE_INT);
		*(int*)p = (nIdle + NUM_ONE) * MEMORY_LENGTH_STEP;
		pMemory = p + SIZE_LPVOID + SIZE_INT;
		SetNextMemory(pMemory, NULL);
	}

	_TMemoryManagerUnLock();
	return pMemory;
}


bool TMemoryManager::ReturnMemory(char *pMemory)
{
	if( NULL == pMemory )
	{
		return false;
	}

	int nMemorySize = GetMemorySize(pMemory);
	if( MEMORT_LENGTH_MAX < nMemorySize )
	{
		delete_trace(pMemory - SIZE_LPVOID - SIZE_INT);
		return true;
	}

	if( NUM_ZERO != nMemorySize % MEMORY_LENGTH_STEP )
	{
		return false;
	}
	
	_TMemoryManagerLock();

	char *pTemp = m_pMemoryArray[(nMemorySize - NUM_ONE) / MEMORY_LENGTH_STEP];
	m_pMemoryArray[(nMemorySize - NUM_ONE) / MEMORY_LENGTH_STEP] = pMemory;
	SetNextMemory(pMemory, pTemp);
	
	_TMemoryManagerUnLock();
	return true;
}

