#include "Graphics/pch.hpp"
#include "StateObject.hpp"

#include "Graphics/GraphicsDeviceManager.hpp"

#include <Core/Hash.hpp>

#include <mutex>

using Microsoft::WRL::ComPtr;

static std::map<size_t, ComPtr<ID3D12StateObject>> s_StateObjectHashMap;

namespace Darius::Graphics::Utils
{
	void StateObject::DestroyAll()
	{
		s_StateObjectHashMap.clear();
	}

	void StateObject::Finalize(std::wstring const& name)
	{

		if (mFinalized)
			return;

		auto desc = GetDesc();

		D_ASSERT(desc.Type == mType);

		// Creating hash
		size_t hashCode = D_CORE::HashState(&desc.Type);
		hashCode = D_CORE::HashState(desc.pSubobjects, desc.NumSubobjects, hashCode);

		ID3D12StateObject** SORef = nullptr;
		bool firstCompile = false;
		{
			static std::mutex s_HashMapMutex;
			auto iter = s_StateObjectHashMap.find(hashCode);

			// Reserve space so the next inquiry will find that someone got herer first.
			if (iter == s_StateObjectHashMap.end())
			{
				SORef = s_StateObjectHashMap[hashCode].GetAddressOf();
				firstCompile = false;
			}
			else
				SORef = iter->second.GetAddressOf();

			if (firstCompile)
			{
				D_HR_CHECK(D_GRAPHICS_DEVICE::GetDevice5()->CreateStateObject(&desc, IID_PPV_ARGS(&mStateObject)));

				mStateObject->SetName(name.c_str());

				s_StateObjectHashMap[hashCode].Attach(mStateObject);
				D_ASSERT(*SORef == mStateObject);
			}
			else
			{
				while (*SORef == nullptr)
					std::this_thread::yield();
				mStateObject = *SORef;
			}

			mFinalized = true;
		}
	}
}