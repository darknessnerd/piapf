#include <iostream>

#include "uuid.h"
#include "net_tools.h"
#include "configuration.h"
#include "pia_client.h"

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
    PiaClient piaClient;

    std::cout << "public ip: " << piaClient.get_public_ip() << std::endl;
    std::cout << piaClient.auth(conf.get_pia_username(), conf.get_pia_password()) << std::endl;
    std::cout << piaClient.generate_signature() << std::endl;
    std::cout << piaClient.bind_port() << std::endl;

    // TODO SAVE on sqlite
    // tracert implementation
    // create scheduled task execution


    return 0;
}

