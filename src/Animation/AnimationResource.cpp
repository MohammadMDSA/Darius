#include "pch.hpp"
#include "AnimationResource.hpp"

#include <Core/Serialization/TypeSerializer.hpp>

#define FBXSDK_SHARED

#include <fbxsdk.h>
#include <fbxsdk/fileio/fbxiosettings.h>

#include "AnimationResource.sgenerated.hpp"

using namespace D_CONTAINERS;
using namespace D_FILE;
using namespace D_RESOURCE;

namespace Darius::Animation
{

	D_CH_RESOURCE_DEF(AnimationResource);

	DVector<ResourceDataInFile> AnimationResource::CanConstructFrom(ResourceType type, Path const& path)
	{
		DVector<ResourceDataInFile> result;
		auto ext = path.extension();
		if (ext == ".anim")
		{
			ResourceDataInFile res;
			res.Name = WSTR2STR(D_FILE::GetFileName(path));
			res.Type = AnimationResource::GetResourceType();
		}

		if (ext == ".fbx")
		{
			// Create the FBX SDK manager
			FbxManager* lSdkManager = FbxManager::Create();

			// Create an IOSettings object.
			FbxIOSettings* ios = FbxIOSettings::Create(lSdkManager, IOSROOT);
			lSdkManager->SetIOSettings(ios);

			// Create an importer.
			FbxImporter* lImporter = FbxImporter::Create(lSdkManager, "");

			// Declare the path and filename of the file containing the scene.
			// In this case, we are assuming the file is in the same directory as the executable.
			auto pathStr = path.string();
			const char* lFilename = pathStr.c_str();


			// Initialize the importer.
			bool lImportStatus = lImporter->Initialize(lFilename, -1, lSdkManager->GetIOSettings());

			if (!lImportStatus) {
				D_LOG_ERROR("Call to FbxImporter::Initialize() failed.");
				std::string msg = "Error returned: " + std::string(lImporter->GetStatus().GetErrorString());
				D_LOG_ERROR(msg);
				return {};
			}

			// Create a new scene so it can be populated by the imported file.
			FbxScene* lScene = FbxScene::Create(lSdkManager, "modelScene");

			// Import the contents of the file into the scene.
			lImporter->Import(lScene);

			// The file has been imported; we can get rid of the importer.
			lImporter->Destroy();

			for (int i = 0; i < lScene->GetSrcObjectCount<FbxAnimStack>(); i++)
			{
				ResourceDataInFile data;
				FbxAnimStack* animStack = lScene->GetSrcObject<FbxAnimStack>(i);

				data.Name = animStack->GetName();
				data.Type = AnimationResource::GetResourceType();
				result.push_back(data);
			}

			lSdkManager->Destroy();

			return result;
		}

		return result;
	}

	bool AnimationResource::GetPropertyData(void* propP, void* currentLayerP, Track& animCurve, const char* channelName)
	{
		FbxPropertyT<FbxDouble3>& prop = *(FbxPropertyT<FbxDouble3>*)propP;
		FbxAnimLayer* currentLayer = (FbxAnimLayer*)currentLayerP;

		animCurve.SetInterpolationMode(InterpolationMode::Linear);

		auto curve = prop.GetCurve(currentLayer, channelName);
		// No curve for this property
		if (!curve)
			return false;

		int componentOffset = -1;
		if (!std::strcmp(channelName, "X"))
			componentOffset = 0;
		else if (!std::strcmp(channelName, "Y"))
			componentOffset = 1;
		else if (!std::strcmp(channelName, "Z"))
			componentOffset = 2;
		else if (!std::strcmp(channelName, "T"))
			componentOffset = 3;

		// It is not x, y, z, w, so in this context it is invalid
		if (componentOffset == -1)
			return false;

		for (UINT keyIndex = 0; keyIndex < (UINT)curve->KeyGetCount(); keyIndex++)
		{
			auto curveKey = curve->KeyGet(keyIndex);
			auto keyTime = (float)curveKey.GetTime().GetSecondDouble();

			Keyframe* keyframe = animCurve.FindOrCreateKeyframeByTime(keyTime);
			keyframe->Time = keyTime;

			switch (componentOffset)
			{
			case 0:
				keyframe->Value.SetX(curveKey.GetValue());
				break;
			case 1:
				keyframe->Value.SetY(curveKey.GetValue());
				break;
			case 2:
				keyframe->Value.SetZ(curveKey.GetValue());
				break;
			case 3:
				keyframe->Value.SetW(curveKey.GetValue());
				break;

			default:
				break;
			}
		}

		return true;
	}

