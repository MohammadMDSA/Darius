#include "Editor/pch.hpp"

#include "GuiManager.hpp"
#include "Windows/ContentWindow.hpp"
#include "Windows/DetailsWindow.hpp"
#include "Windows/GameWindow.hpp"
#include "Windows/ProfilerWindow.hpp"
#include "Windows/SceneWindow.hpp"
#include "Windows/SceneGraphWindow.hpp"
#include "Windows/SequencerWindow.hpp"
#include "Windows/SettingsWindow.hpp"
#include "Windows/ResourceMonitorWindow.hpp"
#include "Editor/EditorContext.hpp"
#include "Editor/Simulation.hpp"

#include <Animation/AnimationResource.hpp>
#include <Core/Application.hpp>
#include <Core/Input.hpp>
#include <Core/Containers/Map.hpp>
#include <Core/TimeManager/TimeManager.hpp>
#include <Core/TimeManager/SystemTime.hpp>
#include <Engine/EngineContext.hpp>
#include <Math/VectorMath.hpp>
#include <Physics/Resources/PhysicsMaterialResource.hpp>
#include <Physics/Components/BoxColliderComponent.hpp>
#include <Physics/Components/SphereColliderComponent.hpp>
#include <Physics/Components/MeshColliderComponent.hpp>
#include <Graphics/GraphicsUtils/Profiling/Profiling.hpp>
#include <Renderer/Rasterization/Renderer.hpp>
#include <Renderer/Resources/MaterialResource.hpp>
#include <Renderer/Resources/TerrainResource.hpp>
#include <Renderer/Resources/StaticMeshResource.hpp>
#include <Renderer/Components/CameraComponent.hpp>
#include <Renderer/Components/LightComponent.hpp>
#include <Renderer/Components/MeshRendererComponent.hpp>
#include <Renderer/Components/SkeletalMeshRendererComponent.hpp>
#include <ResourceManager/ResourceManager.hpp>
#include <ResourceManager/ResourceLoader.hpp>
#include <Scene/EntityComponentSystem/Components/TransformComponent.hpp>
#include <Scene/Resources/PrefabResource.hpp>
#include <Utils/Assert.hpp>

#include <Core/Serialization/TypeSerializer.hpp>

#include <Libs/imgui_wrapper/ImGuiFileDialog/ImGuiFileDialog.h>
#include <Libs/FontIcon/IconsFontAwesome6.h>
#include <imgui.h>
#include <imgui_impl_dx12.h>
#include <imgui_impl_win32.h>

using namespace D_FILE;
using namespace D_SERIALIZATION;
using namespace D_SCENE;
using namespace Darius::Editor::Gui::Windows;

namespace Darius::Editor::Gui::GuiManager
{
	bool											initialzied = false;

	D_CONTAINERS::DUnorderedMap<std::string, Window*>	Windows;
	Json											windowsConfig;

	std::string										LayoutPath;

	void ShowDialogs();

	void RootToolbar();

	void Initialize()
	{
		D_ASSERT(!initialzied);
		initialzied = true;

		D_APP::SubscribeOnAppDeactivated(SaveWindowsState);

		auto winConfigPath = D_EDITOR_CONTEXT::GetEditorWindowsConfigPath();
		if(D_H_ENSURE_FILE(winConfigPath))
			D_FILE::ReadJsonFile(winConfigPath, windowsConfig);

#define RegisterWindow(type) \
{ \
	auto name = type::SGetName(); \
	auto wind = new type(windowsConfig[name]); \
	Windows[name] = wind; \
}	
		// TODO: Use linear allocator to allocate windows
		RegisterWindow(SceneWindow);
		RegisterWindow(SceneGraphWindow);
		RegisterWindow(DetailsWindow);
		RegisterWindow(ResourceMonitorWindow);
		RegisterWindow(ProfilerWindow);
		RegisterWindow(ContentWindow);
		RegisterWindow(SettingsWindow);
		RegisterWindow(GameWindow);
		RegisterWindow(SequencerWindow);

		ImGuiIO& io = ImGui::GetIO();
		// Setup docking
		io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;

		// Setup fonts
		ImFontConfig fontConf;
		io.Fonts->AddFontDefault(&fontConf);
		// Merge fontawesom fonts
		static const ImWchar icons_ranges[] = {ICON_MIN_FA, ICON_MAX_16_FA, 0};
		ImFontConfig icons_config;
		icons_config.MergeMode = true;
		icons_config.PixelSnapH = true;
		io.Fonts->AddFontFromFileTTF("EditorResources/fonts/" FONT_ICON_FILE_NAME_FAS, 12.0f, &icons_config, icons_ranges);

		// Read layout from file
		LayoutPath = Path(D_EDITOR_CONTEXT::GetEditorConfigPath()).append("layout.ini").string();

		io.IniFilename = LayoutPath.c_str();
		ImGui::GetStyle().FrameRounding = 5.f;
		ImGui::GetStyle().PopupRounding = 5.f;
		ImGui::GetStyle().WindowRounding = 5.f;

	}

