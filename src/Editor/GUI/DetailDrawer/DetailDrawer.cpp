#include "Editor/pch.hpp"
#include "DetailDrawer.hpp"

#include <Core/Containers/Map.hpp>
#include <Core/Containers/Vector.hpp>
#include <Utils/Assert.hpp>

#include <imgui.h>
#include <Libs/FontIcon/IconsFontAwesome6.h>
#include <rttr/enumeration.h>

using namespace D_CONTAINERS;
using namespace rttr;

namespace Darius::Editor::Gui::DetailDrawer
{
	bool											_initialied = false;
	DUnorderedMap<rttr::type, std::function<bool(std::string const& label, variant&, PropertyChangeCallback)>>	CustomDrawers;

	bool DrawRecursively(std::string const& label, rttr::instance obj, PropertyChangeCallback callback = nullptr);

	void WarningMarker(const char* desc)
	{

		ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.f, 1.f, 0.f, 1.f));
		ImGui::TextDisabled(ICON_FA_TRIANGLE_EXCLAMATION);
		ImGui::PopStyleColor();
		if (ImGui::IsItemHovered(ImGuiHoveredFlags_DelayShort))
		{
			ImGui::BeginTooltip();
			ImGui::PushTextWrapPos(ImGui::GetFontSize() * 35.0f);
			ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.f, 0.f, 0.f, 1.f));
			ImGui::TextUnformatted(desc);
			ImGui::PopStyleColor();
			ImGui::PopTextWrapPos();
			ImGui::EndTooltip();
		}
	}

	void Initialize()
	{
		D_ASSERT(!_initialied);
		_initialied = true;

		// Bool
		CustomDrawers[type::get<bool>()] = [](std::string const& label, variant& var, PropertyChangeCallback callback)
		{
			bool changed = false;
			bool value = var.to_bool();
			ImGui::PushItemWidth(-1.f);
			if (ImGui::Checkbox(("##" + label).c_str(), &value))
			{
				if (callback)
					callback(label, var);
				var = value;
				changed = true;
			}
			return changed;
		};

		// Char
		CustomDrawers[type::get<char>()] = [](std::string const& label, variant& var, PropertyChangeCallback callback)
		{
			bool changed = false;
			char c[] = { 0, 0 };
			c[0] = var.get_value<char>();
			ImGui::PushItemWidth(-1.f);
			if (ImGui::InputText(("##" + label).c_str(), c, 1))
			{
				if (callback)
					callback(label, var);
				var = c[0];
				changed = true;
			}
			ImGui::PopItemWidth();
			return changed;
		};

		// int8_t
		CustomDrawers[type::get<int8_t>()] = [](std::string const& label, variant& var, PropertyChangeCallback callback)
		{
			bool changed = false;
			int value = var.to_int8();
			ImGui::PushItemWidth(-1.f);
			if (ImGui::DragInt(("##" + label).c_str(), &value, 1.f, INT8_MIN, INT8_MAX, "%d", ImGuiSliderFlags_AlwaysClamp))
			{
				if (callback)
					callback(label, var);
				var = (int8_t)value;
				changed = true;
			}
			ImGui::PopItemWidth();
			return changed;
		};

		// int16_t
		CustomDrawers[type::get<int16_t>()] = [](std::string const& label, variant& var, PropertyChangeCallback callback)
		{
			bool changed = false;
			int value = var.to_int16();
			ImGui::PushItemWidth(-1.f);
			if (ImGui::DragInt(("##" + label).c_str(), &value, 1.f, INT16_MIN, INT16_MAX, "%d", ImGuiSliderFlags_AlwaysClamp))
			{
				if (callback)
					callback(label, var);
				var = (int16_t)value;
				changed = true;
			}
			ImGui::PopItemWidth();
			return changed;
		};

		// int32_t
		CustomDrawers[type::get<int32_t>()] = [](std::string const& label, variant& var, PropertyChangeCallback callback)
		{
			bool changed = false;
			int value = var.to_int32();
			ImGui::PushItemWidth(-1.f);
			if (ImGui::DragInt(("##" + label).c_str(), &value, 1.f, INT32_MIN, INT32_MAX, "%d", ImGuiSliderFlags_AlwaysClamp))
			{
				if (callback)
					callback(label, var);
				var = (int32_t)value;
				changed = true;
			}
			ImGui::PopItemWidth();
			return changed;
		};

		// int64_t
		CustomDrawers[type::get<int64_t>()] = [](std::string const& label, variant& var, PropertyChangeCallback callback)
		{
			bool changed = false;
			int value = (int)var.to_int64();
			ImGui::PushItemWidth(-15.f);
			if (ImGui::DragInt(("##" + label).c_str(), &value, 1.f, INT32_MIN, INT32_MAX, "%d", ImGuiSliderFlags_AlwaysClamp))
			{
				if (callback)
					callback(label, var);
				var = (int64_t)value;
				changed = true;
			}
			ImGui::PopItemWidth();
			ImGui::SameLine();
			WarningMarker("This will behave unexpectedly out of int32 range");
			return changed;
		};

		// uint8_t
		CustomDrawers[type::get<uint8_t>()] = [](std::string const& label, variant& var, PropertyChangeCallback callback)
		{
			bool changed = false;
			int value = (int)var.to_uint8();
			ImGui::PushItemWidth(-1.f);
			if (ImGui::DragInt(("##" + label).c_str(), &value, 1.f, 0, UINT8_MAX, "%d", ImGuiSliderFlags_AlwaysClamp))
			{
				if (callback)
					callback(label, var);
				var = (uint8_t)value;
				changed = true;
			}
			ImGui::PopItemWidth();
			return changed;
		};

		// uint16_t
		CustomDrawers[type::get<uint16_t>()] = [](std::string const& label, variant& var, PropertyChangeCallback callback)
		{
			bool changed = false;
			int value = (int)var.to_uint16();
			ImGui::PushItemWidth(-1.f);
			if (ImGui::DragInt(("##" + label).c_str(), &value, 1.f, 0, UINT16_MAX, "%d", ImGuiSliderFlags_AlwaysClamp))
			{
				if (callback)
					callback(label, var);
				var = (uint16_t)value;
				changed = true;
			}
			ImGui::PopItemWidth();
			return changed;
		};

		// uint32_t
		CustomDrawers[type::get<uint32_t>()] = [](std::string const& label, variant& var, PropertyChangeCallback callback)
		{
			bool changed = false;
			int value = (int)var.to_uint32();
			ImGui::PushItemWidth(-15.f);
			if (ImGui::DragInt(("##" + label).c_str(), &value, 1.f, 0, INT32_MAX, "%d", ImGuiSliderFlags_AlwaysClamp))
			{
				if (callback)
					callback(label, var);
				var = (uint32_t)value;
				changed = true;
			}
			ImGui::PopItemWidth();
			ImGui::SameLine();
			WarningMarker("This will behave unexpectedly out of int32 range");
			return changed;
		};

		// uint64_t
		CustomDrawers[type::get<uint64_t>()] = [](std::string const& label, variant& var, PropertyChangeCallback callback)
		{
			bool changed = false;
			int value = (int)var.to_uint64();
			ImGui::PushItemWidth(-15.f);
			if (ImGui::DragInt(("##" + label).c_str(), &value, 1.f, 0, INT32_MAX, "%d", ImGuiSliderFlags_AlwaysClamp))
			{
				if (callback)
					callback(label, var);
				var = (uint64_t)value;
				changed = true;
			}
			ImGui::PopItemWidth();
			ImGui::SameLine();
			WarningMarker("This will behave unexpectedly out of int32 range");
			return changed;
		};

		// float
		CustomDrawers[type::get<float>()] = [](std::string const& label, variant& var, PropertyChangeCallback callback)
		{
			bool changed = false;
			float value = var.to_float();
			ImGui::PushItemWidth(-1.f);
			if (ImGui::DragFloat(("##" + label).c_str(), &value, 0.01f, FLT_MIN, FLT_MAX, "%.3f", ImGuiSliderFlags_AlwaysClamp))
			{
				if (callback)
					callback(label, var);
				var = value;
				changed = true;
			}
			ImGui::PopItemWidth();
			return changed;
		};

		// double
		CustomDrawers[type::get<double>()] = [](std::string const& label, variant& var, PropertyChangeCallback callback)
		{
			bool changed = false;
			float value = (float)var.to_double();
			ImGui::PushItemWidth(-15.f);
			if (ImGui::DragFloat(("##" + label).c_str(), &value, 0.01f, FLT_MIN, FLT_MAX, "%.3f", ImGuiSliderFlags_AlwaysClamp))
			{
				if (callback)
					callback(label, var);
				var = (double)value;
				changed = true;
			}
			ImGui::PopItemWidth();
			ImGui::SameLine();
			WarningMarker("This will behave unexpectedly out of float range");
			return changed;
		};

	}

	void Shutdown()
	{
		D_ASSERT(_initialied);
	}

	bool DrawAtomicTypes(std::string const& label, type const& t, variant& var, PropertyChangeCallback callback = nullptr)
	{

		bool valueChanged = false;

		if (t.is_enumeration())
		{
			rttr::enumeration e = t.get_enumeration();

			DVector<std::string> values;

			if (ImGui::BeginCombo(("##" + label).c_str(), var.to_string().c_str()))
			{
				for (auto const& enumName : e.get_names())
				{
					auto enumValue = e.name_to_value(enumName);
					if (ImGui::Selectable(enumName.to_string().c_str(), enumValue == var))
					{
						if (callback)
							callback(label, var);
						var = enumValue;
						valueChanged = true;
					}
				}
				ImGui::EndCombo();
			}

			return valueChanged;
		}

		if (t == type::get<std::string>())
		{
			std::string curValue = var.to_string();
			char value[256];
			size_t curTextSize = curValue.size();

			size_t usedSize = std::min(sizeof(value), sizeof(char) * curTextSize);

			memcpy(value, curValue.data(), usedSize);
			ZeroMemory(value + usedSize, sizeof(value) - usedSize);

			if (ImGui::InputText(("##" + label).c_str(), value, 256))
			{
				if (callback)
					callback(label, var);
				var = std::string(value);
				valueChanged = true;
			}

			return valueChanged;
		}

		return valueChanged;
	}

	bool DrawArray(std::string const& label, variant_sequential_view& view, PropertyChangeCallback callback = nullptr)
	{
		return false;

		/*for (auto& item : view)
		{

		}*/
	}

	bool DrawAssociativeContainer(std::string const& label, variant_associative_view& view, PropertyChangeCallback callback = nullptr)
	{
		return false;
	}

	bool DrawRecursively(std::string const& label, rttr::instance ins, PropertyChangeCallback callback)
	{
		instance obj = ins.get_type().get_raw_type().is_wrapper() ? ins.get_wrapped_instance() : ins;

		auto availWidth = ImGui::GetContentRegionAvail().x;
		auto const static labelColRatio = 0.4f;

		bool valueChanged = false;

		auto propList = obj.get_derived_type().get_properties();
		for (auto prop : propList)
		{
			if (prop.get_metadata("NO_SERIALIZE"))
				continue;

			variant propValue = prop.get_value(obj);
			if (!propValue)
				continue; // cannot serialize, because we cannot retrieve the value

			const auto name = prop.get_name();
			auto nameStr = std::string(name);

#define DRAW_LABEL() \
			ImGui::Text(nameStr.c_str()); \
			ImGui::SameLine(availWidth * labelColRatio);\
			//ImGui::PushItemWidth(-availWidth * labelColRatio)

#define POST_ITEM_DRAW() \
			//ImGui::PopItemWidth();

			auto propLabel = label.length() > 0 ? label + "." + nameStr : nameStr;

			const type valueType = prop.get_type();

			// Draw with custom drawer if exists
			if (CustomDrawers.contains(valueType))
			{
				DRAW_LABEL();
				if (CustomDrawers[valueType](propLabel, propValue, callback))
				{
					bool done = prop.set_value(obj, propValue);
					(done);
					auto ff = propValue.to_int8();
					(ff);
					valueChanged = true;
				}
				POST_ITEM_DRAW();
				continue;
			}
			// Draw sequential containers
			else if (valueType.is_sequential_container())
			{
				DRAW_LABEL();

				POST_ITEM_DRAW();
			}
			// Draw associative containers
			else if (valueType.is_associative_container())
			{
				DRAW_LABEL();

				POST_ITEM_DRAW();
			}
			else
			{
				// Draw atomic types (enum, string) (I know string isn't atomic! However, it is considered one in rttr)
				if (valueType.is_enumeration() || valueType == type::get<std::string>())
				{
					DRAW_LABEL();
					if (DrawAtomicTypes(propLabel, valueType, propValue, callback))
					{
						bool done = prop.set_value(obj, propValue);
						(done);
						valueChanged = true;
					}
					POST_ITEM_DRAW();
					continue;
				}
				// Draw object
				else
				{
					DRAW_LABEL();
					ImGui::Indent(50.f);
					ImGui::PushItemWidth(-50.f);
					if (DrawRecursively(propLabel, propValue, callback))
					{
						bool done = prop.set_value(obj, propValue);
						(done);
						valueChanged = true;
					}
					ImGui::PopItemWidth();
					ImGui::Unindent(50.f);
					POST_ITEM_DRAW();
				}
			}
		}
		return valueChanged;
	}

	bool DefaultDetailDrawer(rttr::instance obj, PropertyChangeCallback callback)
	{
		return DrawRecursively("", obj, callback);
	}

	bool DrawDetials(rttr::instance ins, PropertyChangeCallback callback)
	{
		instance obj = ins.get_type().get_raw_type().is_wrapper() ? ins.get_wrapped_instance() : ins;

		type t = obj.get_type();
		// Checking existing serializers and deserializers
		if (CustomDrawers.contains(t))
		{
			variant& var = *obj.try_convert<rttr::variant>();
			return CustomDrawers[t]("", var, callback);
		}

		return DrawRecursively("", ins, callback);
	}

}