	void AnimationResource::ReadResourceFromFile(D_SERIALIZATION::Json const& json)
	{
		auto ext = GetPath().extension();
		if (ext == ".anim")
		{
			mSkeletalAnimation = false;
			ReadNativeAnimationFromFile(json);
			return;
		}
		if (ext == ".fbx")
		{
			mSkeletalAnimation = true;
			ReadFbxAnimationFromFile(json);
			return;
		}
	}

	void AnimationResource::Unload()
	{
		EvictFromGpu();
		mSkeletonNameIndexMap.clear();
		mSkeletalAnimationSequence = Sequence();
	}

	void AnimationResource::ReadNativeAnimationFromFile(D_SERIALIZATION::Json const& json)
	{
		D_SERIALIZATION::Json animData;
		if (!D_FILE::ReadJsonFile(GetPath(), animData))
		{
			D_LOG_ERROR("Could not read animation data from " + GetPath().string());
			return;
		}

		D_SERIALIZATION::Deserialize(*this, animData);
	}

	void AnimationResource::WriteResourceToFile(D_SERIALIZATION::Json& json) const
	{
		if (IsSkeletalAnimation())
			return;

		D_SERIALIZATION::Json animData;
		D_SERIALIZATION::Serialize(*this, animData);

		if (D_FILE::WriteJsonFile(GetPath(), animData))
		{
			D_LOG_ERROR("Unable to write animation data to " + GetPath().string());
			return;
		}
	}

