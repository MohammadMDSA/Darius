#pragma once

#define INLINE __forceinline
#define NODISCARD [[nodiscard]]
#define ALIGN_DECL(size) __declspec(align(size))
#define ALIGN_DECL_256 ALIGN_DECL(256)
#define ALIGN_DECL_16 ALIGN_DECL(16)

#define D_NAMEOF(T) #T
#define D_NAMEOF_C(T) #T.c_str()

#define D_CH_FIELD_ACC(type, name, access) \
access: \
type m##name;

#define D_CH_FIELD_CONST_ACC(type, name, access) \
access: \
const type m##name;

#define D_CH_FIELD(type, name) D_CH_FIELD_ACC(type, name, private)
#define D_CH_FIELD_CONST(type, name) D_CH_FIELD_CONST_ACC(type, name, private)

#define D_CH_R_FIELD_ACC(type, name, access) \
public: \
inline type const& Get##name() const { return m##name; } \
inline type Get##name() { return m##name; } \
D_CH_FIELD_ACC(type, name, access)

#define D_CH_R_FIELD_CONST_ACC(type, name, access) \
public: \
inline type const& Get##name() const { return m##name; } \
inline type Get##name() { return m##name; } \
D_CH_FIELD_CONST_ACC(type, name, access)

#define D_CH_R_FIELD_CONST(type, name) D_CH_R_FIELD_CONST_ACC(type, name, private)

#define D_CH_R_FIELD(type, name) D_CH_R_FIELD_ACC(type, name, private)

#define D_CH_CONST_FIELD(type, name) D_CH_FIELD_CONST_ACC(type, name, private)

#define D_CH_RW_FIELD_ACC(type, name, access) \
public: \
inline type const& Get##name() const { return m##name; } \
inline type Get##name() { return m##name; } \
inline void Set##name(type const& val) { this->m##name = val; } \
D_CH_FIELD_ACC(type, name, access)

#define D_CH_RW_FIELD(type, name) D_CH_RW_FIELD_ACC(type, name, private)

#define D_CH_TYPE_NAME_GETTER(T) \
public: \
static INLINE std::string const GetTypeName() { return D_NAMEOF(T); }

///////////////////////////// Function Helpers
#define _IN_
#define _OUT_
#define _IN_OUT_

///////////////////////////// Helpers
#define STR_WSTR(str) std::string(str.begin(), str.end())
#define WSTR_STR(wstr) std::wstring(wstr.begin(), wstr.end())
#define D_H_CONCAT(A, B) A##B
#define D_H_UNIQUE_NAME(base) D_H_CONCAT(base, __COUNTER__)
#define D_H_UNIQUE_ID __COUNTER__
#define D_H_ENSURE_PATH(path) std::filesystem::exists(path)
#define D_H_ENSURE_DIR(path) (D_H_ENSURE_PATH(path) && std::filesystem::is_directory(path))
#define D_H_ENSURE_FILE(path) (D_H_ENSURE_PATH(path) && !std::filesystem::is_directory(path))

//////////////////////////// Details Draw

#ifdef _D_EDITOR

#define D_H_RESOURCE_SELECTION_DRAW(resourceType, prop, placeHolder, handleFunction) \
{ \
    resourceType* currentResource = prop.Get(); \
     \
    std::string cuurrentResourceName; \
    if(prop.IsValid()) \
    { \
        auto nameW = prop->GetName(); \
        cuurrentResourceName = STR_WSTR(nameW); \
    } \
    else \
        cuurrentResourceName = placeHolder; \
    auto availableSpace = ImGui::GetContentRegionAvail(); \
    auto selectorWidth = 20; \
    ImGui::Button(cuurrentResourceName.c_str(), ImVec2(availableSpace.x - 2 * selectorWidth - 10.f, 0)); \
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
            auto Name = STR_WSTR(prev.Name); \
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
#define D_H_RESOURCE_SELECTION_DRAW_DEF(type, name) D_H_RESOURCE_SELECTION_DRAW_SHORT(type, name, "Select " #name, Set##name)

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

#endif // _D_EDITOR


struct NonCopyable
{
    NonCopyable() = default;
    NonCopyable(const NonCopyable&) = delete;
    NonCopyable& operator=(const NonCopyable&) = delete;
};