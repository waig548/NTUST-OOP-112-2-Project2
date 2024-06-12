#ifndef FTK_SERIALIZER_H
#define FTK_SERIALIZER_H

#include <variant>

#include <nlohmann/json.hpp>

#include "defs.h"
#include "Vec.h"
#include "Expr.h"
#include "Modifier.h"
#include "Entity.h"
#include "Rect.h"
#include "World.h"
#include "Action.h"
#include "Skill.h"
#include "Buff.h"
#include "Item.h"
#include "Inventory.h"
#include "Registry.h"

#define EXPLICIT_SPECIALIZATION template <>
#define TYPE_SPECIALIZATION(T) template <typename T>

#define NLOHMANN_JSON_ADL_DESERIALIZE_DECLARATION(TYPE_NAME) static TYPE_NAME from_json(const json &j);
#define NLOHMANN_ORDERED_JSON_ADL_SERIALIZE_DECLARATION(TYPE_NAME, VALUE_NAME) static void to_json(ordered_json &j, const TYPE_NAME &VALUE_NAME);

#define NLOHMANN_JSON_ADL_DESERIALIZE_DEFINITION(TYPE_NAME) TYPE_NAME adl_serializer<TYPE_NAME>::from_json(const json &j)
#define NLOHMANN_ORDERED_JSON_ADL_SERIALIZE_DEFINITION(TYPE_NAME, VALUE_NAME) void adl_serializer<TYPE_NAME>::to_json(ordered_json &j, const TYPE_NAME &VALUE_NAME)

#define NLOHMANN_JSON_ADL_DESERIALIZE_TEMPLATE_DEFINITION(TYPE_NAME, T) static TYPE_NAME<T> from_json(const json &j)
#define NLOHMANN_ORDERED_JSON_ADL_SERIALIZE_TEMPLATE_DEFINITION(TYPE_NAME, T, VALUE_NAME) static void to_json(ordered_json &j, const TYPE_NAME<T> &VALUE_NAME)

#define NLOHMANN_JSON_ADL_SERIALIZER_DECLARATION_BEGIN(TYPE_NAME) \
    struct adl_serializer<TYPE_NAME>                              \
    {
#define NLOHMANN_JSON_ADL_SERIALIZER_DECLARATION_END \
    }                                                \
    ;

#define NLOHMANN_JSON_ADL_SERIALIZER_DECLARATION(TYPE_NAME, VALUE_NAME)    \
    EXPLICIT_SPECIALIZATION                                                \
    NLOHMANN_JSON_ADL_SERIALIZER_DECLARATION_BEGIN(TYPE_NAME)              \
    NLOHMANN_JSON_ADL_DESERIALIZE_DECLARATION(TYPE_NAME)                   \
    NLOHMANN_ORDERED_JSON_ADL_SERIALIZE_DECLARATION(TYPE_NAME, VALUE_NAME) \
    NLOHMANN_JSON_ADL_SERIALIZER_DECLARATION_END

#define NLOHMANN_JSON_ADL_SERIALIZER_TEMPLATE_DECLARATION_BEGIN(TYPE_NAME, ...) \
    TYPE_SPECIALIZATION(__VA_ARGS__)                                            \
    NLOHMANN_JSON_ADL_SERIALIZER_DECLARATION_BEGIN(TYPE_NAME<__VA_ARGS__>)

#define isSerializedType(type_str) (j.contains("serial_type") && j["serial_type"] == type_str)

NLOHMANN_JSON_NAMESPACE_BEGIN

// redefined to eliminate the need for copy assignment operators
NLOHMANN_JSON_ADL_SERIALIZER_TEMPLATE_DECLARATION_BEGIN(std::vector, T)
NLOHMANN_JSON_ADL_DESERIALIZE_TEMPLATE_DEFINITION(std::vector, T)
{
    std::vector<T> res;
    for (auto &e : j)
        res.push_back(e.get<T>());
    return res;
}
NLOHMANN_ORDERED_JSON_ADL_SERIALIZE_TEMPLATE_DEFINITION(std::vector, T, vector)
{
    j = ordered_json::array();
    for (auto e : vector)
        j.push_back(e);
}
NLOHMANN_JSON_ADL_SERIALIZER_DECLARATION_END

