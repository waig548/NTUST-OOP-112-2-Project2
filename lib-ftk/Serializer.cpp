#include "Serializer.h"

#include <vector>
#include <string>

#include <bimap.hpp>

#include "utils.h"

NLOHMANN_JSON_NAMESPACE_BEGIN

NLOHMANN_JSON_ADL_DESERIALIZE_DEFINITION(uuids::uuid)
{
    // read from json, or generate a new one
    return uuids::uuid::from_string(j.get<std::string>()).value_or(uuids::uuid_system_generator{}());
}

NLOHMANN_ORDERED_JSON_ADL_SERIALIZE_DEFINITION(uuids::uuid, uuid)
{
    j = uuids::to_string(uuid);
}

NLOHMANN_JSON_ADL_DESERIALIZE_DEFINITION(FTK::Range)
{
    auto r = j.get<std::vector<double>>();
    return {r[0], r[1]};
}

NLOHMANN_ORDERED_JSON_ADL_SERIALIZE_DEFINITION(FTK::Range, range)
{
    j = {range.min, range.max};
}

NLOHMANN_JSON_ADL_DESERIALIZE_DEFINITION(FTK::Math::Expression)
{
    return FTK::Math::Expression(j.get<std::string>());
}

NLOHMANN_ORDERED_JSON_ADL_SERIALIZE_DEFINITION(FTK::Math::Expression, expression)
{
    j = expression.rawExpr;
}

NLOHMANN_JSON_ADL_DESERIALIZE_DEFINITION(FTK::Math::Condition)
{
    return FTK::Math::Condition(j.get<std::string>());
}

NLOHMANN_ORDERED_JSON_ADL_SERIALIZE_DEFINITION(FTK::Math::Condition, condition)
{
    j = condition.rawExpr;
}

NLOHMANN_JSON_ADL_DESERIALIZE_DEFINITION(FTK::Modifier)
{
    auto uuid = j["uuid"].get<uuids::uuid>();
    std::string name;
    if (j.contains("name"))
        name = j["name"].get<std::string>();
    auto type = j["type"].get<FTK::ModifierType>();
    auto value = j["value"].get<double>();
    return FTK::Modifier{
        uuid,
        name,
        type,
        value};
}

NLOHMANN_ORDERED_JSON_ADL_SERIALIZE_DEFINITION(FTK::Modifier, modifier)
{
    j["uuid"] = modifier.uuid;
    if (!modifier.name.empty())
        j["name"] = modifier.name;
    j["type"] = modifier.type;
    j["value"] = modifier.value;
}

NLOHMANN_JSON_ADL_DESERIALIZE_DEFINITION(FTK::ModifierTemplate)
{
    std::string name;
    if (j.contains("name"))
        name = j["name"].get<std::string>();
    auto type = j["type"].get<FTK::ModifierType>();
    auto targetPath = j["target_path"].get<std::string>();
    auto value = j["value"].get<double>();
    return FTK::ModifierTemplate(
        name,
        type,
        targetPath,
        value);
}

NLOHMANN_ORDERED_JSON_ADL_SERIALIZE_DEFINITION(FTK::ModifierTemplate, modifierTemplate)
{
    if (!modifierTemplate.name.empty())
        j["name"] = modifierTemplate.name;
    j["type"] = modifierTemplate.type;
    j["value"] = modifierTemplate.value;
    j["target_path"] = modifierTemplate.targetPath;
}

NLOHMANN_JSON_ADL_DESERIALIZE_DEFINITION(FTK::ModifiableValue)
{
    std::vector<FTK::Modifier> rawMods;
    if (j.contains("modifiers"))
        for (auto e : j["modifiers"])
            rawMods.push_back(e.get<FTK::Modifier>());
    return FTK::ModifiableValue{
        j["base"].get<double>(),
        {rawMods.begin(), rawMods.end()}};
}

NLOHMANN_ORDERED_JSON_ADL_SERIALIZE_DEFINITION(FTK::ModifiableValue, value)
{
    j["base"] = value.base;
    if (value.modifiers.size())
        j["modifiers"] = std::vector<FTK::Modifier>{value.modifiers.begin(), value.modifiers.end()};
}

NLOHMANN_JSON_ADL_DESERIALIZE_DEFINITION(FTK::NamedModifiableValue)
{
    return FTK::NamedModifiableValue{
        j["name"].get<std::string>(),
        j.get<FTK::ModifiableValue>()};
}

NLOHMANN_ORDERED_JSON_ADL_SERIALIZE_DEFINITION(FTK::NamedModifiableValue, value)
{
    j["name"] = value.name;
    j.update(static_cast<FTK::ModifiableValue>(value));
}

NLOHMANN_JSON_ADL_DESERIALIZE_DEFINITION(FTK::Entity::Attribute)
{
    return FTK::Entity::Attribute{j.get<FTK::NamedModifiableValue>()};
}

