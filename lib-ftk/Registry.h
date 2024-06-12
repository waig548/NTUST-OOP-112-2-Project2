#ifndef FTK_REGISTRY_H
#define FTK_REGISTRY_H

#include <fstream>
#include <memory>
#include <map>
#include <string>
#include <vector>
#include <stdexcept>

#include <nlohmann/adl_serializer.hpp>

#include "Entity.h"
#include "Item.h"
#include "Buff.h"
#include "Skill.h"

namespace FTK
{

    template <class T>
    class Registry : private std::vector<T>
    {
    public:
        Registry(const Registry<T> &other) : std::vector<T>(other)
        {
        }

        ~Registry() = default;

        T operator[](const std::string &id) const
        {
            return get(id);
        }

        T get(const std::string &id) const
        {
            if (auto it = std::find_if(begin(), end(), [id](T v)
                                       { return v.id == id; });
                it != end())
                return *it;
            throw std::invalid_argument("Object with id " + id + " not found.");
        }

        using std::vector<T>::begin;
        using std::vector<T>::empty;
        using std::vector<T>::end;
        using std::vector<T>::size;

    private:
        Registry(const std::vector<T> &values) : std::vector<T>(values)
        {
        }

        friend nlohmann::adl_serializer<Registry<T>>;
    };

    class MainRegistry
    {
    public:
        void exportAll(const std::string &targetPath) const;

        const std::shared_ptr<Registry<ActiveSkill>> activeSkills;
        const std::shared_ptr<Registry<PassiveSkill>> passiveSkills;
        const std::shared_ptr<Registry<BuffTemplate>> buffTemplates;
        const std::shared_ptr<Registry<ItemTemplate>> itemTemplates;
        const std::shared_ptr<Registry<EquipmentTemplate>> equipmentTemplates;
        // const std::shared_ptr<Registry<PlayerTemplate>> playerTemplates;
        // const std::shared_ptr<Registry<EnemyTemplate>> enemyTemplates;

        static const std::shared_ptr<MainRegistry> getInstance();

    private:
        MainRegistry();

        template <typename T>
        static std::shared_ptr<Registry<T>> load(const std::string &path)
        {
            static const std::string prefix("assets/gamedata/");
            std::ifstream ifs(prefix + path);
            nlohmann::json j;
            ifs >> j;
            std::shared_ptr<Registry<T>> res = j;
            return res;
        }

        template <typename T>
        static std::shared_ptr<Registry<T>> load(const char *path)
        {
            return load<T>(std::string(path));
        }

        template <typename T>
        static void save(const std::string &path, const std::shared_ptr<Registry<T>> &registry)
        {
            if (!std::filesystem::exists(path))
            {
                std::filesystem::create_directories(std::filesystem::path(path).parent_path());
            }
            std::ofstream ofs(path);
            nlohmann::ordered_json j = registry;
            ofs << std::setw(4) << j;
        }

        template <typename T>
        static void save(const char *path)
        {
            return save<T>(std::string(path));
        }
    };
} // namespace FTK

#endif // FTK_REGISTRY_H
