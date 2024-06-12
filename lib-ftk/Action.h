#ifndef FTK_ACTION_H
#define FTK_ACTION_H

#include <nlohmann/json.hpp>
#include <uuid.h>

#include "defs.h"
#include "Expr.h"
#include "Buff.h"
#include "Entity.h"

namespace FTK
{
    struct ActionContext
    {
        Math::Context mathContext = Math::Context::default_global();
        Math::Condition condition = "1";
        std::map<std::string, std::vector<uuids::uuid>> modsToBeRemoved = {};
    };

    class Action
    {
    public:
        Action(ActionType actionType, TargetType targetType, TargetScope targetScope);
        Action(const Action &other);
        virtual ~Action() = default;

        virtual bool shouldHaveEffect(const ActionContext &ctx, bool activeSkill = false) const;
        virtual void apply(const std::shared_ptr<Entity> &source, const std::shared_ptr<Entity> &target, ActionContext &context, bool activeSkill = false) const;

        virtual Math::Context getContext(const std::string &key, const Math::Context &base = Math::Context::default_global()) const;

        virtual std::string getSerialType() const;

        const ActionType actionType;
        const TargetType targetType;
        const TargetScope targetScope;

    private:
        friend nlohmann::adl_serializer<Action>;
        friend nlohmann::adl_serializer<std::shared_ptr<Action>>;
    };

    class DamageAction : public Action
    {
    public:
        DamageAction(ActionType actionType, TargetType targetType, TargetScope targetScope, DamageType damageType, Math::Expression damageExpr);
        DamageAction(const DamageAction &other);
        ~DamageAction();

        bool shouldHaveEffect(const ActionContext &ctx, bool activeSkill = false) const override;
        void apply(const std::shared_ptr<Entity> &source, const std::shared_ptr<Entity> &target, ActionContext &context, bool activeSkill = false) const override;

        Math::Context getContext(const std::string &key, const Math::Context &base = Math::Context::default_global()) const override;

        std::string getSerialType() const override;

        const DamageType damageType;
        const Math::Expression damageExpr;

    private:
        friend nlohmann::adl_serializer<DamageAction>;
    };

    class HealAction : public Action
    {
    public:
        HealAction(ActionType actionType, TargetType targetType, TargetScope targetScope, Math::Expression healExpr);
        HealAction(const HealAction &other);
        ~HealAction();

        virtual bool shouldHaveEffect(const ActionContext &ctx, bool activeSkill = false) const override;
        void apply(const std::shared_ptr<Entity> &source, const std::shared_ptr<Entity> &target, ActionContext &context, bool activeSkill = false) const override;

        Math::Context getContext(const std::string &key, const Math::Context &base = Math::Context::default_global()) const override;

        std::string getSerialType() const override;

        const DamageType damageType = DamageType::Heal;
        const Math::Expression healExpr;

    private:
        friend nlohmann::adl_serializer<DamageAction>;
    };

    class BuffAction : public Action
    {
    public:
        BuffAction(ActionType actionType, TargetType targetType, TargetScope targetScope, const std::vector<BuffBuildData> &buffs);
        BuffAction(const BuffAction &other);
        ~BuffAction();

        void apply(const std::shared_ptr<Entity> &source, const std::shared_ptr<Entity> &target, ActionContext &context, bool activeSkill = false) const override;

        std::string getSerialType() const override;

        const std::vector<BuffBuildData> buffs;

    private:
        friend nlohmann::adl_serializer<BuffAction>;
    };

    class FleeAction : public Action
    {
    public:
        FleeAction(ActionType actionType, TargetType targetType, TargetScope targetScope);
        FleeAction(const FleeAction &other);
        ~FleeAction();

        void apply(const std::shared_ptr<Entity> &source, const std::shared_ptr<Entity> &target, ActionContext &context, bool activeSkill = false) const override;

        std::string getSerialType() const override;

    private:
        friend nlohmann::adl_serializer<FleeAction>;
    };

    class DestroyAction : public Action
    {
    public:
        DestroyAction(ActionType actionType, TargetType targetType, TargetScope targetScope, int destroyAmount);
        DestroyAction(const DestroyAction &other);
        ~DestroyAction();

        virtual bool shouldHaveEffect(const ActionContext &ctx, bool activeSkill = false) const override;
        void apply(const std::shared_ptr<Entity> &source, const std::shared_ptr<Entity> &target, ActionContext &context, bool activeSkill = false) const override;

        std::string getSerialType() const override;

        const int destroyAmount;

    private:
        friend nlohmann::adl_serializer<DestroyAction>;
    };

    class ModifyStatAction : public Action
    {
    public:
        ModifyStatAction(ActionType actionType, TargetType targetType, TargetScope targetScope, const std::string &targetPath, double modifyAmount);
        ModifyStatAction(const ModifyStatAction &other);
        ~ModifyStatAction();

        void apply(const std::shared_ptr<Entity> &source, const std::shared_ptr<Entity> &target, ActionContext &context, bool activeSkill = false) const override;

        std::string getSerialType() const override;

        const std::string targetPath;
        const double modifyAmount;

    private:
        friend nlohmann::adl_serializer<ModifyStatAction>;
    };
} // namespace FTK

#endif // FTK_ACTION_H
