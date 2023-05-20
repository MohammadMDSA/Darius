#pragma once


#include "Containers/Vector.hpp"
#include "Containers/Map.hpp"

#include <Utils/Common.hpp>

#include <rttr/rttr_enable.h>
#include <rttr/type.h>

#ifndef D_CORE
#define D_CORE Darius::Core
#endif // !D_UTILS

namespace Darius::Core
{
	template<class T>
	class Ref;

	struct CountedOwner
	{
		std::wstring				Name = L"";
		rttr::type					Type = rttr::type::get<void>();
		void*						Ref = nullptr;
		UINT						Count = 0;
		std::function<void()>		ChangeCallback = nullptr;

	};

	class Counted : NonCopyable
	{
		RTTR_ENABLE()

	public:

		INLINE Counted() :
			mAnonymous({ L"", rttr::type::get<void>(), nullptr, 0, nullptr})
		{

		};

		~Counted() = default;

		D_CONTAINERS::DUnorderedMap<void*, CountedOwner> const& GetOwners() const
		{
			return mOwners;
		}

		INLINE UINT GetNamedOwnersCount() const
		{
			UINT result = 0;
			for (auto [_, ownerData] : mOwners)
			{
				result += ownerData.Count;
			}
			
			return result;
		}

	private:
		template<class T>
		friend class Ref;

		void AddOwner(CountedOwner owner)
		{
			if (owner.Ref == nullptr)
			{
				mAnonymous.Count++;
				return;
			}

			if (mOwners.contains(owner.Ref))
			{
				mOwners.at(owner.Ref).Count++;
			}
			else
			{
				owner.Count = 1;
				mOwners.insert({ owner.Ref, owner });
			}
		}

		bool RemoveOwner(CountedOwner owner)
		{
			if (owner.Ref == nullptr)
			{
				if (mAnonymous.Count > 0)
				{
					mAnonymous.Count++;
					return true;
				}
				else
					return false;
			}

			if (mOwners.contains(owner.Ref))
			{
				auto& ownerDat = mOwners.at(owner.Ref);
				if (ownerDat.Count == 0)
					return false;
				ownerDat.Count--;
				if (ownerDat.Count <= 0)
					mOwners.erase(owner.Ref);
				return true;
			}
			else
				return false;
		}

		D_CONTAINERS::DUnorderedMap<void*, CountedOwner>	mOwners;
		CountedOwner										mAnonymous;
	};
}