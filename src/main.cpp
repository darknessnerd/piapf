#include <iostream>
#include <cpprest/http_client.h>

#include <iostream>
#include <cpprest/http_client.h>
#include <cpprest/filestream.h>
#include <cpprest/uri.h>
#include <cpprest/json.h>
#include <boost/program_options.hpp>
#include <fstream>
#include <iostream>
#include <exception>
using namespace utility;
using namespace web;
using namespace web::http;
using namespace web::http::client;
using namespace concurrency::streams;
namespace po = boost::program_options;

#include <unistd.h>
#include <sys/types.h>
#include <pwd.h>


int main(int argc, char *argv[])
{

    // Get the Linux home direcory config default
    const char *homedir;
    if ((homedir = getenv("HOME")) == NULL)
    {
        homedir = getpwuid(getuid())->pw_dir;
    }
    std::string defaultConfigPath;
    defaultConfigPath.append(homedir);
    defaultConfigPath.append("/.config/piapf/config");
    std::cout << defaultConfigPath << '\n';
    // Read the main arguments into the vm
    po::variables_map vm;
    // Create options description for file
    po::options_description fileOptions{"File"};
    fileOptions.add_options()("age", po::value<int>(), "Age");

    // Try to open and read the default config dir
    std::ifstream ifs{defaultConfigPath.c_str()};
    try
    {
        if (ifs)
            po::store(parse_config_file(ifs, fileOptions), vm);
    }
    catch (const std::exception &ex)
    {
        std::cerr << "Invalid default configuration file: "  << defaultConfigPath << ex.what() << '\n';
        vm.clear();
    }
    try
    {
        po::options_description generalOptions{"General"};
        generalOptions.add_options()
        ("help,h", "Help screen") // Help option
        ("config,c", po::value<std::string>(), "Config file"); // Config file

        po::store(parse_command_line(argc, argv, generalOptions), vm);

        // If the command line contain the file read the file and if exist override all the configurations
        if (vm.count("config"))
        {
            std::ifstream ifs{vm["config"].as<std::string>().c_str()};
            if (ifs)
                po::store(parse_config_file(ifs, fileOptions), vm);
            else
                throw std::invalid_argument("Config file can't be open");
        }
        // Commit the vm
        notify(vm);

        if (vm.count("help"))
            std::cout << generalOptions << '\n';
        else if (vm.count("age"))
            std::cout << "Your age is: " << vm["age"].as<int>() << '\n';
    }
    catch (const std::exception &ex)
    {
        std::cerr << ex.what() << '\n';
    }




    std::cout << "PIA Service" << std::endl;
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
        std::cout << jsonObject[U("ip")].as_string() << std::endl;
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

    return 0;
}