NLOHMANN_ORDERED_JSON_ADL_SERIALIZE_DEFINITION(FTK::Entity::Attribute, attribute)
{
    j.update(static_cast<FTK::NamedModifiableValue>(attribute));
}

NLOHMANN_JSON_ADL_DESERIALIZE_DEFINITION(FTK::Entity::Stat)
{
    return FTK::Entity::Stat{j.get<FTK::NamedModifiableValue>()};
}

NLOHMANN_ORDERED_JSON_ADL_SERIALIZE_DEFINITION(FTK::Entity::Stat, status)
{
    j.update(static_cast<FTK::NamedModifiableValue>(status));
}

NLOHMANN_JSON_ADL_DESERIALIZE_DEFINITION(FTK::Entity)
{
    auto uuid = j.contains("uuid") ? j["uuid"].get<uuids::uuid>() : ""_json.get<uuids::uuid>();
    auto id = j["id"].get<std::string>();
    auto name = j.contains("name") ? j["name"].get<std::string>() : "";
    auto attributes = j.contains("attributes") ? j["attributes"].get<std::vector<FTK::Entity::Attribute>>() : std::vector<FTK::Entity::Attribute>{};
    auto stats = j.contains("stats") ? j["stats"].get<std::vector<FTK::Entity::Stat>>() : std::vector<FTK::Entity::Stat>{};
    auto pos = j.contains("pos") ? j["pos"].get<FTK::Vec2i>() : FTK::Vec2i{};
    auto prevPos = j.contains("prev_pos") ? j["prev_pos"].get<FTK::Vec2i>() : pos;
    auto skillCD = j.contains("skill_cd") ? j["skill_cd"].get<std::map<std::string, int>>() : std::map<std::string, int>{};
    auto buffs = j.contains("buffs") ? j["buffs"].get<std::multiset<FTK::Buff>>() : std::multiset<FTK::Buff>{};
    auto equipments = j.contains("equipments") ? j["equipments"].get<std::map<FTK::EquipmentType, std::optional<FTK::Equipment>>>() : std::map<FTK::EquipmentType, std::optional<FTK::Equipment>>{};
    return FTK::Entity{uuid, id, name, attributes, stats, pos, prevPos, skillCD, buffs, equipments};
}

NLOHMANN_ORDERED_JSON_ADL_SERIALIZE_DEFINITION(FTK::Entity, entity)
{
    j["serial_type"] = entity.getSerialType();
    j["uuid"] = entity.uuid;
    j["id"] = entity.id;
    j["name"] = entity.name;
    j["attributes"] = entity.attributes;
    j["stats"] = entity.stats;
    j["pos"] = entity.pos;
    j["prev_pos"] = entity.prevPos;
    if (entity.skillCD.size())
        j["skill_cd"] = entity.skillCD;
    if (entity.buffs.size())
        j["buffs"] = entity.buffs;
    j["equipments"] = entity.equipments;
}

NLOHMANN_JSON_ADL_DESERIALIZE_DEFINITION(FTK::Enemy)
{
    return FTK::Enemy{j.get<FTK::Entity>()};
}

NLOHMANN_ORDERED_JSON_ADL_SERIALIZE_DEFINITION(FTK::Enemy, enemy)
{
    j.update(static_cast<FTK::Entity>(enemy));
    j["serial_type"] = enemy.getSerialType();
}

NLOHMANN_JSON_ADL_DESERIALIZE_DEFINITION(FTK::Player)
{
    return FTK::Player(j.get<FTK::Entity>());
}

NLOHMANN_ORDERED_JSON_ADL_SERIALIZE_DEFINITION(FTK::Player, player)
{
    j.update(static_cast<FTK::Entity>(player));
    j["serial_type"] = player.getSerialType();
}

NLOHMANN_JSON_ADL_DESERIALIZE_DEFINITION(std::shared_ptr<FTK::Entity>)
{
    // TODO better ways to deserialize polymorphically
    auto id = j["id"].get<std::string>();
    if (isSerializedType("enemy") || id._Starts_with("enemy:"))
        return j.get<std::shared_ptr<FTK::Enemy>>();
    if (isSerializedType("player") || id._Starts_with("player:"))
        return j.get<std::shared_ptr<FTK::Player>>();
    if (isSerializedType("entity") || id._Starts_with("entity:"))
        return std::make_shared<FTK::Entity>(j.get<FTK::Entity>());
    throw std::invalid_argument("Unknown serial type");
}

NLOHMANN_ORDERED_JSON_ADL_SERIALIZE_DEFINITION(std::shared_ptr<FTK::Entity>, entity)
{
    // TODO better ways to serialize polymorphically
    if (entity->getSerialType() == "enemy")
        j = *std::static_pointer_cast<FTK::Enemy>(entity);
    else if (entity->getSerialType() == "player")
        j = *std::static_pointer_cast<FTK::Player>(entity);
    else
        j = *entity;
}

NLOHMANN_JSON_ADL_DESERIALIZE_DEFINITION(FTK::PlayerTemplate)
{
    return {};
}

