#include "Registry.h"

#include "Serializer.h"

namespace FTK
{
    void MainRegistry::exportAll(const std::string &targetPath) const
    {
        save<ActiveSkill>(targetPath + "/active_skills.json", activeSkills);
        save<PassiveSkill>(targetPath + "/passive_skills.json", passiveSkills);
        save<BuffTemplate>(targetPath + "/buff_templates.json", buffTemplates);
        save<ItemTemplate>(targetPath + "/item_templates.json", itemTemplates);
        save<EquipmentTemplate>(targetPath + "/equipment_templates.json", equipmentTemplates);
    }

    const std::shared_ptr<MainRegistry> MainRegistry::getInstance()
    {
        static const auto instance = std::shared_ptr<MainRegistry>(new MainRegistry());

        return instance;
    }

    MainRegistry::MainRegistry() : activeSkills(load<ActiveSkill>("active_skills.json")),
                                   passiveSkills(load<PassiveSkill>("passive_skills.json")),
                                   buffTemplates(load<BuffTemplate>("buff_templates.json")),
                                   itemTemplates(load<ItemTemplate>("item_templates.json")),
                                   equipmentTemplates(load<EquipmentTemplate>("equipment_templates.json"))
    // playerTemplates(load<PlayerTemplate>("player_template.json")),
    // enemyTemplates(load<EnemyTemplate>("enemy_templates.json"))
    {
    }

} // namespace FTK
