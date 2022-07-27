/**
 * This is supposed to be a rest JSON client
 * It will return a json response
 */

#ifndef ELASTICSEARCH_MANAGER_REST_H
#define ELASTICSEARCH_MANAGER_REST_H

#include <fstream>
#include <utility>
#include <vector>
#include <nlohmann/json.hpp>

#include "httplib.h"
#include "url_parser/url_parser.h"
using json = nlohmann::json;


// Include Filesystem for Linux environments
#if defined(__linux__)
namespace fs = std::filesystem;
#endif


// Include filesystem for Windows systems
#if defined(_WIN32) || defined(__CYGWIN__)
#include <experimental/filesystem>
namespace fs = std::experimental::filesystem;
#endif

class Server{
public:
    Server(const std::string& path){
        if(!setURL(path)) {
            std::cout << "Failed to parse URL" << std::endl;
            exit(1);
        }
    }

    /**
     * Change the url while having a instance set
     * @param path1 new FULL path of the url
     * @return returns true if successfully parsed URL
     */
    bool setURL(const std::string &path1){
        std::optional<URL> tempURL = parse_url(path1);
        if (!tempURL)
            return false;

        url = tempURL.value();
        if (!url.query.empty())
            url.path += "?" + url.query;
        return true;
    }

    /**
     * Append a header
     * @param name name of the header
     * @param value value of the header
     */
    void setHeader(std::string name, std::string value){
        headers.insert({
                               std::move(name),
                               std::move(value)
                       });
    }

    /**
     * Append a cookie
     * @param name name of the cookie
     * @param value value of the cookie
     */
    void setCookie(std::string name, std::string value){
        setHeader("Cookie", name + "=" + value);
    }

    /**
     * Set a file to be uploaded with the POSTFILE function
     * @param name Name of the file seen by the server
     * @param filename Path of where the file is
     * @param data The raw data of the file
     */
    void setFile(std::string name, std::string &filename, std::string &data){
        httplib::MultipartFormData multipartFormData = {std::move(name), data, filename,"application/octet-stream"};
        items.push_back(multipartFormData);
    }

    /**
     * @param loc Location to save the file
     * @return status code of request
     */
    int DOWNLOAD(const std::string& loc){
        httplib::Client cli(url.protocol + "://" + url.domain + ":" + url.port);

        auto res = cli.Get(url.path, headers);
        if (error(res.error())) return -1;

        status = res->status;
        body = res->body;
        // Create and open a text file
        std::ofstream MyFile(loc);
        // Write to the file
        MyFile << body;
        // Close the file
        MyFile.close();
        return status;
    }

    /** Send a bulk request to an elasticsearch server
     * @experimental This is an elasticsearch only function
     * @param vector1 vector containing the whole bulk
     * @return int: result of request, json: the body
     */
    std::tuple<int, json> bulk(const std::vector<std::pair<json, json>> &vector1) {
        std::string bulk = "";
        for (auto &doc : vector1){
            bulk += doc.first.dump() + "\n";
            bulk += doc.second.dump() + "\n";
        }
        if (bulk.empty())
            return {-1, ""};
        return POST(bulk);
    }

    /** Sending a get request
     * @return int: result of request, json: the body
     */
    std::tuple<int, json> GET(){
        httplib::Client cli(url.protocol + "://" + url.domain + ":" + url.port);

        auto res = cli.Get(url.path, headers);
        if (error(res.error())) return {-1, errorMSG(res.error())};

        status = res->status;
        body = json::parse(res->body);
        return {status, body};
    }

    /** Head request
     * @return Result of request
     */
    int HEAD(){
        httplib::Client cli(url.protocol + "://" + url.domain + ":" + url.port);
        auto res = cli.Head(url.path, headers);
        if (error(res.error())) return -1;

        status = res->status;
        return status;
    }

    /** Sending a post request
     * @param data JSON data (not json type, use .dump())
     * @return int: result of request, json: the body
     */
    std::tuple<int, json> POST(const std::string& data){
        httplib::Client cli(url.protocol + "://" + url.domain + ":" + url.port);

        auto res = cli.Post(url.path, headers, data, "application/json");
        if (error(res.error())) return {-1, errorMSG(res.error())};

        status = res->status;
        body = json::parse(res->body);
        return {status, body};
    }

    /**
     * Upload a file, requires setFile function to be called first
     * @return int: result of request, json: the body
     */
    std::tuple<int, json> POSTFILE(){
        httplib::Client cli(url.protocol + "://" + url.domain + ":" + url.port);

        auto res = cli.Post(url.path, headers, items);
        if (error(res.error())) return {-1, errorMSG(res.error())};

        status = res->status;
        body = json::parse(res->body);
        return {status, body};
    }

    /** Sending a put request
     * @param data JSON data
     * @return int: result of request, json: the body
     */
    std::tuple<int, json> PUT(const std::string& data){
        httplib::Client cli(url.protocol + "://" + url.domain + ":" + url.port);
        httplib::Headers something;

        auto res = cli.Put(url.path, headers, data, "application/json");
        if (error(res.error())) return {-1, errorMSG(res.error())};

        status = res->status;
        body = json::parse(res->body);
        return {status, body};
    }

    /** Sending a delete request
     * @param data JSON data
     * @return int: result of request, json: the body
     */
    std::tuple<int, json> DEL(){
        httplib::Client cli(url.protocol + "://" + url.domain + ":" + url.port);

        auto res = cli.Delete(url.path, headers);
        if (error(res.error())) return {-1, errorMSG(res.error())};

        status = res->status;
        body = json::parse(res->body);
        return {status, body};
    }
    int status;
    json body;
    URL url;

private:
    static bool error(httplib::Error err){
        if (err != httplib::Error::Success)
            return true;
    }
    json errorMSG(httplib::Error err){
        json j = {
                {"error", to_string(err)}
        };
        return j;
    }

    std::string protocol;
    std::string host;
    int port;
    httplib::Headers headers = {};
    httplib::MultipartFormDataItems items;
};

#endif //ELASTICSEARCH_MANAGER_REST_H
