#include <boost/align/align.hpp>
#include <boost/align/is_aligned.hpp>
#include <boost/align/aligned_alloc.hpp>
#include <boost/align/align_down.hpp>
#include <boost/align/align_up.hpp>

#define D_malloc(T) static_cast<T>(Darius::Core::Memory::aligned_alloc(alignof(T), sizeof(T)))
#define D_alloc_aligned(size, alignment) Darius::Core::Memory::aligned_alloc(alignment, size)

#define D_free(ptr) Darius::Core::Memory::aligned_free(ptr);

using namespace boost::alignment;

namespace Darius
{
	namespace Core
	{
		namespace Memory
		{
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

//// Overridden 'normal' new/delete
//void* operator new (size_t size);
//void* operator new[](size_t size);
//void operator delete(void* mem);
//void operator delete[](void* mem);
//
//// Aligned versions of new/delete
//void* operator new[](size_t size, Align alignment);
//void* operator new (size_t size, Align alignment);
//void operator delete (void* mem, Align alignment);
//void operator delete[](void* mem, Align alignment);