#include "Editor/pch.hpp"
#include "Window.hpp"

#include <imgui.h>

#include "Window.sgenerated.hpp"

namespace Darius::Editor::Gui::Windows
{

	Window::Window(D_SERIALIZATION::Json& settings):
		mWidth(1.f),
		mHeight(1.f),
		mPosX(0.f),
		mPosY(0.f),
		mHovered(false),
		mFocused(false),
		mConfig(settings)
	{
		D_H_OPTIONS_LOAD_BASIC_DEFAULT("Opened", mOpened, false);
	}

	void Window::PrepareGUI()
	{
		auto min = ImGui::GetWindowContentRegionMin();
		auto max = ImGui::GetWindowContentRegionMax();
		mContentMinX = min.x;
		mContentMinY = min.y;
		mWidth = std::max(max.x - min.x, 1.f);
		mHeight = std::max(max.y - min.y, 1.f);
		auto pos = ImGui::GetWindowPos();
		mPosX = pos.x;
		mPosY = pos.y;

		mFocused = ImGui::IsWindowFocused();
		mHovered = ImGui::IsWindowHovered();
		mAppearing = !ImGui::IsWindowCollapsed();
	}

	RECT Window::GetRect() const
	{
		float contentPosX = mPosX + mContentMinX;
		float contentPosY = mPosY + mContentMinY;

		return {(long)contentPosX, (long)contentPosY, (long)(contentPosX + mWidth), (long)(contentPosY + mHeight)};
	}

	bool Window::ReadFloatConfig(std::string const& key, float& output)
	{
		if (!mConfig.contains(key))
			return false;
		auto& record = mConfig.at(key);
		D_ASSERT(record.is_number_float());
		output = record;
		return true;
	}

	bool Window::ReadBoolConfig(std::string const& key, bool& output)
	{
		if (!mConfig.contains(key))
			return false;
		auto& record = mConfig.at(key);
		D_ASSERT(record.is_boolean());
		output = record;
		return true;
	}

	bool Window::ReadIntConfig(std::string const& key, int& output)
	{
		if (!mConfig.contains(key))
			return false;
		auto& record = mConfig.at(key);
		D_ASSERT(record.is_number_integer());
		output = record;
		return true;
	}

	bool Window::ReadUintConfig(std::string const& key, UINT& output)
	{
		if (!mConfig.contains(key))
			return false;
		auto& record = mConfig.at(key);
		D_ASSERT(record.is_number_unsigned());
		output = record;
		return true;
	}

	bool Window::ReadVector2Config(std::string const& key, D_MATH::Vector2& output)
	{
		if (!mConfig.contains(key))
			return false;
		auto& record = mConfig.at(key);
		D_ASSERT(record.is_array() && record.size() == 2);
		D_ASSERT(record[0].is_number_float());
		D_ASSERT(record[1].is_number_float());
		output.SetX(record[0]);
		output.SetY(record[1]);
		return true;
	}

	bool Window::ReadVector3Config(std::string const& key, D_MATH::Vector3& output)
	{
		if (!mConfig.contains(key))
			return false;
		auto& record = mConfig.at(key);
		D_ASSERT(record.is_array() && record.size() == 3);
		D_ASSERT(record[0].is_number_float());
		D_ASSERT(record[1].is_number_float());
		D_ASSERT(record[2].is_number_float());
		output.SetX(record[0]);
		output.SetY(record[1]);
		output.SetZ(record[2]);
		return true;
	}

	bool Window::ReadVector4Config(std::string const& key, D_MATH::Vector4& output)
	{
		if (!mConfig.contains(key))
			return false;
		auto& record = mConfig.at(key);
		D_ASSERT(record.is_array() && record.size() == 4);
		D_ASSERT(record[0].is_number_float());
		D_ASSERT(record[1].is_number_float());
		D_ASSERT(record[2].is_number_float());
		D_ASSERT(record[3].is_number_float());
		output.SetX(record[0]);
		output.SetY(record[1]);
		output.SetZ(record[2]);
		output.SetW(record[3]);
		return true;
	}

	bool Window::ReadColorConfig(std::string const& key, D_MATH::Color& output)
	{
		if (!mConfig.contains(key))
			return false;
		auto& record = mConfig.at(key);
		D_ASSERT(record.is_array() && record.size() == 4);
		D_ASSERT(record[0].is_number_float());
		D_ASSERT(record[1].is_number_float());
		D_ASSERT(record[2].is_number_float());
		D_ASSERT(record[3].is_number_float());
		output.SetR(record[0]);
		output.SetG(record[1]);
		output.SetB(record[2]);
		output.SetA(record[3]);
		return true;

	}

	bool Window::ReadQuaternionConfig(std::string const& key, D_MATH::Quaternion& output)
	{
		if (!mConfig.contains(key))
			return false;
		auto& record = mConfig.at(key);
		D_ASSERT(record.is_array() && record.size() == 4);
		D_ASSERT(record[0].is_number_float());
		D_ASSERT(record[1].is_number_float());
		D_ASSERT(record[2].is_number_float());
		D_ASSERT(record[3].is_number_float());
		output.SetX(record[0]);
		output.SetY(record[1]);
		output.SetZ(record[2]);
		output.SetW(record[3]);
		return true;

	}

	void Window::WriteFloatConfig(std::string const& key, float input)
	{
		mConfig[key] = input;
	}

	void Window::WriteBoolConfig(std::string const& key, bool input)
	{
		mConfig[key] = input;
	}

	void Window::WriteIntConfig(std::string const& key, int input)
	{
		mConfig[key] = input;
	}

	void Window::WriteUintConfig(std::string const& key, UINT input)
	{
		mConfig[key] = input;
	}

	void Window::WriteVector2Config(std::string const& key, D_MATH::Vector2 const& input)
	{
		mConfig[key] = { input.GetX(), input.GetY() };
	}

	void Window::WriteVector3Config(std::string const& key, D_MATH::Vector3 const& input)
	{
		mConfig[key] = { input.GetX(), input.GetY(), input.GetZ() };
	}

	void Window::WriteVector4Config(std::string const& key, D_MATH::Vector4 const& input)
	{
		mConfig[key] = { input.GetX(), input.GetY(), input.GetZ(), input.GetW() };
	}

	void Window::WriteColorConfig(std::string const& key, D_MATH::Color const& input)
	{
		mConfig[key] = { input.GetR(), input.GetG(), input.GetB(), input.GetA() };
	}

	void Window::WriteQuaternionConfig(std::string const& key, D_MATH::Quaternion const& input)
	{
		mConfig[key] = { input.GetX(), input.GetY(), input.GetZ(), input.GetW() };
	}

}