	void Shutdown()
	{
		D_ASSERT(initialzied);

		// Unallocating windows objects
		D_CONTAINERS::DVector<Window*> toBeRemoved;
		for(auto& keyVal : Windows)
		{
			toBeRemoved.push_back(keyVal.second);
		}
		Windows.clear();
		for(int i = 0; i < toBeRemoved.size(); i++)
		{
			Window* win = toBeRemoved[i];
			delete win;
		}

		D_FILE::WriteJsonFile(D_EDITOR_CONTEXT::GetEditorWindowsConfigPath(), windowsConfig);
	}

	void Update(float deltaTime)
	{
		D_PROFILING::ScopedTimer guiProfiling(L"Update Gui");

		{
			D_PROFILING::ScopedTimer windowProfiling(L"Update Windows");
			for(auto& kv : Windows)
				kv.second->Update(deltaTime);
		}

		{
			D_PROFILING::ScopedTimer guiSetupProfiling(L"Setup Gui Draw");
			// Prepare imgui
			ImGui_ImplDX12_NewFrame();
			ImGui_ImplWin32_NewFrame();
			ImGui::NewFrame();
		}

		DrawGUI();

		// Handle global shortcuts
		if(!ImGui::GetIO().WantTextInput)
		{
			GameObject* selectedObj = D_EDITOR_CONTEXT::GetSelectedGameObject();

			// Copy
			if((D_INPUT::IsPressed(D_INPUT::DigitalInput::KeyLControl) || D_INPUT::IsPressed(D_INPUT::DigitalInput::KeyRControl)) && D_INPUT::IsFirstPressed(D_INPUT::DigitalInput::KeyC))
			{
				if(selectedObj)
				{
					D_EDITOR_CONTEXT::SetClipboard(selectedObj);
				}
			}

			// Paste
			if((D_INPUT::IsPressed(D_INPUT::DigitalInput::KeyLControl) || D_INPUT::IsPressed(D_INPUT::DigitalInput::KeyRControl)) && D_INPUT::IsFirstPressed(D_INPUT::DigitalInput::KeyV))
			{
				if(D_EDITOR_CONTEXT::IsGameObjectInClipboard())
				{
					GameObject* pastedGo;
					D_SERIALIZATION::Json goJson;
					D_EDITOR_CONTEXT::GetClipboardJson(true, goJson);
					D_WORLD::LoadGameObject(goJson, &pastedGo, true);

					if(selectedObj)
					{
						pastedGo->SetParent(selectedObj, GameObject::AttachmentType::KeepLocal);
					}
				}
			}

			// Delete
			if((D_INPUT::IsPressed(D_INPUT::DigitalInput::KeyDelete)))
			{
				if(selectedObj)
				{
					if(D_SIMULATE::IsSimulating())
						D_WORLD::DeleteGameObject(selectedObj);
					else
						D_WORLD::DeleteGameObjectImmediately(selectedObj);
				}
			}
		}

		{
			D_PROFILING::ScopedTimer guiRender(L"Gui Render");
			ImGui::Render();
		}
	}

	void Render()
	{
		for(auto& kv : Windows)
			if(kv.second->IsAppearing())
				kv.second->Render();
	}

	void DrawGUI()
	{
		D_ASSERT_M(initialzied, "Gui Manager is not initialized yet!");

		D_PROFILING::ScopedTimer windowProfiling(L"Draw Gui");

		{
			D_PROFILING::ScopedTimer drawFrameProfiling(L"Draw Frame");

			const ImGuiViewport* viewport = ImGui::GetMainViewport();
			ImGui::SetNextWindowPos(viewport->WorkPos);
			ImGui::SetNextWindowSize(viewport->WorkSize);
			ImGui::SetNextWindowViewport(viewport->ID);
			ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
			ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
			ImGuiWindowFlags windowFlags = ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_MenuBar;
			windowFlags |= ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus;

			ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));

			ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0.f, 0.05f, 0.1f, 1.f));
			ImGui::Begin("Editor GUI Root", (bool*)1, windowFlags);
			ImGui::PopStyleColor();
			RootToolbar();
			ImGui::PopStyleVar(3);

			ImGuiID dockspaceId = ImGui::GetID("EditorDockspace");
			ImGui::DockSpace(dockspaceId, ImVec2(0.0f, 0.0f), ImGuiDockNodeFlags_None);

