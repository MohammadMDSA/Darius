#pragma once
#include "StaticConstructor.hpp"

#define INLINE __forceinline
#define NODISCARD [[nodiscard]]
#define ALIGN_DECL(size) __declspec(align(size))
#define ALIGN_DECL_256 ALIGN_DECL(256)
#define ALIGN_DECL_16 ALIGN_DECL(16)

#define D_NAMEOF(T) #T
#define D_NAMEOF_C(T) #T.c_str()

#define D_CH_TYPE_NAME_GETTER(T) \
public: \
static INLINE std::string const GetTypeName() { return D_NAMEOF(T); }

///////////////////////////// Function Helpers
#define _IN_
#define _OUT_
#define _IN_OUT_

///////////////////////////// Helpers
#define D_H_CONCAT(A, B) A##B
#define D_H_UNIQUE_NAME(base) D_H_CONCAT(base, __COUNTER__)
#define D_H_UNIQUE_ID __COUNTER__
#define D_H_ENSURE_PATH(path) std::filesystem::exists(path)
#define D_H_ENSURE_DIR(path) (D_H_ENSURE_PATH(path) && std::filesystem::is_directory(path))
#define D_H_ENSURE_FILE(path) (D_H_ENSURE_PATH(path) && !std::filesystem::is_directory(path))

//////////////////////////// Details Draw

#ifdef _D_EDITOR

#define D_H_RESOURCE_DRAG_DROP_DESTINATION(resourceType, handleSetter) \
{\
	if(ImGui::BeginDragDropTarget()) \
    { \
        ImGuiPayload const* imPayload = ImGui::GetDragDropPayload(); \
        Darius::Utils::DDragDropPayload const* payload = (Darius::Utils::DDragDropPayload const*)(imPayload->Data); \
		if(payload && payload->IsCompatible(Darius::Utils::DDragDropPayload::Type::Resource, std::to_string(resourceType::GetResourceType()))) \
		{ \
			if (ImGuiPayload const* payload = ImGui::AcceptDragDropPayload(D_PAYLOAD_TYPE_RESOURCE)) \
			{ \
				handleSetter(((Darius::Utils::DDragDropPayload const*)(imPayload->Data))->ResourcePayload.Handle); \
			} \
			ImGui::EndDragDropTarget(); \
		} \
	} \
}

#define D_H_RESOURCE_SELECTION_DRAW(resourceType, prop, placeHolder, handleFunction) \
{ \
    resourceType* currentResource = prop.Get(); \
     \
    std::string cuurrentResourceName; \
    if(prop.IsValid()) \
    { \
        auto nameW = prop->GetName(); \
        cuurrentResourceName = WSTR2STR(nameW); \
    } \
    else \
        cuurrentResourceName = placeHolder; \
    auto availableSpace = ImGui::GetContentRegionAvail(); \
    auto selectorWidth = 20.f; \
    ImGui::Button(cuurrentResourceName.c_str(), ImVec2(availableSpace.x - 2 * selectorWidth - 10.f, 0)); \
    D_H_RESOURCE_DRAG_DROP_DESTINATION(resourceType, handleFunction) \
	ImGui::SameLine(availableSpace.x - selectorWidth); \
		if (ImGui::Button(ICON_FA_ELLIPSIS_VERTICAL, ImVec2(selectorWidth, 0))) \
		{ \
			ImGui::OpenPopup(placeHolder " Res"); \
		} \
			\
			if (ImGui::BeginPopup(placeHolder " Res")) \
			{ \
				auto resources = D_RESOURCE::GetResourcePreviews(resourceType::GetResourceType()); \
				int idx = 0; \
				for (auto prev : resources) \
				{ \
					bool selected = currentResource && prev.Handle.Id == currentResource->GetId() && prev.Handle.Type == currentResource->GetType(); \
					\
					auto Name = WSTR2STR(prev.Name); \
					ImGui::PushID((Name + std::to_string(idx)).c_str()); \
					if (ImGui::Selectable(Name.c_str(), &selected)) \
					{ \
						handleFunction(prev.Handle); \
						valueChanged = true; \
					} \
						ImGui::PopID(); \
						\
						idx++; \
				} \
					\
						ImGui::EndPopup(); \
			} \
}
#define D_H_RESOURCE_SELECTION_DRAW_SHORT(type, name, label) D_H_RESOURCE_SELECTION_DRAW(type, m##name, label, Set##name)
#define D_H_RESOURCE_SELECTION_DRAW_DEF(type, name) D_H_RESOURCE_SELECTION_DRAW_SHORT(type, name, "Select " #name)

