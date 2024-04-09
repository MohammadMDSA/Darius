#include "Memory.hpp"

#include <Utils/Assert.hpp>

using namespace D_CORE_THREADING;

namespace Darius::Core::Memory
{
    // Godot memory

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
    // A faster version of memcopy that uses SSE instructions.  TODO:  Write an ARM variant if necessary.

#if _DEBUG
    SafeNumeric<uint64_t> MemoryManager::MemUsage;
    SafeNumeric<uint64_t> MemoryManager::MaxUsage;
#endif // _DEBUG

    SafeNumeric<uint64_t> MemoryManager::AllocCount;


    void* MemoryManager::AllocStatic(size_t bytes, bool padAlign) {
#ifdef DEBUG_ENABLED
        bool prepad = true;
#else
        bool prepad = padAlign;
#endif

        void* mem = malloc(bytes + (prepad ? D_PAD_ALIGN : 0));

        if (mem == nullptr)
            return nullptr; 

        AllocCount.Increment();

        if (prepad) {
            uint64_t* s = (uint64_t*)mem;
            *s = bytes;

            uint8_t* s8 = (uint8_t*)mem;

#ifdef DEBUG_ENABLED
            uint64_t new_mem_usage = mem_usage.add(p_bytes);
            max_usage.exchange_if_greater(new_mem_usage);
#endif
            return s8 + D_PAD_ALIGN;
        }
        else {
            return mem;
        }
    }

    void* MemoryManager::ReallocStatic(void* memory, size_t bytes, bool padAlign) {
        if (memory == nullptr) {
            return AllocStatic(bytes, padAlign);
        }

        uint8_t* mem = (uint8_t*)memory;

#ifdef DEBUG_ENABLED
        bool prepad = true;
#else
        bool prepad = padAlign;
#endif

        if (prepad) {
            mem -= D_PAD_ALIGN;
            uint64_t* s = (uint64_t*)mem;

#ifdef DEBUG_ENABLED
            if (p_bytes > *s) {
                uint64_t new_mem_usage = mem_usage.add(p_bytes - *s);
                max_usage.exchange_if_greater(new_mem_usage);
            }
            else {
                mem_usage.sub(*s - p_bytes);
            }
#endif

            if (bytes == 0) {
                free(mem);
                return nullptr;
            }
            else {
                *s = bytes;

                mem = (uint8_t*)realloc(mem, bytes + D_PAD_ALIGN);
                if(mem == nullptr)
                    return nullptr;

                s = (uint64_t*)mem;

                *s = bytes;

                return mem + D_PAD_ALIGN;
            }
        }
        else {
            mem = (uint8_t*)realloc(mem, bytes);

            if(mem == nullptr && bytes > 0)
                return nullptr;

            return mem;
        }
    }

    void MemoryManager::FreeStatic(void* ptr, bool padAlign) {
        
        if (ptr == nullptr)
            return;

        uint8_t* mem = (uint8_t*)ptr;

#ifdef DEBUG_ENABLED
        bool prepad = true;
#else
        bool prepad = padAlign;
#endif

        AllocCount.Decrement();

        if (prepad) {
            mem -= D_PAD_ALIGN;

#ifdef DEBUG_ENABLED
            uint64_t* s = (uint64_t*)mem;
            mem_usage.sub(*s);
#endif

            free(mem);
        }
        else {
            free(mem);
        }
    }

    uint64_t MemoryManager::GetMemAvailable() {
        return -1; // 0xFFFF...
    }

    uint64_t MemoryManager::GetMemUsage() {
#ifdef DEBUG_ENABLED
        return mem_usage.get();
#else
        return 0;
#endif
    }

    uint64_t MemoryManager::GetMemMaxUsage() {
#ifdef DEBUG_ENABLED
        return max_usage.get();
#else
        return 0;
#endif
    }