NLOHMANN_JSON_ADL_SERIALIZER_TEMPLATE_DECLARATION_BEGIN(std::set, T)
NLOHMANN_JSON_ADL_DESERIALIZE_TEMPLATE_DEFINITION(std::set, T)
{
    auto tmp = j.get<std::vector<T>>();
    return std::set<T>(tmp.begin(), tmp.end());
}
NLOHMANN_ORDERED_JSON_ADL_SERIALIZE_TEMPLATE_DEFINITION(std::set, T, set)
{
    j = std::vector<T>(set.begin(), set.end());
}
NLOHMANN_JSON_ADL_SERIALIZER_DECLARATION_END

NLOHMANN_JSON_ADL_SERIALIZER_TEMPLATE_DECLARATION_BEGIN(std::multiset, T)
NLOHMANN_JSON_ADL_DESERIALIZE_TEMPLATE_DEFINITION(std::multiset, T)
{
    auto tmp = j.get<std::vector<T>>();
    return std::multiset<T>(tmp.begin(), tmp.end());
}
NLOHMANN_ORDERED_JSON_ADL_SERIALIZE_TEMPLATE_DEFINITION(std::multiset, T, multiset)
{
    j = std::vector<T>(multiset.begin(), multiset.end());
}
NLOHMANN_JSON_ADL_SERIALIZER_DECLARATION_END

NLOHMANN_JSON_ADL_SERIALIZER_TEMPLATE_DECLARATION_BEGIN(std::unordered_set, T)
NLOHMANN_JSON_ADL_DESERIALIZE_TEMPLATE_DEFINITION(std::unordered_set, T)
{
    auto tmp = j.get<std::vector<T>>();
    return std::unordered_set<T>(tmp.begin(), tmp.end());
}
NLOHMANN_ORDERED_JSON_ADL_SERIALIZE_TEMPLATE_DEFINITION(std::unordered_set, T, unordered_set)
{
    j = std::vector<T>(unordered_set.begin(), unordered_set.end());
}
NLOHMANN_JSON_ADL_SERIALIZER_DECLARATION_END

NLOHMANN_JSON_ADL_SERIALIZER_TEMPLATE_DECLARATION_BEGIN(std::unordered_multiset, T)
NLOHMANN_JSON_ADL_DESERIALIZE_TEMPLATE_DEFINITION(std::unordered_multiset, T)
{
    auto tmp = j.get<std::vector<T>>();
    return std::unordered_multiset<T>(tmp.begin(), tmp.end());
}
NLOHMANN_ORDERED_JSON_ADL_SERIALIZE_TEMPLATE_DEFINITION(std::unordered_multiset, T, unordered_multiset)
{
    j = std::vector<T>(unordered_multiset.begin(), unordered_multiset.end());
}
NLOHMANN_JSON_ADL_SERIALIZER_DECLARATION_END

template <typename K, typename V>
struct adl_serializer<std::map<K, V>>
{
    static std::map<K, V> from_json(const json &j)
    {
        std::map<K, V> res;
        if (!j.is_object())
            throw std::invalid_argument("Not a JSON object");
        for (auto item : j.items())
        {
            res.emplace(json(item.key()).get<K>(), item.value().get<V>());
        }
        return res;
    }
    static void to_json(ordered_json &j, const std::map<K, V> &map)
    {
        j = ordered_json::object();
        for (auto p : map)
        {
            auto serializedKey = ordered_json(p.first);
            if (!serializedKey.is_string())
                throw std::invalid_argument("Invalid key type (must be serialized to string)");
            j[serializedKey.get<std::string>()] = p.second;
        }
    }
};

template <typename K, typename V>
struct adl_serializer<std::multimap<K, V>>
{
    static std::multimap<K, V> from_json(const json &j)
    {
        std::multimap<K, V> res;
        if (!j.is_object())
            throw std::invalid_argument("Not a JSON object");
        for (auto item : j.items())
        {
            res[json(item.key()).get<K>()] = item.value;
        }
        return res;
    }
    static void to_json(ordered_json &j, const std::multimap<K, V> &multimap)
    {
        j = ordered_json::object();
        for (auto p : multimap)
        {
            if (!ordered_json(p.first).is_string())
                throw std::invalid_argument("Invalid key type (must be serialized to string)");
            j[ordered_json(p.first).get<std::string>()] = p.second;
        }
    }
};

