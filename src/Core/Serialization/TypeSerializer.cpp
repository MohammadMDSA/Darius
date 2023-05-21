#include "Core/pch.hpp"

#include "TypeSerializer.hpp"

#include <Utils/Log.hpp>

using namespace rttr;

namespace Darius::Core::Serialization
{
	D_CONTAINERS::DUnorderedMap<rttr::type, std::pair<std::function<void(rttr::instance const&, Json&)>, std::function<void(rttr::variant&, Json const&)>>> typeSerializers = { };

	bool __RegisterSerializer(rttr::type type, std::function<void(rttr::instance const&, Json&)> serializer, std::function<void(rttr::variant&, Json const&)> deserializer)
	{
		if (typeSerializers.contains(type))
			return false;

		std::pair<std::function<void(rttr::instance const&, Json&)>, std::function<void(rttr::variant&, Json const&)>> serDesPair = { serializer, deserializer };

		typeSerializers.emplace(type, serDesPair);

		return true;
	}

	void to_json_recursively(rttr::instance const obj, Json& json);

	bool write_variant(const variant& var, Json& json);

	bool write_atomic_types_to_json(type const& t, variant const& var, Json& json)
	{
		if (t.is_arithmetic())
		{
			if (t == type::get<bool>())
				json = var.to_bool();
			else if (t == type::get<char>())
				json = var.get_value<char>();
			else if (t == type::get<int8_t>())
				json = var.to_int8();
			else if (t == type::get<int16_t>())
				json = var.to_int16();
			else if (t == type::get<int32_t>())
				json = var.to_int32();
			else if (t == type::get<int64_t>())
				json = var.to_int64();
			else if (t == type::get<uint8_t>())
				json = var.to_uint8();
			else if (t == type::get<uint16_t>())
				json = var.to_uint16();
			else if (t == type::get<uint32_t>())
				json = var.to_uint32();
			else if (t == type::get<uint64_t>())
				json = var.to_uint64();
			else if (t == type::get<float>())
				json = var.to_float();
			else if (t == type::get<double>())
				json = var.to_double();

			return true;
		}
		else if (t.is_enumeration())
		{
			bool ok = false;
			auto result = var.to_string(&ok);
			if (ok)
			{
				json = var.to_string();
			}
			else
			{
				ok = false;
				auto value = var.to_uint64(&ok);
				if (ok)
					json = value;
				else
					json = nullptr;
			}

			return true;
		}
		else if (t == type::get<std::string>())
		{
			json = var.to_string();
			return true;
		}

		return false;
	}

	void write_array(variant_sequential_view const& view, Json& json)
	{
		for (const auto& item : view)
		{
			json.push_back(Json());
			auto& el = json.back();
			if (item.is_sequential_container())
			{
				write_array(item.create_sequential_view(), el);
			}
			else
			{
				variant wrapped_var = item.extract_wrapped_value();
				type value_type = wrapped_var.get_type();

				auto wrapped_type = value_type.is_wrapper() ? value_type.get_wrapped_type() : value_type;
				bool is_wrapper = wrapped_type != value_type;
				auto intendedType = is_wrapper ? wrapped_type : value_type;
				// Checking existing serializers and deserializers
				if (typeSerializers.contains(intendedType))
				{
					typeSerializers[intendedType].first(is_wrapper ? wrapped_var.extract_wrapped_value() : wrapped_var, el);
				}
				else if (value_type.is_arithmetic() || value_type == type::get<std::string>() || value_type.is_enumeration())
				{
					write_atomic_types_to_json(value_type, wrapped_var, el);
				}
				else // object
				{
					to_json_recursively(wrapped_var, el);
				}
			}
		}
	}


	/////////////////////////////////////////////////////////////////////////////////////////

	void write_associative_container(variant_associative_view const& view, Json& json)
	{
		static const string_view key_name("key");
		static const string_view value_name("value");


		if (view.is_key_only_type())
		{
			for (auto& item : view)
			{
				json.push_back(Json());
				write_variant(item.first, json.back());
			}
		}
		else
		{
			for (auto& item : view)
			{
				json.push_back(Json());
				auto& obj = json.back();

				auto keyNameStr = std::string(key_name.data());
				write_variant(item.first, obj[keyNameStr]);

				auto valueNameStr = std::string(value_name.data());
				write_variant(item.second, obj[valueNameStr]);
			}
		}
	}

