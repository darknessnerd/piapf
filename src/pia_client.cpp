#include <exception>
#include <iostream>
#include <cpprest/filestream.h>
#include <cpprest/uri.h>
#include <cpprest/json.h>
#include <string>

#include <chrono>
#include <iomanip> // put time

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

/**
 * Generate a UTC ISO8601-formatted timestamp
 * and return as std::string
 */
string PiaClient::currentISO8601TimeUTC() {
  auto now = std::chrono::system_clock::now();
  auto itt = std::chrono::system_clock::to_time_t(now);
  std::ostringstream ss;
  ss << std::put_time(gmtime(&itt), "%FT%TZ");
  return ss.str();
}

/**
 * from  ISO8601-formatted std::string
 * return a linux timestamp
 */
long PiaClient::stringToTimestamp(const std::string &time) {
  std::tm tm = {};
  std::stringstream ss(time);
  ss >> std::get_time(&tm, "%Y-%m-%dT%H:%M:%S");
  std::chrono::system_clock::time_point tp =  std::chrono::system_clock::from_time_t(std::mktime(&tm));
  return std::chrono::duration_cast<std::chrono::seconds>( tp.time_since_epoch() ).count();
}
int PiaClient::get_port() const
{
    json::value payload = json::value::parse(base64::base64_decode(this->payload));
    return payload[U("port")].as_integer();
}
bool PiaClient::isExpired()
{
    if(this->token.empty() || this->payload.empty() || this->signature.empty())
    {
        return true;
    }
    json::value payload = json::value::parse(base64::base64_decode(this->payload));

    std::string expire_at = payload[U("expires_at")].as_string();
    std::cout << "expire at: " << expire_at << std::endl;
    int port = payload[U("port")].as_integer();
    std::cout << "binding port: " << port << std::endl;

    long now = std::chrono::duration_cast<std::chrono::seconds>( std::chrono::system_clock::now().time_since_epoch()).count();
    long expire_at_timestamp = this->stringToTimestamp(expire_at);
    std::cout << "now: " << now << "expire at: " << expire_at_timestamp << std::endl;
    std::cout << "delta time  = " << expire_at_timestamp - now << std::endl;
    // 7 days before the expire update the token
    if(expire_at_timestamp - now < 604800 )
    {
        return true;
    }
    return false;
}

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

bool PiaClient::auth(const string &username, const string &password)
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