NLOHMANN_JSON_ADL_SERIALIZER_TEMPLATE_DECLARATION_BEGIN(std::shared_ptr, T)
NLOHMANN_JSON_ADL_DESERIALIZE_TEMPLATE_DEFINITION(std::shared_ptr, T)
{
    // T must be copy constructible
    return std::shared_ptr<T>(new T(j.get<T>()));
}
NLOHMANN_ORDERED_JSON_ADL_SERIALIZE_TEMPLATE_DEFINITION(std::shared_ptr, T, sharedPtr)
{
    j = *sharedPtr;
}
NLOHMANN_JSON_ADL_SERIALIZER_DECLARATION_END

NLOHMANN_JSON_ADL_SERIALIZER_TEMPLATE_DECLARATION_BEGIN(std::unique_ptr, T)
NLOHMANN_JSON_ADL_DESERIALIZE_TEMPLATE_DEFINITION(std::unique_ptr, T)
{
    // T must be copy constructible
    return std::unique_ptr<T>(new T(j.get<T>()));
}
NLOHMANN_ORDERED_JSON_ADL_SERIALIZE_TEMPLATE_DEFINITION(std::unique_ptr, T, sharedPtr)
{
    j = *sharedPtr;
}
NLOHMANN_JSON_ADL_SERIALIZER_DECLARATION_END

NLOHMANN_JSON_ADL_SERIALIZER_TEMPLATE_DECLARATION_BEGIN(std::optional, T)
NLOHMANN_JSON_ADL_DESERIALIZE_TEMPLATE_DEFINITION(std::optional, T)
{
    // T must be copy constructible
    if (j.is_null())
        return std::nullopt;
    return std::make_optional<T>(j.get<T>());
}
NLOHMANN_ORDERED_JSON_ADL_SERIALIZE_TEMPLATE_DEFINITION(std::optional, T, optional)
{
    if (optional.has_value())
        j = optional.value();
}
NLOHMANN_JSON_ADL_SERIALIZER_DECLARATION_END

NLOHMANN_JSON_ADL_SERIALIZER_DECLARATION(uuids::uuid, uuid)

NLOHMANN_JSON_ADL_SERIALIZER_DECLARATION(FTK::Range, range)

NLOHMANN_JSON_ADL_SERIALIZER_TEMPLATE_DECLARATION_BEGIN(FTK::Vec, T)
NLOHMANN_JSON_ADL_DESERIALIZE_TEMPLATE_DEFINITION(FTK::Vec, T)
{
    auto r = j.get<std::vector<T>>();
    return FTK::Vec<T>(r[0], r[1]);
}
NLOHMANN_ORDERED_JSON_ADL_SERIALIZE_TEMPLATE_DEFINITION(FTK::Vec, T, v)
{
    j = {v.x, v.y};
}
NLOHMANN_JSON_ADL_SERIALIZER_DECLARATION_END

NLOHMANN_JSON_ADL_SERIALIZER_DECLARATION(FTK::Math::Expression, expression)
NLOHMANN_JSON_ADL_SERIALIZER_DECLARATION(FTK::Math::Condition, condition)

NLOHMANN_JSON_ADL_SERIALIZER_DECLARATION(FTK::Modifier, modifier)
NLOHMANN_JSON_ADL_SERIALIZER_DECLARATION(FTK::ModifierTemplate, modifierTemplate)
NLOHMANN_JSON_ADL_SERIALIZER_DECLARATION(FTK::ModifiableValue, value)
NLOHMANN_JSON_ADL_SERIALIZER_DECLARATION(FTK::NamedModifiableValue, value)

