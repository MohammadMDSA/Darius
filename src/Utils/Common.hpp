#pragma once
#include "StaticConstructor.hpp"

#define INLINE __forceinline
#define NODISCARD [[nodiscard]]
#define ALIGN_DECL(size) __declspec(align(size))
#define ALIGN_DECL_256 ALIGN_DECL(256)
#define ALIGN_DECL_16 ALIGN_DECL(16)

#define D_STRINGIFY(T) #T
#define D_NAMEOF(T) #T
#define D_NAMEOF_C(T) #T.c_str()
#define D_PREPROCESSOR_JOIN

#define D_CH_TYPE_NAME_GETTER(T) \
public: \
static INLINE std::string const GetTypeName() { return D_NAMEOF(T); }

#ifdef __COUNTER__
// Created a variable with a unique name
#define ANONYMOUS_VARIABLE( Name ) D_PREPROCESSOR_JOIN(Name, __COUNTER__)
#else
// Created a variable with a unique name.
// Less reliable than the __COUNTER__ version.
#define ANONYMOUS_VARIABLE( Name ) D_PREPROCESSOR_JOIN(Name, __LINE__)
#endif

/** Thread-safe call once helper for void functions, similar to std::call_once without the std::once_flag */
#define D_CALL_ONCE(Func, ...) static int32 ANONYMOUS_VARIABLE(ThreadSafeOnce) = ((Func)(__VA_ARGS__), 1)

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

#define D_H_DETAILS_DRAW_ENUM_SELECTION(EnumType, GetterFunc, SetterFunc) \
{ \
	static auto enumeration = rttr::type::get<EnumType>().get_enumeration(); \
	auto currentValue = GetterFunc(); \
\
	if (ImGui::BeginCombo("##" #SetterFunc, enumeration.value_to_name(currentValue).data())) \
	{ \
		for (auto value : enumeration.get_values()) \
		{ \
			bool selected = value.get_value<EnumType>() == currentValue; \
			if (ImGui::Selectable(enumeration.value_to_name(value).data(), &selected)) \
			{ \
				SetterFunc(value.get_value<EnumType>()); \
				valueChanged = true; \
			} \
		} \
		ImGui::EndCombo(); \
	} \
}

#define D_H_DETAILS_DRAW_ENUM_SELECTION_SIMPLE(EnumType, PropName) \
D_H_DETAILS_DRAW_ENUM_SELECTION(EnumType, Get##PropName, Set##PropName)

#define D_H_DETAILS_DRAW_BEGIN_TABLE(...) \
if (ImGui::BeginTable((std::string("Layout" __VA_ARGS__) + ClassName()).c_str(), 2, ImGuiTableFlags_BordersInnerV | ImGuiTableFlags_PreciseWidths | ImGuiTableFlags_NoSavedSettings)) \
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
	if (ImGui::SliderInt("##" label, &value, min, max, "%d", ImGuiSliderFlags_AlwaysClamp | ImGuiSliderFlags_ClampOnInput)) \
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

#define D_H_OPTION_DRAW_COLOR3(label, tag, variable, hdr) \
{ \
	ImGui::Text(label); \
	ImGui::SameLine(inputOffset); \
	float* value = variable.GetPtr(); \
	ImGui::PushItemWidth(inputWidth); \
	static ImGuiColorEditFlags flag = ImGuiColorEditFlags_Float; \
	if(hdr) flag |= ImGuiColorEditFlags_HDR; \
	if(ImGui::ColorEdit3("##" label, value, flag)) \
	{ \
		variable = D_MATH::Color(value[0], value[1], value[2]); \
		options[tag] = {value[0], value[1], value[2]}; \
		settingsChanged = true; \
	} \
	ImGui::PopItemWidth(); \
} \

#define D_H_OPTION_DRAW_COLOR4(label, tag, variable, hdr) \
{ \
	ImGui::Text(label); \
	ImGui::SameLine(inputOffset); \
	float* value = variable.GetPtr(); \
	ImGui::PushItemWidth(inputWidth); \
	static ImGuiColorEditFlags flag = ImGuiColorEditFlags_Float | ImGuiColorEditFlags_AlphaBar | ImGuiColorEditFlags_AlphaPreview; \
	if(hdr) flag |= ImGuiColorEditFlags_HDR; \
	if(ImGui::ColorEdit4("##" label, value, flag)) \
	{ \
		variable = D_MATH::Color(value[0], value[1], value[2], value[3]); \
		options[tag] = {value[0], value[1], value[2], value[3]}; \
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

#endif // _D_EDITOR

// Subsystem Settings Loader
#define D_H_OPTIONS_LOAD_BASIC(key, variable) \
if(settings.contains(key)) variable = settings.at(key);

// Subsystem Settings Loader
#define D_H_OPTIONS_LOAD_BASIC_DEFAULT(key, variable, defaultValue) \
D_H_OPTIONS_LOAD_BASIC(key, variable) else variable = defaultValue;

#define D_H_OPTIONS_LOAD_COLOR(key, variable) \
if(settings.contains(key)) \
{ \
	D_CONTAINERS::DVector<float> vec;\
	settings[key].get_to(vec); \
	variable = D_MATH::Color(vec[0], vec[1], vec[2]); \
}

#define D_H_OPTIONS_LOAD_COLOR_DEFAULT(key, variable, defaultValue) \
D_H_OPTIONS_LOAD_COLOR(key, variable) else variable = defaultValue;

#define D_H_OPTIONS_LOAD_VEC3(key, variable) \
if(settings.contains(key)) \
{ \
	D_CONTAINERS::DVector<float> vec;\
	settings[key].get_to(vec); \
	variable = D_MATH::Vector3(vec[0], vec[1], vec[2]); \
}

#define D_H_OPTIONS_LOAD_VEC3_DEFAULT(key, variable, defaultValue) \
D_H_OPTIONS_LOAD_VEC3(key, variable) else variable = defaultValue;


/////////////////////////////////
//// Code Generation ////////////
/////////////////////////////////
#define BODY_MACRO_COMBINE_INNER(A,B,C,D) A##B##C##D
#define BODY_MACRO_COMBINE(A,B,C,D) BODY_MACRO_COMBINE_INNER(A,B,C,D)
#if CODEGEN_BUILD
#define GENERATED_BODY() class DClass() __CodeGenIdentifier__;
#else
#define GENERATED_BODY() BODY_MACRO_COMBINE(CURRENT_FILE_ID,_,__LINE__,_GENERATED_BODY)
#endif


struct NonCopyable
{
	NonCopyable() = default;
	NonCopyable(const NonCopyable&) = delete;
	NonCopyable& operator=(const NonCopyable&) = delete;
};