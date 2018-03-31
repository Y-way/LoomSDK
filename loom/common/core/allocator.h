/*
 * ===========================================================================
 * Loom SDK
 * Copyright 2011, 2012, 2013
 * The Game Engine Company, LLC
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 * ===========================================================================
 */

#ifndef _CORE_ALLOCATOR_H_
#define _CORE_ALLOCATOR_H_

#include "loom/common/core/assert.h"

/**************************************************************************
 * Loom Memory Allocation API
 *
 * USAGE
 *
 * (Note that you can pass NULL instead of an allocator
 * to use the global heap.)
 *
 * loom_allocator_t *someAllocator = ...;
 * MyClass *mc = lmNew(someAllocator) MyClass(someArg);
 * lmDelete(someAllocator) mc;
 *
 * MyStruct *ms = lmAlloc(someAllocator, sizeof(ms));
 * lmFree(someAllocator, ms);
 *
 * RATIONALE
 *
 * It is useful to have global control of memory allocation. The Loom
 * Memory API enables reporting, debugging, various allocation strategies,
 * run time configuration of memory manager operation, and so on. We also
 * want to be able to compose allocators, ie, add allocation tracking
 * backed by an allocation strategy of our choice.
 *
 * OVERVIEW
 *
 * Rather than hooking the global new/delete/malloc/free calls, which can
 * be brittle and hard to work with, we provide lmAlloc, lmFree, lmNew,
 * lmDelete calls which take a loom_allocator_t.
 *
 * The core allocator code is written in C, with lmNew/lmDelete implemented
 * and only available under C++.
 *
 * The array new/delete operators are not provided due to limitations in
 * C++'s support for overloading those operators with parameters. We
 * recommend using a vector/array class instead of using new[]/delete[].
 *
 * ALLOCATORS
 *
 * Using the right kind of allocation strategy can dramatically improve
 * performance and memory efficiency. In addition, it can be convenient to
 * consider each subsystem's memory allocation independently. A
 * loom_allocator_t is used to represent an allocator, and factory
 * functions are provided for common use cases (like a fixed pool allocator,
 * one backed by the CRT heap, jemalloc, or other cases).
 *
 * Proxy allocators are also available, ie, to keep track of the memory used
 * by an allocator.
 *
 * FURTHER USAGE EXAMPLES
 *
 * The unit tests for the allocator system should be helpful in understanding
 * usage.
 *
 */

// This defines the alignment mask (alignment size minus one) used in manual
// allocation with variably offset custom data fields
#define LOOM_ALLOCATOR_ALIGN_MASK (8-1)

// This should be a multiple of 16 - we need 16-byte alignment because of SSE
#define LOOM_ALLOCATOR_METADATA_SIZE 16

#include "loom/common/core/log.h"
extern loom_logGroup_t gAllocatorLogGroup;

