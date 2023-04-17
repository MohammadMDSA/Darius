#include "Editor/pch.hpp"
#include "DetailDrawer.hpp"

#include <Core/Containers/Map.hpp>
#include <Core/Containers/Vector.hpp>
#include <Utils/Assert.hpp>

#include <imgui.h>
#include <Libs/FontIcon/IconsFontAwesome6.h>
#include <rttr/enumeration.h>
#include <rttr/variant.h>

#include <sstream>

using namespace D_CONTAINERS;
using namespace rttr;

#define LABEL_COL_RATIO 0.4f

#define DRAW_LABEL() \
			ImGui::Text(nameStr.c_str()); \
			ImGui::SameLine(availWidth * LABEL_COL_RATIO);\

namespace Darius::Editor::Gui::DetailDrawer
{
	bool											_initialied = false;
	DUnorderedMap<rttr::type, std::function<bool(std::string const&, instance, PropertyChangeCallback)>>	CustomDrawers;

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
		CustomDrawers[type::get<bool>()] = [](std::string const& label, instance var, PropertyChangeCallback callback)
		{
			bool changed = false;
			auto p = var.try_convert<bool>();
			auto value = *p;
			ImGui::PushItemWidth(-1.f);
			if (ImGui::Checkbox(("##" + label).c_str(), &value))
			{
				if (callback)
					callback(label, var);
				*p = value;
				changed = true;
			}
			return changed;
		};

		// Char
		CustomDrawers[type::get<char>()] = [](std::string const& label, instance var, PropertyChangeCallback callback)
		{
			bool changed = false;
			auto p = var.try_convert<char>();
			int value = *p - CHAR_MIN;
			ImGui::PushItemWidth(-1.f);
			std::stringstream ss;
			ss << "%d (";
			ss << (char)value;
			ss << ")";
			if (ImGui::SliderInt(("##" + label).c_str(), &value, 0, UCHAR_MAX, ss.str().c_str(), ImGuiSliderFlags_AlwaysClamp))
			{
				if (callback)
					callback(label, var);
				*p = (char)(value + CHAR_MIN);
				changed = true;
			}
			ImGui::PopItemWidth();
			return changed;
		};

		// int8_t
		CustomDrawers[type::get<int8_t>()] = [](std::string const& label, instance var, PropertyChangeCallback callback)
		{
			bool changed = false;
			auto p = var.try_convert<int8_t>();
			int value = *p;
			ImGui::PushItemWidth(-1.f);
			if (ImGui::DragInt(("##" + label).c_str(), &value, 1.f, INT8_MIN, INT8_MAX, "%d", ImGuiSliderFlags_AlwaysClamp))
			{
				if (callback)
					callback(label, var);
				*p = (int8_t)value;
				changed = true;
			}
			ImGui::PopItemWidth();
			return changed;
		};

		// int16_t
		CustomDrawers[type::get<int16_t>()] = [](std::string const& label, instance var, PropertyChangeCallback callback)
		{
			bool changed = false;
			auto p = var.try_convert<int16_t>();
			int value = *p;
			ImGui::PushItemWidth(-1.f);
			if (ImGui::DragInt(("##" + label).c_str(), &value, 1.f, INT16_MIN, INT16_MAX, "%d", ImGuiSliderFlags_AlwaysClamp))
			{
				if (callback)
					callback(label, var);
				*p = (int16_t)value;
				changed = true;
			}
			ImGui::PopItemWidth();
			return changed;
		};

		// int32_t
		CustomDrawers[type::get<int32_t>()] = [](std::string const& label, instance var, PropertyChangeCallback callback)
		{
			bool changed = false;
			auto p = var.try_convert<int32_t>();
			auto value = *p;
			ImGui::PushItemWidth(-1.f);
			if (ImGui::DragInt(("##" + label).c_str(), &value, 1.f, INT32_MIN, INT32_MAX, "%d", ImGuiSliderFlags_AlwaysClamp))
			{
				if (callback)
					callback(label, var);
				*p = value;
				changed = true;
			}
			ImGui::PopItemWidth();
			return changed;
		};

