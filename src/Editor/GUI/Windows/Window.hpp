#pragma once

#include <Graphics/CommandContext.hpp>
#include <Graphics/GraphicsUtils/Profiling/Profiling.hpp>
#include <Math/VectorMath.hpp>

#include "Window.generated.hpp"

namespace Darius::Editor::Gui::GuiManager
{
	void DrawGUI();
	void _DrawMenuBar();
}

#define D_CH_EDITOR_WINDOW_BODY_RAW(type, name) \
public: \
static INLINE std::string SGetName() { return name; } \
INLINE virtual std::string GetName() const { return SGetName(); }

#define D_CH_EDITOR_WINDOW_BODY(type, name) \
D_CH_EDITOR_WINDOW_BODY_RAW(type, name) \
public: \
type(D_SERIALIZATION::Json& config); \
~type();

namespace Darius::Editor::Gui::Windows
{
	class DClass(Serialize) Window
	{
		GENERATED_BODY();

	public:
		Window(D_SERIALIZATION::Json& config);
		virtual ~Window() = default;

		Window(Window const& other) = delete;

		virtual std::string 		GetName() const = 0;

		virtual INLINE void			Render() {};
		virtual void				Update(float dt) = 0;
		virtual void				DrawGUI() = 0;
		void						PrepareGUI();

		RECT						GetRect() const;

		bool						ReadFloatConfig(std::string const& key, float& output);
		bool						ReadBoolConfig(std::string const& key, bool& output);
		bool						ReadIntConfig(std::string const& key, int& output);
		bool						ReadUintConfig(std::string const& key, UINT& output);
		bool						ReadVector2Config(std::string const& key, D_MATH::Vector2& output);
		bool						ReadVector3Config(std::string const& key, D_MATH::Vector3& output);
		bool						ReadVector4Config(std::string const& key, D_MATH::Vector4& output);
		bool						ReadColorConfig(std::string const& key, D_MATH::Color& output);
		bool						ReadQuaternionConfig(std::string const& key, D_MATH::Quaternion& output);

		void						WriteFloatConfig(std::string const&, float input);
		void						WriteBoolConfig(std::string const&, bool input);
		void						WriteIntConfig(std::string const&, int input);
		void						WriteUintConfig(std::string const&, UINT input);
		void						WriteVector2Config(std::string const&, D_MATH::Vector2 const& input);
		void						WriteVector3Config(std::string const&, D_MATH::Vector3 const& input);
		void						WriteVector4Config(std::string const&, D_MATH::Vector4 const& input);
		void						WriteColorConfig(std::string const&, D_MATH::Color const& input);
		void						WriteQuaternionConfig(std::string const&, D_MATH::Quaternion const& input);

		INLINE bool					IsAppearing() const { return mAppearing; }

	protected:

		friend void Darius::Editor::Gui::GuiManager::DrawGUI();
		friend void Darius::Editor::Gui::GuiManager::_DrawMenuBar();

		DField(Get[inline], Set[inline])
		bool						mOpened;

		float						mPadding[2] = { 8.f, 8.f };

		float						mWidth;
		float						mHeight;

		float						mPosX;
		float						mPosY;

		float						mContentMinX;
		float						mContentMinY;

		D_SERIALIZATION::Json&		mConfig;

		uint8_t						mHovered : 1;
		uint8_t						mFocused : 1;
		uint8_t						mAppearing : 1;

	};

}

File_Window_GENERATED