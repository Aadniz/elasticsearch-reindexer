#include <iostream>
#include <string>

// Libs
#include "include/rest.h"
#include "include/url_parser/url_parser.h"
#include <nlohmann/json.hpp>
using json = nlohmann::json;

// Other stuff
#include "include/useful_funcs/hashing.h"
#include "include/useful_funcs/find.h"
#include "include/custom.h"


int main(int argc, char **argv) {
    /**
     * Handling the arguments
     */
    std::vector<std::string> args;
    for (int i = 1; i < argc; ++i)
        args.emplace_back(argv[i]);

    if (args.size() > 2){
        std::cout << "Too many arguments, maximum 2" << std::endl;
        return 1;
    }

    /**
     * Handling the source server and the destination server
     */

    /**
     * SOURCE
     */
    URL source;
    if (args.empty()){
        std::cout << "\n<<-->> Enter the source server <<-->>" << std::endl;
        std::cout << "Valid options are something like:" << std::endl;
        std::cout << "  - https://192.168.1.190:9200/mydb" << std::endl;
        std::cout << "  - http://192.168.1.190:9201/" << std::endl;
        std::cout << "  - 192.168.1.190:9201/" << std::endl;
        std::cout << "  - /mydb (assuming localhost at port 9200)" << std::endl;
        std::cout << "> ";
        std::string input;
        std::getline(std::cin, input);
        args.push_back(input);
    }

    // testing to see if it's a valid url
    std::string arg1 = args.front();
    std::optional<URL> url;
    url = parse_url(arg1);
    if (!url){
        url = parse_url("http://" + arg1);
        if (!url) {
            url = parse_url("http://127.0.0.1:9200" + arg1);
            if (!url) {
                url = parse_url("http://127.0.0.1:9200/" + arg1);
                if (!url) {
                    std::cout << "Failed to parse: " << arg1 << std::endl;
                    return 1;
                }
            }
        }
    }
    source = url.value();
    Server src(source.to_string() + "/_aliases");
    if (source.path.empty()){
        std::cout << "<<-->> Enter the source index <<-->>" << std::endl;
        auto [status, j] = src.GET();
        for (auto &item : j.items())
            std::cout << "- " << item.key() << std::endl;
        std::cout << source.to_string() << "/";
        std::string input;
        std::getline(std::cin, input);
        source.path = "/" + input;
    }
    src.setURL(source.to_string());
    src.GET();
    if (src.status != 200){
        std::cout << "Encountered non 200 status: " << src.status << std::endl;
        return 1;
    }

    /**
     * DESTINATION
     */
    URL destination;
    if (args.size() == 1){
        std::cout << "\n<<-->> Enter the destination server <<-->>" << std::endl;
        std::cout << "Valid options are something like:" << std::endl;
        std::cout << "  - https://"<<source.domain<<":"<<source.port<<"/mydb" << std::endl;
        std::cout << "  - http://192.168.1.190:9201/" << std::endl;
        std::cout << "  - 192.168.1.190:9201/" << std::endl;
        std::cout << "  - /mydb (assuming "<< source.domain <<" at port "<< source.port <<")" << std::endl;
        std::cout << "> ";
        std::string input;
        std::getline(std::cin, input);
        args.push_back(input);
    }

    // testing to see if it's a valid url
    std::string arg2 = args.back();
    url = parse_url(arg2);
    if (!url){
        url = parse_url("http://" + arg2);
        if (!url) {
            url = parse_url(source.protocol + "://" +source.domain+ ":" + source.port + arg2);
            if (!url) {
                url = parse_url(source.protocol + "://" +source.domain+ ":" + source.port + "/" + arg2);
                if (!url) {
                    std::cout << "Failed to parse: " << arg2 << std::endl;
                    return 1;
                }
            }
        }
    }
    destination = url.value();
    Server dst(destination.to_string() + "/_aliases");
    if (destination.path.empty()){
        std::cout << "<<-->> Enter the destination index <<-->>" << std::endl;
        auto [status, j] = dst.GET();
        for (auto &item : j.items())
            std::cout << "- " << item.key() << std::endl;
        std::cout << destination.to_string() << "/";
        std::string input;
        std::getline(std::cin, input);
        destination.path = "/" + input;
    }

    // See if destination index exists
    dst.setURL(destination.to_string());
    dst.GET();
    if (dst.status != 200 && dst.status != 404){
        std::cout << "Destination server encountered a unexpected status code: " << dst.status << std::endl;
        return 1;
    }
    bool deleted = false;
    if (dst.status == 200){
        Server count(destination.to_string() + "/_count");
        count.GET();
        if (count.status != 200){
            std::cout << "Destination server encountered a unexpected status code: " << count.status << std::endl;
            return 1;
        }
        long num = count.body["count"];
        std::cout << destination.path.substr(1, destination.path.size() - 1) << " already exists in " << destination.domain << " with " << num << " documents" << std::endl;
        std::cout << "Do you want to delete it, and reindex it?" << std::endl;
        std::cout << "  (Type \"" << destination.path.substr(1, destination.path.size() - 1) << "\" to delete it)\n> ";
        std::string input;
        std::getline(std::cin, input);
        if (input == destination.path.substr(1, destination.path.size() - 1)){
            std::cout << "Deleting " << input << " from " << destination.domain << " in:" << std::endl;
            for (int i = 10; i > 0; --i) {
                std::cout << " " << i << std::endl;
                sleep(1);
            }
            // TODO: delete
            deleted = true;
        }
    }
    if (dst.status == 404 || deleted){
        std::cout << "Destination index \""<< destination.path << "\" Not found." << std::endl;
        std::cout << "Do you want to copy the mapping?\n> " << std::endl;
        std::cout << "Not supported yet" << std::endl;
        return 1;
    }

    /**
     * Settings before starting
     */
    std::vector<std::string> anchors;
    src.setURL(source.to_string() + "/_mapping");
    src.GET();
    std::string db = source.path.substr(1, source.path.size() - 1);
    std::cout << std::endl;
    for (auto &item : src.body[db]["mappings"]["properties"].items()){
        json value = src.body[db]["mappings"]["properties"][item.key()];
        std::string type = value["type"];
        std::vector<std::string> validAchors = {"boolean","long","float","date","integer","short","byte","double","half_float","scaled_float","date_nanos","binary","ip"};
        if (find(type, validAchors)) {
            std::cout << "- " << item.key() << " (" << type << ")" << std::endl;
            anchors.push_back(item.key());
        }
    }

    std::cout << "\nElasticsearch needs an anchor point." << std::endl;
    std::cout << "What field from above should the reindexing order by? (recommend timestamps)\n> ";
    std::string input;
    std::getline(std::cin, input);
    if (!find(input, anchors)){
        std::cout << "No such field: " << input << std::endl;
        return 0;
    }
    std::string anchor = input;


    /**
     * Summary
     */
    std::cout << std::endl;
    std::cout << "<<-->> Summary of what is going to happen <<-->>" << std::endl;
    std::cout << "Protocol: " << source.protocol << " (" << (source.protocol == "http" ? "Insecure" : "Secure") << ")" << std::endl;
    std::cout << "Host:     " << source.domain << std::endl;
    std::cout << "Port:     " << source.port << std::endl;
    std::cout << "db:       " << source.path << std::endl;
    std::cout << "full url: " << source.to_string() << std::endl;
    std::cout << "          " << std::endl;
    std::cout << "   ↓ ↓ ↓  " << std::endl;
    std::cout << "          " << std::endl;
    std::cout << "Protocol: " << destination.protocol << " (" << (destination.protocol == "http" ? "Insecure" : "Secure") << ")" << std::endl;
    std::cout << "Host:     " << destination.domain << std::endl;
    std::cout << "Port:     " << destination.port << std::endl;
    std::cout << "db:       " << destination.path << std::endl;
    std::cout << "full url: " << destination.to_string() << std::endl;
    std::cout << "          " << std::endl;
    std::cout << "          " << std::endl;
    std::cout << "Anchor:   " << anchor << std::endl;
    std::cout << "          " << std::endl;

    std::cout << "Looks ok? (y/ok/q)\n> ";


    std::getline(std::cin, input);
    if (input != "y" && input != "ok"){
        std::cout << "aborted" << std::endl;
        return 0;
    }

    /**
     * GOOOOO
     */

    /**
     * Amount of documents to retrieve per request
     */
     const int REQUEST_SIZE = 500;

     /**
      * Maximum number before changing the achorpoint
      */
     const int MAX_CYCLE = 4000;

     /**
      * From value increases with REQUEST_SIZE for each loop
      */
     int FROM = 0;

    /**
    * Destination count httplib client connection. Use count.GET(), then count.body["count"] to get the numberz
    */
    Server count(destination.to_string() + "/_count");
    count.GET();
    if (count.status != 200){
        std::cout << "Destination server encountered a unexpected status code: " << count.status << std::endl;
        return 1;
    }
    /**
     * This value is to inform about the progress
     */
    long AMOUNT_INDEXED = count.body["count"];

    /**
     * Source count httplib client connection. Use count.GET(), then count.body["count"] to get the numberz
     */
    count.setURL(source.to_string() + "/_count");
    count.GET();
    if (count.status != 200){
        std::cout << "Source server encountered a unexpected status code: " << count.status << std::endl;
        return 1;
    }
    /**
     * This value is used to compare with AMOUNT_INDEXED
     */
    long TOTAL_DOCUMENTS = count.body["count"];

    /**
     * Achorpoint needs to be a json because it dynamically changes based on what anchor type has been chosen
     */
     json anchorPoint = {
        {"anchor", 0}
     };

     Custom custom;
     while (true){
         // Measure the time
         auto start = std::chrono::high_resolution_clock::now();

         // Creating the search query
         src.setURL(source.to_string() + "/_doc/_search");
         json j;
         j["from"] = FROM;
         j["size"] = REQUEST_SIZE;
         j["query"]["bool"]["must"]["range"][anchor]["gte"] = anchorPoint["anchor"];
         j["sort"][anchor]["order"] = "ASC";
         auto [status, body] = src.POST(j.dump());
         if (status != 200){
             std::cout << "Failed. Trying again in 10 sec" << std::endl;
             sleep(10);
             continue;
         }

         json lastAnchorPoint = {
                 {"anchor", 0}
         };

         /**
          * First tuple element is the index, type and so on.
          * The other tuple element is the info
          */
         std::vector<std::pair<json, json>> bulk;
         dst.setURL(destination.to_string("url") + "/_bulk");

         if (10 > body["hits"]["total"]["value"]){
             std::cout << "DONE!" << std::endl;
             return 0;
         }

         // Adding results to the bulk
         for (auto &doc : body["hits"]["hits"]){
            /**
             * Specific for this script
             */

             doc["_source"]["Category"] = custom.generateCats(doc["_source"]["Category"], doc["_source"]["TrackerId"]);
             std::string newName = custom.generateName(doc["_source"]["Category"]);
             if (!newName.empty())
                doc["_source"]["CategoryDesc"] = newName;

             std::string docID = doc["_id"];

             std::pair<json, json> pair;
             json bulkHeader = {
                     {
                         "index", {
                             {"_index", destination.path.substr(1, destination.path.size() - 1)},
                             {"_type", "_doc"},
                             {"_id", docID}
                         }
                     }
             };
             pair = {bulkHeader, doc["_source"]};
             bulk.push_back(pair);
             lastAnchorPoint["anchor"] = doc["_source"][anchor];
         }

         // Now we do the bulk
         int successes = 0;
         auto [status2, body2] = dst.bulk(bulk);
         if (body2["errors"] == true) {
             std::cout << "Bulk encountered errors. Please read through my hero" << std::endl;
             std::cout << body2.dump() << std::endl;
             exit(1);
         }
         for (auto &item : body2["items"]){
             if (item["index"]["result"] == "created") {
                 AMOUNT_INDEXED += 1;
                 successes += 1;
             }
         }

         // Capture the time
         auto elapsed = std::chrono::high_resolution_clock::now() - start;
         long long time_taken = std::chrono::duration_cast<std::chrono::milliseconds>(elapsed).count();

         std::cout << (AMOUNT_INDEXED*100)/TOTAL_DOCUMENTS << "% " << AMOUNT_INDEXED << " / " << TOTAL_DOCUMENTS << ", took: " << std::setprecision(2) << time_taken << "ms (" << successes << " documents created)" << std::endl;

         FROM += REQUEST_SIZE;

         if (FROM + REQUEST_SIZE > MAX_CYCLE){
             anchorPoint = lastAnchorPoint;
             FROM = 0;
         }
     }

    return 0;
}

//for (auto &doc : body["hits"]["hits"]){
    //
    //  Specific for this script
    //
    //std::string docID = sha1(doc["_source"]["Title"]);
    //
    //dst.setURL(destination.to_string() + "/_doc/" + docID);
    //auto [status2, body2] = dst.PUT(doc["_source"].dump());
    //std::cout << body2["result"] << ": " << doc["_source"]["Title"] << std::endl;
    //lastAnchorPoint["anchor"] = doc["_source"][anchor];
//}