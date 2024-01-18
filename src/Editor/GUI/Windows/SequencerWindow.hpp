#pragma once

#include "Window.hpp"

#include "Editor/GUI/Components/AnimationSequencerInterfaceComponent.hpp"

namespace Darius::Scene
{
    class GameObject;
}

namespace Darius::Animation
{
    class AnimationResource;
    class AnimationComponent;
}

namespace Darius::Editor::Gui::Windows
{
    
	class SequencerWindow : public Window
	{
		D_CH_EDITOR_WINDOW_BODY_RAW(SequencerWindow, "Sequencer");

        using GameObject = Darius::Scene::GameObject;
        using ComponentBase = Darius::Scene::ECS::Components::ComponentBase;
        using AnimationResource = Darius::Animation::AnimationResource;
        using AnimationComponent = Darius::Animation::AnimationComponent;

	public:
		SequencerWindow(D_SERIALIZATION::Json& config);
		~SequencerWindow();

		SequencerWindow(SequencerWindow const& other) = delete;

		virtual void                DrawGUI() override;
        virtual void				Update(float dt) override;


	private:

        void                        SetReferenceGameObject(GameObject* referenceGameObject);

        void                        AttachGameObjectObservers();
        void                        RemoveGameObjectObservers();

        void                        CleanUpGameObjectData();

        void                        UpdateComponents();
        void                        RefreshComponentSequencers();

        void                        DrawNullObjectMessage() const;
        void                        DrawAddComponentButton();

        bool                        AddComponentToEdit(ComponentBase* comp);

        D_CONTAINERS::DVector<ComponentBase*> GetUnusedComponents() const;
        bool                        AnyUnusedComponentsLeft() const;

        AnimationResource*          GetAssociatedAnimationResource() const;
        AnimationComponent*         GetAssociatedAnimationComponent() const;

        static void                 ConfigureSequencerForComponent(Components::AnimationSequence& seq, ComponentBase* comp, D_ANIMATION::Sequence* keyframeSequence, D_ANIMATION::AnimationResource* animationResource, int& currentFrameRef);

        GameObject*                 mReferenceGameObject;
        D_CONTAINERS::DVector<ComponentBase*> mReferenceComponents;
        D_CONTAINERS::DVector<std::string> mReferenceComponentNames;

        D_CONTAINERS::DUnorderedMap<std::string, D_GUI_COMPONENT::AnimationSequence> mComponentSequencers;

        D_CORE::SignalConnection    mComponentChangeSignalConnection;

        // Sequencer data
        int                         mSelectedEntry;
        int                         mSelectedSequence;
        int                         mCurrentFrame;

        AnimationResource*          mCachedAnimation;
	};
}