	bool write_variant(variant const& var, Json& json)
	{
		auto value_type = var.get_type();
		auto wrapped_type = value_type.is_wrapper() ? value_type.get_wrapped_type() : value_type;
		bool is_wrapper = wrapped_type != value_type;

		auto intendedType = is_wrapper ? wrapped_type : value_type;

		// Checking existing serializers and deserializers
		if (typeSerializers.contains(intendedType))
		{
			typeSerializers[intendedType].first(is_wrapper ? var.extract_wrapped_value() : var, json);
			return true;
		}

		if (write_atomic_types_to_json(intendedType,
			is_wrapper ? var.extract_wrapped_value() : var, json))
		{
		}
		else if (var.is_sequential_container())
		{
			write_array(var.create_sequential_view(), json);
		}
		else if (var.is_associative_container())
		{
			write_associative_container(var.create_associative_view(), json);
		}
		else
		{
			auto child_props = is_wrapper ? wrapped_type.get_properties() : value_type.get_properties();
			if (!child_props.empty())
			{
				to_json_recursively(var, json);
			}
			else
			{
				bool ok = false;
				auto text = var.to_string(&ok);
				if (!ok)
				{
					json = text;
					return false;
				}

				json = text;
			}
		}

		return true;
	}

	void to_json_recursively(rttr::instance const ins, Json& json)
	{

		instance obj = ins.get_type().get_raw_type().is_wrapper() ? ins.get_wrapped_instance() : ins;

		type t = obj.get_type();
		// Checking existing serializers and deserializers
		if (typeSerializers.contains(t))
		{
			typeSerializers[t].first(obj, json);
			return;
		}

		auto prop_list = obj.get_derived_type().get_properties();
		for (auto prop : prop_list)
		{
			if (prop.get_metadata("NO_SERIALIZE"))
				continue;

			variant prop_value = prop.get_value(obj);
			if (!prop_value)
				continue; // cannot serialize, because we cannot retrieve the value

			const auto name = prop.get_name();
			auto nameStr = std::string(name);

			if (!write_variant(prop_value, json[nameStr]))
			{
				D_LOG_ERROR("cannot serialize property: " << name);
			}
		}

	}

	void Serialize(rttr::instance const obj, Json& json)
	{

		if (!obj.is_valid())
			return;

		to_json_recursively(obj, json);
	}

	void SerializeSequentialContainer(rttr::variant const& var, Json& json)
	{
		if (!var.is_sequential_container())
			return;

		write_array(var.create_sequential_view(), json);
	}

	/////////////////////////////////////////////////////////////////////////////////////////
	/////////////////////////////////////////////////////////////////////////////////////////
	///////////////////////////       Deserializer       ////////////////////////////////////
	/////////////////////////////////////////////////////////////////////////////////////////
	/////////////////////////////////////////////////////////////////////////////////////////
	/////////////////////////////////////////////////////////////////////////////////////////


	/////////////////////////////////////////////////////////////////////////////////////////

	void fromjson_recursively(instance obj, Json const& json_object);

	/////////////////////////////////////////////////////////////////////////////////////////

	variant extract_basic_types(Json const& json_value)
	{
		if (json_value.is_string())
			return std::string(json_value.get<std::string>());

		else if (json_value.is_null())
			return variant();

		else if (json_value.is_boolean())
			return json_value.get<bool>();

		else if (json_value.is_number())
		{
			if (json_value.is_number_integer())
				return json_value.get<INT64>();
			else if (json_value.is_number_float())
				return json_value.get<float>();
			else if (json_value.is_number_unsigned())
				return json_value.get<UINT64>();
			return variant();
		}

		// we handle only the basic types here
		else if (json_value.is_object() || json_value.is_array())
			return variant();

		return variant();
	}


	/////////////////////////////////////////////////////////////////////////////////////////

	static void write_array_recursively(variant_sequential_view& view, Json const& json_array_value)
	{
		view.set_size(json_array_value.size());
		const type array_value_type = view.get_rank_type(1);

		for (size_t i = 0; i < json_array_value.size(); ++i)
		{
			auto& json_index_value = json_array_value[i];
			if (json_index_value.is_array())
			{
				auto sub_array_view = view.get_value(i).create_sequential_view();
				write_array_recursively(sub_array_view, json_index_value);
			}
			else if (json_index_value.is_object())
			{
				variant var_tmp = view.get_value(i);
				variant wrapped_var = var_tmp.extract_wrapped_value();
				fromjson_recursively(wrapped_var, json_index_value);
				view.set_value(i, wrapped_var);
			}
			else
			{
				const type intendedType = array_value_type.is_wrapper() ? array_value_type.get_wrapped_type() : array_value_type;
				if (typeSerializers.contains(intendedType))
				{
					variant v;
					typeSerializers[intendedType].second(v, json_index_value);
					v.convert(array_value_type);
					view.set_value(i, v);
				}
				variant extracted_value = extract_basic_types(json_index_value);
				if (extracted_value.convert(intendedType))
					view.set_value(i, extracted_value);
			}
		}
	}

