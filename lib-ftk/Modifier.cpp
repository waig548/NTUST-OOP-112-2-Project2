#include "Modifier.h"

namespace FTK
{
    Modifier::Modifier(const std::string &name, ModifierType type, double value) : Modifier(uuids::uuid_system_generator{}(), name, type, value)
    {
    }

    Modifier::Modifier(const uuids::uuid uuid, const std::string &name, ModifierType type, double value) : uuid(uuid), name(name), type(type), value(value)
    {
    }

    Modifier::Modifier(const Modifier &other) : uuid(other.uuid), name(other.name), type(other.type), value(other.value)
    {
    }

    double Modifier::applyModifiers(double target, const std::vector<Modifier> &mods)
    {
        if (mods.empty())
            return target;
        return funcs[mods[0].type](target, mods);
    }

    ModifierTemplate::ModifierTemplate(const std::string &name, ModifierType type, const std::string &targetPath, double value) : name(name), type(type), targetPath(targetPath), value(value)
    {
    }

    ModifierTemplate::ModifierTemplate(const ModifierTemplate &other) : ModifierTemplate(other.name, other.type, other.targetPath, other.value)
    {
    }

    std::pair<std::string, Modifier> ModifierTemplate::build() const
    {
        return createModifierFromTemplate(*this);
    }

    std::pair<std::string, Modifier> ModifierTemplate::createModifierFromTemplate(const ModifierTemplate &base)
    {
        return std::make_pair(base.targetPath, Modifier(base.name, base.type, base.value));
    }

    std::map<ModifierType, std::function<double(double, const std::vector<Modifier> &)>> Modifier::funcs = {
        std::make_pair(ModifierType::Nop, [](double target, const std::vector<Modifier> &modifiers)
                       { return target; }),
        std::make_pair(ModifierType::DirectAdd, [](double target, const std::vector<Modifier> &modifiers)
                       {
            double result = target;
            std::for_each(modifiers.begin(), modifiers.end(), [&result](Modifier mod){
                result += mod.value;
            });
            return result; }),
        std::make_pair(ModifierType::DirectMult, [](double target, const std::vector<Modifier> &modifiers)
                       {
            double mults = 0;
            std::for_each(modifiers.begin(), modifiers.end(), [&mults](Modifier mod){
                mults += mod.value;
            });
            return target*(1+mults); }),
        std::make_pair(ModifierType::FinalAdd, [](double target, const std::vector<Modifier> &modifiers)
                       {
            double result = target;
            std::for_each(modifiers.begin(), modifiers.end(), [&result](Modifier mod){
                result += mod.value;
            });
            return result; }),
        std::make_pair(ModifierType::FinalMult, [](double target, const std::vector<Modifier> &modifiers)
                       {
            double result = target;
            std::for_each(modifiers.begin(), modifiers.end(), [&result](Modifier mod){
                result *= mod.value;
            });
            return result; }),
        std::make_pair(ModifierType::Override, [](double target, const std::vector<Modifier> &modifiers)
                       { return (std::min_element(modifiers.begin(), modifiers.end(), [](auto a, auto b)
                                                  { return a.value < b.value; }))
                             ->value; }),
    };

    ModifiableValue::ModifiableValue(double value, std::multiset<Modifier> modifiers) : base(value), modifiers(modifiers)
    {
        eval();
    }

    ModifiableValue::ModifiableValue(const ModifiableValue &other) : ModifiableValue(other.base, other.modifiers)
    {
    }

    ModifiableValue::~ModifiableValue()
    {
    }

    double ModifiableValue::get() const
    {
        return value;
    }

    void ModifiableValue::set(double value)
    {
        base = value;
        eval();
    }

    void ModifiableValue::addModifier(const Modifier &mod)
    {
        modifiers.insert(mod);
        eval();
    }

    void ModifiableValue::removeModifier(const uuids::uuid &uuid)
    {
        if (auto it = std::find_if(modifiers.begin(), modifiers.end(), [&uuid](const Modifier &mod)
                                   { return mod.uuid == uuid; });
            it != modifiers.end())
        {
            modifiers.erase(it);
            eval();
        }
    }

    void ModifiableValue::eval()
    {
        double res = base;
        auto it = modifiers.begin();
        while (it != modifiers.end())
        {
            std::vector<Modifier> tmp;
            do
            {
                tmp.push_back(*it);
                it++;
            } while (it != modifiers.end() && tmp[0].type == it->type);
            res = Modifier::applyModifiers(res, tmp);
        }
        value = res;
    }

    NamedModifiableValue::NamedModifiableValue(const std::string &name, const ModifiableValue &value) : name(name), ModifiableValue(value)
    {
    }

    NamedModifiableValue::NamedModifiableValue(const std::string &name, double base, std::multiset<Modifier> modifiers) : NamedModifiableValue(name, ModifiableValue(base, modifiers))
    {
    }

    NamedModifiableValue::NamedModifiableValue(const NamedModifiableValue &other) : NamedModifiableValue(other.name, other.base, other.modifiers)
    {
    }

    NamedModifiableValue::~NamedModifiableValue()
    {
    }

    bool operator==(const Modifier &a, const Modifier &b)
    {
        return a.uuid == b.uuid;
    }

    bool operator<(const Modifier &a, const Modifier &b)
    {
        return a.type < b.type && a.name < b.name && a.value < b.value && a.uuid < b.uuid;
    }

} // namespace FTK
