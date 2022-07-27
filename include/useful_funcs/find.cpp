//
// Created by chiya on 7/26/22.
//

#include "find.h"


bool find(const std::string &needle, std::vector<std::string> &haystack){
    if(std::find(haystack.begin(), haystack.end(), needle) != haystack.end()) {
        return true;
    }
    return false;
}