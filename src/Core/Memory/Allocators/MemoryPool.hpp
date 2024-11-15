#pragma once

#include "Core/Containers/Map.hpp"

#include "Core/Memory/Memory.hpp"

#include <Utils/Log.hpp>
#include <Utils/Assert.hpp>

#ifndef D_MEMORY
#define D_MEMORY Darius::Core::Memory
#endif // !D_MEMORY_ALLOC

namespace Darius::Core::Memory
{
	class Allocator;

	class ObjectPool
	{
	public:
		void					Init(Allocator* allocator, uint32_t poolSize, uint32_t objectSize);
		void					Shutdown();

		uint32_t				ObtainObject(); // Returns index to the resource
		void					ReleaseObject(uint32_t index);
		void					FreeAllObjects();

		void*					AccessObject(uint32_t index);
		void const*				AccessObject(uint32_t index) const;

		INLINE bool				HasAvailableMemory() const { return mFreeIndicesHead < mPoolSize; }

		static constexpr uint32_t   kInvalidIndex = 0xffffffff;

	protected:

		std::byte*				mMemory = nullptr;
		uint32_t*				mFreeIndices = nullptr;
		Allocator*				mAllocator = nullptr;

		uint32_t				mFreeIndicesHead = 0u;
		uint32_t				mPoolSize = 16u;
		uint32_t				mObjectsSize = 4u;
		uint32_t				mUsedIndices = 0u;
	};

	template<typename T>
	class TypedObjectPool : private ObjectPool
	{
	public:
		void					Init(Allocator* allocator, uint32_t poolSize);
		void					Shutdown();

		template<typename... Args>
		T*						Alloc(Args const&&...args);
		void					Release(T* object);

		T*						Get(uint32_t index);
		T const*				Get(uint32_t index) const;

		uint32_t				GetIndex(T const* object) const;
		bool					TryGetIndex(T const* object, uint32_t& result) const;

		INLINE bool				HasAvailableMemory() const { return mFreeIndicesHead < mPoolSize; }

		static constexpr uint32_t   kInvalidIndex = 0xffffffff;

	private:

		D_CONTAINERS::DUnorderedMap<T const*, uint32_t>	mObjectIndexMapping;
	};

	template<typename T>
	inline void TypedObjectPool<T>::Init(Allocator* allocator, uint32_t poolSize)
	{
		ObjectPool::Init(allocator, poolSize, sizeof(T));
	}

	template<typename T>
	inline void TypedObjectPool<T>::Shutdown()
	{
		if (mFreeIndicesHead != 0)
		{
			D_LOG_ERROR("Object pool has unfreed resources");
		}

		mObjectIndexMapping.clear();

		ObjectPool::Shutdown();
	}

	template<typename T>
	template<typename... Args>
	inline T* TypedObjectPool<T>::Alloc(Args const&&...args)
	{
		auto objIndex = ObjectPool::ObtainObject();
		if (objIndex != kInvalidIndex)
		{
			T* obj = Get(objIndex);
			
			D_ASSERT(!mObjectIndexMapping.Contains(obj));
			mObjectIndexMapping[obj] = objIndex;
			
			DMemNew_Placement(obj, T(args...));
			return obj;
		}

		return nullptr;
	}

	template<typename T>
	inline void TypedObjectPool<T>::Release(T* object)
	{
		auto search = mObjectIndexMapping.find(object);

		D_ASSERT(search != mObjectIndexMapping.end());

		auto resIndex = search->second;
		object->~T();

		ObjectPool::ReleaseObject(resIndex);

		mObjectIndexMapping.erase(object);
	}

	template<typename T>
	inline T* TypedObjectPool<T>::Get(uint32_t index)
	{
		return reinterpret_cast<T*>(ObjectPool::AccessObject(index));
	}

	template<typename T>
	inline T const* TypedObjectPool<T>::Get(uint32_t index) const
	{
		return reinterpret_cast<T const*>(ObjectPool::AccessObject(index));
	}

	template<typename T>
	uint32_t TypedObjectPool<T>::GetIndex(T const* object) const
	{
		auto search = mObjectIndexMapping.find(object);
		if (search == mObjectIndexMapping.end())
			return kInvalidIndex;

		return search->second;
	}

	template<typename T>
	bool TypedObjectPool<T>::TryGetIndex(T const* object, uint32_t& result) const
	{
		auto search = mObjectIndexMapping.find(object);
		if (search == mObjectIndexMapping.end())
			return false;

		result = search->second;
		return true;
	}

}