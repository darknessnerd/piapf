#include <pwd.h> // getenv, getenv
#include <unistd.h> // getuid, getpwuid

#include <iostream>
#include <fstream>

#include "configuration.h"

using std::string;
namespace po = boost::program_options;

Configuration::Configuration():fileOptions("config")
{
    // Get the Linux home direcory config default
    const char *homedir;
    if ((homedir = getenv("HOME")) == NULL)
    {
        homedir = getpwuid(getuid())->pw_dir;
    }
    string defaultConfigPath;
    defaultConfigPath.append(homedir);
    defaultConfigPath.append("/.config/piapf/config");
    this->default_config_path= defaultConfigPath;

    // Define all the options for the config file
    fileOptions.add_options()("username", po::value<std::string>()->required(), "Private internet access username");
    fileOptions.add_options()("password", po::value<std::string>()->required(), "Private internet access password");
    fileOptions.add_options()("vpn.iface", po::value<std::string>()->required(), "Vpn interface");
    fileOptions.add_options()("deluged.username", po::value<std::string>()->required(), "Deluge daemon username");
    fileOptions.add_options()("deluged.password", po::value<std::string>()->required(), "Deluge daemon password");
    fileOptions.add_options()("deluged.host", po::value<std::string>()->required(), "Deluge daemon host");
    fileOptions.add_options()("deluged.port", po::value<int>()->required(), "Deluge daemon port");
    fileOptions.add_options()("check.frequency", po::value<int>()->required(), "Check the port binding every x minutes");



    // Try to open and read the default config file
    std::ifstream ifs{this->default_config_path.c_str()};
    try
    {
        if (ifs)
            po::store(parse_config_file(ifs, fileOptions), this->vm);
    }
    catch (const std::exception &ex)
    {
        std::cerr << "Invalid default configuration file: "  << this->default_config_path << ex.what() << '\n';
        this->vm.clear();
    }
}

int Configuration::parse(int argc, char *argv[])
{
    try
    {
        po::options_description generalOptions{"General"};
        generalOptions.add_options()
        ("help,h", "Help screen") // Help option
        ("config,c", po::value<std::string>(), "Config file"); // Config file

        po::store(parse_command_line(argc, argv, generalOptions), this->vm);

        // If the command line contain the file read the file and if exist override all the configurations
        if (vm.count("config"))
        {
            std::ifstream ifs{vm["config"].as<std::string>().c_str()};
            if (ifs)
                po::store(parse_config_file(ifs, this->fileOptions), this->vm);
            else
                throw std::invalid_argument("Config file can't be open");
        }
        // Commit the vm
        notify(vm);

        if (vm.count("help"))
        {
            std::cout << generalOptions << '\n';
            return 0;
        }
        std::cout << "######################################## Loaded configuration ##########################" << std::endl;
        // Check content
        if (vm.count("username"))
            std::cout << "username: " << this->vm["username"].as<std::string>() << '\n';
        else
            throw std::invalid_argument("invalid_argument: username");
        if (vm.count("password"))
            std::cout << "password: ****" << '\n';
        else
            throw std::invalid_argument("invalid_argument: password");
        if (vm.count("vpn.iface"))
            std::cout << "vpn.iface: " << this->vm["vpn.iface"].as<std::string>() << '\n';
        else
            throw std::invalid_argument("invalid_argument: vpn.iface");
        if (vm.count("deluged.username"))
            std::cout << "deluged.username: " << this->vm["deluged.username"].as<std::string>() << '\n';
        else
            throw std::invalid_argument("invalid_argument: deluged.username");
        if (vm.count("deluged.password"))
            std::cout << "deluged.password: ***" << '\n';
        else
            throw std::invalid_argument("invalid_argument: deluged.password");
        if (vm.count("deluged.port"))
            std::cout << "deluged.port: " << this->vm["deluged.port"].as<int>() << '\n';
        else
            throw std::invalid_argument("invalid_argument: deluged.port");
        if (vm.count("check.frequency"))
            std::cout << "check.frequency " << this->vm["check.frequency"].as<int>() << '\n';
        else
            throw std::invalid_argument("invalid_argument: check.frequency");
        std::cout << "######################################## End configuration    ##########################" << std::endl;
    }
    catch (const std::exception &ex)
    {
        std::cerr << ex.what() << '\n';
        return -1;
    }
    return 1;
}

string Configuration::get_pia_username() const
{
    return this->vm["username"].as<string>();
}

string Configuration::get_pia_password() const
{
    return this->vm["password"].as<string>();
}

int Configuration::get_check_frequency() const
{
    return this->vm["check.frequency"].as<int>();
}

string Configuration::get_vpn_iface() const
{
    return this->vm["vpn.iface"].as<string>();
}

string Configuration::get_deulged_host() const
{
    return this->vm["deluged.host"].as<string>();
}

string Configuration::get_deluged_username() const
{
    return this->vm["deluged.username"].as<string>();
}

string Configuration::get_deluged_password() const
{
    return this->vm["deluged.password"].as<string>();
}

int Configuration::get_deluged_port() const
{
    return this->vm["deluged.port"].as<int>();
}

string Configuration::get_default_config_path() const
{
    return this->default_config_path;
}

Configuration::~Configuration()
{
    this->vm.clear();
}
