#include "Entity.h"

#include "defs.h"
#include "Registry.h"

static std::string AttrPattern("attr.");
static std::string StatPattern("stat.");
namespace FTK
{
    Entity::Attribute::Attribute(const std::string &name, double base) : Attribute(name, base, {})
    {
    }

    Entity::Attribute::Attribute(const Attribute &other) : Attribute(other.name, other.base, other.modifiers)
    {
    }

    Entity::Attribute::~Attribute()
    {
    }

    Entity::Attribute::Attribute(const NamedModifiableValue &value) : NamedModifiableValue(value)
    {
    }

    Entity::Attribute::Attribute(const std::string &name, double base, const std::multiset<Modifier> &modifiers) : Attribute(NamedModifiableValue(name, base, modifiers))
    {
    }

    void Entity::Attribute::eval()
    {
        NamedModifiableValue::eval();
        if (auto att = ATTRIBUTE_RANGES(name); att.has_value())
            value = std::max(std::min(value, att->range.max), att->range.min);
    }

    Entity::Stat::Stat(const std::string &name, double value) : Stat(name, value, {})
    {
    }

    Entity::Stat::Stat(const Stat &other) : Stat(other.name, other.value, other.modifiers)
    {
    }

    Entity::Stat::~Stat()
    {
    }

    double Entity::Stat::increment()
    {
        set(value + 1);
        return get();
    }

    double Entity::Stat::decrement()
    {
        set(value - 1);
        return get();
    }

    Entity::Stat::Stat(const NamedModifiableValue &value) : NamedModifiableValue(value)
    {
    }

    Entity::Stat::Stat(const std::string &name, double value, const std::multiset<Modifier> &modifiers) : Stat(NamedModifiableValue(name, value, modifiers))
    {
    }

    Entity::Entity(const Entity &other) : Entity(other.uuid, other.id, other.name, other.attributes, other.stats, other.pos, other.prevPos, other.skillCD, other.buffs, other.equipments)
    {
    }

    Entity::~Entity()
    {
    }

    double Entity::operator[](const std::string &key) const
    {
        return get(key);
    }

    Vec2i Entity::getPos() const
    {
        return pos;
    }

    Vec2i Entity::getPrevPos() const
    {
        return prevPos;
    }

    DamageType Entity::getDamageType() const
    {
        return defaultDamageType;
    }

    std::map<std::string, int> Entity::getSkillCD() const
    {
        return skillCD;
    }

    std::map<std::string, int> Entity::getActiveSkillCD() const
    {
        std::map<std::string, int> res;
        for (auto p : skillCD)
            if (p.first._Starts_with("active"))
                res[p.first] = p.second;
        return res;
    }

    std::map<std::string, int> Entity::getPassiveSkillCD() const
    {
        std::map<std::string, int> res;
        for (auto p : skillCD)
            if (p.first._Starts_with("passive"))
                res[p.first] = p.second;
        return res;
    }

    std::multiset<Buff> Entity::getBuffs() const
    {
        return buffs;
    }

    std::map<EquipmentType, std::optional<Equipment>> Entity::getEquipments() const
    {
        return equipments;
    }

    double Entity::get(const std::string &key) const
    {
        if (auto it = std::find_if(attributes.begin(), attributes.end(), [&key](const Attribute &attr)
                                   { return attr.name == key; });
            it != attributes.end())
            return it->get();
        if (auto it = std::find_if(stats.begin(), stats.end(), [&key](const Stat &s)
                                   { return s.name == key; });
            it != stats.end())
            return it->get();
        throw std::invalid_argument("Key not found");
    }

    int Entity::getAsInt(const std::string &key) const
    {
        return (int)get(key);
    }

    bool Entity::isDead() const
    {
        return getAsInt("hp") <= 0;
    }

