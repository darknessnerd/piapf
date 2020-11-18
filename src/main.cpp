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
    PiaClient piaClient;

    std::cout << "public ip: " << piaClient.get_public_ip() << std::endl;
    std::cout << piaClient.auth(conf.get_pia_username(), conf.get_pia_password()) << std::endl;
    std::cout << piaClient.generate_signature() << std::endl;
    std::cout << piaClient.bind_port() << std::endl;

    // TODO SAVE on sqlite
    // create scheduled task execution

    return 0;
}