NLOHMANN_ORDERED_JSON_ADL_SERIALIZE_DEFINITION(FTK::PlayerTemplate, playerTemplate)
{
}

NLOHMANN_JSON_ADL_DESERIALIZE_DEFINITION(FTK::EnemyTemplate)
{
    return {};
}

NLOHMANN_ORDERED_JSON_ADL_SERIALIZE_DEFINITION(FTK::EnemyTemplate, enemyTemplate)
{
}

NLOHMANN_JSON_ADL_DESERIALIZE_DEFINITION(FTK::RectEntity)
{
    auto uuid = j.contains("uuid") ? j["uuid"].get<uuids::uuid>() : ""_json.get<uuids::uuid>();
    auto id = j["id"].get<std::string>();
    auto name = j["name"].get<std::string>();
    auto type = j["type"].get<FTK::RectEntityType>();
    auto pos = j["pos"].get<FTK::Vec2i>();
    return FTK::RectEntity(uuid, id, name, type, pos);
}

NLOHMANN_ORDERED_JSON_ADL_SERIALIZE_DEFINITION(FTK::RectEntity, rectEntity)
{
    j["serial_type"] = rectEntity.getSerialType();
    j["uuid"] = rectEntity.uuid;
    j["id"] = rectEntity.id;
    j["name"] = rectEntity.name;
    j["type"] = rectEntity.type;
    j["pos"] = rectEntity.pos;
}

NLOHMANN_JSON_ADL_DESERIALIZE_DEFINITION(FTK::ShopRectEntity)
{
    auto base = j.get<FTK::RectEntity>();
    auto inventory = j["inventory"].get<std::shared_ptr<FTK::Inventory>>();
    return FTK::ShopRectEntity(
        base.uuid,
        base.id,
        base.name,
        base.type,
        base.pos,
        inventory);
}

NLOHMANN_ORDERED_JSON_ADL_SERIALIZE_DEFINITION(FTK::ShopRectEntity, shopRectEntity)
{
    j.update(static_cast<FTK::RectEntity>(shopRectEntity));
    j["inventory"] = shopRectEntity.inventory;
    j["serial_type"] = shopRectEntity.getSerialType();
}

NLOHMANN_JSON_ADL_DESERIALIZE_DEFINITION(FTK::RestRectEntity)
{
    auto base = j.get<FTK::RectEntity>();
    auto restActions = j.get<std::vector<std::shared_ptr<FTK::Action>>>();
    auto restCondition = j.get<FTK::Math::Condition>();
    return FTK::RestRectEntity(
        base.uuid,
        base.id,
        base.name,
        base.type,
        base.pos,
        restActions,
        restCondition);
}

NLOHMANN_ORDERED_JSON_ADL_SERIALIZE_DEFINITION(FTK::RestRectEntity, restRectEntity)
{
    j.update(static_cast<FTK::RectEntity>(restRectEntity));
    j["rest_actions"] = restRectEntity.restActions;
    j["rest_condition"] = restRectEntity.restCondition;
    j["serial_type"] = restRectEntity.getSerialType();
}

NLOHMANN_JSON_ADL_DESERIALIZE_DEFINITION(std::shared_ptr<FTK::RectEntity>)
{
    // TODO better ways to deserialize polymorphically
    auto type = j["type"].get<FTK::RectEntityType>();
    if (isSerializedType("shop_rect_entity") || type == FTK::RectEntityType::Shop)
        return j.get<std::shared_ptr<FTK::ShopRectEntity>>();
    if (isSerializedType("rest_rect_entity") || type == FTK::RectEntityType::Rest)
        return j.get<std::shared_ptr<FTK::ShopRectEntity>>();
    if (isSerializedType("rect_entity") || type == FTK::RectEntityType::None)
        return j.get<std::shared_ptr<FTK::ShopRectEntity>>();
    throw std::invalid_argument("Unknown serial type");
}

NLOHMANN_ORDERED_JSON_ADL_SERIALIZE_DEFINITION(std::shared_ptr<FTK::RectEntity>, rectEntity)
{
    // TODO better ways to serialize polymorphically
    if (rectEntity->getSerialType() == "shop_rect_entity")
        j = *std::static_pointer_cast<FTK::ShopRectEntity>(rectEntity);
    else if (rectEntity->getSerialType() == "rest_rect_entity")
        j = *std::static_pointer_cast<FTK::RestRectEntity>(rectEntity);
    else
        j = *rectEntity;
}
NLOHMANN_JSON_ADL_DESERIALIZE_DEFINITION(FTK::Rect)
{
    return FTK::Rect{
        j["id"].get<std::string>(),
        j["metadata"].get<int>()};
}

NLOHMANN_ORDERED_JSON_ADL_SERIALIZE_DEFINITION(FTK::Rect, rect)
{
    j["id"] = rect.id;
    j["metadata"] = rect.metadata;
}