    int Entity::getWeaponDiceRoll() const
    {
        if (equipments.at(EquipmentType::Weapon))
            return MainRegistry::getInstance()->equipmentTemplates->get(equipments.at(EquipmentType::Weapon)->id).weaponDiceRolls;
        return 1;
    }

    Math::Context Entity::getMathContext(const std::string &key, const Math::Context &base) const
    {
        Math::Context res = base;
        res[key] = Math::Context();
        for (auto a : attributes)
            res[key][a.name] = a.get();
        for (auto s : stats)
            res[key][s.name] = s.get();
        res[key]["atk"] = getDamageType() == DamageType::Physical ? get("p_atk") : getDamageType() == DamageType::Magical ? get("m_atk")
                                                                                                                          : 0;
        return res;
    }

    bool Entity::isEnemy() const
    {
        return false;
    }

    bool Entity::isPlayer() const
    {
        return false;
    }

    void Entity::setSkillCD(const std::string &skillID, int newCD)
    {
        if (skillCD.count(skillID))
            skillCD[skillID] = newCD;
    }

    void Entity::resetSkillCD(const std::string &skillID)
    {
        if (skillID._Starts_with("active"))
            setSkillCD(skillID, MainRegistry::getInstance()->activeSkills->get(skillID).baseCooldown);
        if (skillID._Starts_with("passive"))
            setSkillCD(skillID, MainRegistry::getInstance()->passiveSkills->get(skillID).baseCooldown);
    }

    void Entity::updateSkillCD()
    {
        for (auto &p : skillCD)
            if (p.second > 0)
                p.second--;
    }

    void Entity::addBuff(const Buff &buff)
    {
        uuids::uuid markedRemoved = {};
        if (auto it = std::find_if(buffs.begin(), buffs.end(), [buff](Buff b)
                                   { return buff == b; });
            it != buffs.end())
        {
            if (it->getTurns() <= buff.getTurns())
            {
                it->setTurns(buff.getTurns());
                return;
            }
        }

        if (auto it = buffs.begin(); it != buffs.end())
        {
            while (it != buffs.end())
            {
                if (*it < buff)
                {
                    markedRemoved = it->uuid;
                    break;
                }
                it++;
            }
        }
        if (!markedRemoved.is_nil())
            removeBuff(markedRemoved);
        auto toBeInserted = buff;
        for (auto p : toBeInserted.getModifierData())
        {
            if (p.first._Starts_with(AttrPattern))
            {
                auto realPath = p.first.substr(AttrPattern.size());
                for (auto mod : p.second)
                {
                    addModifierToAttr(realPath, mod);
                    toBeInserted.markAttached(p.first, mod.uuid);
                }
            }
            else if (p.first._Starts_with(StatPattern))
            {
                auto realPath = p.first.substr(AttrPattern.size());
                for (auto mod : p.second)
                {
                    addModifierToStat(realPath, mod);
                    toBeInserted.markAttached(p.first, mod.uuid);
                }
            }
        }
        buffs.insert(toBeInserted);
        updateValues();
    }

    void Entity::removeBuff(const uuids::uuid &buffUUID)
    {
        if (auto it = std::find_if(buffs.begin(), buffs.end(), [buffUUID](Buff b)
                                   { return b.uuid == buffUUID; });
            it != buffs.end())
        {
            auto modsToRemove = it->getAttachedModifiers();

            for (auto p : modsToRemove)
            {
                if (p.first._Starts_with(AttrPattern))
                {
                    auto realPath = p.first.substr(AttrPattern.size());
                    for (auto modUUID : p.second)
                        removeModifierFromAttr(realPath, modUUID);
                }
                else if (p.first._Starts_with(StatPattern))
                {
                    auto realPath = p.first.substr(AttrPattern.size());
                    for (auto modUUID : p.second)
                        removeModifierFromStat(realPath, modUUID);
                }
            }
            buffs.erase(it);
        }
        updateValues();
    }

