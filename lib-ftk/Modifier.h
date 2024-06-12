#ifndef FTK_MODIFIER_H
#define FTK_MODIFIER_H

#include <algorithm>
#include <functional>
#include <map>
#include <set>

#include <uuid.h>
#include <nlohmann/json.hpp>

namespace FTK
{
    enum class ModifierType
    {
        Nop,
        DirectAdd,
        DirectMult,
        FinalAdd,
        FinalMult,
        Override
    };

    NLOHMANN_JSON_SERIALIZE_ENUM(ModifierType,
                                 {{ModifierType::Nop, "nop"},
                                  {ModifierType::DirectAdd, "direct_add"},
                                  {ModifierType::DirectMult, "direct_mult"},
                                  {ModifierType::FinalAdd, "final_add"},
                                  {ModifierType::FinalMult, "final_mult"},
                                  {ModifierType::Override, "override"}})

    class Modifier
    {
    public:
        Modifier(const std::string &name, ModifierType type, double value);
        Modifier(const uuids::uuid uuid, const std::string &name, ModifierType type, double value);
        Modifier(const Modifier &other);
        ~Modifier() = default;

        const uuids::uuid uuid;
        const std::string name;
        const ModifierType type;
        const double value;

        friend bool operator==(const Modifier &a, const Modifier &b);
        friend bool operator<(const Modifier &a, const Modifier &b);

        static double applyModifiers(double target, const std::vector<Modifier> &mods);

    private:
        static std::map<ModifierType, std::function<double(double, const std::vector<Modifier> &)>> funcs;
    };

    class ModifierTemplate
    {
    public:
        ModifierTemplate(const std::string &name, ModifierType type, const std::string &targetPath, double value);
        ModifierTemplate(const ModifierTemplate &other);
        ~ModifierTemplate() = default;

        std::pair<std::string, Modifier> build() const;

        const std::string name;
        const ModifierType type;
        const std::string targetPath;
        const double value;

        static std::pair<std::string, Modifier> createModifierFromTemplate(const ModifierTemplate &base);
    };

    class ModifiableValue
    {
    public:
        ModifiableValue(double base, std::multiset<Modifier> modifiers = {});
        ModifiableValue(const ModifiableValue &other);
        virtual ~ModifiableValue();

        virtual double get() const;
        virtual void set(double value);

        virtual void addModifier(const Modifier &mod);
        virtual void removeModifier(const uuids::uuid &uuid);

    protected:
        virtual void eval();

        double value, base;
        std::multiset<Modifier> modifiers;

        friend nlohmann::adl_serializer<ModifiableValue>;
    };

    class NamedModifiableValue : public ModifiableValue
    {
    public:
        explicit NamedModifiableValue(const std::string &name, const ModifiableValue &value);
        NamedModifiableValue(const std::string &name, double base, std::multiset<Modifier> modifiers = {});
        NamedModifiableValue(const NamedModifiableValue &other);
        virtual ~NamedModifiableValue();

        const std::string name;
    };

} // namespace FTK

#endif // FTK_MODIFIER_H