#define D_H_DETAILS_DRAW_BEGIN_TABLE(...) \
if (ImGui::BeginTable((std::string("Layout" __VA_ARGS__) + ClassName()).c_str(), 2, ImGuiTableFlags_BordersInnerV | ImGuiTableFlags_PreciseWidths)) \
{ \
ImGui::TableSetupColumn("label", ImGuiTableColumnFlags_WidthStretch, 1); \
ImGui::TableSetupColumn("value", ImGuiTableColumnFlags_WidthStretch, 2);

#define D_H_DETAILS_DRAW_PROPERTY(prop) \
ImGui::TableNextRow(); \
ImGui::TableSetColumnIndex(0); \
ImGui::Text(prop); \
ImGui::TableSetColumnIndex(1); \

#define D_H_DETAILS_DRAW_END_TABLE() \
ImGui::EndTable(); \
} \

// Subsystems Option Components
#define D_H_OPTION_DRAW_BEGIN() \
float availX = ImGui::GetContentRegionAvail().x; \
float colRatio = 0.4f; \
float inputOffset = availX * colRatio; \
float inputWidth = availX * (1.f - colRatio); \
bool settingsChanged = false;

#define D_H_OPTION_DRAW_END() \
return settingsChanged;

#define D_H_OPTION_DRAW_CHECKBOX(label, tag, variable) \
{ \
	ImGui::Text(label); \
	ImGui::SameLine(inputOffset); \
	bool value = variable; \
	ImGui::PushItemWidth(inputWidth); \
	if (ImGui::Checkbox("##" label, &value)) \
	{ \
		options[tag] = variable = value; \
		settingsChanged = true; \
	} \
	ImGui::PopItemWidth(); \
}

#define D_H_OPTION_DRAW_CHOICE(label, tag, variable, choicesTexts, numOptions) \
{ \
	ImGui::Text(label); \
	ImGui::SameLine(inputOffset); \
	ImGui::PushItemWidth(inputWidth); \
	if (ImGui::BeginCombo("##" label, choicesTexts[variable])) \
	{ \
		for (int i = 0; i < numOptions; i++) \
		{ \
			if (ImGui::Selectable(choicesTexts[i], i == variable)) \
			{ \
				options[tag] = variable = i; \
				settingsChanged = true; \
			} \
		} \
		ImGui::EndCombo(); \
	} \
	ImGui::PopItemWidth(); \
} \

#define D_H_OPTION_DRAW_INT_SLIDER(label, tag, variable, min, max) \
{ \
	ImGui::Text(label); \
	ImGui::SameLine(inputOffset); \
	int value = variable; \
	ImGui::PushItemWidth(inputWidth); \
	if (ImGui::SliderInt("##" label, &value, min, max, "%d", ImGuiSliderFlags_AlwaysClamp)) \
	{ \
		options[tag] = variable = value; \
		settingsChanged = true; \
	} \
	ImGui::PopItemWidth(); \
} \

#define D_H_OPTION_DRAW_FLOAT(label, tag, variable, ...) \
{ \
	ImGui::Text(label); \
	ImGui::SameLine(inputOffset); \
	float value = variable; \
	ImGui::PushItemWidth(inputWidth); \
	if (ImGui::DragFloat("##" label, &value, __VA_ARGS__, ImGuiSliderFlags_AlwaysClamp)) \
	{ \
		options[tag] = variable = value; \
		settingsChanged = true; \
	} \
	ImGui::PopItemWidth(); \
} \

#define D_H_OPTION_DRAW_FLOAT_SLIDER(label, tag, variable, min, max) \
{ \
	ImGui::Text(label); \
	ImGui::SameLine(inputOffset); \
	float value = variable; \
	ImGui::PushItemWidth(inputWidth); \
	if (ImGui::SliderFloat("##" label, &value, min, max, "%.3f", ImGuiSliderFlags_AlwaysClamp)) \
	{ \
		options[tag] = variable = value; \
		settingsChanged = true; \
	} \
	ImGui::PopItemWidth(); \
} \

#define D_H_OPTION_DRAW_FLOAT_SLIDER_EXP(label, tag, variable, min, max) \
{ \
	ImGui::Text(label); \
	ImGui::SameLine(inputOffset); \
	float value = variable; \
	ImGui::PushItemWidth(inputWidth); \
	if (ImGui::SliderFloat("##" label, &value, min, max, "%.3f", ImGuiSliderFlags_Logarithmic | ImGuiSliderFlags_AlwaysClamp)) \
	{ \
		options[tag] = variable = value; \
		settingsChanged = true; \
	} \
	ImGui::PopItemWidth(); \
} \

// Subsystem Settings Loader
#define D_H_OPTIONS_LOAD_BASIC(key, variable) \
if(settings.contains(key)) variable = settings.at(key);

// Subsystem Settings Loader
#define D_H_OPTIONS_LOAD_BASIC_DEFAULT(key, variable, defaultValue) \
D_H_OPTIONS_LOAD_BASIC(key, variable) else variable = defaultValue;



#endif // _D_EDITOR


struct NonCopyable
{
	NonCopyable() = default;
	NonCopyable(const NonCopyable&) = delete;
	NonCopyable& operator=(const NonCopyable&) = delete;
};