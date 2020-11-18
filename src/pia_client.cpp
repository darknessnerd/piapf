#include <exception>
#include <iostream>
#include <cpprest/filestream.h>
#include <cpprest/uri.h>
#include <cpprest/json.h>

#include <string>

#include "base64.h"
#include "pia_client.h"
#include "shell.h"
using namespace utility;
using namespace web;
using namespace web::http;
using namespace web::http::client;
using namespace concurrency::streams;
using std::string;
using std::ostringstream;
using shell::cmd;
#include <regex>


PiaClient::PiaClient():pia_host("https://www.privateinternetaccess.com")
{
    //ctor
    config.set_validate_certificates(false);
    ostringstream oss;
    // the ip it taken from traceroute -m 1 privateinternetaccess.com | tail -n 1 | awk '{print $2}'
    std::string ip = cmd{"traceroute -m 1 privateinternetaccess.com | tail -n 1 | awk '{print $2}'"}.str();
    ip = std::regex_replace(ip, std::regex("^\\s+"), std::string(""));
    ip = std::regex_replace(ip, std::regex("\\s+$"), std::string(""));
    std::cout << "privateinternetaccess.com: " << ip;
    oss << "https://" << ip << ":19999";
    this->pia_request_uri = oss.str();
}

void PiaClient::process_request(const pplx::task<web::http::http_response> &task,std::function<void(const json::value &jsonObject)> handler_func)
{
    // Get the response.
    auto request = task.then([](http_response response)
    {
        // Check the status code.
        if (response.status_code() != 200)
        {
            throw std::runtime_error("Returned " + std::to_string(response.status_code()));
        }
        return response.extract_json(true);
    })
    // Parse the user details.
    .then(handler_func);
    // Wait for the concurrent tasks to finish.
    try
    {
        request.wait();
    }
    catch (const std::exception &e)
    {
        ostringstream oss;
        oss << "Error pia_client::process_request: " << e.what() ;
        throw std::runtime_error(oss.str());
    }
}

bool PiaClient::bind_port()
{
    bool result = false;
    auto request = http_client(U(this->pia_request_uri), this->config)
                   .request(methods::GET,uri_builder()
                            .append_path(U("bindPort"))
                            .append_query(U("payload"), this->payload)
                            .append_query(U("signature"), this->signature).to_string(), U("application/json"));
    auto handle_responce  = [&](json::value jsonObject)
    {
        std::cout << "bind port : "<< jsonObject.serialize() << std::endl;
        if(jsonObject[U("status")].as_string().compare("OK") == 0)
        {
            result = true;
        }
    };
    try
    {
        this->process_request(request,handle_responce);
    }
    catch (const std::exception &e)
    {
        return false;
    }
    return result;
}
bool PiaClient::generate_signature()
{
    bool result = false;
    string signature;
    string payload;
    try
    {
        this->process_request(http_client(U(this->pia_request_uri), this->config)
                              .request(methods::GET,uri_builder()
                                       .append_path(U("getSignature"))
                                       .append_query(U("token"), token).to_string(), U("application/json")),
                              [&](json::value jsonObject)
        {
            std::cout << "sig: "<< jsonObject.serialize() << std::endl;
            std::cout <<  base64::base64_decode(jsonObject[U("payload")].as_string()) << std::endl;
            payload = jsonObject[U("payload")].as_string();
            signature = jsonObject[U("signature")].as_string();
        }

                             );
    }
    catch (const std::exception &e)
    {
        return false;
    }
    this->signature = signature;
    this->payload = payload;
    return true;

}

bool PiaClient::auth(string username, string password)
{
    std::string token;
    // Make a POST request.
    json::value jsonObject;
    jsonObject[U("username")] = json::value::string(U(username));
    jsonObject[U("password")] = json::value::string(U(password));

    bool result = false;
    try
    {
        this->process_request(http_client(U(this->pia_host), config)
                              .request(methods::POST,
                                       uri_builder(U("api"))
                                       .append_path(U("client"))
                                       .append_path(U("v2"))
                                       .append_path(U("token"))
                                       .to_string(),
                                       jsonObject.serialize(), U("application/json")),
                              [&](json::value jsonObject)
        {
            token = jsonObject[U("token")].as_string();
        }

                             );
    }
    catch (const std::exception &e)
    {
        return false;
    }
    this->token = token;
    return true;
}

string PiaClient::get_public_ip()
{
    std::string public_ip;
    auto request = http_client(U("https://api.ipify.org"), this->config)
                   .request(methods::GET,uri_builder().append_query(U("format"), "json").to_string());
    auto handle_response = [&](json::value jsonObject)
    {
        public_ip = jsonObject[U("ip")].as_string();
    };

    try
    {
        this->process_request(request, handle_response);
    }
    catch (const std::exception &e)
    {
        return string();
    }
    return public_ip;
}

PiaClient::~PiaClient()
{
    //dtor
}
