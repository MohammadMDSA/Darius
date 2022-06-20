#pragma once
#include <boost/align/align.hpp>
#include <boost/align/is_aligned.hpp>
#include <boost/align/aligned_alloc.hpp>
#include <boost/align/align_down.hpp>
#include <boost/align/align_up.hpp>
#include <boost/align/aligned_allocator.hpp>
#include <new>

#define D_malloc(T) reinterpret_cast<T*>(Darius::Core::Memory::aligned_alloc(alignof(T), sizeof(T)))

#define D_malloc_par(T, ...) new (Darius::Core::Memory::Align(alignof(T))) T(__VA_ARGS__)

#define D_alloc_aligned(size, alignment) Darius::Core::Memory::aligned_alloc(alignment, size)

#define D_free(ptr) Darius::Core::Memory::aligned_free(ptr);

namespace Darius
{
	namespace Core
	{
		namespace Memory
		{
			template<typename T, std::size_t Alignment=64>
			using AlignedAllocator = boost::alignment::aligned_allocator<T, Alignment>;

			class Align
			{
			public:
				explicit Align(int value) : m_value(value) {}
				int GetValue() const
				{
					return m_value;
				}

			private:
				int m_value;
			};

			inline void* align(std::size_t alignment, std::size_t size, void*& ptr, std::size_t& space)
			{
				return boost::alignment::align(alignment, size, ptr, space);
			}

			template<class T>
			inline constexpr T align_up(T value, std::size_t alignment) noexcept
			{
				return boost::alignment::align_up(value, alignment);
			}

			template<class T>
			inline constexpr T align_down(T value, std::size_t alignment) noexcept
			{
				return boost::alignment::align_down(value, alignment);
			}

			inline void* aligned_alloc(std::size_t alignment, std::size_t size)
			{
				return boost::alignment::aligned_alloc(alignment, size);
			}

			inline void aligned_free(void* ptr)
			{
				boost::alignment::aligned_free(ptr);
			}

			inline bool is_aligned(const volatile void* ptr, std::size_t alignment) noexcept
			{
				return boost::alignment::is_aligned(ptr, alignment);
			}

			template<class T>
			inline constexpr bool is_aligned(T value, std::size_t alignment) noexcept
			{
				return boost::alignment::is_aligned(value, alignment);
			}
		}
	}
}

using namespace Darius::Core::Memory;

// Overridden 'normal' new/delete
void* operator new (size_t size)
{
	return malloc(size);
}

void* operator new[](size_t size)
{
	return malloc(size);
}

void operator delete(void* mem)
{
	free(mem);
}

void operator delete[](void* mem)
{
	free(mem);
}

template<typename T>
void* operator new(size_t size, T data)
{
	auto ptr = aligned_alloc(alignof(T), size);
	if (ptr != nullptr)
		*ptr = data;
	return ptr;
}

// Aligned versions of new/delete
void* operator new[](size_t size, Darius::Core::Memory::Align alignment)
{
	return aligned_alloc(alignment.GetValue(), size);
}
void* operator new (size_t size, Darius::Core::Memory::Align alignment)
{
	return aligned_alloc(alignment.GetValue(), size);
}

void operator delete (void* mem, Darius::Core::Memory::Align alignment)
{
	aligned_free(mem);
}

void operator delete[](void* mem, Darius::Core::Memory::Align alignment)
{
	aligned_free(mem);
}