    void Entity::clearBuffs()
    {
        std::vector<uuids::uuid> markedRemoved;
        for (auto b : buffs)
            markedRemoved.push_back(b.uuid);
        for (auto uuid : markedRemoved)
            removeBuff(uuid);
    }

    void Entity::checkBuffs(int threshold)
    {
        std::vector<uuids::uuid> markedRemoved;
        if (auto it = buffs.begin(); it != buffs.end())
        {
            while (it != buffs.end())
            {
                if (it->expired(threshold))
                    markedRemoved.push_back(it->uuid);
                it++;
            }
        }
        for (auto uuid : markedRemoved)
            removeBuff(uuid);
    }

    void Entity::updateBuffs()
    {
        if (auto it = buffs.begin(); it != buffs.end())
        {
            while (it != buffs.end())
            {
                it->decTurns();
                it++;
            }
        }
        checkBuffs();
    }

    void Entity::addEquipment(const std::optional<Equipment> &equipment)
    {
        if (!equipment)
            return;
        auto equipData = MainRegistry::getInstance()->equipmentTemplates->get(equipment->id);
        if (equipments.at(equipData.equipmentType))
            throw std::invalid_argument("Already has an equipment of the same type");
        auto toBeEquipped = *equipment;
        for (auto p : toBeEquipped.getModifierData())
        {
            if (p.first._Starts_with(AttrPattern))
            {
                auto realPath = p.first.substr(AttrPattern.size());
                for (auto mod : p.second)
                {
                    addModifierToAttr(realPath, mod);
                    toBeEquipped.markModAttached(p.first, mod.uuid);
                }
            }
            else if (p.first._Starts_with(StatPattern))
            {
                auto realPath = p.first.substr(AttrPattern.size());
                for (auto mod : p.second)
                {
                    addModifierToStat(realPath, mod);
                    toBeEquipped.markModAttached(p.first, mod.uuid);
                }
            }
        }
        addSkills(equipData.skills);
        equipments.erase(equipData.equipmentType);
        equipments.emplace(equipData.equipmentType, toBeEquipped);
        updateValues();
    }

    std::optional<Equipment> Entity::removeEquipment(EquipmentType slot)
    {
        auto res = equipments.at(slot);
        if (res)
        {
            auto modsToRemove = res->getAttachedModifiers();
            for (auto p : modsToRemove)
            {
                if (p.first._Starts_with(AttrPattern))
                {
                    auto realPath = p.first.substr(AttrPattern.size());
                    for (auto modUUID : p.second)
                    {
                        removeModifierFromAttr(realPath, modUUID);
                        res->markModDetached(p.first, modUUID);
                    }
                }
                else if (p.first._Starts_with(StatPattern))
                {
                    auto realPath = p.first.substr(AttrPattern.size());
                    for (auto modUUID : p.second)
                    {
                        removeModifierFromStat(realPath, modUUID);
                        res->markModDetached(p.first, modUUID);
                    }
                }
            }
            equipments.at(slot).reset();
            removeSkills(MainRegistry::getInstance()->equipmentTemplates->get(res->id).skills);
            updateValues();
        }
        return res;
    }

    void Entity::set(const std::string &key, double value)
    {
        _set(key, value);
        updateValues();
    }

    void Entity::inc(const std::string &key, double increment)
    {
        set(key, get(key) + increment);
    }

    void Entity::dec(const std::string &key, double decrement)
    {
        set(key, get(key) - decrement);
    }

    const std::vector<Entity::Attribute> Entity::getAttributes() const
    {
        return attributes;
    }

    const std::vector<Entity::Stat> Entity::getStats() const
    {
        return stats;
    }

    void Entity::setPos(const Vec2i &newPos, bool retreat)
    {
        if (!retreat)
            prevPos = pos;
        pos = newPos;
    }

    Entity::Entity(const std::string &id, const std::string &name, const Vec2i &pos) : Entity(uuids::uuid_system_generator{}(), id, name, {}, {}, pos, pos, {}, {}, {})
    {
    }

