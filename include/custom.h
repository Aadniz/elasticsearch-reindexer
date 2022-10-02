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
#include "useful_funcs/find.h"

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
            if (!longFind(cat, result))
                result.emplace_back(cat);
            long id = 0;
            if (cat >= 10000){
                if (!lookupTable["custom"].contains(indexer))
                    continue;
                if (!lookupTable["custom"][indexer].contains(std::to_string(cat)))
                    continue;

                std::string key = lookupTable["custom"][indexer][std::to_string(cat)]["id"];
                id = std::stol(key);
            }else{
                if (!lookupTable["general"].contains(std::to_string(cat)))
                    continue;
                std::string key = lookupTable["general"][std::to_string(cat)]["id"];
                id = std::stol(key);
            }
            int num = 1000;
            while (id % 1000000000 != 0){
                if (!longFind(id, result))
                    result.emplace_back(id);
                id = (int)(id / num) * num;
                num *= num;
            }
        }
        return result;
    }

    float virusDetection(json &j){
        std::vector<long> pccats = {4000000, 4001000, 4002000, 4003000, 4004000, 5003000}; // TODO: remove this
        std::vector<float> score; // Base value
        //$score[] = (in_array(4000000, $categories)) ? 1.0 : 0.0; // If category is PC
        bool found = false;
        for (auto &cat : j["Category"])
            if (longFind(cat, pccats))
                found = true;
        if (found)
            score.emplace_back(1.0); // If category is PC, TODO: remove this
        else
            score.emplace_back(0.0);
        score.emplace_back(fmin(1.0, (float)j["Seeders"] / 50000)); // if seeders is gigantic
        score.emplace_back((float)j["Seeders"] / 10 > j["Peers"] ? 1.0 : 0.0); // seed to peer ratio
        score.emplace_back(fmin(1.0, 52428800.f / ((float)j["Size"] + 1.f))); // if size is very small
        //$score[] = (in_array($tracker, ["thepiratebay", "torlock"]) && in_array(4000000, $categories)) ? 1.0 : 0.0; // If thepiratebay or torlock
        if (found && (j["TrackerId"] == "thepiratebay" || j["TrackerId"] == "torlock"))
            score.emplace_back(1.0);
        else
            score.emplace_back(0.0);

        float sum = 0.0;
        for (auto &elm : score){
            sum += elm;
        }
        sum = sum / (float)score.size();
        sum = fmin(sum, 1.0);
        sum = fmax(sum, 0.0);

        return sum;
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