	void AnimationResource::ReadFbxAnimationFromFile(D_SERIALIZATION::Json const& json)
	{

		// Create the FBX SDK manager
		FbxManager* lSdkManager = FbxManager::Create();

		// Create an IOSettings object.
		FbxIOSettings* ios = FbxIOSettings::Create(lSdkManager, IOSROOT);
		lSdkManager->SetIOSettings(ios);

		// Create an importer.
		FbxImporter* lImporter = FbxImporter::Create(lSdkManager, "");

		// Declare the path and filename of the file containing the scene.
		// In this case, we are assuming the file is in the same directory as the executable.
		auto pathStr = GetPath().string();
		const char* lFilename = pathStr.c_str();


		// Initialize the importer.
		bool lImportStatus = lImporter->Initialize(lFilename, -1, lSdkManager->GetIOSettings());

		if (!lImportStatus) {
			D_LOG_ERROR("Call to FbxImporter::Initialize() failed.");
			std::string msg = "Error returned: " + std::string(lImporter->GetStatus().GetErrorString());
			D_LOG_ERROR(msg);
			lSdkManager->Destroy();
			return;
		}

		// Create a new scene so it can be populated by the imported file.
		FbxScene* lScene = FbxScene::Create(lSdkManager, "modelScene");

		// Import the contents of the file into the scene.
		lImporter->Import(lScene);

		// The file has been imported; we can get rid of the importer.
		lImporter->Destroy();

		FbxAnimStack* targetAnimStack = 0;

		for (int i = 0; i < lScene->GetSrcObjectCount<FbxAnimStack>(); i++)
		{
			FbxAnimStack* animStack = lScene->GetSrcObject<FbxAnimStack>(i);
			std::string stackName = animStack->GetName();
			if (GetName() == STR2WSTR(stackName))
			{
				targetAnimStack = animStack;
				break;
			}
		}

		if (!targetAnimStack)
		{
			auto name = GetName();
			D_LOG_WARN("Animation " << WSTR2STR(name) << " from path " << GetPath().string() << " could not be read");
			lSdkManager->Destroy();
			return;
		}
		// Getting the layer that we want to work with
		auto currentLayer = targetAnimStack->GetMember<FbxAnimLayer>();

		// Get frame rate from the scene.
		FbxTime::EMode mode = lScene->GetGlobalSettings().GetTimeMode();
		const float scene_frame_rate =
			static_cast<float>((mode == FbxTime::eCustom)
				? lScene->GetGlobalSettings().GetCustomFrameRate()
				: FbxTime::GetFrameRate(mode));

		// Creating a mapping from joint name to it's index (animations will use this index)
		D_CONTAINERS::DVector<FbxSkeleton*> jointVec;
		mSkeletonNameIndexMap.clear();
		{
			auto skeltCnt = lScene->GetMemberCount<FbxSkeleton>();
			jointVec.resize(skeltCnt);
			int index = 0;
			for (int skeletonIndex = 0; skeletonIndex < skeltCnt; skeletonIndex++)
			{
				auto skeleton = lScene->GetMember<FbxSkeleton>(skeletonIndex);
				auto node = skeleton->GetNode();
				if (!node)
					continue;

				jointVec[index] = skeleton;
				mSkeletonNameIndexMap[node->GetName()] = index;

				index++;
			}
		}

		// Adding translation, scale, and rotation curves of joints
		for (int i = 0; i < jointVec.size(); i++)
		{
			FbxSkeleton* skeleton = jointVec[i];
			auto node = skeleton->GetNode();
			std::string nodeName = node->GetName();
			// Translation
			{
				Track animCurve;
				auto curveNode = node->LclTranslation.GetCurveNode(currentLayer);
				if (curveNode)
				{
					for (UINT channelIdx = 0; channelIdx < curveNode->GetChannelsCount(); channelIdx++)
					{
						GetPropertyData(&node->LclTranslation, currentLayer, animCurve, curveNode->GetChannelName(channelIdx));
					}

					for (auto& kf : animCurve.GetKeyframes())
						kf.Value.SetW(1.f);

				}
				mSkeletalAnimationSequence.AddTrack(nodeName + ".Translation", animCurve);
			}

			// Scale
			{
				Track animCurve;
				auto curveNode = node->LclScaling.GetCurveNode(currentLayer);
				if (curveNode)
				{
					for (UINT channelIdx = 0; channelIdx < curveNode->GetChannelsCount(); channelIdx++)
					{
						GetPropertyData(&node->LclScaling, currentLayer, animCurve, curveNode->GetChannelName(channelIdx));
					}

				}
				mSkeletalAnimationSequence.AddTrack(nodeName + ".Scale", animCurve);
			}

			// Rotation
			{
				Track animCurve;
				auto curveNode = node->LclRotation.GetCurveNode(currentLayer);
				if (curveNode)
				{
					for (UINT channelIdx = 0; channelIdx < curveNode->GetChannelsCount(); channelIdx++)
					{
						GetPropertyData(&node->LclRotation, currentLayer, animCurve, curveNode->GetChannelName(channelIdx));
					}
				}

				mSkeletalAnimationSequence.AddTrack(nodeName + ".Rotation", animCurve);

			}

		}

		lSdkManager->Destroy();
	}

	float AnimationResource::GetDuration() const
	{
		if (IsSkeletalAnimation())
			return mSkeletalAnimationSequence.GetDuration();
		
		if (mComponentAnimation.size() <= 0)
			return 0.f;

		auto earliestAnim = *std::min_element(mComponentAnimation.begin(), mComponentAnimation.end(), [](auto el) { return el.AnimationSequence.GetStartTime(); });

		auto latestAnim = *std::max_element(mComponentAnimation.begin(), mComponentAnimation.end(), [](auto el) { return el.AnimationSequence.GetEndTime(); });

		return latestAnim.AnimationSequence.GetEndTime() - earliestAnim.AnimationSequence.GetStartTime();
	}
}