#ifdef __cplusplus
extern "C" {
#endif

/************************************************************************
 * C ALLOCATION MACROS
 *
 * Implement malloc/free/realloc type functionality using Loom allocators.
 ************************************************************************/
#define lmAlloc(allocator, size)           lmAlloc_inner(allocator, size, __FILE__, __LINE__)
#define lmCalloc(allocator, count, size)   lmCalloc_inner(allocator, count, size, __FILE__, __LINE__)
#define lmFree(allocator, ptr)             lmFree_inner(allocator, ptr, __FILE__, __LINE__)
#define lmSafeFree(allocator, obj)         if (obj) { lmFree(allocator, obj); obj = NULL; }
#define lmRealloc(allocator, ptr, newSize) lmRealloc_inner(allocator, ptr, newSize, __FILE__, __LINE__)

#define lmAllocVerifyAll() loom_debugAllocator_verifyAll(__FILE__, __LINE__)

typedef struct loom_allocator loom_allocator_t;

void *lmAlloc_inner(loom_allocator_t *allocator, size_t size, const char *file, int line);
void *lmCalloc_inner(loom_allocator_t *allocator, size_t count, size_t size, const char *file, int line);
void lmFree_inner(loom_allocator_t *allocator, void *ptr, const char *file, int line);
void *lmRealloc_inner(loom_allocator_t *allocator, void *ptr, size_t size, const char *file, int line);

// Call this before you do any allocations to start the allocation system!
//
// Note: Loom calls this for you in most scenarios.
void loom_allocator_startup();

// Returns the current amount of memory allocated through lmAlloc in bytes
// NOTE: Returns 0 unless LOOM_ALLOCATOR_CHECK is enabled
unsigned int loom_allocator_getAllocatedMemory();

// Allocate a new heap allocator using the provided allocator as backing
// store.
void loom_allocator_initializeHeapAllocator(loom_allocator_t *a);

// Return a pointer to the global OS heap.
loom_allocator_t *loom_allocator_getGlobalHeap();

// Allocate a new fixed pool allocator, one that can allocate up to
// itemCount items of itemSize size.
loom_allocator_t *loom_allocator_initializeFixedPoolAllocator(loom_allocator_t *parent, size_t itemSize, size_t itemCount);

// Allocate a new arena proxy allocator. This allocator keeps track of all
// the allocations that pass through it, and they are all freed automatically
// when the allocator is destroy'ed. It does this by adding 2*sizeof(void*)
// bytes to each allocation for a doubly linked list, so be aware of this
// if using it with a fixed size allocator.
loom_allocator_t *loom_allocator_initializeArenaProxyAllocator(loom_allocator_t *parent);

// The tracker proxy allows reporting of total allocations and total
// allocated footprint in bytes. It passes allocations through to its
// parent allocator, and imposes sizeof(size_t) overhead on each allocation.
loom_allocator_t *loom_allocator_initializeTrackerProxyAllocator(loom_allocator_t *parent);
void loom_allocator_getTrackerProxyStats(loom_allocator_t *thiz, size_t *allocatedBytes, size_t *allocatedCount);

// Destroy an allocator. Depending on the allocator's implementation this
// may also free all of its allocations (like in the arena proxy).
void loom_allocator_destroy(loom_allocator_t *a);

/************************************************************************
* Custom Allocator API.
*
* You can provide your own allocator modules. They need to have an
* initialization function that fills out and returns a loom_allocator
* instance allocated from the parent allocator.
*
* loom_allocator_alloc_t should allocate new memory. loom_allocator_free_t
* should free that memory given a pointer. loom_allocator_realloc_t should
* obey realloc() semantics. loom_allocator_destructor_t should clean up
* the allocator.
************************************************************************/
typedef void *(*loom_allocator_alloc_t)(loom_allocator_t *thiz, size_t size, const char *file, int line);
typedef void (*loom_allocator_free_t)(loom_allocator_t *thiz, void *ptr, const char *file, int line);
typedef void *(*loom_allocator_realloc_t)(loom_allocator_t *thiz, void *ptr, size_t newSize, const char *file, int line);
typedef void (*loom_allocator_destructor_t)(loom_allocator_t *thiz);

struct loom_allocator
{
    const char                  *name;
    void                        *userdata;

    loom_allocator_alloc_t      allocCall;
    loom_allocator_free_t       freeCall;
    loom_allocator_realloc_t    reallocCall;
    loom_allocator_destructor_t destroyCall;

    loom_allocator_t            *parent;
};

// Allocation callback function pointers
typedef void (*loom_allocator_callback_free_t)(loom_allocator_t *thiz, void *inner, size_t size, const char *file, int line);

typedef struct loom_debugAllocatorCallbacks loom_debugAllocatorCallbacks_t;
struct loom_debugAllocatorCallbacks
{
    loom_allocator_callback_free_t onFree;
    loom_debugAllocatorCallbacks_t* next;
};

// Register allocation function callbacks struct in the
// debug allocator callbacks list.
//
// While the debug allocator is enabled, the appropriate registered
// callback functions get called on every allocation/free.
//
// E.g. when some block of bytes is deallocated using `lmFree`,
// all of the valid registered `onFree` functions get called with
// the (inner) pointer to the deallocated block, the size of the block
// and the deallocation source file path and line number.
void loom_debugAllocator_registerCallbacks(loom_debugAllocatorCallbacks_t* callbacks);

// Verify all the allocated blocks made from all the tracked debug allocators
// using the provided source file and line as the source of the failure.
// Note: use `lmAllocVerifyAll()` to automatically provide the file and line.
void loom_debugAllocator_verifyAll(const char* file, int line);



#ifdef __cplusplus
}; // close extern "C"
#endif

// Define C++ memory API
#ifdef __cplusplus

/************************************************************************
* C++ ALLOCATION API
*
* It is problematic to override new/delete globally, so we require developers
* to use lmNew and lmFree instead. new Foo() becomes
* lmNew(someAllocator) Foo(), and delete myFoo becomes
* lmDelete(someAllocator, myfoo). We do not support the delete[] or new[]
* operators, if you want an array use the templated vector class.
*
* `obj` can change addresses after `loom_destructInPlace`, so we use
* its return value, which is the original address, for freeing the memory.
*
************************************************************************/
#define lmNew(allocator)                new(allocator, __FILE__, __LINE__, (LS::FunctionDisambiguator*) NULL)
#define lmDelete(allocator, obj)        { lmFree(allocator, loom_destructInPlace(obj)); }
#define lmSafeDelete(allocator, obj)    if (obj) { lmFree(allocator, loom_destructInPlace(obj)); obj = NULL; }
#include <new>

namespace LS { struct FunctionDisambiguator {}; }

inline void *operator new(size_t size, loom_allocator_t *a, const char *file, int line, LS::FunctionDisambiguator* disamb)
{
    return lmAlloc(a, size);
}


inline void operator delete(void *p, loom_allocator_t *a, const char *file, int line, LS::FunctionDisambiguator* disamb)
{
    lmFree(a, p);
}


inline void operator delete(void *p, loom_allocator_t *a)
{
    lmFree(a, p);
}

