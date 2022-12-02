#include "Physics/pch.hpp"
#include "PhysicsMaterialResource.hpp"

#include <Utils/Common.hpp>

#include <imgui.h>

using namespace D_SERIALIZATION;

namespace Darius::Physics
{

	D_CH_RESOURCE_DEF(PhysicsMaterialResource);

	PhysicsMaterialResource::PhysicsMaterialResource(D_CORE::Uuid uuid, std::wstring const& path, std::wstring const& name, DResourceId id, bool isDefault) :
		Resource(uuid, path, name, id, isDefault)
	{
		auto core = D_PHYSICS::GetCore();
		mPxData = core->createMaterial(0.5f, 0.5f, 0.5f);
	}

	void PhysicsMaterialResource::WriteResourceToFile() const
	{
		if (!mPxData)
			return;

		Json j = {
			{ "StaticFriction", mPxData->getStaticFriction() },
			{ "DynamicFriction", mPxData->getDynamicFriction() },
			{ "Restitution", mPxData->getRestitution() }
		};

		std::ofstream os(GetPath());
		os << j;
		os.close();
	}

	void PhysicsMaterialResource::ReadResourceFromFile()
	{
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

	bool PhysicsMaterialResource::UploadToGpu(D_GRAPHICS::GraphicsContext& context)
	{
		return true;
	}

	void PhysicsMaterialResource::Unload()
	{
		EvictFromGpu();
		mPxData->release();
		mPxData = nullptr;
	}

	bool PhysicsMaterialResource::DrawDetails(float params[])
	{
		bool valueChanged = false;

		D_H_DETAILS_DRAW_BEGIN_TABLE();

		// Static Friction
		{
			D_H_DETAILS_DRAW_PROPERTY("Static Friction");
			float staticFriction = mPxData->getStaticFriction();
			if (ImGui::DragFloat("##staticFriction", &staticFriction, 0.01f, 0.f, 1.f, "%.3f"))
			{
				mPxData->setStaticFriction(staticFriction);
				valueChanged = true;
			}
		}

		// Dynamic Friction
		{
			D_H_DETAILS_DRAW_PROPERTY("Dynamic Friction");
			float dynamicFriction = mPxData->getDynamicFriction();
			if (ImGui::DragFloat("##dynamicFriction", &dynamicFriction, 0.01f, 0.f, 1.f, "%.3f"))
			{
				mPxData->setDynamicFriction(dynamicFriction);
				valueChanged = true;
			}
		}

		// Restitution
		{
			D_H_DETAILS_DRAW_PROPERTY("Restitution");
			float restitution = mPxData->getRestitution();
			if (ImGui::DragFloat("##restitution", &restitution, 0.01f, 0.f, 1.f, "%.3f"))
			{
				mPxData->setRestitution(restitution);
				valueChanged = true;
			}
		}

		D_H_DETAILS_DRAW_END_TABLE();

		if (valueChanged)
			MakeDiskDirty();

		return false;
	}
}
