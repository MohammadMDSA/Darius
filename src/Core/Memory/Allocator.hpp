#ifndef D_MEMORY
#define D_MEMORY Darius::Core::Memory
#endif // !D_MEMORY

namespace Darius::Core::Memory
{
    template<typename T>
    class _Allocator {
    public:
        //    typedefs
        typedef T value_type;
        typedef value_type* pointer;
        typedef const value_type* const_pointer;
        typedef value_type& reference;
        typedef const value_type& const_reference;
        typedef std::size_t size_type;
        typedef std::ptrdiff_t difference_type;

    public:
        //    convert an allocator<T> to allocator<U>
        template<typename U>
        struct rebind {
            typedef _Allocator<U> other;
        };

    public:
        inline explicit _Allocator() {}
        inline ~_Allocator() {}
        inline explicit _Allocator(_Allocator const&) {}
        template<typename U>
        inline explicit _Allocator(_Allocator<U> const&) {}

        //    address
        inline pointer address(reference r) { return &r; }
        inline const_pointer address(const_reference r) { return &r; }

        //    memory allocation
        virtual inline pointer allocate(size_type cnt) = 0;

        virtual inline void deallocate(pointer p, size_type) = 0;

        //    size
        virtual inline size_type max_size() const = 0;

        //    construction/destruction
        virtual inline void construct(pointer p, const T& t) = 0;
        virtual inline void destroy(pointer p) = 0;
    };
}