NLOHMANN_JSON_ADL_DESERIALIZE_DEFINITION(FTK::World)
{
    const auto dimension = j["dimension"].get<FTK::Vec2i>();
    const auto pattern = j["pattern"].get<std::vector<std::string>>();
    const auto key = j["key"].get<std::map<std::string, FTK::Rect>>();
    const auto visibility = j.contains("visibility") ? j["visibility"].get<std::vector<std::string>>() : std::vector<std::string>(dimension.getY(), std::string(dimension.getX(), 'F'));

    std::vector<std::shared_ptr<FTK::Rect>> rects;
    if (pattern.size() != dimension.getY())
        throw std::invalid_argument("Rect data row dimension not match\nrequired: " + std::to_string(dimension.getY()) + " found:" + std::to_string(pattern.size()));
    for (int i = 0; i < dimension.getY(); i++)
    {
        if (pattern[i].size() != dimension.getX())
            throw std::invalid_argument("Rect data column dimension not match at row " + std::to_string(i) + "\nrequired: " + std::to_string(dimension.getX()) + " found:" + std::to_string(pattern[i].size()));
        for (int j = 0; j < dimension.getX(); j++)
        {
            auto symbol = std::string(1, pattern[i][j]);
            if (!key.count(symbol))
                throw std::invalid_argument("Invalid symbol '" + symbol + "'at position (" + std::to_string(j) + ", " + std::to_string(i) + ") in Rect data");
            auto rect = std::make_shared<FTK::Rect>(key.at(symbol));
            if (visibility[i][j] == 'T')
                rect->markVisible();
            rects.push_back(rect);
        }
    }

    const auto rectEntities = j["rect_entities"].get<std::vector<std::shared_ptr<FTK::RectEntity>>>();
    for (auto re : rectEntities)
    {
        auto pos = re->pos;
        rects[pos.getY() * dimension.getX() + pos.getX()]->attachRectEntity(re);
    }
    const auto entities = j["entities"].get<std::vector<std::shared_ptr<FTK::Entity>>>();
    const auto players = j["players"].get<std::vector<std::shared_ptr<FTK::Player>>>();

    return FTK::World(dimension, rects /*, {}*/, entities, players);
}

NLOHMANN_ORDERED_JSON_ADL_SERIALIZE_DEFINITION(FTK::World, world)
{
    static const std::vector<std::pair<char, FTK::Rect>> defaultRectSymbolMap{
        {'.', {"rect:grass", 0}},
        {'#', {"rect:rock", 0}}};

    auto rectSymbolMap = defaultRectSymbolMap;
    char nextSymbol = 'A';

    j["dimension"] = world.dimension;
    std::vector<std::string> pattern;
    std::vector<std::string> visibility;
    std::vector<std::shared_ptr<FTK::RectEntity>> rectEntities;
    for (int i = 0; i < world.dimension.getY(); i++)
    {
        pattern.push_back({});
        visibility.push_back({});
        for (int j = 0; j < world.dimension.getX(); j++)
        {
            auto cur = world.rects[i * world.dimension.getX() + j];
            if (cur->isVisible())
                visibility[i].push_back('T');
            else
                visibility[i].push_back('F');
            if (cur->hasRectEntity())
                rectEntities.push_back(cur->getRectEntity());
            if (auto it = std::find_if(rectSymbolMap.begin(), rectSymbolMap.end(), [cur](auto p)
                                       { return p.second.getID() == cur->getID() && p.second.getMetadata() == cur->getMetadata(); });
                it == rectSymbolMap.end())
            {
                while (std::find_if(rectSymbolMap.begin(), rectSymbolMap.end(), [nextSymbol](auto p)
                                    { return p.first == nextSymbol; }) != rectSymbolMap.end())
                    nextSymbol++;
                rectSymbolMap.push_back({nextSymbol, *cur});
                pattern[i].push_back(nextSymbol);
                nextSymbol++;
            }
            else
            {
                pattern[i].push_back(it->first);
            }
        }
        if (pattern[i].size() != world.dimension.getX())
            throw std::exception("Something went wrong when converting rects into symbols");
    }
    j["pattern"] = pattern;
    std::map<std::string, FTK::Rect> key;
    for (auto e : rectSymbolMap)
        key.emplace(std::string(1, e.first), e.second);
    j["key"] = key;
    j["visibility"] = visibility;
    j["rect_entities"] = rectEntities;
    j["entities"] = world.entities;
    j["players"] = world.players;
}

NLOHMANN_JSON_ADL_DESERIALIZE_DEFINITION(FTK::Action)
{
    auto actionType = j.contains("action_type") ? j["action_type"].get<FTK::ActionType>() : FTK::ActionType::Nop;
    auto targetType = j.contains("target_type") ? j["target_type"].get<FTK::TargetType>() : FTK::TargetType::None;
    auto targetScope = FTK::TargetScope::Ally;
    if (targetType != FTK::TargetType::None &&
        targetType != FTK::TargetType::Self &&
        targetType != FTK::TargetType::Main &&
        targetType != FTK::TargetType::SplashExcludingMain)
        targetScope = j["target_scope"].get<FTK::TargetScope>();
    auto immediate = j.contains("immediate") ? j["immediate"].get<bool>() : false;
    return FTK::Action(actionType, targetType, targetScope);
}

