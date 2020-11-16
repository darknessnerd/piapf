#include <exception>
#include <sstream>

#include <cpprest/http_client.h>
#include <cpprest/filestream.h>
#include <cpprest/uri.h>
#include <cpprest/json.h>



#include <vector>

#include "uuid.h"
#include "net_tools.h"
#include "configuration.h"
#include "base64.h"

using namespace utility;
using namespace web;
using namespace web::http;
using namespace web::http::client;
using namespace concurrency::streams;


int main(int argc, char *argv[])
{

    Configuration conf;

    // Parse args and overrid the config directory if -c option exist
    int parse_result = conf.parse(argc, argv);
    if(parse_result!=1)
        return parse_result;


    std::cout << "PIA Service" << std::endl;
    // Get the ip from the vpn interface
    std::string vpn_local_ip = net_tools::get_ip_address(conf.get_vpn_iface());
    std::string private_internet_access_ip = net_tools::dns_lookup("privateinternetaccess.com")[0]; // TODO CHEK if empty
    std::string client_id = UUID::generate_uuid();
    std::cout << vpn_local_ip << std::endl;
    std::cout << private_internet_access_ip << std::endl;
    std::cout << client_id << std::endl;

    http_client_config config;
    config.set_validate_certificates(false);
    // Make a GET request.
    auto requestJson = http_client(U("https://api.ipify.org"), config)
                       .request(methods::GET,uri_builder().append_query(U("format"), "json").to_string())
                       // Get the response.
                       .then([](http_response response)
    {
        // Check the status code.
        if (response.status_code() != 200)
        {
            throw std::runtime_error("Returned " + std::to_string(response.status_code()));
        }

        // Convert the response body to JSON object.
        return response.extract_json();
    })
    // Parse the user details.
    .then([](json::value jsonObject)
    {
        std::cout << "vpn external ip: "<< jsonObject[U("ip")].as_string() << std::endl;
    });
    // Wait for the concurrent tasks to finish.
    try
    {
        requestJson.wait();
    }
    catch (const std::exception &e)
    {
        printf("Error exception:%s\n", e.what());
    }
    std::string token;

    // Make a POST request.
    json::value jsonObject;
	jsonObject[U("username")] = json::value::string(U(conf.get_pia_username()));
    jsonObject[U("password")] = json::value::string(U(conf.get_pia_password()));
    requestJson = http_client(U("https://www.privateinternetaccess.com"), config)
                       .request(methods::POST,
                                uri_builder(U("api"))
                                .append_path(U("client"))
                                .append_path(U("v2"))
                                .append_path(U("token"))
                                .to_string(),
                                jsonObject.serialize(), U("application/json")
                                )
                       // Get the response.
                       .then([](http_response response)
    {
        // Check the status code.
        if (response.status_code() != 200)
        {
            throw std::runtime_error("Returned " + std::to_string(response.status_code()));
        }

        // Convert the response body to JSON object.
        return response.extract_json();
    })
    // Parse the user details.
    .then([&](json::value jsonObject)
    {
        std::cout << "get token result: "<< jsonObject.serialize() << std::endl;
        token = jsonObject[U("token")].as_string();
    });
    // Wait for the concurrent tasks to finish.
    try
    {
        requestJson.wait();
    }
    catch (const std::exception &e)
    {
        printf("Error exception:%s\n", e.what());
    }

    std::ostringstream oss;
    // the ip it taken from traceroute -m 1 privateinternetaccess.com | tail -n 1 | awk '{print $2}'
    oss << "https://" << "10.43.18.1" << ":19999";
    std::string requestUrl = oss.str();
    std::string signature;
    std::string payload;
    requestJson = http_client(U(requestUrl), config)
                       .request(methods::GET,uri_builder()
                                .append_path(U("getSignature"))
                                .append_query(U("token"), token).to_string(), U("application/json"))
                       // Get the response.
                       .then([](http_response response)
    {
        // Check the status code.
        if (response.status_code() != 200)
        {
            throw std::runtime_error("Returned " + std::to_string(response.status_code()));
        }

        return response.extract_json(true);
    })
    // Parse the user details.
    .then([&](json::value jsonObject)
    {
        std::cout << "sig: "<< jsonObject.serialize() << std::endl;
        std::cout <<  base64::base64_decode(jsonObject[U("payload")].as_string()) << std::endl;
        payload = jsonObject[U("payload")].as_string();
        signature = jsonObject[U("signature")].as_string();
    });
    // Wait for the concurrent tasks to finish.
    try
    {
        requestJson.wait();
    }
    catch (const std::exception &e)
    {
        printf("Error exception:%s\n", e.what());
    }


    // BIND THE PORT
    requestJson = http_client(U(requestUrl), config)
                       .request(methods::GET,uri_builder()
                                .append_path(U("bindPort"))
                                .append_query(U("payload"), payload)
                                .append_query(U("signature"), signature).to_string(), U("application/json"))
                       // Get the response.
                       .then([](http_response response)
    {
        // Check the status code.
        if (response.status_code() != 200)
        {
            throw std::runtime_error("Returned " + std::to_string(response.status_code()));
        }

        return response.extract_json(true);
    })
    // Parse the user details.
    .then([](json::value jsonObject)
    {
        std::cout << "bind port : "<< jsonObject.serialize() << std::endl;
    });
    // Wait for the concurrent tasks to finish.
    try
    {
        requestJson.wait();
    }
    catch (const std::exception &e)
    {
        printf("Error exception:%s\n", e.what());
    }

    // TODO SAVE on sqlite
    // TODO refactor the code
    // create scheduled task execution


    return 0;
}

