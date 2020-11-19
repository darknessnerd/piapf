#ifndef CONFIGURATION_H
#define CONFIGURATION_H

#include <string>
#include <boost/program_options.hpp>

class Configuration
{
public:
    Configuration();
    virtual ~Configuration();
    int parse(int argc, char *argv[]);
    std::string get_default_config_path() const;
    std::string get_vpn_iface() const;
    std::string get_pia_username() const;
    std::string get_pia_password() const;
    std::string get_deulged_host() const;
    std::string get_deluged_username() const;
    std::string get_deluged_password() const;
    int get_deluged_port() const;
    int get_check_frequency() const;

protected:

private:
    std::string default_config_path;
    boost::program_options::variables_map vm;
    boost::program_options::options_description fileOptions;
};

#endif // CONFIGURATION_H
