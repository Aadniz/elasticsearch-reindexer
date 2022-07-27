//
// Created by chiya on 7/26/22.
//

#ifndef ELASTICSEARCH_MANAGER_URL_PARSER_H
#define ELASTICSEARCH_MANAGER_URL_PARSER_H

#include <string>
#include <optional>

struct URL{
    std::string protocol;
    std::string domain;
    std::string port;
    std::string path;
    std::string query;
    /**
     * @param option optional can either be all or url
     */
    inline std::string to_string(const std::string& option = "all") const{
        if (option == "all")
            return protocol + "://" + domain + ":" + port + path + (query.empty() ? "" : "?") + query;
        else if(option == "url")
            return protocol + "://" + domain + ":" + port;
    }
};

std::optional<URL> parse_url(const std::string& url); //with boost


#endif //ELASTICSEARCH_MANAGER_URL_PARSER_H
