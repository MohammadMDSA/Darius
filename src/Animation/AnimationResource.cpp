#include "pch.hpp"
#include "AnimationResource.hpp"

#define FBXSDK_SHARED

#include <fbxsdk.h>
#include <fbxsdk/fileio/fbxiosettings.h>

using namespace fbxsdk;

namespace Darius::Animation
{

	D_CH_RESOURCE_DEF(AnimationResource);

	DVector<ResourceDataInFile> AnimationResource::CanConstructFrom(ResourceType type, Path const& path)
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

		DVector<ResourceDataInFile> result;

		for (int i = 0; i < lScene->GetSrcObjectCount<FbxAnimStack>(); i++)
		{
			ResourceDataInFile data;
			FbxAnimStack* animStack = lScene->GetSrcObject<FbxAnimStack>(i);

			data.Name = animStack->GetName();
			data.Type = AnimationResource::GetResourceType();
			result.push_back(data);
		}

		return result;
	}

	bool AnimationResource::GetPropertyData(int jointIndex, void* propP, void* currentLayerP, AnimationCurve& animCurve, const char* channelName)
	{
		FbxPropertyT<FbxDouble3>& prop = *(FbxPropertyT<FbxDouble3>*)propP;
		FbxAnimLayer* currentLayer = (FbxAnimLayer*)currentLayerP;

		animCurve.TargetNode = jointIndex;
		// TODO: Extract right interpolation
		animCurve.Interpolation = AnimationCurve::kLinear;
		animCurve.KeyFrameOffset = mKeyframes.size();
		animCurve.KeyFrameFormat = AnimationCurve::kFloat;
		animCurve.KeyFrameStride = sizeof(float) / 4;


		auto curve = prop.GetCurve(currentLayer, channelName);
		// No curve for this property
		if (!curve)
			return false;

		for (int keyIndex = 0; keyIndex < curve->KeyGetCount(); keyIndex++)
		{
			auto curveKey = curve->KeyGet(keyIndex);
			auto keyTime = curveKey.GetTime().GetSecondDouble();

			if (keyIndex == 0)
			{
				animCurve.StartTime = keyTime;
			}

			animCurve.KeyframeTimeMap.insert({ keyTime, keyIndex });

			// Increasing keyframe vector size
			for (int i = 0; i < 4; i++) mKeyframes.push_back(0);

			auto keyDataLoc = mKeyframes.data() + animCurve.KeyFrameOffset + keyIndex * sizeof(float);
			*(float*)keyDataLoc = curveKey.GetValue();
		}

		return true;
	}

	void AnimationResource::ReadResourceFromFile()
	{
		ZeroMemory(&mAnimationData, sizeof(AnimationLayer));
		mCurvesData.clear();
		mKeyframes.clear();

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
			if (GetName() == WSTR_STR(stackName))
			{
				targetAnimStack = animStack;
				break;
			}
		}

		if (!targetAnimStack)
		{
			auto name = GetName();
			D_LOG_WARN("Animation " << STR_WSTR(name) << " from path " << GetPath().string() << " could not be read");
			return;
		}
		// Getting the layer that we want to work with
		auto currentLayer = targetAnimStack->GetMember<FbxAnimLayer>();

		// Get take info (duration)
		auto takeInfo = lScene->GetTakeInfo(targetAnimStack->GetName());
		mAnimationData.Duration = (float)(takeInfo->mLocalTimeSpan.GetStop() - takeInfo->mLocalTimeSpan.GetStart()).GetSecondDouble();

		// Get frame rate from the scene.
		FbxTime::EMode mode = lScene->GetGlobalSettings().GetTimeMode();
		const float scene_frame_rate =
			static_cast<float>((mode == FbxTime::eCustom)
				? lScene->GetGlobalSettings().GetCustomFrameRate()
				: FbxTime::GetFrameRate(mode));

		// Creating a mapping from joint name to it's index (animations will use this index)
		D_CONTAINERS::DUnorderedMap<FbxSkeleton*, int> skeletonMap;
		mSkeletonNameIndexMap.clear();
		{
			auto skeltCnt = lScene->GetMemberCount<FbxSkeleton>();
			int index = 0;
			for (int skeletonIndex = 0; skeletonIndex < skeltCnt; skeletonIndex++)
			{
				auto skeleton = lScene->GetMember<FbxSkeleton>(skeletonIndex);
				auto node = skeleton->GetNode();
				if (!node)
					continue;

				skeletonMap[skeleton] = index;
				mSkeletonNameIndexMap[node->GetName()] = index;

				index++;
			}
		}

		// Adding translation, scale, and rotation curves of joints
		for (auto [skeleton, jointIndex] : skeletonMap)
		{
			auto node = skeleton->GetNode();

			// Translation
			{
				auto curveNode = node->LclTranslation.GetCurveNode(currentLayer);
				if (curveNode)
					for (int channelIdx = 0; channelIdx < curveNode->GetChannelsCount(); channelIdx++)
					{
						AnimationCurve animCurve;
						animCurve.TargetPath = AnimationCurve::kTranslation;
						if (GetPropertyData(jointIndex, &node->LclTranslation, currentLayer, animCurve, curveNode->GetChannelName(channelIdx)))
						{
							animCurve.ChannelIndex = channelIdx;
							mCurvesData.push_back(animCurve);
						}
					}
			}

			// Scale
			{
				auto curveNode = node->LclScaling.GetCurveNode(currentLayer);
				if (curveNode)
				for (int channelIdx = 0; channelIdx < curveNode->GetChannelsCount(); channelIdx++)
				{
					AnimationCurve animCurve;
					animCurve.TargetPath = AnimationCurve::kScale;
					if (GetPropertyData(jointIndex, &node->LclScaling, currentLayer, animCurve, curveNode->GetChannelName(channelIdx)))
					{
						animCurve.ChannelIndex = channelIdx;
						mCurvesData.push_back(animCurve);
					}
				}
			}

			// Rotation
			{
				auto curveNode = node->LclRotation.GetCurveNode(currentLayer);
				if (curveNode)
				for (int channelIdx = 0; channelIdx < curveNode->GetChannelsCount(); channelIdx++)
				{
					AnimationCurve animCurve;
					animCurve.TargetPath = AnimationCurve::kRotation;
					if (GetPropertyData(jointIndex, &node->LclRotation, currentLayer, animCurve, curveNode->GetChannelName(channelIdx)))
					{
						animCurve.ChannelIndex = channelIdx;
						mCurvesData.push_back(animCurve);
					}
				}

			}

		}

	}
}
