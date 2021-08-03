
#pragma once

#include <string>
#include <vector>

namespace game
{
    struct ConditionData
    {
        std::string name;
        bool value;
    };
    std::vector<ConditionData> LoadConditionsFromFile(const char* condition_file);
    void WriteConditionsToFile(const char* condition_file, const std::vector<ConditionData>& conditions);

    void LoadConditionsFromFile(const char* condition_file, class ConditionSystem* condition_system);
}