NLOHMANN_JSON_ADL_SERIALIZER_DECLARATION(FTK::Entity::Attribute, attribute)
NLOHMANN_JSON_ADL_SERIALIZER_DECLARATION(FTK::Entity::Stat, status)
NLOHMANN_JSON_ADL_SERIALIZER_DECLARATION(FTK::Entity, entity)
NLOHMANN_JSON_ADL_SERIALIZER_DECLARATION(FTK::Player, player)
NLOHMANN_JSON_ADL_SERIALIZER_DECLARATION(FTK::Enemy, enemy)
NLOHMANN_JSON_ADL_SERIALIZER_DECLARATION(std::shared_ptr<FTK::Entity>, entity)
NLOHMANN_JSON_ADL_SERIALIZER_DECLARATION(FTK::PlayerTemplate, playerTemplate)
NLOHMANN_JSON_ADL_SERIALIZER_DECLARATION(FTK::EnemyTemplate, enemyTemplate)

NLOHMANN_JSON_ADL_SERIALIZER_DECLARATION(FTK::RectEntity, rectEntity)
NLOHMANN_JSON_ADL_SERIALIZER_DECLARATION(FTK::ShopRectEntity, shopRectEntity)
NLOHMANN_JSON_ADL_SERIALIZER_DECLARATION(FTK::RestRectEntity, restRectEntity)
NLOHMANN_JSON_ADL_SERIALIZER_DECLARATION(std::shared_ptr<FTK::RectEntity>, rectEntity)
NLOHMANN_JSON_ADL_SERIALIZER_DECLARATION(FTK::Rect, rect)

NLOHMANN_JSON_ADL_SERIALIZER_DECLARATION(FTK::World, world)

NLOHMANN_JSON_ADL_SERIALIZER_DECLARATION(FTK::Action, action)
NLOHMANN_JSON_ADL_SERIALIZER_DECLARATION(FTK::DamageAction, damageAction)
NLOHMANN_JSON_ADL_SERIALIZER_DECLARATION(FTK::HealAction, healAction)
NLOHMANN_JSON_ADL_SERIALIZER_DECLARATION(FTK::BuffAction, buffAction)
NLOHMANN_JSON_ADL_SERIALIZER_DECLARATION(FTK::FleeAction, fleeAction)
NLOHMANN_JSON_ADL_SERIALIZER_DECLARATION(FTK::DestroyAction, destroyAction)
NLOHMANN_JSON_ADL_SERIALIZER_DECLARATION(FTK::ModifyStatAction, modifyStatAction)
NLOHMANN_JSON_ADL_SERIALIZER_DECLARATION(std::shared_ptr<FTK::Action>, action)

NLOHMANN_JSON_ADL_SERIALIZER_DECLARATION(FTK::ActiveSkill, activeSkill)
NLOHMANN_JSON_ADL_SERIALIZER_DECLARATION(FTK::PassiveSkill, passiveSkill)

NLOHMANN_JSON_ADL_SERIALIZER_DECLARATION(FTK::Buff, buff)
NLOHMANN_JSON_ADL_SERIALIZER_DECLARATION(FTK::BuffTemplate, buffTemplate)
NLOHMANN_JSON_ADL_SERIALIZER_DECLARATION(FTK::BuffBuildData, buffBuildData)

NLOHMANN_JSON_ADL_SERIALIZER_DECLARATION(FTK::ItemData, itemData)
NLOHMANN_JSON_ADL_SERIALIZER_DECLARATION(FTK::ItemTemplate, itemTemplate)
NLOHMANN_JSON_ADL_SERIALIZER_DECLARATION(FTK::Equipment, equipment)
NLOHMANN_JSON_ADL_SERIALIZER_DECLARATION(FTK::EquipmentTemplate, equipmentTemplate)

NLOHMANN_JSON_ADL_SERIALIZER_DECLARATION(FTK::Inventory, inventory)

NLOHMANN_JSON_ADL_SERIALIZER_TEMPLATE_DECLARATION_BEGIN(FTK::Registry, T)
NLOHMANN_JSON_ADL_DESERIALIZE_TEMPLATE_DEFINITION(FTK::Registry, T)
{
    return FTK::Registry<T>(j.get<std::vector<T>>());
}
NLOHMANN_ORDERED_JSON_ADL_SERIALIZE_TEMPLATE_DEFINITION(FTK::Registry, T, registry)
{
    j = std::vector<T>(registry);
}
NLOHMANN_JSON_ADL_SERIALIZER_DECLARATION_END

NLOHMANN_JSON_NAMESPACE_END

#endif // FTK_SERIALIZER_H