NLOHMANN_ORDERED_JSON_ADL_SERIALIZE_DEFINITION(FTK::Action, action)
{
    j["serial_type"] = action.getSerialType();
    j["action_type"] = action.actionType;
    j["target_type"] = action.targetType;
    if (action.targetType != FTK::TargetType::None &&
        action.targetType != FTK::TargetType::Self &&
        action.targetType != FTK::TargetType::Main &&
        action.targetType != FTK::TargetType::SplashExcludingMain)
        j["target_scope"] = action.targetScope;
}

NLOHMANN_JSON_ADL_DESERIALIZE_DEFINITION(FTK::DamageAction)
{
    auto base = j.get<FTK::Action>();
    return FTK::DamageAction(
        base.actionType,
        base.targetType,
        base.targetScope,
        j["damage_type"].get<FTK::DamageType>(),
        j["damage_expr"].get<FTK::Math::Expression>());
}

NLOHMANN_ORDERED_JSON_ADL_SERIALIZE_DEFINITION(FTK::DamageAction, damageAction)
{
    j.update(static_cast<FTK::Action>(damageAction));
    j["damage_type"] = damageAction.damageType;
    j["damage_expr"] = damageAction.damageExpr;
    j["serial_type"] = damageAction.getSerialType();
}

NLOHMANN_JSON_ADL_DESERIALIZE_DEFINITION(FTK::HealAction)
{
    auto base = j.get<FTK::Action>();
    return FTK::HealAction(
        base.actionType,
        base.targetType,
        base.targetScope,
        j["heal_expr"].get<FTK::Math::Expression>());
}

NLOHMANN_ORDERED_JSON_ADL_SERIALIZE_DEFINITION(FTK::HealAction, healAction)
{
    j.update(static_cast<FTK::Action>(healAction));
    j["heal_expr"] = healAction.healExpr;
    j["serial_type"] = healAction.getSerialType();
}

NLOHMANN_JSON_ADL_DESERIALIZE_DEFINITION(FTK::BuffAction)
{
    auto base = j.get<FTK::Action>();
    auto buffs = j["buffs"].get<std::vector<FTK::BuffBuildData>>();
    return FTK::BuffAction{
        base.actionType,
        base.targetType,
        base.targetScope,
        buffs};
}

NLOHMANN_ORDERED_JSON_ADL_SERIALIZE_DEFINITION(FTK::BuffAction, buffAction)
{
    j.update(static_cast<FTK::Action>(buffAction));
    j["buffs"] = buffAction.buffs;
    j["serial_type"] = buffAction.getSerialType();
}

NLOHMANN_JSON_ADL_DESERIALIZE_DEFINITION(FTK::FleeAction)
{
    auto base = j.get<FTK::Action>();
    return FTK::FleeAction(
        base.actionType,
        base.targetType,
        base.targetScope);
}

NLOHMANN_ORDERED_JSON_ADL_SERIALIZE_DEFINITION(FTK::FleeAction, fleeAction)
{
    j.update(static_cast<FTK::Action>(fleeAction));
    j["serial_type"] = fleeAction.getSerialType();
}

NLOHMANN_JSON_ADL_DESERIALIZE_DEFINITION(FTK::DestroyAction)
{
    auto base = j.get<FTK::Action>();
    auto destroyAmount = j["destroy_amount"].get<int>();
    return FTK::DestroyAction(
        base.actionType,
        base.targetType,
        base.targetScope,
        destroyAmount);
}

NLOHMANN_ORDERED_JSON_ADL_SERIALIZE_DEFINITION(FTK::DestroyAction, destroyAction)
{
    j.update(static_cast<FTK::Action>(destroyAction));
    j["destroy_amount"] = destroyAction.destroyAmount;
    j["serial_type"] = destroyAction.getSerialType();
}

NLOHMANN_JSON_ADL_DESERIALIZE_DEFINITION(FTK::ModifyStatAction)
{
    auto base = j.get<FTK::Action>();
    auto targetPath = j["target_path"].get<std::string>();
    auto modifyAmount = j["modify_amount"].get<double>();
    return FTK::ModifyStatAction(
        base.actionType,
        base.targetType,
        base.targetScope,
        targetPath,
        modifyAmount);
}

NLOHMANN_ORDERED_JSON_ADL_SERIALIZE_DEFINITION(FTK::ModifyStatAction, modifyStatAction)
{
    j.update(static_cast<FTK::Action>(modifyStatAction));
    j["target_path"] = modifyStatAction.targetPath;
    j["modify_amount"] = modifyStatAction.modifyAmount;
    j["serial_type"] = modifyStatAction.getSerialType();
}