#pragma region Toolbar
			_DrawMenuBar();
			ShowDialogs();
#pragma endregion

			ImGui::End();
		}

		{
			static bool show_demo_window = false;
			if(show_demo_window)
				ImGui::ShowDemoWindow(&show_demo_window);

		}

		{
			D_PROFILING::ScopedTimer windowDrawProfiling(L"Draw Window Gui");

			for(auto& kv : Windows)
			{
				auto wind = kv.second;

				ImGui::SetNextWindowBgAlpha(1.f);

				if(wind->IsOpened())
				{
					ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(wind->mPadding[0], wind->mPadding[1]));

					bool opened = true;
					if(ImGui::Begin(wind->GetName().c_str(), &opened))
					{
						wind->PrepareGUI();

						wind->DrawGUI();
					}
					if(!opened)
						wind->SetOpened(false);

					ImGui::End();
					ImGui::PopStyleVar();

				}
			}
		}
	}

	void ShowDialogs()
	{

		if(ImGuiFileDialog::Instance()->Display("LoadScene"))
		{
			// action if OK
			if(ImGuiFileDialog::Instance()->IsOk())
			{
				std::string filePathName = ImGuiFileDialog::Instance()->GetFilePathName();
				D_WORLD::Unload();

				if(D_WORLD::Load(STR2WSTR(filePathName)))
					D_EDITOR_CONTEXT::SetSelectedGameObject(nullptr);
			}

			// close
			ImGuiFileDialog::Instance()->Close();
		}

		if(ImGuiFileDialog::Instance()->Display("SaveScene"))
		{
			// action if OK
			if(ImGuiFileDialog::Instance()->IsOk())
			{
				std::string filePathName = ImGuiFileDialog::Instance()->GetFilePathName();
				if(D_WORLD::Create(STR2WSTR(filePathName)))
				{
					D_WORLD::Save();
				}
			}

			// close
			ImGuiFileDialog::Instance()->Close();
		}

#pragma warning(push)
#pragma warning(disable: 4616 4302)
		if(ImGuiFileDialog::Instance()->Display("SaveResource"))
		{
			// action if OK
			if(ImGuiFileDialog::Instance()->IsOk())
			{
				std::string _filePathName = ImGuiFileDialog::Instance()->GetFilePathName();
				std::wstring filePathName = STR2WSTR(_filePathName);

				D_RESOURCE::ResourceType type = (D_RESOURCE::ResourceType)ImGuiFileDialog::Instance()->GetUserDatas();
				D_RESOURCE::GetManager()->CreateResource(type, filePathName, D_FILE::GetFileName(filePathName));
			}

			// Close
			ImGuiFileDialog::Instance()->Close();
		}

		if(ImGuiFileDialog::Instance()->Display("SavePrefab"))
		{
			if(ImGuiFileDialog::Instance()->IsOk())
			{
				std::string _filePath = ImGuiFileDialog::Instance()->GetFilePathName();
				std::wstring filePath = STR2WSTR(_filePath);


				auto prefabResHandle = D_RESOURCE::GetManager()->CreateResource<D_SCENE::PrefabResource>(filePath, D_FILE::GetFileName(filePath));

				auto refGameObject = reinterpret_cast<D_SCENE::GameObject*>(ImGuiFileDialog::Instance()->GetUserDatas());

				auto prefabRes = D_RESOURCE::GetResourceSync<D_SCENE::PrefabResource>(prefabResHandle);
				prefabRes->CreateFromGameObject(refGameObject);
			}

			// Close
			ImGuiFileDialog::Instance()->Close();
		}
#pragma warning(pop)
	}

	void DrawGammAddMenu(GameObject* const contextGameObject)
	{

#define CreateParentedGameObject() \
GameObject* created = D_WORLD::CreateGameObject(); \
if (validContext) \
{ \
	created->GetTransform()->SetLocalPosition(D_MATH::Vector3::Zero); \
	created->SetParent(contextGameObject, GameObject::AttachmentType::KeepLocal); \
} \
else \
	created->GetTransform()->SetPosition(GetWindow<SceneWindow>()->SuggestSpawnPositionOnYPlane()); \

		D_PROFILING::ScopedTimer _Prof(L"Draw Add GameObject Menu");

		bool validContext = contextGameObject && contextGameObject->IsValid();
		if(ImGui::BeginMenu(ICON_FA_PLUS "  Add"))
		{
			if(ImGui::MenuItem(ICON_FA_FILE "  Empty Game Object"))
			{
				CreateParentedGameObject();
				D_EDITOR_CONTEXT::SetSelectedGameObject(created);
			}

			if(!validContext)
				ImGui::BeginDisabled();
			if(ImGui::MenuItem(ICON_FA_ARROW_TURN_UP "  Parent Game Object"))
			{
				GameObject* created = D_WORLD::CreateGameObject();
				D_ASSERT(created);
				D_ASSERT(contextGameObject);
				GameObject* currentParent = contextGameObject->GetParent();
				created->SetParent(currentParent, GameObject::AttachmentType::KeepLocal);
				contextGameObject->SetParent(created, GameObject::AttachmentType::KeepWorld);
				D_EDITOR_CONTEXT::SetSelectedGameObject(created);
			}
			if(!validContext)
				ImGui::EndDisabled();


			if(ImGui::BeginMenu(ICON_FA_LIGHTBULB "  Light"))
			{
				if(ImGui::MenuItem(ICON_FA_SUN "  Directional Light"))
				{
					CreateParentedGameObject();
					auto lightComp = created->AddComponent<D_RENDERER::LightComponent>();
					lightComp->SetLightType(D_RENDERER_LIGHT::LightSourceType::DirectionalLight);
					created->SetName("Directional Light");
					D_EDITOR_CONTEXT::SetSelectedGameObject(created);
				}

				if(ImGui::MenuItem(ICON_FA_LIGHTBULB "  Point Light"))
				{
					CreateParentedGameObject();
					auto lightComp = created->AddComponent<D_RENDERER::LightComponent>();
					lightComp->SetLightType(D_RENDERER_LIGHT::LightSourceType::PointLight);
					created->SetName("Point Light");
					D_EDITOR_CONTEXT::SetSelectedGameObject(created);
				}

				if(ImGui::MenuItem(ICON_FA_LIGHTBULB "  Spot Light"))
				{
					CreateParentedGameObject();
					auto lightComp = created->AddComponent<D_RENDERER::LightComponent>();
					lightComp->SetLightType(D_RENDERER_LIGHT::LightSourceType::SpotLight);
					created->SetName("Spot Light");
					D_EDITOR_CONTEXT::SetSelectedGameObject(created);
				}

				ImGui::EndMenu();
			}

			if(ImGui::MenuItem(ICON_FA_VIDEO "  Camera"))
			{
				CreateParentedGameObject();
				created->AddComponent<D_RENDERER::CameraComponent>();
				created->SetName("Camera");
				D_EDITOR_CONTEXT::SetSelectedGameObject(created);
			}

			// 3D Menu
			if(ImGui::BeginMenu(ICON_FA_CUBES " 3D"))
			{
				if(ImGui::MenuItem(ICON_FA_CUBE "  Cube"))
				{
					CreateParentedGameObject();
					D_RENDERER::MeshRendererComponent* comp = created->AddComponent<D_RENDERER::MeshRendererComponent>();
					D_RESOURCE::ResourceHandle resHandle = D_RENDERER::GetDefaultGraphicsResource(D_RENDERER::DefaultResource::BoxMesh);
					D_RESOURCE::ResourceHandle matHandle = D_RENDERER::GetDefaultGraphicsResource(D_RENDERER::DefaultResource::Material);
					auto boxMeshRes = D_RESOURCE::GetResourceSync<D_RENDERER::StaticMeshResource>(resHandle);
					auto defaultMat = D_RESOURCE::GetResourceSync<D_RENDERER::MaterialResource>(matHandle);
					comp->SetMesh(boxMeshRes.Get());
					comp->SetMaterial(0, defaultMat.Get());
					auto collider = created->AddComponent<D_PHYSICS::BoxColliderComponent>();
					collider->SetTrigger(false);
					created->SetName("Cube");
					D_EDITOR_CONTEXT::SetSelectedGameObject(created);
				}

				if(ImGui::MenuItem(ICON_FA_CIRCLE "  Sphere"))
				{
					CreateParentedGameObject();
					D_RENDERER::MeshRendererComponent* comp = created->AddComponent<D_RENDERER::MeshRendererComponent>();
					D_RESOURCE::ResourceHandle resHandle = D_RENDERER::GetDefaultGraphicsResource(D_RENDERER::DefaultResource::SphereMesh);
					D_RESOURCE::ResourceHandle matHandle = D_RENDERER::GetDefaultGraphicsResource(D_RENDERER::DefaultResource::Material);
					auto spMeshRes = D_RESOURCE::GetResourceSync<D_RENDERER::StaticMeshResource>(resHandle);
					auto defaultMat = D_RESOURCE::GetResourceSync<D_RENDERER::MaterialResource>(matHandle);
					comp->SetMesh(spMeshRes.Get());
					comp->SetMaterial(0, defaultMat.Get());
					auto collider = created->AddComponent<D_PHYSICS::SphereColliderComponent>();
					collider->SetTrigger(false);
					created->SetName("Sphere");
					D_EDITOR_CONTEXT::SetSelectedGameObject(created);
				}

				if(ImGui::MenuItem(ICON_FA_WEIGHT_HANGING "  Static Mesh"))
				{
					CreateParentedGameObject();
					created->AddComponent<D_RENDERER::MeshRendererComponent>();
					created->SetName("Static Mesh");
					D_EDITOR_CONTEXT::SetSelectedGameObject(created);
				}

				if(ImGui::MenuItem(ICON_FA_BONE "  Skeletal Mesh"))
				{
					CreateParentedGameObject();
					created->AddComponent<D_RENDERER::SkeletalMeshRendererComponent>();
					created->SetName("Skeletal Mesh");
					D_EDITOR_CONTEXT::SetSelectedGameObject(created);
				}

				ImGui::EndMenu();
			}

			ImGui::EndMenu();
		}

#undef CreateParentedGameObject
	}

	void AddBoxColliderToAllMeshInstances(D_RENDERER::StaticMeshResource* mesh)
	{
		if(!mesh)
			return;

		D_CONTAINERS::DList<GameObject*> gos;

		D_WORLD::IterateComponents<D_RENDERER::MeshRendererComponent>([=, &gos](D_RENDERER::MeshRendererComponent& comp)
			{
				auto go = comp.GetGameObject();
				if(comp.GetMesh() != mesh)
					return;

				if(go->HasComponent<D_PHYSICS::BoxColliderComponent>())
					return;

				gos.push_back(go);
			});

		for(auto go : gos)
		{
			auto boxComp = go->AddComponent<D_PHYSICS::BoxColliderComponent>();
			boxComp->OnPostComponentAddInEditor();
		}

	}

	void AddMeshColliderToAllMeshInstances(D_RENDERER::StaticMeshResource* mesh, bool convex)
	{
		if(!mesh)
			return;

		D_CONTAINERS::DList<GameObject*> gos;

		D_WORLD::IterateComponents<D_RENDERER::MeshRendererComponent>([=, &gos](D_RENDERER::MeshRendererComponent& comp)
			{
				auto go = comp.GetGameObject();
				if(comp.GetMesh() != mesh)
					return;

				if(go->HasComponent<D_PHYSICS::MeshColliderComponent>())
					return;

				gos.push_back(go);
			});

		for(auto go : gos)
		{
			auto meshComp = go->AddComponent<D_PHYSICS::MeshColliderComponent>();
			meshComp->SetConvex(convex);
			meshComp->OnPostComponentAddInEditor();
		}

	}

	void GoHandler(GameObject* go, GameObject* refParent, D_RENDERER::MaterialResource* defaultMat, bool toRoot)
	{

		auto mod = false;
		if(auto sk = go->GetComponent<D_RENDERER::SkeletalMeshRendererComponent>())
		{
			if(auto mesh = sk->GetMesh())
			{
				mesh->SetInverted(true);
				for(int i = 0; i < mesh->GetMeshData()->mDraw.size(); i++)
				{
					mesh->SetMaterial(i, defaultMat);
				}
			}
			mod = true;
		}

		if(auto sm = go->GetComponent<D_RENDERER::MeshRendererComponent>())
		{
			if(auto mesh = sm->GetMesh())
			{
				mesh->SetInverted(true);
				for(int i = 0; i < mesh->GetMeshData()->mDraw.size(); i++)
				{
					mesh->SetMaterial(i, defaultMat);
				}
			}
			mod = true;
		}

		if(mod)
		{
			auto parent = go->GetParent();
			if(parent != refParent && parent)
				go->SetName(parent->GetName());

			go->SetParent(nullptr, GameObject::AttachmentType::KeepWorld);
		}
	}

	void _DrawMenuBar()
	{
		D_PROFILING::ScopedTimer menubarProfiling(L"Draw Menubar");

		if(ImGui::BeginMenuBar())
		{
			if(ImGui::BeginMenu(ICON_FA_FILE "  File"))
			{
				if(ImGui::MenuItem(ICON_FA_FLOPPY_DISK "  Save Project"))
					D_RESOURCE::SaveAll();

				ImGui::Separator();

				auto simulating = D_SIMULATE::IsSimulating();

				if(simulating)
					ImGui::BeginDisabled();

				if(ImGui::MenuItem(ICON_FA_XMARK"  Close Scene"))
				{
					D_WORLD::Unload();
					D_EDITOR_CONTEXT::SetSelectedGameObject(nullptr);
				}

				if(ImGui::MenuItem(ICON_FA_FLOPPY_DISK "  Save Scene"))
				{
					if(D_WORLD::IsLoaded())
						D_WORLD::Save();
					else
						ImGuiFileDialog::Instance()->OpenDialog("SaveScene", "Create Scene File", ".dar", D_ENGINE_CONTEXT::GetAssetsPath().string());
				}

				if(ImGui::MenuItem(ICON_FA_FOLDER "  Load Scene"))
				{

					ImGuiFileDialog::Instance()->OpenDialog("LoadScene", "Choose Scene File", ".dar", D_ENGINE_CONTEXT::GetAssetsPath().string(), 1, nullptr);

				}

				if(simulating)
					ImGui::EndDisabled();

				ImGui::EndMenu();
			}

			if(ImGui::BeginMenu("Game Object"))
			{
				DrawGammAddMenu(D_EDITOR_CONTEXT::GetSelectedGameObject());

				if(ImGui::MenuItem(ICON_FA_TRASH "  Delete Game Object", (const char*)0, false, D_EDITOR_CONTEXT::GetSelectedGameObject() != nullptr))
				{
					D_WORLD::DeleteGameObject(D_EDITOR_CONTEXT::GetSelectedGameObject());
					D_EDITOR_CONTEXT::SetSelectedGameObject(nullptr);
				}

				ImGui::EndMenu();
			}

			if(ImGui::BeginMenu("Resource"))
			{
				if(ImGui::BeginMenu("Create"))
				{
					if(ImGui::MenuItem("Material"))
					{
						ImGuiFileDialog::Instance()->OpenDialog("SaveResource", "Create Material", ".mat", D_ENGINE_CONTEXT::GetAssetsPath().string(), 1, (void*)D_RENDERER::MaterialResource::GetResourceType());
					}

					if(ImGui::MenuItem("Terrain"))
					{
						ImGuiFileDialog::Instance()->OpenDialog("SaveResource", "Create Terrain", ".terrain", D_ENGINE_CONTEXT::GetAssetsPath().string(), 1, (void*)D_RENDERER::TerrainResource::GetResourceType());
					}

					if(ImGui::MenuItem("Animation"))
					{
						ImGuiFileDialog::Instance()->OpenDialog("SaveResource", "Create Animation", ".anim", D_ENGINE_CONTEXT::GetAssetsPath().string(), 1, (void*)D_ANIMATION::AnimationResource::GetResourceType());
					}

					ImGui::Separator();

					if(ImGui::MenuItem("Physics Material"))
					{
						ImGuiFileDialog::Instance()->OpenDialog("SaveResource", "Create Physics Material", ".physmat", D_ENGINE_CONTEXT::GetAssetsPath().string(), 1, (void*)D_PHYSICS::PhysicsMaterialResource::GetResourceType());
					}

					ImGui::EndMenu();
				}

				if(ImGui::MenuItem("Debug Button"))
				{
					auto selected = D_EDITOR_CONTEXT::GetSelectedGameObject();

					if(selected)
					{
						auto mat = selected->GetComponent<D_RENDERER::MeshRendererComponent>()->GetMaterial(0);

						selected->VisitDescendants([=](auto go)
							{
								GoHandler(go, selected, mat.Get(), false);
							});

					}
				}

				if(ImGui::MenuItem("Debug Button 2"))
				{
					auto selected = D_EDITOR_CONTEXT::GetSelectedGameObject();
					auto mat = selected->GetComponent<D_RENDERER::MeshRendererComponent>()->GetMaterial(0).Get();

					D_WORLD::IterateComponents<D_RENDERER::MeshRendererComponent>([material = mat](auto& comp)
						{
							D_RENDERER::StaticMeshResource* mesh = comp.GetMesh();
							mesh->SetInverted(true);
							for(int i = 0; i < (int)mesh->GetMeshData()->mDraw.size(); i++)
							{
								mesh->SetMaterial(i, material);
							}
						});
				}

				if(ImGui::MenuItem("Debug Button 3"))
				{
					auto scale = D_MATH::Vector3(0.01f);
					auto invScale = D_MATH::Vector3(100.f);
					D_WORLD::IterateComponents<D_MATH::TransformComponent>([=](D_MATH::TransformComponent& comp)
						{
							auto localScale = comp.GetLocalScale();
							if(!(localScale < D_MATH::Vector3(0.1f)).All3())
								return;
							auto go = comp.GetGameObject();

							bool reposition = false;

							auto staticMeshRenderer = go->GetComponent<D_RENDERER::MeshRendererComponent>();
							if(staticMeshRenderer)
							{
								reposition = true;
								comp.SetLocalScale(localScale * invScale);
								auto staticMesh = staticMeshRenderer->GetMesh();
								if(staticMesh)
									staticMesh->SetScale(scale);
							}


							go->VisitDescendants([=](GameObject* desc)
								{
									auto staticMeshRenderer = desc->GetComponent<D_RENDERER::MeshRendererComponent>();
									if(!staticMeshRenderer)
										return;

									auto staticMesh = staticMeshRenderer->GetMesh();
									if(!staticMesh)
										return;

									staticMesh->SetScale(scale);

									//if(desc->GetParent() == go)
									{
										auto trans = desc->GetTransform();
										trans->SetLocalPosition(trans->GetLocalPosition() * scale);
									}
								});

						});
				}

				if(ImGui::MenuItem("Fix box collider sizes"))
				{
					D_WORLD::IterateComponents<D_PHYSICS::BoxColliderComponent>([](D_PHYSICS::BoxColliderComponent& box)
						{
							auto go = box.GetGameObject();
							auto meshComp = go->GetComponent<D_RENDERER::MeshRendererComponent>();

							if(!meshComp)
							{
								auto goname = go->GetName();
								(goname);
								D_PLATFORM_BREAK();
								return;
							}

							auto mesh = meshComp->GetMesh();
							if(!mesh)
								return;
							static auto scl = D_MATH::Vector3(0.01f);
							if(mesh->GetScale().NearEquals(scl, 0.1f))
							{
								box.SetHalfExtents(box.GetHalfExtents() * scl);
								box.SetCenterOffset(box.GetCenterOffset() * scl);
							}
							else
							{
								auto goname = go->GetName();
								(goname);
								D_PLATFORM_BREAK();
								return;
							}
						});
				}

				if(ImGui::MenuItem("Add mesh collider"))
				{
					auto selected = D_EDITOR_CONTEXT::GetSelectedGameObject();

					if(selected)
					{
						selected->VisitChildren([](auto go)
							{
								D_ECS::CompRef<D_RENDERER::MeshRendererComponent> mesh = go->GetComponent<D_RENDERER::MeshRendererComponent>();
								if(!mesh.IsValid() || !mesh->GetMesh())
									return;

								D_PHYSICS::MeshColliderComponent* meshCollider = go->AddComponent<D_PHYSICS::MeshColliderComponent>();
								meshCollider->SetReferenceMesh(mesh->GetMesh());
								meshCollider->SetConvex(false);
							});
					}
				}

				if(ImGui::MenuItem("Add convex mesh collider"))
				{
					auto selected = D_EDITOR_CONTEXT::GetSelectedGameObject();

					if(selected)
					{
						selected->VisitChildren([](auto go)
							{
								D_ECS::CompRef<D_RENDERER::MeshRendererComponent> mesh = go->GetComponent<D_RENDERER::MeshRendererComponent>();
								if(!mesh.IsValid() || !mesh->GetMesh())
									return;

								D_PHYSICS::MeshColliderComponent* meshCollider = go->AddComponent<D_PHYSICS::MeshColliderComponent>();
								meshCollider->SetReferenceMesh(mesh->GetMesh());
								meshCollider->SetConvex(true);
							});
					}
				}

				if(ImGui::MenuItem("Add box collider"))
				{
					auto selected = D_EDITOR_CONTEXT::GetSelectedGameObject();

					if(selected)
					{
						selected->VisitChildren([](auto go)
							{
								D_ECS::CompRef<D_RENDERER::MeshRendererComponent> mesh = go->GetComponent<D_RENDERER::MeshRendererComponent>();
								if(!mesh.IsValid() || !mesh->GetMesh())
									return;

								D_PHYSICS::BoxColliderComponent* box = go->AddComponent<D_PHYSICS::BoxColliderComponent>();
								box->OnPostComponentAddInEditor();
							});
					}
				}

				{
					auto mesh = dynamic_cast<D_RENDERER::StaticMeshResource*>(D_EDITOR_CONTEXT::GetSelectedDetailed());
					bool meshColliderSelected = mesh;

					if(!meshColliderSelected)
						ImGui::BeginDisabled();

					if(ImGui::BeginMenu("Mesh Resource Collider"))
					{
						if(ImGui::MenuItem("Box Collider"))
							AddBoxColliderToAllMeshInstances(mesh);

						if(ImGui::MenuItem("Convex Mesh Collider"))
							AddMeshColliderToAllMeshInstances(mesh, true);

						if(ImGui::MenuItem("Mesh Collider"))
							AddMeshColliderToAllMeshInstances(mesh, false);

						ImGui::EndMenu();
					}

					if(!meshColliderSelected)
						ImGui::EndDisabled();
				}
				ImGui::EndMenu();
			}
			if(ImGui::BeginMenu("Window"))
			{
				for(auto& kv : Windows)
				{
					if(ImGui::MenuItem(kv.second->GetName().c_str()))
					{
						kv.second->SetOpened(true);
					}
				}
				ImGui::EndMenu();
			}
			ImGui::EndMenuBar();
		}
	}

	void RootToolbar()
	{
		auto width = ImGui::GetWindowWidth();
		auto buttonSize = 30.f;
		auto margin = 5.f;

		ImGui::Spacing();
		ImGui::Spacing();

		ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(margin, margin));

		auto offset = (width - 3 * buttonSize) / 2;

		auto isSimulating = D_SIMULATE::IsSimulating();

		// Play button
		ImGui::SameLine(offset);
		if(ImGui::Button(isSimulating ? ICON_FA_STOP : ICON_FA_PLAY, ImVec2(buttonSize, 0)))
			isSimulating ? D_SIMULATE::Stop() : D_SIMULATE::Run();

		// Update states
		isSimulating = D_SIMULATE::IsSimulating();
		auto isPaused = D_SIMULATE::IsPaused();



		// Pause button
		// Deciding puase button color
		if(isPaused)
			ImGui::PushStyleColor(ImGuiCol_Button, {0.26f, 0.59f, 1.f, 1.f});

		ImGui::SameLine();

		// Disable if not simulating
		if(!isSimulating)
			ImGui::BeginDisabled();

		// The button itself
		if(ImGui::Button(ICON_FA_PAUSE, ImVec2(buttonSize, 0)))
			isPaused ? D_SIMULATE::Resume() : D_SIMULATE::Pause();

		// Cleaning up disabled and color status if necessary
		if(!isSimulating)
			ImGui::EndDisabled();
		if(isPaused)
			ImGui::PopStyleColor();

		isPaused = D_SIMULATE::IsPaused();

		// Step button
		ImGui::SameLine();

		if(!isPaused)
			ImGui::BeginDisabled();
		if(ImGui::Button(ICON_FA_FORWARD_STEP, ImVec2(buttonSize, 0)))
			D_SIMULATE::Step();
		if(!isPaused)
			ImGui::EndDisabled();

		ImGui::PopStyleVar();
	}

	void SaveWindowsState()
	{
		D_SERIALIZATION::Json config;

		for(auto const& wPair : Windows)
		{
			auto const& window = wPair.second;
			D_SERIALIZATION::Json winConfig;

			winConfig["Opened"] = window->IsOpened();

			config.emplace(window->GetName(), winConfig);
		}

		D_FILE::WriteJsonFile(D_EDITOR_CONTEXT::GetEditorWindowsConfigPath(), config);
	}

	Window* GetWindow(std::string const& name)
	{
		auto it = Windows.find(name);
		if(it != Windows.end())
			return it->second;
		return nullptr;
	}

}