// Construct the type with preallocated memory (construct with no allocation)
// Usage: loom_constructInPlace<CustomType>(preallocatedMemoryOfSufficientSize);
#pragma warning( disable: 4345 )
template<typename T>
T* loom_constructInPlace(void* memory)
{
    return new (memory)T();
}

// Destruct the type without freeing memory (calls the destructor)
template<typename T>
T* loom_destructInPlace(T *t)
{
    if (t == NULL) return NULL;
    t->~T();
    return t;
}

// Array per-type properties, currently only used to determine which types
// are fundamental to avoid constructing them.
template<typename T> struct ArrayAlloc { enum { fundamental = false }; };
template<typename T> struct ArrayAlloc<T*> { enum { fundamental = true }; };
template<> struct ArrayAlloc<bool> { enum { fundamental = true }; };
template<> struct ArrayAlloc<char> { enum { fundamental = true }; };
template<> struct ArrayAlloc<wchar_t> { enum { fundamental = true }; };
template<> struct ArrayAlloc<signed char> { enum { fundamental = true }; };
template<> struct ArrayAlloc<short int> { enum { fundamental = true }; };
template<> struct ArrayAlloc<int> { enum { fundamental = true }; };
template<> struct ArrayAlloc<long int> { enum { fundamental = true }; };
template<> struct ArrayAlloc<long long int> { enum { fundamental = true }; };
template<> struct ArrayAlloc<unsigned char> { enum { fundamental = true }; };
template<> struct ArrayAlloc<unsigned short int> { enum { fundamental = true }; };
template<> struct ArrayAlloc<unsigned int> { enum { fundamental = true }; };
template<> struct ArrayAlloc<unsigned long int> { enum { fundamental = true }; };
template<> struct ArrayAlloc<unsigned long long int> { enum { fundamental = true }; };
template<> struct ArrayAlloc<double> { enum { fundamental = true }; };
template<> struct ArrayAlloc<long double> { enum { fundamental = true }; };

// Injects array metadata of size LOOM_ALLOCATOR_METADATA_SIZE at the beginning
// of an existing memory block. Returns a pointer to the first element
template<typename T>
static T* loom_newArray_inject(void* arr, unsigned int nr) {
    lmSafeAssert(arr, "Unable to inject metadata into a null array. Probably out of memory while allocating.");
    *(static_cast<unsigned int*>(arr)) = nr;
    arr = reinterpret_cast<T*>(reinterpret_cast<size_t>(arr) + LOOM_ALLOCATOR_METADATA_SIZE);
    return static_cast<T*>(arr);
}

// Allocate and inject an array of size nr
template<typename T>
static T* loom_newArray_alloc(loom_allocator_t *allocator, unsigned int nr)
{
    void* arr = (void*)lmAlloc(allocator, LOOM_ALLOCATOR_METADATA_SIZE + nr * sizeof(T));
    return loom_newArray_inject<T>(arr, nr);
}

// Allocate, zero-initialize and inject an array of size nr
template<typename T>
static T* loom_newArray_calloc(loom_allocator_t *allocator, unsigned int nr)
{
    void* arr = (void*)lmCalloc(allocator, 1, LOOM_ALLOCATOR_METADATA_SIZE + nr * sizeof(T));
    return loom_newArray_inject<T>(arr, nr);
}

// Constructs a new array of types of length nr using the provided allocator
// (use NULL for the default allocator).
//
// Use this or utArray instead of lmNew for constructing arrays.
// Non-fundamental types are constructed in order using loom_constructInPlace.
// Fundamental types are zero-initialized.
//
// Note that this function may allocate slightly more memory than expected
// as it has to remember the array length.
//
template<typename T>
T* loom_newArray(loom_allocator_t *allocator, unsigned int nr)
{
    T* arr;

    if (ArrayAlloc<T>::fundamental) {
        arr = loom_newArray_calloc<T>(allocator, nr);
    } else {
        arr = loom_newArray_alloc<T>(allocator, nr);
        for (unsigned int i = 0; i < nr; i++)
        {
            loom_constructInPlace<T>(&(arr[i]));
        }
    }

    return arr;
}

// Deconstructs an array allocated with loom_newArray and frees the allocated memory
// The types are destructed in reverse order using loom_destructInPlace
//
// This function only works with arrays allocated with loom_newArray
// as it has to access the array length in order to destruct the types
template<typename T>
void loom_deleteArray(loom_allocator_t *allocator, T *arr)
{
    if (arr == NULL) return;
    void* fullArray = reinterpret_cast<void *>(reinterpret_cast<size_t>(arr) - LOOM_ALLOCATOR_METADATA_SIZE);
    unsigned int nr = *(static_cast<unsigned int*>(fullArray));
    if (!ArrayAlloc<T>::fundamental) {
        while (nr > 0)
        {
            nr--;
            loom_destructInPlace<T>(&arr[nr]);
        }
    }
    lmFree(allocator, fullArray);
}

#endif
#endif
