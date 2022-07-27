//
// Created by chiya on 7/26/22.
//

#include "hashing.h"


std::string sha1(const std::string &str){
    unsigned char hash[20];
    SHA_CTX sha1;
    SHA1_Init(&sha1);
    SHA1_Update(&sha1, str.c_str(), str.size());
    SHA1_Final(hash, &sha1);
    std::stringstream ss;
    for(unsigned char i : hash)
    {
        ss << std::hex << std::setw(2) << std::setfill('0') << (int)i;
    }
    return ss.str();
}