#include "Editor/pch.hpp"
#include "SequencerWindow.hpp"

#include "Editor/EditorContext.hpp"

#include <Scene/Gameobject.hpp>
#include <Scene/EntityComponentSystem/Components/ComponentBase.hpp>
#include <Scene/EntityComponentSystem/Components/TransformComponent.hpp>

#include <imgui.h>
#include <ImSequencer.h>
#include <Libs/FontIcon/IconsFontAwesome6.h>

//using namespace D_ANIMATION;
using namespace D_CONTAINERS;
using namespace D_ECS_COMP;

namespace Darius::Editor::Gui::Windows
{
	SequencerWindow::SequencerWindow(D_SERIALIZATION::Json& config) :
		Window(config),
		mReferenceGameObject(nullptr),
		mSelectedEntry(-1),
		mSelectedSequence(-1),
		mFirstFrame(0),
		mLastFrame(0),
		mCurrentFrame(0)
	{
	}

	SequencerWindow::~SequencerWindow()
	{
		CleanUpGameObjectData();
	}

	void SequencerWindow::Update(float dt)
	{
		auto selected = D_EDITOR_CONTEXT::GetSelectedDetailed();

		if (selected == mReferenceGameObject)
			return;

		// Is game object?
		D_SCENE::GameObject* selectedGameObject = dynamic_cast<D_SCENE::GameObject*>(selected);
		if (selectedGameObject)
		{
			// Select game object
			SetReferenceGameObject(selectedGameObject);
		}
		else // Is animation resource
		{
			SetReferenceGameObject(selectedGameObject);
		}
	}

	void SequencerWindow::DrawNullObjectMessage() const
	{
		auto windowSize = ImGui::GetWindowSize();
		static const std::string errorMessage = "Select a game object with an Animation Component attached and animation resource selected";
		auto textWidth = ImGui::CalcTextSize(errorMessage.c_str()).x;

		ImGui::SetCursorPos({ (windowSize.x - textWidth) * 0.5f, windowSize.y / 2 });
		ImGui::Text(errorMessage.c_str());
	}

	void SequencerWindow::DrawAddComponentButton()
	{
		bool hasAnyToDraw = AnyUnusedComponentsLeft();

		if (!hasAnyToDraw)
			ImGui::BeginDisabled(true);

		// Showing the add button
		if (ImGui::Button(ICON_FA_PLUS))
		{
			ImGui::OpenPopup("AddComponent");
		}

		if (!hasAnyToDraw)
			ImGui::EndDisabled();

	}

	void SequencerWindow::DrawGUI()
	{
		if (mReferenceGameObject == nullptr)
		{
			DrawNullObjectMessage();
			return;
		}

		// Top header
		ImGui::PushItemWidth(130);
		DrawAddComponentButton();
		ImGui::SameLine();
		ImGui::InputInt("Frame ", &mCurrentFrame);
		ImGui::SameLine();
		ImGui::InputInt("Frame Max", &mLastFrame);
		ImGui::PopItemWidth();

		// Component add popup
		if (ImGui::BeginPopup("AddComponent"))
		{
			for (auto compToAdd : GetUnusedComponents())
			{
				if (ImGui::Selectable(compToAdd->GetDisplayName().c_str()))
				{
					AddComponentToEdit(compToAdd);
				}
			}

			ImGui::EndPopup();
		}

		if (mComponentSequencers.size() <= 0)
		{
			Components::AnimationSequence seq;
			static bool _true = true;
			ImSequencer::Sequencer(&seq, &mCurrentFrame, &_true, &mSelectedEntry, &mFirstFrame, ImSequencer::SEQUENCER_EDIT_STARTEND | ImSequencer::SEQUENCER_ADD | ImSequencer::SEQUENCER_COPYPASTE | ImSequencer::SEQUENCER_CHANGE_FRAME);
			// add a UI to edit that particular item
			if (mSelectedEntry != -1)
			{
				//const MySequence::MySequenceItem& item = mySequence.myItems[selectedEntry];
				//ImGui::Text("I am a %s, please edit me", SequencerItemTypeNames[item.mType]);
				//// switch (type) ....
			}
			return;
		}

		int firstSequenceOptions = ImSequencer::SEQUENCER_EDIT_STARTEND | ImSequencer::SEQUENCER_ADD | ImSequencer::SEQUENCER_DEL | ImSequencer::SEQUENCER_COPYPASTE | ImSequencer::SEQUENCER_CHANGE_FRAME;
		bool first = true;
		for (auto& [compName, seq] : mComponentSequencers)
		{
			ImSequencer::Sequencer(&seq, &mCurrentFrame, &seq.mExpanded, &mSelectedEntry, &mFirstFrame, first ? firstSequenceOptions : 0);
			// add a UI to edit that particular item
			if (mSelectedEntry != -1)
			{
				//const MySequence::MySequenceItem& item = mySequence.myItems[selectedEntry];
				//ImGui::Text("I am a %s, please edit me", SequencerItemTypeNames[item.mType]);
				//// switch (type) ....
			}

			first = false;
		}
	}

