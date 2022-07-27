//
// Created by chiya on 7/26/22.
//

#ifndef ELASTICSEARCH_MANAGER_HASHING_H
#define ELASTICSEARCH_MANAGER_HASHING_H

// Libraries
#include <iostream>
#include <openssl/sha.h>
#include <string>
#include <iomanip>

std::string sha1(const std::string &str);

#endif //ELASTICSEARCH_MANAGER_HASHING_H
