#pragma once
#include "pch.hpp"
#include "Counted.hpp"

#include <Core/Exceptions/Exception.hpp>
#include <Utils/Assert.hpp>
#include <Utils/Common.hpp>

#include <optional>

#ifndef D_CORE
#define D_CORE Darius::Core
#endif // !D_UTILS

namespace Darius::Core
{

	// This class is heavily inspired from Godot Engine
	template<class T>
	class Ref
	{
	public:

		INLINE bool operator==(const T* ptr) const {
			return mReference == ptr;
		}
		INLINE bool operator!=(const T* ptr) const {
			return mReference != ptr;
		}

		INLINE bool operator<(const Ref<T>& other) const {
			return mReference < other.mReference;
		}
		INLINE bool operator==(const Ref<T>& other) const {
			return mReference == other.mReference;
		}
		INLINE bool operator!=(const Ref<T>& other) const {
			return mReference != other.mReference;
		}

		INLINE T* operator*() const {
			return mReference;
		}

		INLINE T* operator->() const {
			return mReference;
		}

		INLINE T* Get() const {
			return mReference;
		}

		INLINE void operator=(Ref const& other)
		{
			Reference(other);
		}

		template<class OTHER>
		void operator=(Ref<OTHER> const& otherRef)
		{
			Counted* otherCounted = const_cast<Counted*>(static_cast<Counted const*>(otherRef.Get));

			if (!otherCounted)
			{
				Unref();
				return;
			}

			Ref r;
			r.mReference = dynamic_cast<T*>(otherCounted);
			Reference(r);
			r.mReference = nullptr;
		}

		template<class OTHER>
		void ReferencePointer(OTHER* ptr)
		{
			if (mReference == ptr)
				return;

			Unref();

			T* r = dynamic_cast<T*>(ptr);
			if (r)
				ReferencePtr(r);
		}

		Ref(Ref const& other)
		{
			Reference(other);
		}

		template<class OTHER>
		Ref(Ref<OTHER> const& other)
		{
			Counted* otherCounted = const_cast<Counted*>(static_cast<Counted const*>(other.Get()));

			if (!otherCounted)
			{
				Unref();
				return;
			}

			Ref r;
			r.mReference = dynamic_cast<T*>(otherCounted);
			Reference(r);
			r.mReference = nullptr;
		}

		Ref(T* ptr)
		{
			if (ptr)
				ReferencePtr(ptr);
		}

		INLINE virtual bool IsValid() const { return mReference != nullptr; }
		INLINE bool IsNull() const { return mReference == nullptr; }

		void Unref()
		{
			if (mReference && mReference->Unreference())
				mReference->Release();

			mReference = nullptr;
		}

		Ref() {}
		virtual ~Ref() { Unref(); }

	private:
		void Reference(Ref const& other)
		{
			if (other.mReference == mReference)
				return;

			Unref();

			mReference = other.mReference;
			if (mReference)
				mReference->Reference();
		}

		void ReferencePtr(T* ref)
		{
			D_ASSERT(ref != nullptr);

			if (ref->InitRef())
				mReference = ref;
		}

		T* mReference = nullptr;
	};
}
