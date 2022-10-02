//
// Created by chiya on 7/26/22.
//

#ifndef ELASTICSEARCH_MANAGER_FIND_H
#define ELASTICSEARCH_MANAGER_FIND_H

#include <string>
#include <algorithm>
#include <vector>


bool find(const std::string &needle, std::vector<std::string> &haystack);
bool longFind(const long &needle, std::vector<long> &haystack);

#endif //ELASTICSEARCH_MANAGER_FIND_H