	variant extract_value(Json const& json_value, const type& t)
	{
		variant extracted_value = extract_basic_types(json_value);
		const bool could_convert = extracted_value.convert(t);
		if (!could_convert)
		{
			if (json_value.is_object())
			{
				constructor ctor = t.get_constructor();
				for (auto& item : t.get_constructors())
				{
					if (item.get_instantiated_type() == t)
						ctor = item;
				}
				extracted_value = ctor.invoke();
				fromjson_recursively(extracted_value, json_value);
			}
		}

		return extracted_value;
	}

	static void write_associative_view_recursively(variant_associative_view& view, Json const& json_array_value)
	{
		for (size_t i = 0; i < json_array_value.size(); ++i)
		{
			auto& json_index_value = json_array_value[i];
			if (json_index_value.is_object()) // a key-value associative view
			{
				if (json_array_value.contains("key") &&
					json_array_value.contains("value"))
				{
					auto key_var = extract_value(json_array_value["key"], view.get_key_type());
					auto value_var = extract_value(json_array_value["value"], view.get_value_type());
					if (key_var && value_var)
					{
						view.insert(key_var, value_var);
					}
				}
			}
			else // a key-only associative view
			{
				variant extracted_value = extract_basic_types(json_index_value);
				if (extracted_value && extracted_value.convert(view.get_key_type()))
					view.insert(extracted_value);
			}
		}
	}

	/////////////////////////////////////////////////////////////////////////////////////////

	void fromjson_recursively(instance obj2, Json const& json_object)
	{
		instance obj = obj2.get_type().get_raw_type().is_wrapper() ? obj2.get_wrapped_instance() : obj2;


		type t = obj.get_type();
		// Checking existing serializers and deserializers
		if (typeSerializers.contains(t))
		{
			rttr::variant& var = *obj.try_convert<rttr::variant>();
			typeSerializers[t].second(var, json_object);
			return;
		}

		const auto prop_list = obj.get_derived_type().get_properties();

		for (auto prop : prop_list)
		{
			auto propName = std::string(prop.get_name().data());

			if (!json_object.contains(propName) || prop.is_readonly())
				continue;

			Json const& json_value = json_object[propName];

			const type value_t = prop.get_type();
			auto wrapped_type = value_t.is_wrapper() ? value_t.get_wrapped_type() : value_t;
			bool is_wrapper = wrapped_type != value_t;
			auto intendedType = is_wrapper ? wrapped_type : value_t;

			if (typeSerializers.contains(intendedType))
			{
				variant v;
				typeSerializers[intendedType].second(v, json_value);
				v.convert(value_t);
				prop.set_value(obj, v);
				continue;
			}
			else if (json_value.is_array())
			{
				variant var;
				if (value_t.is_sequential_container())
				{
					var = prop.get_value(obj);
					auto view = var.create_sequential_view();
					write_array_recursively(view, json_value);
				}
				else if (value_t.is_associative_container())
				{
					var = prop.get_value(obj);
					auto associative_view = var.create_associative_view();
					write_associative_view_recursively(associative_view, json_value);
				}

				prop.set_value(obj, var);
				continue;
			}
			else if (json_value.is_object())
			{
				variant var = prop.get_value(obj);
				fromjson_recursively(var, json_value);
				prop.set_value(obj, var);
				continue;
			}
			else
			{
				variant extracted_value = extract_basic_types(json_value);
				if (extracted_value.convert(value_t)) // REMARK: CONVERSION WORKS ONLY WITH "const type", check whether this is correct or not!
					prop.set_value(obj, extracted_value);
			}
		}
	}


	/////////////////////////////////////////////////////////////////////////////////////////
	/////////////////////////////////////////////////////////////////////////////////////////
	/////////////////////////////////////////////////////////////////////////////////////////


	void Deserialize(rttr::instance obj, Json const& json)
	{

		fromjson_recursively(obj, json);

	}
}