	void SequencerWindow::SetReferenceGameObject(GameObject* referenceGameObject)
	{

		if (mReferenceGameObject != referenceGameObject)
		{
			CleanUpGameObjectData();
		}

		mReferenceGameObject = referenceGameObject;

		if (mReferenceGameObject && mReferenceGameObject->IsValid())
		{
			AttachGameObjectObservers();

			mComponentSequencers.emplace(D_MATH::TransformComponent::ClassName(), Components::AnimationSequence());
			auto& seq = mComponentSequencers.at(D_MATH::TransformComponent::ClassName());
			ConfigureSequencerForComponent(seq, referenceGameObject->GetTransform());
		}

		UpdateComponents();
	}

	void SequencerWindow::ConfigureSequencerForComponent(Components::AnimationSequence& seq, ComponentBase* comp)
	{
		seq.SetReferenceComponent(comp);
	}

	void SequencerWindow::CleanUpGameObjectData()
	{
		RemoveGameObjectObservers();
		mComponentSequencers.clear();
	}

	void SequencerWindow::RefreshComponentSequencers()
	{
		// Making a list of used components
		DVector<std::string> usedComponentNames;
		usedComponentNames.reserve(mComponentSequencers.size());
		for (auto& [compName, _] : mComponentSequencers)
			usedComponentNames.push_back(compName);

		mComponentSequencers.clear();

		for (auto& compName : usedComponentNames)
		{
			auto comp = mReferenceGameObject->GetComponent(compName);

			// Component has been removed
			if (!comp)
				continue;

			mComponentSequencers.emplace(compName, Components::AnimationSequence());
			auto& addedSeq = mComponentSequencers.at(compName);
			ConfigureSequencerForComponent(addedSeq, comp);
		}
	}

	void SequencerWindow::UpdateComponents()
	{
		// Clearing current components and names
		mReferenceComponents.clear();
		mReferenceComponentNames.clear();

		// No valid reference go? then we are done
		if (!mReferenceGameObject || !mReferenceGameObject->IsValid())
			return;

		// Fetching components
		mReferenceComponents = mReferenceGameObject->GetComponents(true);
		mReferenceComponentNames.reserve(mReferenceComponents.size());

		// Adding component names
		for (auto const* component : mReferenceComponents)
			mReferenceComponentNames.push_back(component->GetDisplayName());
	}

	void SequencerWindow::AttachGameObjectObservers()
	{
		if (!mReferenceGameObject || !mReferenceGameObject->IsValid())
			return;

		mComponentChangeSignalConnection = mReferenceGameObject->OnComponentSetChange.connect([&](auto go)
			{
				UpdateComponents();
			});
	}

	void SequencerWindow::RemoveGameObjectObservers()
	{
		if (mComponentChangeSignalConnection.connected())
			mComponentChangeSignalConnection.disconnect();
	}

	D_CONTAINERS::DVector<ComponentBase*> SequencerWindow::GetUnusedComponents() const
	{
		D_CONTAINERS::DVector<ComponentBase*> result;

		for (auto comp : mReferenceComponents)
		{
			if (!mComponentSequencers.contains(comp->GetComponentName()))
				result.push_back(comp);
		}

		return result;
	}

	bool SequencerWindow::AnyUnusedComponentsLeft() const
	{
		return mComponentSequencers.size() < mReferenceComponents.size();
	}

	bool SequencerWindow::AddComponentToEdit(ComponentBase* comp)
	{
		auto componentName = comp->GetComponentName();

		if (mComponentSequencers.contains(componentName))
			return false;

		D_ASSERT(mReferenceGameObject->GetComponent(componentName) == comp);

		mComponentSequencers.emplace(componentName, Components::AnimationSequence());
		auto& addedSeq = mComponentSequencers.at(componentName);

		ConfigureSequencerForComponent(addedSeq, comp);

		return true;
	}
}