    void SIMDMemCopy(void* __restrict _Dest, const void* __restrict _Source, size_t NumQuadwords)
    {
        D_ASSERT(IsAligned(_Dest, 16));
        D_ASSERT(IsAligned(_Source, 16));

        __m128i* __restrict Dest = (__m128i * __restrict)_Dest;
        const __m128i* __restrict Source = (const __m128i * __restrict)_Source;

        // Discover how many quadwords precede a cache line boundary.  Copy them separately.
        size_t InitialQuadwordCount = (4 - ((size_t)Source >> 4) & 3) & 3;
        if (InitialQuadwordCount > NumQuadwords)
            InitialQuadwordCount = NumQuadwords;

        switch (InitialQuadwordCount)
        {
        case 3: _mm_stream_si128(Dest + 2, _mm_load_si128(Source + 2));	 // Fall through
        case 2: _mm_stream_si128(Dest + 1, _mm_load_si128(Source + 1));	 // Fall through
        case 1: _mm_stream_si128(Dest + 0, _mm_load_si128(Source + 0));	 // Fall through
        default:
            break;
        }

        if (NumQuadwords == InitialQuadwordCount)
            return;

        Dest += InitialQuadwordCount;
        Source += InitialQuadwordCount;
        NumQuadwords -= InitialQuadwordCount;

        size_t CacheLines = NumQuadwords >> 2;

        switch (CacheLines)
        {
        default:
        case 10: _mm_prefetch((char*)(Source + 36), _MM_HINT_NTA);	// Fall through
        case 9:  _mm_prefetch((char*)(Source + 32), _MM_HINT_NTA);	// Fall through
        case 8:  _mm_prefetch((char*)(Source + 28), _MM_HINT_NTA);	// Fall through
        case 7:  _mm_prefetch((char*)(Source + 24), _MM_HINT_NTA);	// Fall through
        case 6:  _mm_prefetch((char*)(Source + 20), _MM_HINT_NTA);	// Fall through
        case 5:  _mm_prefetch((char*)(Source + 16), _MM_HINT_NTA);	// Fall through
        case 4:  _mm_prefetch((char*)(Source + 12), _MM_HINT_NTA);	// Fall through
        case 3:  _mm_prefetch((char*)(Source + 8), _MM_HINT_NTA);	// Fall through
        case 2:  _mm_prefetch((char*)(Source + 4), _MM_HINT_NTA);	// Fall through
        case 1:  _mm_prefetch((char*)(Source + 0), _MM_HINT_NTA);	// Fall through

            // Do four quadwords per loop to minimize stalls.
            for (size_t i = CacheLines; i > 0; --i)
            {
                // If this is a large copy, start prefetching future cache lines.  This also prefetches the
                // trailing quadwords that are not part of a whole cache line.
                if (i >= 10)
                    _mm_prefetch((char*)(Source + 40), _MM_HINT_NTA);

                _mm_stream_si128(Dest + 0, _mm_load_si128(Source + 0));
                _mm_stream_si128(Dest + 1, _mm_load_si128(Source + 1));
                _mm_stream_si128(Dest + 2, _mm_load_si128(Source + 2));
                _mm_stream_si128(Dest + 3, _mm_load_si128(Source + 3));

                Dest += 4;
                Source += 4;
            }

        case 0:	// No whole cache lines to read
            break;
        }

        // Copy the remaining quadwords
        switch (NumQuadwords & 3)
        {
        case 3: _mm_stream_si128(Dest + 2, _mm_load_si128(Source + 2));	 // Fall through
        case 2: _mm_stream_si128(Dest + 1, _mm_load_si128(Source + 1));	 // Fall through
        case 1: _mm_stream_si128(Dest + 0, _mm_load_si128(Source + 0));	 // Fall through
        default:
            break;
        }

        _mm_sfence();
    }

    void SIMDMemFill(void* __restrict _Dest, __m128 FillVector, size_t NumQuadwords)
    {
        D_ASSERT(IsAligned(_Dest, 16));

        register const __m128i Source = _mm_castps_si128(FillVector);
        __m128i* __restrict Dest = (__m128i * __restrict)_Dest;

        switch (((size_t)Dest >> 4) & 3)
        {
        case 1: _mm_stream_si128(Dest++, Source); --NumQuadwords;	 // Fall through
        case 2: _mm_stream_si128(Dest++, Source); --NumQuadwords;	 // Fall through
        case 3: _mm_stream_si128(Dest++, Source); --NumQuadwords;	 // Fall through
        default:
            break;
        }

        size_t WholeCacheLines = NumQuadwords >> 2;

        // Do four quadwords per loop to minimize stalls.
        while (WholeCacheLines--)
        {
            _mm_stream_si128(Dest++, Source);
            _mm_stream_si128(Dest++, Source);
            _mm_stream_si128(Dest++, Source);
            _mm_stream_si128(Dest++, Source);
        }

        // Copy the remaining quadwords
        switch (NumQuadwords & 3)
        {
        case 3: _mm_stream_si128(Dest++, Source);	 // Fall through
        case 2: _mm_stream_si128(Dest++, Source);	 // Fall through
        case 1: _mm_stream_si128(Dest++, Source);	 // Fall through
        default:
            break;
        }

        _mm_sfence();
    }

}

void* operator new(size_t p_size, const char* p_description) {
    return D_MEMORY::MemoryManager::AllocStatic(p_size, false);
}

void* operator new(size_t p_size, void* (*p_allocfunc)(size_t p_size)) {
    return p_allocfunc(p_size);
}

void operator delete(void* p_mem, const char* p_description) {
    D_ASSERT_M(false, "Call to placement delete should not happen.");
}

void operator delete(void* p_mem, void* (*p_allocfunc)(size_t p_size)) {
    D_ASSERT_M(false, "Call to placement delete should not happen.");
}

void operator delete(void* p_mem, void* p_pointer, size_t check, const char* p_description) {
    D_ASSERT_M(false, "Call to placement delete should not happen.");
}

