#include "Physics/pch.hpp"
#include "PhysicsMaterialResource.hpp"

#include <Scene/EntityComponentSystem/Components/TransformComponent.hpp>
#include <Utils/Common.hpp>

#ifdef _D_EDITOR
#include <imgui.h>
#endif

#include "PhysicsMaterialResource.sgenerated.hpp"

using namespace D_SERIALIZATION;

namespace Darius::Physics
{

	D_CH_RESOURCE_DEF(PhysicsMaterialResource);

	PhysicsMaterialResource::PhysicsMaterialResource(D_CORE::Uuid const& uuid, std::wstring const& path, std::wstring const& name, D_RESOURCE::DResourceId id, D_RESOURCE::Resource* parent, bool isDefault) :
		Resource(uuid, path, name, id, parent, isDefault)
	{
		auto core = D_PHYSICS::GetCore();
		mPxData = core->createMaterial(0.5f, 0.5f, 0.5f);
	}

	bool PhysicsMaterialResource::WriteResourceToFile(D_SERIALIZATION::Json& json) const
	{
		if (!mPxData)
		{
			D_LOG_ERROR("Physics material data is not present to write to file: " << GetPath().string());
			return false;
		}

		Json j = {
			{ "StaticFriction", mPxData->getStaticFriction() },
			{ "DynamicFriction", mPxData->getDynamicFriction() },
			{ "Restitution", mPxData->getRestitution() }
		};

		std::ofstream os(GetPath());
		os << j;
		os.close();

		return true;
	}

	void PhysicsMaterialResource::ReadResourceFromFile(D_SERIALIZATION::Json const& json, bool& dirtyDisk)
	{
		dirtyDisk = false;

		Json j;
		std::ifstream is(GetPath());
		is >> j;
		is.close();

		float staticFriction = 0.f;
		float dynamicFriction = 0.f;
		float restitution = 0.f;

		if (j.contains("StaticFriction"))
			staticFriction = j["StaticFriction"];

		if (j.contains("DynamicFriction"))
			staticFriction = j["DynamicFriction"];

		if (j.contains("Restitution"))
			staticFriction = j["Restitution"];
		
		if (mPxData)
			mPxData->release();
		mPxData = D_PHYSICS::GetCore()->createMaterial(staticFriction, dynamicFriction, restitution);
	}

	void PhysicsMaterialResource::Unload()
	{
		EvictFromGpu();
		mPxData->release();
		mPxData = nullptr;
	}

	void PhysicsMaterialResource::SetStaticFriction(float value)
	{
		value = D_MATH::Clamp(value, 0.f, 1.f);

		if (value == GetStaticFriction())
			return;

		mPxData->setStaticFriction(value);

		MakeDiskDirty();

		SignalChange();
	}

	void PhysicsMaterialResource::SetDynamicFriction(float value)
	{
		value = D_MATH::Clamp(value, 0.f, 1.f);

		if (value == GetDynamicFriction())
			return;

		mPxData->setDynamicFriction(value);

		MakeDiskDirty();

		SignalChange();
	}

	void PhysicsMaterialResource::SetRestitution(float value)
	{
		value = D_MATH::Clamp(value, 0.f, 1.f);

		if (value == GetRestitution())
			return;

		mPxData->setRestitution(value);

		MakeDiskDirty();

		SignalChange();
	}

#ifdef _D_EDITOR
	bool PhysicsMaterialResource::DrawDetails(float params[])
	{
		D_H_DETAILS_DRAW_BEGIN_TABLE();

		// Static Friction
		{
			D_H_DETAILS_DRAW_PROPERTY("Static Friction");
			float staticFriction = GetStaticFriction();
			if (ImGui::DragFloat("##staticFriction", &staticFriction, 0.01f, 0.f, 1.f, "%.3f"))
			{
				SetStaticFriction(staticFriction);
			}
		}

		// Dynamic Friction
		{
			D_H_DETAILS_DRAW_PROPERTY("Dynamic Friction");
			float dynamicFriction = GetDynamicFriction();
			if (ImGui::DragFloat("##dynamicFriction", &dynamicFriction, 0.01f, 0.f, 1.f, "%.3f"))
			{
				SetDynamicFriction(dynamicFriction);
			}
		}

		// Restitution
		{
			D_H_DETAILS_DRAW_PROPERTY("Restitution");
			float restitution = GetRestitution();
			if (ImGui::DragFloat("##restitution", &restitution, 0.01f, 0.f, 1.f, "%.3f"))
			{
				SetRestitution(restitution);
			}
		}

		D_H_DETAILS_DRAW_END_TABLE();

		return false;
	}
#endif
}