NLOHMANN_JSON_ADL_DESERIALIZE_DEFINITION(std::shared_ptr<FTK::Action>)
{
    auto actionType = j["action_type"].get<FTK::ActionType>();
    // TODO better ways to deserialize polymorphically
    if (isSerializedType("damage_action") || actionType == FTK::ActionType::Damage)
        return j.get<std::shared_ptr<FTK::DamageAction>>();
    if (isSerializedType("heal_action") || actionType == FTK::ActionType::Heal)
        return j.get<std::shared_ptr<FTK::HealAction>>();
    if (isSerializedType("buff_action") || actionType == FTK::ActionType::Buff || actionType == FTK::ActionType::Debuff)
        return j.get<std::shared_ptr<FTK::BuffAction>>();
    if (isSerializedType("flee_action") || actionType == FTK::ActionType::Flee)
        return j.get<std::shared_ptr<FTK::FleeAction>>();
    if (isSerializedType("destroy_action") || actionType == FTK::ActionType::Destroy)
        return j.get<std::shared_ptr<FTK::DestroyAction>>();
    if (isSerializedType("modify_stat_action") || actionType == FTK::ActionType::ModifyStat)
        return j.get<std::shared_ptr<FTK::ModifyStatAction>>();
    if (isSerializedType("action"))
        return std::make_shared<FTK::Action>(j.get<FTK::Action>());
    throw std::invalid_argument("Unknown serial type");
}

NLOHMANN_ORDERED_JSON_ADL_SERIALIZE_DEFINITION(std::shared_ptr<FTK::Action>, action)
{
    // TODO better ways to serialize polymorphically
    if (action->getSerialType() == "damage_action")
        j = *std::static_pointer_cast<FTK::DamageAction>(action);
    else if (action->getSerialType() == "heal_action")
        j = *std::static_pointer_cast<FTK::HealAction>(action);
    else if (action->getSerialType() == "buff_action")
        j = *std::static_pointer_cast<FTK::BuffAction>(action);
    else if (action->getSerialType() == "flee_action")
        j = *std::static_pointer_cast<FTK::FleeAction>(action);
    else if (action->getSerialType() == "destroy_action")
        j = *std::static_pointer_cast<FTK::DestroyAction>(action);
    else if (action->getSerialType() == "modify_stat_action")
        j = *std::static_pointer_cast<FTK::ModifyStatAction>(action);
    else
        j = *action;
}

NLOHMANN_JSON_ADL_DESERIALIZE_DEFINITION(FTK::ActiveSkill)
{
    auto id = j["id"].get<std::string>();
    auto name = j["name"].get<std::string>();
    auto diceRolls = j["dice_rolls"].get<size_t>();
    auto rollChanceExpr = j["roll_chance_expr"].get<FTK::Math::Expression>();
    auto allowPartial = j.contains("allow_partial") ? j["allow_partial"].get<bool>() : false;
    auto skillType = j["skill_type"].get<FTK::SkillType>();
    auto targetType = j["target_type"].get<FTK::TargetType>();
    auto targetScope = FTK::TargetScope::Ally;
    if (targetType != FTK::TargetType::None && targetType != FTK::TargetType::Self)
        targetScope = j["target_scope"].get<FTK::TargetScope>();
    auto actions = j["actions"].get<std::vector<std::shared_ptr<FTK::Action>>>();
    auto baseCooldown = j["base_cooldown"].get<size_t>();
    auto description = j["description"].get<std::string>();

    return FTK::ActiveSkill{id, name, diceRolls, rollChanceExpr, allowPartial, skillType, targetType, targetScope, actions, baseCooldown, description};
}

NLOHMANN_ORDERED_JSON_ADL_SERIALIZE_DEFINITION(FTK::ActiveSkill, activeSkill)
{
    j["id"] = activeSkill.id;
    j["name"] = activeSkill.name;
    j["dice_rolls"] = activeSkill.diceRolls;
    j["roll_chance_expr"] = activeSkill.rollChanceExpr;
    if (activeSkill.allowPartial)
        j["allow_partial"] = activeSkill.allowPartial;
    j["skill_type"] = activeSkill.skillType;
    j["target_type"] = activeSkill.targetType;
    if (activeSkill.targetType != FTK::TargetType::None && activeSkill.targetType != FTK::TargetType::Self)
        j["target_scope"] = activeSkill.targetScope;
    j["actions"] = activeSkill.actions;
    j["base_cooldown"] = activeSkill.baseCooldown;
    j["description"] = activeSkill.description;
}

NLOHMANN_JSON_ADL_DESERIALIZE_DEFINITION(FTK::PassiveSkill)
{
    auto id = j["id"].get<std::string>();
    auto name = j["name"].get<std::string>();
    auto triggerType = j["trigger_type"].get<FTK::PassiveTriggerType>();
    auto requireActiveSkill = j.contains("require_active_skill") ? j["require_active_skill"].get<bool>() : false;
    auto actions = j["actions"].get<std::vector<std::shared_ptr<FTK::Action>>>();
    auto condition = j["condition"].get<FTK::Math::Condition>();
    auto baseCooldown = j["base_cooldown"].get<size_t>();
    auto description = j["description"].get<std::string>();
    return FTK::PassiveSkill{
        id,
        name,
        triggerType,
        requireActiveSkill,
        actions,
        condition,
        baseCooldown,
        description};
}