		// int64_t
		CustomDrawers[type::get<int64_t>()] = [](std::string const& label, instance var, PropertyChangeCallback callback)
		{
			bool changed = false;
			auto p = var.try_convert<int64_t>();
			auto value = (int)*p;
			ImGui::PushItemWidth(-15.f);
			if (ImGui::DragInt(("##" + label).c_str(), &value, 1.f, INT32_MIN, INT32_MAX, "%d", ImGuiSliderFlags_AlwaysClamp))
			{
				if (callback)
					callback(label, var);
				*p = (int64_t)value;
				changed = true;
			}
			ImGui::PopItemWidth();
			ImGui::SameLine();
			WarningMarker("This will behave unexpectedly out of int32 range");
			return changed;
		};

		// uint8_t
		CustomDrawers[type::get<uint8_t>()] = [](std::string const& label, instance var, PropertyChangeCallback callback)
		{
			bool changed = false;
			auto p = var.try_convert<uint8_t>();
			auto value = (int)*p;
			ImGui::PushItemWidth(-1.f);
			if (ImGui::DragInt(("##" + label).c_str(), &value, 1.f, 0, UINT8_MAX, "%d", ImGuiSliderFlags_AlwaysClamp))
			{
				if (callback)
					callback(label, var);
				*p = (uint8_t)value;
				changed = true;
			}
			ImGui::PopItemWidth();
			return changed;
		};

		// uint16_t
		CustomDrawers[type::get<uint16_t>()] = [](std::string const& label, instance var, PropertyChangeCallback callback)
		{
			bool changed = false;
			auto p = var.try_convert<uint16_t>();
			auto value = (int)*p;
			ImGui::PushItemWidth(-1.f);
			if (ImGui::DragInt(("##" + label).c_str(), &value, 1.f, 0, UINT16_MAX, "%d", ImGuiSliderFlags_AlwaysClamp))
			{
				if (callback)
					callback(label, var);
				*p = (uint16_t)value;
				changed = true;
			}
			ImGui::PopItemWidth();
			return changed;
		};

		// uint32_t
		CustomDrawers[type::get<uint32_t>()] = [](std::string const& label, instance var, PropertyChangeCallback callback)
		{
			bool changed = false;
			auto p = var.try_convert<uint32_t>();
			auto value = (int)*p;
			ImGui::PushItemWidth(-15.f);
			if (ImGui::DragInt(("##" + label).c_str(), &value, 1.f, 0, INT32_MAX, "%d", ImGuiSliderFlags_AlwaysClamp))
			{
				if (callback)
					callback(label, var);
				*p = (uint32_t)value;
				changed = true;
			}
			ImGui::PopItemWidth();
			ImGui::SameLine();
			WarningMarker("This will behave unexpectedly out of int32 range");
			return changed;
		};

		// uint64_t
		CustomDrawers[type::get<uint64_t>()] = [](std::string const& label, instance var, PropertyChangeCallback callback)
		{
			bool changed = false;
			auto p = var.try_convert<uint64_t>();
			auto value = (int)*p;
			ImGui::PushItemWidth(-15.f);
			if (ImGui::DragInt(("##" + label).c_str(), &value, 1.f, 0, INT32_MAX, "%d", ImGuiSliderFlags_AlwaysClamp))
			{
				if (callback)
					callback(label, var);
				*p = (uint64_t)value;
				changed = true;
			}
			ImGui::PopItemWidth();
			ImGui::SameLine();
			WarningMarker("This will behave unexpectedly out of int32 range");
			return changed;
		};

		// float
		CustomDrawers[type::get<float>()] = [](std::string const& label, instance var, PropertyChangeCallback callback)
		{
			bool changed = false;
			auto p = var.try_convert<float>();
			auto value = *p;
			ImGui::PushItemWidth(-1.f);
			if (ImGui::DragFloat(("##" + label).c_str(), &value, 0.01f, FLT_MIN, FLT_MAX, "%.3f", ImGuiSliderFlags_AlwaysClamp))
			{
				if (callback)
					callback(label, var);
				*p = (float)value;
				changed = true;
			}
			ImGui::PopItemWidth();
			return changed;
		};