    Entity::Entity(const uuids::uuid uuid, const std::string &id, const std::string &name, const std::vector<Attribute> &attributes, const std::vector<Stat> &stats, const Vec2i &pos, const Vec2i &prevPos, const std::map<std::string, int> &skillCD, const std::multiset<Buff> &buffs, const std::map<EquipmentType, std::optional<Equipment>> &equipments) : uuid(uuid), id(id), name(name), attributes(attributes), stats(stats), pos(pos), prevPos(prevPos), skillCD(skillCD), buffs(buffs), equipments(equipments)
    {
        for (auto attr : GeneralAttributeDefs)
            createAttributeIfMissing(attr.name, attr.defaultValue);
        createStatIfMissing("hp", get("max_hp"));
        createStatIfMissing("dice_guarentee", 0);

        this->equipments.try_emplace(EquipmentType::Weapon);
        this->equipments.try_emplace(EquipmentType::Armor);
        this->equipments.try_emplace(EquipmentType::Accessory);

        this->skillCD.try_emplace("active:basic_attack", 0);
    }

    Entity::Attribute &Entity::findAttr(const std::string &key)
    {
        if (auto it = std::find_if(attributes.begin(), attributes.end(), [&key](const Attribute &attr)
                                   { return attr.name == key; });
            it != attributes.end())
            return *it;
        throw std::invalid_argument("Key not found");
    }

    Entity::Stat &Entity::findStat(const std::string &key)
    {
        if (auto it = std::find_if(stats.begin(), stats.end(), [&key](const Stat &s)
                                   { return s.name == key; });
            it != stats.end())
            return *it;
        throw std::invalid_argument("Key not found");
    }

    void Entity::_set(const std::string &key, double value)
    {
        if (auto it = std::find_if(stats.begin(), stats.end(), [&key](const Stat &s)
                                   { return s.name == key; });
            it != stats.end())
            it->set(value);
    }

    void Entity::initEquipments()
    {
        auto equipList = MainRegistry::getInstance()->equipmentTemplates;
        std::vector<std::string> tmp;
        for (auto &p : equipments)
            if (p.second)
            {
                tmp.push_back(p.second->id);
                p.second.reset();
            }

        for (auto e : tmp)
            addEquipment(equipList->get(e).build());
    }

    void Entity::initBuffs()
    {
        auto buffList = MainRegistry::getInstance()->buffTemplates;
        std::vector<std::pair<std::string, int>> tmp;
        for (auto buff : buffs)
            tmp.push_back({buff.id, buff.getTurns()});
        clearBuffs();
        for (auto p : tmp)
            addBuff(buffList->get(p.first).build(p.second));
    }

    void Entity::updateValues()
    {
        _set("hp", std::max(0.0, std::min(get("max_hp"), get("hp"))));
    }

    void Entity::addSkills(const std::vector<std::string> &skillIDs)
    {
        for (auto s : skillIDs)
            skillCD.try_emplace(s, 0);
    }

    void Entity::removeSkills(const std::vector<std::string> &skillIDs)
    {
        std::set<std::string> allocatedSkills;
        auto equipList = MainRegistry::getInstance()->equipmentTemplates;
        for (auto e : equipments)
            if (e.second)
                for (auto s : equipList->get(e.second->id).skills)
                    allocatedSkills.insert(s);
        for (auto s : skillIDs)
            if (!allocatedSkills.count(s))
                skillCD.erase(s);
    }

    void Entity::addModifierToAttr(const std::string &key, const Modifier &mod)
    {
        auto &attr = findAttr(key);
        attr.addModifier(mod);
        updateValues();
    }

    void Entity::removeModifierFromAttr(const std::string &key, const uuids::uuid modUUID)
    {
        auto &attr = findAttr(key);
        attr.removeModifier(modUUID);
        updateValues();
    }