NLOHMANN_ORDERED_JSON_ADL_SERIALIZE_DEFINITION(FTK::PassiveSkill, passiveSkill)
{
    j["id"] = passiveSkill.id;
    j["name"] = passiveSkill.name;
    j["trigger_type"] = passiveSkill.triggerType;
    j["require_active_skill"] = passiveSkill.requireActiveSkill;
    j["actions"] = passiveSkill.actions;
    j["condition"] = passiveSkill.condition;
    j["base_cooldown"] = passiveSkill.baseCooldown;
    j["description"] = passiveSkill.description;
}

NLOHMANN_JSON_ADL_DESERIALIZE_DEFINITION(FTK::Buff)
{
    auto uuid = j["uuid"].get<uuids::uuid>();
    auto id = j["id"].get<std::string>();
    auto turns = j["turns"].get<int>();
    std::map<std::string, std::set<uuids::uuid>> attachedModifiers;
    if (j.contains("attached_modifiers"))
        attachedModifiers = j["attached_modifiers"].get<std::map<std::string, std::set<uuids::uuid>>>();
    std::map<std::string, std::multiset<FTK::Modifier>> modifierData;
    if (j.contains("modifier_data"))
        modifierData = j["modifier_data"].get<std::map<std::string, std::multiset<FTK::Modifier>>>();

    return FTK::Buff(
        uuid,
        id,
        turns,
        attachedModifiers,
        modifierData);
}

NLOHMANN_ORDERED_JSON_ADL_SERIALIZE_DEFINITION(FTK::Buff, buff)
{
    j["uuid"] = buff.uuid;
    j["id"] = buff.id;
    j["turns"] = buff.turns;
    if (buff.attachedModifiers.size())
        j["attached_modifiers"] = buff.attachedModifiers;
    if (buff.modifierData.size())
        j["modifier_data"] = buff.modifierData;
}

NLOHMANN_JSON_ADL_DESERIALIZE_DEFINITION(FTK::BuffTemplate)
{
    auto id = j["id"].get<std::string>();
    auto name = j["name"].get<std::string>();
    auto effectType = j["effect_type"].get<FTK::EffectType>();
    std::vector<FTK::ModifierTemplate> modifiers;
    std::vector<std::shared_ptr<FTK::Action>> actions;
    switch (effectType)
    {
    case FTK::EffectType::None:
    case FTK::EffectType::SkipTurn:
        break;
    case FTK::EffectType::Modifier:
        modifiers = j["modifiers"].get<std::vector<FTK::ModifierTemplate>>();
        break;
    case FTK::EffectType::Action:
        actions = j["actions"].get<std::vector<std::shared_ptr<FTK::Action>>>();
        break;
    default:
        break;
    }
    auto description = j["description"].get<std::string>();
    return FTK::BuffTemplate{id, name, effectType, modifiers, actions, description};
}

NLOHMANN_ORDERED_JSON_ADL_SERIALIZE_DEFINITION(FTK::BuffTemplate, buffTemplate)
{
    j["id"] = buffTemplate.id;
    j["name"] = buffTemplate.name;
    j["effect_type"] = buffTemplate.effectType;
    if (buffTemplate.modifiers.size())
        j["modifiers"] = buffTemplate.modifiers;
    if (buffTemplate.actions.size())
        j["actions"] = buffTemplate.actions;
    j["description"] = buffTemplate.description;
}

NLOHMANN_JSON_ADL_DESERIALIZE_DEFINITION(FTK::BuffBuildData)
{
    auto id = j["id"].get<std::string>();
    auto turns = j["turns"].get<int>();

    return FTK::BuffBuildData{id, turns};
}

NLOHMANN_ORDERED_JSON_ADL_SERIALIZE_DEFINITION(FTK::BuffBuildData, buffBuildData)
{
    j["id"] = buffBuildData.id;
    j["turns"] = buffBuildData.turns;
}

NLOHMANN_JSON_ADL_DESERIALIZE_DEFINITION(FTK::ItemData)
{
    auto id = j["id"].get<std::string>();
    auto amount = j["amount"].get<int>();
    return FTK::ItemData{id, amount};
}

NLOHMANN_ORDERED_JSON_ADL_SERIALIZE_DEFINITION(FTK::ItemData, itemData)
{
    j["id"] = itemData.id;
    j["amount"] = itemData.amount;
}

