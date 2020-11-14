#include <exception>


#include <cpprest/http_client.h>
#include <cpprest/filestream.h>
#include <cpprest/uri.h>
#include <cpprest/json.h>



#include <vector>

#include "uuid.h"
#include "net_tools.h"
#include "configuration.h"

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
    auto requestJson = http_client(U("https://api.ipify.org?format=json"), config)
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


    // TODO - auth token     save the token on sqlite
    // TODO - get signature
    // TODO - bind port


    return 0;
}