    void Entity::addModifierToStat(const std::string &key, const Modifier &mod)
    {
        auto &stat = findStat(key);
        stat.addModifier(mod);
        updateValues();
    }

    void Entity::removeModifierFromStat(const std::string &key, const uuids::uuid modUUID)
    {
        auto &stat = findStat(key);
        stat.removeModifier(modUUID);
        updateValues();
    }

    std::string Entity::getSerialType() const
    {
        return "entity";
    }

    void Entity::createAttributeIfMissing(const std::string &key, double defaultValue)
    {
        if (auto it = std::find_if(attributes.begin(), attributes.end(), [&key](const Attribute &attr)
                                   { return attr.name == key; });
            it == attributes.end())

            attributes.push_back(Attribute(key, defaultValue));
    }

    void Entity::createStatIfMissing(const std::string &key, double defaultValue)
    {
        if (auto it = std::find_if(stats.begin(), stats.end(), [&key](const Stat &s)
                                   { return s.name == key; });
            it == stats.end())
            stats.push_back(Stat(key, defaultValue));
    }

    Player::Player(const Player &other) : Player(other.uuid, other.id, other.name, other.attributes, other.stats, other.pos, other.prevPos, other.skillCD, other.buffs, other.equipments)
    {
    }

    bool Player::isPlayer() const
    {
        return true;
    }

    int Player::getAP() const
    {
        return get("ap");
    }

    double Player::getAPChance() const
    {
        return std::min(0.9, get("speed") / 100);
    }

    Player::Player(const Entity &dataModel) : Entity(dataModel)
    {
        for (auto attr : PlayerAdditionalAttributeDefs)
            createAttributeIfMissing(attr.name, attr.defaultValue);
        createStatIfMissing("focus", getAsInt("max_focus"));
        createStatIfMissing("max_ap", 0);
        createStatIfMissing("ap", 0);

        skillCD.try_emplace("active:flee", 0);
        skillCD.try_emplace("active:dummy_buff", 0);
        skillCD.try_emplace("active:seppuku");
    }

    Player::Player(const uuids::uuid uuid, const std::string &id, const std::string &name, const std::vector<Attribute> &attributes, const std::vector<Stat> &stats, const Vec2i &pos, const Vec2i &prevPos, const std::map<std::string, int> &skillCD, const std::multiset<Buff> &buffs, const std::map<EquipmentType, std::optional<Equipment>> &equipments) : Player(Entity(uuid, id, name, attributes, stats, pos, prevPos, skillCD, buffs, equipments))
    {
    }

    void Player::updateValues()
    {
        Entity::updateValues();
        _set("focus", std::max(0.0, std::min(get("focus"), get("max_focus"))));
        _set("ap", std::max(0.0, std::min(get("ap"), get("max_ap"))));
    }

    std::string Player::getSerialType() const
    {
        return "player";
    }

    Enemy::Enemy(const Enemy &other) : Enemy(other.uuid, other.id, other.name, other.attributes, other.stats, other.pos, other.prevPos, other.skillCD, other.buffs, other.equipments)
    {
    }

    bool Enemy::isEnemy() const
    {
        return true;
    }

    Enemy::Enemy(const Entity &dataModel) : Entity(dataModel)
    {
    }

    Enemy::Enemy(const uuids::uuid uuid, const std::string &id, const std::string &name, const std::vector<Attribute> &attributes, const std::vector<Stat> &stats, const Vec2i &pos, const Vec2i &prevPos, const std::map<std::string, int> &skillCD, const std::multiset<Buff> &buffs, const std::map<EquipmentType, std::optional<Equipment>> &equipments) : Enemy(Entity(uuid, id, name, attributes, stats, pos, prevPos, skillCD, buffs, equipments))
    {
    }

    std::string Enemy::getSerialType() const
    {
        return "enemy";
    }

} // namespace FTK