NLOHMANN_JSON_ADL_DESERIALIZE_DEFINITION(FTK::ItemTemplate)
{
    auto id = j["id"].get<std::string>();
    auto name = j["name"].get<std::string>();
    auto availableInCombat = j.contains("available_in_combat") ? j["available_in_combat"].get<bool>() : false;
    auto actionsOnUse = j["actions"].get<std::vector<std::shared_ptr<FTK::Action>>>();
    auto shopValue = j["shop_value"].get<int>();
    auto description = j["description"].get<std::string>();
    return FTK::ItemTemplate{id, name, availableInCombat, actionsOnUse, shopValue, description};
}

NLOHMANN_ORDERED_JSON_ADL_SERIALIZE_DEFINITION(FTK::ItemTemplate, itemTemplate)
{
    j["id"] = itemTemplate.id;
    j["name"] = itemTemplate.name;
    if (itemTemplate.availableInCombat)
        j["available_in_combat"] = itemTemplate.availableInCombat;
    j["actions"] = itemTemplate.actionsOnUse;
    j["shop_value"] = itemTemplate.shopValue;
    j["description"] = itemTemplate.description;
}

NLOHMANN_JSON_ADL_DESERIALIZE_DEFINITION(FTK::Equipment)
{
    auto uuid = j.contains("uuid") ? j["uuid"].get<uuids::uuid>() : ""_json.get<uuids::uuid>();
    auto id = j["id"].get<std::string>();
    std::map<std::string, std::set<uuids::uuid>> attachedModifiers;
    if (j.contains("attached_modifiers"))
        attachedModifiers = j["attached_modifiers"].get<std::map<std::string, std::set<uuids::uuid>>>();
    std::map<std::string, std::multiset<FTK::Modifier>> modifierData;
    if (j.contains("modifier_data"))
        modifierData = j["modifier_data"].get<std::map<std::string, std::multiset<FTK::Modifier>>>();

    return FTK::Equipment(
        uuid,
        id,
        attachedModifiers,
        modifierData);
}

NLOHMANN_ORDERED_JSON_ADL_SERIALIZE_DEFINITION(FTK::Equipment, equipment)
{
    j["uuid"] = equipment.uuid;
    j["id"] = equipment.id;
    if (equipment.attachedModifiers.size())
        j["attached_modifiers"] = equipment.attachedModifiers;
    if (equipment.modifierData.size())
        j["modifier_data"] = equipment.modifierData;
}

NLOHMANN_JSON_ADL_DESERIALIZE_DEFINITION(FTK::EquipmentTemplate)
{
    auto id = j["id"].get<std::string>();
    auto name = j["name"].get<std::string>();
    auto equipmentType = j["equipment_type"].get<FTK::EquipmentType>();
    auto weaponDiceRolls = 1;
    auto weaponDamageType = FTK::DamageType::Physical;
    if (equipmentType == FTK::EquipmentType::Weapon)
    {
        weaponDiceRolls = j["weapon_dice_rolls"].get<int>();
        weaponDamageType = j["weapon_damage_type"].get<FTK::DamageType>();
    }
    auto modifiers = j.contains("modifiers") ? j["modifiers"].get<std::vector<FTK::ModifierTemplate>>() : std::vector<FTK::ModifierTemplate>{};
    auto skills = j.contains("skills") ? j["skills"].get<std::vector<std::string>>() : std::vector<std::string>{};
    auto shopValue = j["shop_value"].get<int>();
    auto description = j["description"].get<std::string>();
    return FTK::EquipmentTemplate{id, name, equipmentType, weaponDiceRolls, weaponDamageType, modifiers, skills, shopValue, description};
}

NLOHMANN_ORDERED_JSON_ADL_SERIALIZE_DEFINITION(FTK::EquipmentTemplate, equipmentTemplate)
{
    j["id"] = equipmentTemplate.id;
    j["name"] = equipmentTemplate.name;
    j["equipment_type"] = equipmentTemplate.equipmentType;
    if (equipmentTemplate.equipmentType == FTK::EquipmentType::Weapon)
    {
        j["weapon_dice_rolls"] = equipmentTemplate.weaponDiceRolls;
        j["weapon_damage_type"] = equipmentTemplate.weaponDamageType;
    }
    j["modifiers"] = equipmentTemplate.modifiers;
    j["skills"] = equipmentTemplate.skills;
    j["shop_value"] = equipmentTemplate.shopValue;
    j["description"] = equipmentTemplate.description;
}

NLOHMANN_JSON_ADL_DESERIALIZE_DEFINITION(FTK::Inventory)
{
    if (j.empty())
        return FTK::Inventory();
    auto gold = j["gold"].get<int>();
    auto items = j["items"].get<std::multiset<FTK::ItemData>>();
    auto equipments = j["equipments"].get<std::multiset<FTK::Equipment>>();
    return FTK::Inventory(gold, items, equipments);
}

NLOHMANN_ORDERED_JSON_ADL_SERIALIZE_DEFINITION(FTK::Inventory, inventory)
{
    j["gold"] = inventory.gold;
    j["items"] = inventory.items;
    j["equipments"] = inventory.equipments;
}

NLOHMANN_JSON_NAMESPACE_END
