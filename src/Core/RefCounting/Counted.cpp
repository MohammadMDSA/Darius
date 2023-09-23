#include "Core/pch.hpp"
#include "Counted.hpp"

namespace Darius::Core
{
	Counted::Counted()
	{
		mRefCount.Init();
		mRefCountInit.Init();
	}

	bool Counted::InitRef()
	{
		if (Reference())
		{
			if (!IsReferenced() && mRefCountInit.Unref())
			{
				Unreference(); // First referencing is already 1, so compensate for the ref above
			}

			return true;
		}

		else
			return false;
	}

	UINT Counted::GetReferenceCount() const
	{
		return mRefCount.Get();
	}

	bool Counted::Reference()
	{
		return mRefCount.Ref();
	}

	bool Counted::Unreference()
	{
		return mRefCount.Unref();
	}
}