		// double
		CustomDrawers[type::get<double>()] = [](std::string const& label, instance var, PropertyChangeCallback callback)
		{
			bool changed = false;
			auto p = var.try_convert<double>();
			auto value = (float)*p;
			ImGui::PushItemWidth(-15.f);
			if (ImGui::DragFloat(("##" + label).c_str(), &value, 0.01f, FLT_MIN, FLT_MAX, "%.3f", ImGuiSliderFlags_AlwaysClamp))
			{
				if (callback)
					callback(label, var);
				*p = (double)value;
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

	bool InternalDrawDetials(std::string const& label, rttr::instance ins, rttr::type const& type, PropertyChangeCallback callback)
	{
		//// Checking existing serializers and deserializers
		if (CustomDrawers.contains(type))
		{
			return CustomDrawers[type](label, ins, callback);
		}

		return DrawRecursively(label, ins, callback);
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
		bool valueChanged = false;

		ImGui::Indent(50.f);
		ImGui::NewLine();
		ImGui::BeginGroup();
		ImGui::Separator();

		int index = 0;
		for (auto& item : view)
		{
			if (index > 0)
				ImGui::Separator();

			auto propLabel = label + "." + std::to_string(index);

			bool elChanged = false;

			variant wrappedVar = item.extract_wrapped_value();
			type valueType = wrappedVar.get_type();

			if (CustomDrawers.contains(valueType))
			{
				if (CustomDrawers[valueType](propLabel, wrappedVar, callback))
				{
					valueChanged = true;
					elChanged = true;
				}
			}
			else if (item.is_sequential_container())
			{
				auto arrView = item.create_sequential_view();
				if (DrawArray(propLabel, arrView, callback))
				{
					valueChanged = true;
					elChanged = true;
					view.set_value(index, item.extract_wrapped_value());
				}
			}
			else
			{
				if (valueType == type::get<std::string>() || valueType.is_enumeration())
				{
					if (DrawAtomicTypes(propLabel, valueType, wrappedVar, callback))
					{
						valueChanged = true;
						elChanged = true;
					}
				}
				else // object
				{
					if (DrawRecursively(propLabel, wrappedVar, callback))
					{
						valueChanged = true;
						elChanged = true;
					}
				}

				if (elChanged)
					view.set_value(index, wrappedVar);
			}
			index++;
		}


		ImGui::Separator();
		ImGui::EndGroup();
		ImGui::Unindent(50.f);

		return valueChanged;
	}

	// Associative containers are not supported by default
	bool DrawAssociativeContainer(std::string const& label, variant_associative_view& view, PropertyChangeCallback callback = nullptr)
	{
		return false;
	}

	bool DrawRecursively(std::string const& label, rttr::instance ins, PropertyChangeCallback callback)
	{
		instance obj = ins.get_type().get_raw_type().is_wrapper() ? ins.get_wrapped_instance() : ins;

		auto availWidth = ImGui::GetContentRegionAvail().x;

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

			auto propLabel = label.length() > 0 ? label + "." + nameStr : nameStr;

			const type valueType = prop.get_type();

			// Draw with custom drawer if exists
			if (CustomDrawers.contains(valueType))
			{
				DRAW_LABEL();
				if (CustomDrawers[valueType](propLabel, propValue, callback))
				{
					prop.set_value(obj, propValue);
					valueChanged = true;
				}
				continue;
			}
			// Draw sequential containers
			else if (valueType.is_sequential_container())
			{
				DRAW_LABEL();
				auto seqView = propValue.create_sequential_view();
				if (DrawArray(propLabel, seqView, callback))
				{
					prop.set_value(obj, propValue);
				}
			}
			// Draw associative containers
			else if (valueType.is_associative_container())
			{
				// Not supported
				continue;
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
					continue;
				}
				// Draw object
				else
				{
					DRAW_LABEL();
					ImGui::Indent(50.f);
					ImGui::NewLine();
					ImGui::BeginGroup();
					ImGui::Separator();
					if (DrawRecursively(propLabel, propValue, callback))
					{
						bool done = prop.set_value(obj, propValue);
						(done);
						valueChanged = true;
					}
					ImGui::Separator();
					ImGui::EndGroup();
					ImGui::Unindent(50.f);
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
		return InternalDrawDetials("", ins, ins.get_type(), callback);
	}

}
