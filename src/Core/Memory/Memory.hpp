#pragma once


#include "Core/MultiThreading/SafeNumeric.hpp"

#include <Utils/Common.hpp>

#include <new>
#include <type_traits>
#include <xmmintrin.h>
#include <boost/align/align.hpp>
//#include <boost/align/is_aligned.hpp>
#include <boost/align/aligned_alloc.hpp>
//#include <boost/align/align_down.hpp>
//#include <boost/align/align_up.hpp>
#include <boost/align/aligned_allocator.hpp>
//#include <boost/pool/object_pool.hpp>
//#include <boost/pool/pool_alloc.hpp>

#ifndef D_MEMORY
#define D_MEMORY Darius::Core::Memory
#endif // !D_MEMORY

#ifndef D_PAD_ALIGN
#define D_PAD_ALIGN 16 //must always be greater than this at much
#endif

namespace Darius::Core::Memory
{
	// Godot Memory
	/* Copyright (c) 2014-present Godot Engine contributors (see AUTHORS.md). */
	/* Copyright (c) 2007-2014 Juan Linietsky, Ariel Manzur.                  */
	/*                                                                        */
	/* Permission is hereby granted, free of charge, to any person obtaining  */
	/* a copy of this software and associated documentation files (the        */
	/* "Software"), to deal in the Software without restriction, including    */
	/* without limitation the rights to use, copy, modify, merge, publish,    */
	/* distribute, sublicense, and/or sell copies of the Software, and to     */
	/* permit persons to whom the Software is furnished to do so, subject to  */
	/* the following conditions:                                              */
	/*                                                                        */
	/* The above copyright notice and this permission notice shall be         */
	/* included in all copies or substantial portions of the Software.        */
	/*                                                                        */
	/* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,        */
	/* EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF     */
	/* MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. */
	/* IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY   */
	/* CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,   */
	/* TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE      */
	/* SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.                 */

	class MemoryManager
	{
#if _DEBUG
		static D_CORE_THREADING::SafeNumeric<uint64_t> MaxUsage;
		static D_CORE_THREADING::SafeNumeric<uint64_t> MemUsage;
#endif

		static D_CORE_THREADING::SafeNumeric<uint64_t> AllocCount;

	public:
		static void* AllocStatic(size_t bytes, bool padAlign = false);
		static void* ReallocStatic(void* memory, size_t, bool padAlign = false);
		static void FreeStatic(void* ptr, bool padAlign = false);

		static uint64_t GetMemAvailable();
		static uint64_t GetMemUsage();
		static uint64_t GetMemMaxUsage();
	};

	class DefaultAllocator
	{
	public:
		INLINE static void* Alloc(size_t memory) { return MemoryManager::AllocStatic(memory, false); }
		INLINE static void Free(void* ptr) { MemoryManager::FreeStatic(ptr, false); }
	};

	INLINE void PostInitializeHandler(void*) {}

	template<class T>
	INLINE T* _PostInitialize(T* obj) {
		PostInitializeHandler(obj);
		return obj;
	}

	// Returns whether should be deleted
	INLINE bool PreDeleteHandler(void*) { return true; }

	template<class T>
	void MemDelete(T* obj)
	{
		if (!PreDeleteHandler(obj))
			return; // Doesn't want to be deleted

		if (!std::is_trivially_destructible<T>::value)
			obj->~T();

		MemoryManager::FreeStatic(obj, false);
	}

	template<class T, class A>
	void MemDeleteAllocator(T* obj)
	{
		if (!PreDeleteHandler(obj))
			return;
		if (!std::is_trivially_destructible<T>::value)
			obj->~T();

		A::Free(obj);
	}

	template<typename T>
	T* MemNewArrayTamplate(size_t elements)
	{
		if (elements == 0)
			return nullptr;

		/** overloading operator new[] cannot be done , because it may not return the real allocated address (it may pad the 'element count' before the actual array). Because of that, it must be done by hand. This is the
		same strategy used by std::vector, and the Vector class, so it should be safe.*/

		size_t len = sizeof(T) * elements;
		uint64_t* mem = (uint64_t*)MemoryManager::AllocStatic(len, true);
		T* failptr = nullptr;

		if (mem == nullptr)
			return failptr;
		
		*(mem - 1) = elements;

		if (!std::is_trivially_constructible<T>::value) {
			T* elems = (T*)mem;

			/* call operator new */
			for (size_t i = 0; i < elements; i++) {
				new (&elems[i]) T;
			}
		}

		return (T*)mem;
	}

