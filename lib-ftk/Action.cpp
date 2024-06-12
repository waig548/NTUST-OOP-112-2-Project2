#include "Action.h"

#include <effolkronium/random.hpp>

#include "Buff.h"
#include "Registry.h"

static FTK::Math::Expression diceMult("rolled_result/dice_rolls");
static FTK::Math::Condition requireCriticalSuccess("rolled_result==dice_rolls");
static FTK::Math::Condition requireCriticalFail("rolled_result==0");
static FTK::Math::Condition requireAny("rolled_result>0");

namespace FTK
{
    Action::Action(ActionType actionType, TargetType targetType, TargetScope targetScope)
        : actionType(actionType), targetType(targetType), targetScope(targetScope)
    {
    }

    Action::Action(const Action &other) : Action(other.actionType, other.targetType, other.targetScope)
    {
    }

    bool Action::shouldHaveEffect(const ActionContext &ctx, bool activeSkill) const
    {
        if (!ctx.condition.eval(ctx.mathContext))
            return false;
        if (activeSkill)
            return requireCriticalSuccess.eval(ctx.mathContext);
        return true;
    }

    void Action::apply(const std::shared_ptr<Entity> &source, const std::shared_ptr<Entity> &target, ActionContext &context, bool activeSkill) const
    {
    }

    Math::Context Action::getContext(const std::string &key, const Math::Context &base) const
    {
        auto res = base;
        res[key] = Math::Context();
        res[key]["action_type"] = (int)actionType;
        res[key]["target_type"] = (int)targetType;
        res[key]["target_scope"] = (int)targetScope;
        return res;
    }

    std::string Action::getSerialType() const
    {
        return "action";
    }

    DamageAction::DamageAction(ActionType actionType, TargetType targetType, TargetScope targetScope, DamageType damageType, Math::Expression damageExpr)
        : Action(actionType, targetType, targetScope), damageType(damageType), damageExpr(damageExpr)
    {
    }

    DamageAction::DamageAction(const DamageAction &other) : DamageAction(other.actionType, other.targetType, other.targetScope, other.damageType, other.damageExpr)
    {
    }

    DamageAction::~DamageAction()
    {
    }

    bool DamageAction::shouldHaveEffect(const ActionContext &ctx, bool activeSkill) const
    {
        if (!ctx.condition.eval(ctx.mathContext))
            return false;
        if (!activeSkill)
            return true;
        return true;
        return requireAny(ctx.mathContext);
    }

    void DamageAction::apply(const std::shared_ptr<Entity> &source, const std::shared_ptr<Entity> &target, ActionContext &context, bool activeSkill) const
    {
        static auto physicalAbsorbtionFormula = Math::Expression("target.p_def/(target.p_def+50)");
        static auto magicalAbsorbtionFormula = Math::Expression("target.m_def/(target.m_def+50)");

        if (damageType != DamageType::Default && damageType != DamageType::Physical && damageType != DamageType::Magical && damageType != DamageType::True)
            return;

        DamageType dmgT = damageType;
        if (damageType == DamageType::Default)
            dmgT = source->getDamageType();
        double dmg = damageExpr.eval(context.mathContext);
        if (dmgT == DamageType::Physical)
            dmg *= 1 - physicalAbsorbtionFormula.eval(context.mathContext);
        else if (dmgT == DamageType::Magical)
            dmg *= 1 - magicalAbsorbtionFormula.eval(context.mathContext);
        if (activeSkill)
        {
            if (cparse::calculator::calculate("skill.allow_partial", context.mathContext).asBool())
                dmg *= diceMult.eval(context.mathContext);
            else
                dmg *= requireCriticalSuccess.eval(context.mathContext);
        }
        dmg *= target->get("dmg_taken");
        if (activeSkill)
        {
            context.mathContext["main_damage"] = (int)dmg;
            context.mathContext["did_damage"] = (int)dmg > 0;
        }
        target->dec("hp", (int)dmg);
    }

    Math::Context DamageAction::getContext(const std::string &key, const Math::Context &base) const
    {
        auto res = Action::getContext(key, base);
        res[key]["damage_type"] = (int)damageType;
        return res;
    }

    std::string DamageAction::getSerialType() const
    {
        return "damage_action";
    }

    HealAction::HealAction(ActionType actionType, TargetType targetType, TargetScope targetScope, Math::Expression healExpr)
        : Action(actionType, targetType, targetScope), healExpr(healExpr)
    {
    }

    HealAction::HealAction(const HealAction &other) : HealAction(other.actionType, other.targetType, other.targetScope, other.healExpr)
    {
    }

    HealAction::~HealAction()
    {
    }

