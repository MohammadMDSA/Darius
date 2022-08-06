#pragma once


#include "Containers/Vector.hpp"
#include "Containers/Map.hpp"

#include <Utils/Common.hpp>

#ifndef D_CORE
#define D_CORE Darius::Core
#endif // !D_UTILS

using namespace D_CONTAINERS;

namespace Darius::Core
{
	template<class T>
	class Ref;

	struct CountedOwner
	{
		std::wstring				Name = L"";
		std::string					Type = "";
		void*						Ref = nullptr;
		UINT						Count = 0;

	};

	class Counted : NonCopyable
	{
	public:

		Counted() = default;
		~Counted() = default;

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
				mOwners[owner.Ref].Count++;
			}
			else
			{
				owner.Count = 0;
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
				if (mOwners[owner.Ref].Count == 0)
					return false;
				mOwners[owner.Ref].Count--;
				if (mOwners[owner.Ref].Count <= 0)
					mOwners.erase(owner.Ref);
				return true;
			}
			else
				return false;
		}

		DMap<void*, CountedOwner>				mOwners;
		CountedOwner							mAnonymous;
	};
}