	template <typename T>
	size_t MemArrayLength(T const* obj) {
		uint64_t* ptr = (uint64_t*)obj;
		return *(ptr - 1);
	}

	template <typename T>
	void MemDeleteArray(T* obj) {
		uint64_t* ptr = (uint64_t*)obj;

		if (!std::is_trivially_destructible<T>::value) {
			uint64_t elem_count = *(ptr - 1);

			for (uint64_t i = 0; i < elem_count; i++) {
				obj[i].~T();
			}
		}

		MemoryManager::free_static(ptr, true);
	}

	template<typename T, std::size_t Alignment = 64>
	using AlignedAllocator = boost::alignment::aligned_allocator<T, Alignment>;

	class Alignment
	{
	public:
		explicit Alignment(int value) : m_value(value) {}
		int GetValue() const
		{
			return m_value;
		}

	private:
		int m_value;
	};

	template <typename T>
	INLINE T AlignUpWithMask(T value, size_t mask)
	{
		return (T)(((size_t)value + mask) & ~mask);
	}

	template <typename T>
	INLINE T AlignDownWithMask(T value, size_t mask)
	{
		return (T)((size_t)value & ~mask);
	}

	INLINE void* Align(std::size_t alignment, std::size_t size, void*& ptr, std::size_t& space)
	{
		return boost::alignment::align(alignment, size, ptr, space);
	}

	template<class T>
	INLINE constexpr T AlignUp(T value, std::size_t alignment) noexcept
	{
		return AlignUpWithMask(value, alignment - 1);
	}

	template<class T>
	INLINE constexpr T AlignDown(T value, std::size_t alignment) noexcept
	{
		return AlignDownWithMask(value, alignment - 1);
	}

	INLINE void* AlignedAlloc(std::size_t alignment, std::size_t size)
	{
		return boost::alignment::aligned_alloc(alignment, size);
	}

	INLINE void AlignedFree(void* ptr)
	{
		boost::alignment::aligned_free(ptr);
	}

	template<class T>
	INLINE constexpr bool IsAligned(T value, std::size_t alignment) noexcept
	{
		return 0 == ((size_t)value & (alignment - 1));
	}

	template <typename T>
	INLINE T DivideByMultiple(T value, size_t alignment)
	{
		return (T)((value + alignment - 1) / alignment);
	}

	void SIMDMemCopy(void* __restrict Dest, const void* __restrict Source, size_t NumQuadwords);
	void SIMDMemFill(void* __restrict Dest, __m128 FillVector, size_t NumQuadwords);
}

void* operator new(size_t size, const char* description); // operator new that takes a description and uses MemoryStaticPool
void* operator new(size_t size, void* (*allocfunc)(size_t size)); // operator new that takes a description and uses MemoryStaticPool

void* operator new(size_t size, void* pointer, size_t check, const char* description); // operator new that takes a description and uses a pointer to the preallocated memory

void operator delete(void* mem, const char* description);
void operator delete(void* mem, void* (*allocfunc)(size_t size));
void operator delete(void* mem, void* pointer, size_t check, const char* description);

#define DMemAlloc(size) D_MEMORY::MemoryManager::AllocStatic(size)
#define DMemRealloc(mem, size) D_MEMORY::MemoryManager::ReallocStatic(mem, size)
#define DMemFree(mem) D_MEMORY::MemoryManager::FreeStatic(mem)

#define DMemNew(clazz) D_MEMORY::_PostInitialize(new ("") clazz)
#define DMemNew_Allocator(clazz, allocator) D_MEMORY::_PostInitialize(new (allocator::Alloc) clazz)
#define DMemNew_Placement(placement, clazz)D_MEMORY::_PostInitialize(new (placement) clazz)

#define DMemDelete_NotNull(v) \
{ \
	if(v) \
		MemDelete(v); \
}

#define DMemNew_Array(T, count) D_MEMORY::MemNewArrayTamplate<T>(count)
