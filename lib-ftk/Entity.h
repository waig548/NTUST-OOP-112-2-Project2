#ifndef FTK_ENTITY_H
#define FTK_ENTITY_H

#include <string>
#include <map>
#include <set>

#include <nlohmann/adl_serializer.hpp>

#include "defs.h"
#include "Modifier.h"
#include "Vec.h"
#include "Expr.h"
#include "Buff.h"
#include "Item.h"

namespace FTK
{
    class Entity
    {
    public:
        class Attribute : private NamedModifiableValue
        {
        public:
            Attribute(const std::string &name, double base);
            Attribute(const Attribute &other);
            ~Attribute();

            using NamedModifiableValue::get;

            using NamedModifiableValue::addModifier;
            using NamedModifiableValue::removeModifier;

            using NamedModifiableValue::name;

        private:
            explicit Attribute(const NamedModifiableValue &modifiableValue);
            Attribute(const std::string &name, double base, const std::multiset<Modifier> &modifiers);

            void eval();

            friend nlohmann::adl_serializer<Attribute>;
        };

        class Stat : private NamedModifiableValue
        {
        public:
            Stat(const std::string &name, double value);
            Stat(const Stat &other);
            ~Stat();

            using NamedModifiableValue::get;
            using NamedModifiableValue::set;

            double increment();
            double decrement();

            using NamedModifiableValue::addModifier;
            using NamedModifiableValue::removeModifier;

            using NamedModifiableValue::name;

        private:
            explicit Stat(const NamedModifiableValue &modifiableValue);
            Stat(const std::string &name, double base, const std::multiset<Modifier> &modifiers);

            friend nlohmann::adl_serializer<Stat>;
        };

        Entity(const std::string &id, const std::string &name, const Vec2i &pos);
        Entity(const Entity &other);
        virtual ~Entity();

        double operator[](const std::string &key) const;

        Vec2i getPos() const;
        Vec2i getPrevPos() const;
        DamageType getDamageType() const;
        std::map<std::string, int> getSkillCD() const;
        std::map<std::string, int> getActiveSkillCD() const;
        std::map<std::string, int> getPassiveSkillCD() const;
        std::multiset<Buff> getBuffs() const;
        std::map<EquipmentType, std::optional<Equipment>> getEquipments() const;

        double get(const std::string &key) const;
        int getAsInt(const std::string &key) const;

        bool isDead() const;

        int getWeaponDiceRoll() const;

        Math::Context getMathContext(const std::string &key, const Math::Context &base = Math::Context::default_global()) const;

        virtual bool isEnemy() const;
        virtual bool isPlayer() const;

        void setSkillCD(const std::string &skillID, int newCD);
        void resetSkillCD(const std::string &skillID);
        void updateSkillCD();

        void addBuff(const Buff &buff);
        void removeBuff(const uuids::uuid &buffUUID);

        void clearBuffs();
        void checkBuffs(int threshold = 0);
        void updateBuffs();

        void addEquipment(const std::optional<Equipment> &equipment);
        std::optional<Equipment> removeEquipment(EquipmentType slot);

        void set(const std::string &key, double value);
        void inc(const std::string &key, double increment = 1);
        void dec(const std::string &key, double decrement = 1);

        const std::vector<Attribute> getAttributes() const;
        const std::vector<Stat> getStats() const;

        void setPos(const Vec2i &newPos, bool retreat = false);

        const uuids::uuid uuid;
        const std::string id;
        const std::string name;

    protected:
        Entity(const uuids::uuid uuid, const std::string &id, const std::string &name, const std::vector<Attribute> &attributes, const std::vector<Stat> &stats, const Vec2i &pos, const Vec2i &prevPos, const std::map<std::string, int> &skillCD, const std::multiset<Buff> &buffs, const std::map<EquipmentType, std::optional<Equipment>> &equipments);

        Attribute &findAttr(const std::string &key);
        Stat &findStat(const std::string &key);

        void _set(const std::string &key, double value);

        void initEquipments();
        void initBuffs();

        virtual void updateValues();

        void addSkills(const std::vector<std::string> &skillIDs);
        void removeSkills(const std::vector<std::string> &skillIDs);

        void addModifierToAttr(const std::string &key, const Modifier &mod);
        void removeModifierFromAttr(const std::string &key, const uuids::uuid modUUID);

        void addModifierToStat(const std::string &key, const Modifier &mod);
        void removeModifierFromStat(const std::string &key, const uuids::uuid modUUID);

        void createAttributeIfMissing(const std::string &key, double defaultValue);
        void createStatIfMissing(const std::string &key, double defaultValue);

        std::vector<Attribute> attributes;
        std::vector<Stat> stats;
        Vec2i pos;
        Vec2i prevPos;
        DamageType defaultDamageType = DamageType::Physical;
        std::map<std::string, int> skillCD;
        std::multiset<Buff> buffs;
        std::map<EquipmentType, std::optional<Equipment>> equipments;

        virtual std::string getSerialType() const;

        friend class GameManager;

        friend class Player;
        friend class Enemy;

        friend nlohmann::adl_serializer<Entity>;
        friend nlohmann::adl_serializer<std::shared_ptr<Entity>>;
    };

    class Player : public Entity
    {
    public:
        Player(const Player &other);

        bool isPlayer() const override;

        int getAP() const;
        double getAPChance() const;

    private:
        explicit Player(const Entity &dataModel);
        Player(const uuids::uuid uuid, const std::string &id, const std::string &name, const std::vector<Attribute> &attributes, const std::vector<Stat> &stats, const Vec2i &pos, const Vec2i &prevPos, const std::map<std::string, int> &skillCD, const std::multiset<Buff> &buffs, const std::map<EquipmentType, std::optional<Equipment>> &equipments);

        void updateValues() override;

        std::string getSerialType() const override;

        friend nlohmann::adl_serializer<Player>;
    };

    class PlayerTemplate
    {
    };

    class Enemy : public Entity
    {
    public:
        Enemy(const Enemy &other);

        bool isEnemy() const override;

    private:
        explicit Enemy(const Entity &dataModel);
        Enemy(const uuids::uuid uuid, const std::string &id, const std::string &name, const std::vector<Attribute> &attributes, const std::vector<Stat> &stats, const Vec2i &pos, const Vec2i &prevPos, const std::map<std::string, int> &skillCD, const std::multiset<Buff> &buffs, const std::map<EquipmentType, std::optional<Equipment>> &equipments);

        std::string getSerialType() const override;

        friend nlohmann::adl_serializer<Enemy>;
    };

    class EnemyTemplate
    {
    };

} // namespace FTK

#endif // FTK_ENTITY_H