    bool HealAction::shouldHaveEffect(const ActionContext &ctx, bool activeSkill) const
    {
        if (!ctx.condition.eval(ctx.mathContext))
            return false;
        if (!activeSkill)
            return true;
        return true;
        return requireAny(ctx.mathContext);
    }

    void HealAction::apply(const std::shared_ptr<Entity> &source, const std::shared_ptr<Entity> &target, ActionContext &context, bool activeSkill) const
    {
        if (damageType != DamageType::Heal)
            return;
        double healAmt = healExpr.eval(context.mathContext);
        target->inc("hp", (int)healAmt);
    }

    Math::Context HealAction::getContext(const std::string &key, const Math::Context &base) const
    {
        auto res = Action::getContext(key, base);
        res[key]["damage_type"] = (int)damageType;
        return res;
    }

    std::string HealAction::getSerialType() const
    {
        return "heal_action";
    }

    BuffAction::BuffAction(ActionType actionType, TargetType targetType, TargetScope targetScope, const std::vector<BuffBuildData> &buffs)
        : Action(actionType, targetType, targetScope), buffs(buffs)
    {
    }

    BuffAction::BuffAction(const BuffAction &other) : BuffAction(other.actionType, other.targetType, other.targetScope, other.buffs)
    {
    }

    BuffAction::~BuffAction()
    {
    }

    void BuffAction::apply(const std::shared_ptr<Entity> &source, const std::shared_ptr<Entity> &target, ActionContext &context, bool activeSkill) const
    {
        for (auto buff : buffs)
            target->addBuff(MainRegistry::getInstance()->buffTemplates->get(buff.id).build(buff.turns));
    }

    std::string BuffAction::getSerialType() const
    {
        return "buff_action";
    }

    FleeAction::FleeAction(ActionType actionType, TargetType targetType, TargetScope targetScope)
        : Action(actionType, targetType, targetScope)
    {
    }

    FleeAction::FleeAction(const FleeAction &other) : FleeAction(other.actionType, other.targetType, other.targetScope)
    {
    }

    FleeAction::~FleeAction()
    {
    }

    void FleeAction::apply(const std::shared_ptr<Entity> &source, const std::shared_ptr<Entity> &target, ActionContext &context, bool activeSkill) const
    {
    }

    std::string FleeAction::getSerialType() const
    {
        return "flee_action";
    }

    DestroyAction::DestroyAction(ActionType actionType, TargetType targetType, TargetScope targetScope, int destroyAmount)
        : Action(actionType, targetType, targetScope), destroyAmount(destroyAmount)
    {
    }

    DestroyAction::DestroyAction(const DestroyAction &other) : DestroyAction(other.actionType, other.targetType, other.targetScope, other.destroyAmount)
    {
    }

    DestroyAction::~DestroyAction()
    {
    }

    bool DestroyAction::shouldHaveEffect(const ActionContext &ctx, bool activeSkill) const
    {
        if (!ctx.condition.eval(ctx.mathContext))
            return false;
        return requireAny.eval(ctx.mathContext);
    }

    void DestroyAction::apply(const std::shared_ptr<Entity> &source, const std::shared_ptr<Entity> &target, ActionContext &context, bool activeSkill) const
    {
        std::vector<EquipmentType> box;
        for (auto p : target->getEquipments())
            if (p.second)
                box.push_back(p.first);
        if (!box.empty())
        {
            effolkronium::random_static::shuffle(box);
            target->removeEquipment(box.front());
        }
    }

    std::string DestroyAction::getSerialType() const
    {
        return "destroy_action";
    }

    ModifyStatAction::ModifyStatAction(ActionType actionType, TargetType targetType, TargetScope targetScope, const std::string &targetPath, double modifyAmount)
        : Action(actionType, targetType, targetScope), targetPath(targetPath), modifyAmount(modifyAmount)
    {
    }

    ModifyStatAction::ModifyStatAction(const ModifyStatAction &other) : ModifyStatAction(other.actionType, other.targetType, other.targetScope, other.targetPath, other.modifyAmount)
    {
    }

    ModifyStatAction::~ModifyStatAction()
    {
    }

    void ModifyStatAction::apply(const std::shared_ptr<Entity> &source, const std::shared_ptr<Entity> &target, ActionContext &context, bool activeSkill) const
    {
        auto realPath = targetPath;
        if (targetPath._Starts_with("stat."))
            realPath = targetPath.substr(std::string("stat.").size());
        target->set(realPath, target->get(realPath) + modifyAmount);
    }

    std::string ModifyStatAction::getSerialType() const
    {
        return "modify_stat_action";
    }

} // namespace FTK
