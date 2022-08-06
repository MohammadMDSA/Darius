#pragma once

#define INLINE __forceinline
#define NODISCARD [[nodiscard]]
#define ALIGN_DECL(size) __declspec(align(size))
#define ALIGN_DECL_256 ALIGN_DECL(256)

#define D_NAMEOF(T) #T
#define D_NAMEOF_C(T) (#T).c_str()

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

#define D_CH_R_FIELD(type, name) D_CH_R_FIELD_ACC(type, name, private)

#define D_CH_R_CONST_FIELD(type, name) D_CH_FIELD_CONST_ACC(type, name, private)

#define D_CH_RW_FIELD_ACC(type, name, access) \
public: \
inline type const& Get##name() const { return m##name; } \
inline type Get##name() { return m##name; } \
inline void Set##name(type val) { this->m##name = val; } \
D_CH_FIELD_ACC(type, name, access)

#define D_CH_RW_FIELD(type, name) D_CH_RW_FIELD_ACC(type, name private)

#define D_CH_TYPE_NAME_GETTER(T) \
public: \
static INLINE std::string const GetTypeName() { return D_NAMEOF(T); }

/////////////////////////////Helpers
#define STR_WSTR(str) std::string(str.begin(), str.end())
#define WSTR_STR(str) std::wstring(str.begin(), str.end())

struct NonCopyable
{
    NonCopyable() = default;
    NonCopyable(const NonCopyable&) = delete;
    NonCopyable& operator=(const NonCopyable&) = delete;
};