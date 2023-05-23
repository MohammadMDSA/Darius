#include "Core/pch.hpp"

#include "TypeSerializer.hpp"

#include <Utils/Log.hpp>
#include <Utils/Assert.hpp>

using namespace rttr;
using namespace D_CONTAINERS;
using namespace D_CORE;

namespace Darius::Core::Serialization
{
	D_CONTAINERS::DUnorderedMap<rttr::type, std::function<void(rttr::instance const&, Json&)>> typeSerializers = { };
	D_CONTAINERS::DUnorderedMap<rttr::type, std::function<void(rttr::variant&, Json const&)>> typeDeserializers = { };

	bool												_initialized = false;

	void Initialize()
	{
		D_ASSERT(!_initialized);
		_initialized = true;

		D_SERIALIZATION::RegisterSerializer<D_CORE::Uuid>(D_CORE::UuidToJson, D_CORE::UuidFromJson);
		D_SERIALIZATION::RegisterSerializer<UuidWrapper>(nullptr, DeserializeUuidWrapper);
	}

	void Shutdown()
	{
		D_ASSERT(_initialized);
	}


	bool __RegisterSerializer(rttr::type type, std::function<void(rttr::instance const&, Json&)> serializer, std::function<void(rttr::variant&, Json const&)> deserializer)
	{

		bool any = false;
		if (serializer != nullptr)
		{
			if (!typeSerializers.contains(type))
			{
				typeSerializers.emplace(type, serializer);
				any = true;
			}
			
		}

		if (deserializer != nullptr)
		{
			if (!typeDeserializers.contains(type))
			{
				typeDeserializers.emplace(type, deserializer);
				any = true;
			}
		}

		return any;
	}

	void to_json_recursively(rttr::instance const obj, Json& json, SerializationContext const& reference);

	bool write_variant(const variant& var, Json& json, SerializationContext const& reference);

	bool write_atomic_types_to_json(type const& t, variant const& var, Json& json, SerializationContext const& context)
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
		else if (t == type::get<UuidWrapper>())
		{
			UuidWrapper uuidWrapper = var.convert<UuidWrapper>();
			auto& uuid = uuidWrapper.Uuid;

			// No need to rereference
			if (uuidWrapper.Param != D_SERIALIZATION_UUID_PARAM_GAMEOBJECT || !context.Rereference)
			{
				D_CORE::UuidToJson(uuid, json);
				return true;
			}

			// Have to rereference. Check if key exists.
			if (context.ReferenceMap.contains(uuid))
			{
				// Replace new reference
				D_CORE::UuidToJson(context.ReferenceMap.at(uuid), json);
			}
			else if (context.MaintainExternalReferences)
			{
				// Key doesn't exist. Keep the current reference.
				D_CORE::UuidToJson(uuid, json);
			}
			else
			{
				// Neither can replace nor keep the current. So put null.
				D_CORE::UuidToJson(Uuid(), json);
			}
			return true;
		}


		return false;
	}

	void write_array(variant_sequential_view const& view, Json& json, SerializationContext const& context)
	{
		for (const auto& item : view)
		{
			json.push_back(Json());
			auto& el = json.back();
			if (item.is_sequential_container())
			{
				write_array(item.create_sequential_view(), el, context);
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
					typeSerializers[intendedType](is_wrapper ? wrapped_var.extract_wrapped_value() : wrapped_var, el);
				}
				else if (intendedType.is_arithmetic() || intendedType == type::get<std::string>() || intendedType.is_enumeration() || intendedType == type::get<UuidWrapper>())
				{
					write_atomic_types_to_json(intendedType, wrapped_var, el, context);
				}
				else // object
				{
					to_json_recursively(wrapped_var, el, context);
				}
			}
		}
	}


	/////////////////////////////////////////////////////////////////////////////////////////

	void write_associative_container(variant_associative_view const& view, Json& json, SerializationContext const& context)
	{
		static const string_view key_name("key");
		static const string_view value_name("value");


		if (view.is_key_only_type())
		{
			for (auto& item : view)
			{
				json.push_back(Json());
				write_variant(item.first, json.back(), context);
			}
		}
		else
		{
			for (auto& item : view)
			{
				json.push_back(Json());
				auto& obj = json.back();

				auto keyNameStr = std::string(key_name.data());
				write_variant(item.first, obj[keyNameStr], context);

				auto valueNameStr = std::string(value_name.data());
				write_variant(item.second, obj[valueNameStr], context);
			}
		}
	}

	bool write_variant(variant const& var, Json& json, SerializationContext const& context)
	{
		auto value_type = var.get_type();
		auto wrapped_type = value_type.is_wrapper() ? value_type.get_wrapped_type() : value_type;
		bool is_wrapper = wrapped_type != value_type;

		auto intendedType = is_wrapper ? wrapped_type : value_type;

		// Checking existing serializers and deserializers
		if (typeSerializers.contains(intendedType))
		{
			typeSerializers[intendedType](is_wrapper ? var.extract_wrapped_value() : var, json);
			return true;
		}

		if (write_atomic_types_to_json(intendedType,
			is_wrapper ? var.extract_wrapped_value() : var, json, context))
		{
		}
		else if (var.is_sequential_container())
		{
			write_array(var.create_sequential_view(), json, context);
		}
		else if (var.is_associative_container())
		{
			write_associative_container(var.create_associative_view(), json, context);
		}
		else
		{
			auto child_props = is_wrapper ? wrapped_type.get_properties() : value_type.get_properties();
			if (!child_props.empty())
			{
				to_json_recursively(var, json, context);
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

	void to_json_recursively(rttr::instance const ins, Json& json, SerializationContext const& context)
	{

		instance obj = ins.get_type().get_raw_type().is_wrapper() ? ins.get_wrapped_instance() : ins;

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

			if (!write_variant(prop_value, json[nameStr], context))
			{
				D_LOG_ERROR("cannot serialize property: " << name);
			}
		}

	}

	void Serialize(rttr::instance const obj, Json& json, SerializationContext const& context)
	{
		if (!obj.is_valid())
			return;

		to_json_recursively(obj, json, context);
	}

	void Serialize(rttr::instance const obj, Json& json)
	{
		Serialize(obj, json, { false, true, {} });
	}

	void SerializeSequentialContainer(rttr::variant const& var, Json& json)
	{
		if (!var.is_sequential_container())
			return;

		SerializeSequentialContainer(var, json, { false, true, {} });
	}

	void SerializeSequentialContainer(rttr::variant const& var, Json& json, SerializationContext const& context)
	{
		if (!var.is_sequential_container())
			return;

		write_array(var.create_sequential_view(), json, context);
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
				if (typeDeserializers.contains(intendedType))
				{
					variant v;
					typeDeserializers[intendedType](v, json_index_value);
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

	void write_associative_view_recursively(variant_associative_view& view, Json const& json_array_value)
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
		if (typeDeserializers.contains(t))
		{
			rttr::variant& var = *obj.try_convert<rttr::variant>();
			typeDeserializers[t](var, json_object);
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

			if (typeDeserializers.contains(intendedType))
			{
				variant v;
				typeDeserializers[intendedType](v, json_value);
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
