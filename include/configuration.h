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
    std::string get_default_config_path();
    std::string get_vpn_iface();
    std::string get_pia_username();
    std::string get_pia_password();

protected:

private:
    std::string default_config_path;
    boost::program_options::variables_map vm;
    boost::program_options::options_description fileOptions;
};

#endif // CONFIGURATION_H
