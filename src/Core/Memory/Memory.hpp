#pragma once

#include <Utils/Common.hpp>

#include <xmmintrin.h>
#include <boost/align/align.hpp>
#include <boost/align/is_aligned.hpp>
#include <boost/align/aligned_alloc.hpp>
#include <boost/align/align_down.hpp>
#include <boost/align/align_up.hpp>
#include <boost/align/aligned_allocator.hpp>
#include <boost/pool/object_pool.hpp>
#include <boost/pool/pool_alloc.hpp>
//#include <new>

#define D_malloc(T) reinterpret_cast<T*>(Darius::Core::Memory::AlignedAlloc(alignof(T), sizeof(T)))

#define D_malloc_par(T, ...) new (Darius::Core::Memory::Align(alignof(T))) T(__VA_ARGS__)

#define D_alloc_aligned(size, alignment) Darius::Core::Memory::AlignedAlloc(alignment, size)

#define D_free(ptr) Darius::Core::Memory::AlignedFree(ptr);

#define D_MEMORY Darius::Core::Memory

namespace Darius::Core::Memory
{

	//using PoolAllocator = 

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

using namespace Darius::Core::Memory;

// Overridden 'normal' new/delete
//void* operator new (size_t size)
//{
//	return malloc(size);
//}
//
//void* operator new[](size_t size)
//{
//	return malloc(size);
//}
//
//void operator delete(void* mem)
//{
//	free(mem);
//}
//
//void operator delete[](void* mem)
//{
//	free(mem);
//}

//template<typename T>
//void* operator new(size_t size, T data)
//{
//	auto ptr = AlignedAlloc(alignof(T), size);
//	if (ptr != nullptr)
//		*ptr = data;
//	return ptr;
//}
//
//// Aligned versions of new/delete
//void* operator new[](size_t size, Darius::Core::Memory::Alignment alignment)
//{
//	return AlignedAlloc(alignment.GetValue(), size);
//}
//void* operator new (size_t size, Darius::Core::Memory::Alignment alignment)
//{
//	return AlignedAlloc(alignment.GetValue(), size);
//}
//
//void operator delete (void* mem, Darius::Core::Memory::Alignment alignment)
//{
//	AlignedFree(mem);
//}
//
//void operator delete[](void* mem, Darius::Core::Memory::Alignment alignment)
//{
//	AlignedFree(mem);
//}
