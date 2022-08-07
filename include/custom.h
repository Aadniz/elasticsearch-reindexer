//
// Created by chiya on 8/6/22.
//

#ifndef ELASTICSEARCH_MANAGER_CUSTOM_H
#define ELASTICSEARCH_MANAGER_CUSTOM_H

#include <vector>
#include "rest.h"
#include <iostream>
#include <string>
#include <fmt/format.h>

class Custom{
public:
    Custom(){
        std::ifstream f("resources/lookup.json");
        lookupTable = json::parse(f);

        std::ifstream f2("resources/categories.json");
        categories = json::parse(f2);
    }

    std::vector<long> generateCats(std::vector<long> cats, std::string indexer){
        std::vector<long> result;
        for (auto &cat : cats){
            result.emplace_back(cat);
            if (cat >= 10000){
                if (!lookupTable["custom"].contains(indexer))
                    continue;
                if (!lookupTable["custom"][indexer].contains(std::to_string(cat)))
                    continue;

                std::string key = lookupTable["custom"][indexer][std::to_string(cat)]["id"];
                long id = std::stol(key);
                result.emplace_back(id);
            }else{
                if (!lookupTable["general"].contains(std::to_string(cat)))
                    continue;
                std::string key = lookupTable["general"][std::to_string(cat)]["id"];
                long id = std::stol(key);
                result.emplace_back(id);
            }
        }
        return result;
    }

    std::string generateName(std::vector<long> cats){
        std::string name;
        int deepest = 0;
        for (auto &cat : cats){
            // if between 10000 and 999999, then skip
            if (cat >= 10000 && 1000000 > cat)
                continue;
            if (10000 > cat) // if below 10000 id, then skip
                continue;

            std::string mask = fmt::format("{:09}", cat);
            int part_size = 3;
            std::vector<std::string> maskArray;
            std::string maskName;
            for (int i = 0; i < 10; i++) {
                if (i % part_size == 0 && i != 0) {
                    maskArray.emplace_back(maskName);
                    maskName = "";
                }
                maskName += mask[i];
            }
            std::string parent = maskArray[0] + "000000";
            std::string child = maskArray[0] + maskArray[1] + "000";
            std::string grandchild = maskArray[0] + maskArray[1] + maskArray[2];
            if (parent == child && parent == grandchild){ // 002000000
                if (categories["000000000"]["children"].contains(parent)) {
                    if (deepest > 0)
                        continue;
                    deepest = 0;
                    if (parent == "010000000" && !name.empty()) // Preserve existing category
                        continue;
                    name = categories["000000000"]["children"][parent]["name"];
                }
            }else if (grandchild == child){ // 002001000
                if (categories["000000000"]["children"].contains(parent) && categories["000000000"]["children"][parent]["children"].contains(child)) {
                    if (deepest > 1)
                        continue;
                    deepest = 1;
                    std::string name1 = categories["000000000"]["children"][parent]["name"];
                    std::string name2 = categories["000000000"]["children"][parent]["children"][child]["name"];
                    name = name1 + "/" + name2;
                }
            }else{ // 002001001
                if (categories["000000000"]["children"].contains(parent) && categories["000000000"]["children"][parent]["children"].contains(child) && categories["000000000"]["children"][parent]["children"][child]["children"].contains(grandchild)) {
                    if (deepest > 2)
                        continue;
                    deepest = 2;
                    std::string name1 = categories["000000000"]["children"][parent]["name"];
                    std::string name2 = categories["000000000"]["children"][parent]["children"][child]["name"];
                    std::string name3 = categories["000000000"]["children"][parent]["children"][child]["children"][grandchild]["name"];
                    name = name1 + "/" + name2 + "/" + name3;
                }
            }
        }
        return name;
    }

private:
    json lookupTable;
    json categories;
    const int length = 9;
};

#endif //ELASTICSEARCH_MANAGER_CUSTOM_H
