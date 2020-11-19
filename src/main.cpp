#include <iostream>
#include <csignal>
#include <atomic>
#include <thread>
#include <iostream>

#include "uuid.h"
#include "net_tools.h"
#include "configuration.h"
#include "pia_client.h"
#include "shell.h"
std::atomic<bool> alive(true);
std::atomic<int> signum(0);
void signalHandler( int sg )
{
    // cleanup and close up stuff here
    // terminate program
    alive.store(false);
    signum.store(sg);
}

void runnable(const Configuration &conf)
{
    std::cout << "PIA Service" << std::endl;
    // Get the ip from the vpn interface
    PiaClient piaClient;
    std::cout << "public ip: " << piaClient.get_public_ip() << std::endl;


    while(alive.load())
    {
        std::cout << "check if is expireing: " << std::endl;
        if(piaClient.isExpired())
        {
            std::cout << "Expired retry authentication: " << std::endl;
            std::cout << piaClient.auth(conf.get_pia_username(), conf.get_pia_password()) << std::endl;
            std::cout << piaClient.generate_signature() << std::endl;
        }
        std::cout << "Bind the port: " << std::endl;
        std::cout << piaClient.bind_port() << std::endl;

        // Set the port on deluge client
        // generate the shell command
        // example: connect ip:port username password; config --set listen_ports (port,port)
        std::ostringstream oss;
        oss << "deluge-console \"connect " << conf.get_deulged_host()  <<":" << conf.get_deluged_port();
        oss << " " <<  conf.get_deluged_username() << " " <<  conf.get_deluged_password();
        oss <<  "; config --set listen_ports ("<< piaClient.get_port()<<","<<piaClient.get_port()<<")\"";
        std::cout << oss.str() << std::endl;
        std::cout << shell::cmd{oss.str()}.str() << std::endl;
        std::this_thread::sleep_for(std::chrono::milliseconds(conf.get_check_frequency()*60*1000));
    }
}

int main(int argc, char *argv[])
{

    // register signal SIGINT and signal handler
    signal(SIGINT, signalHandler);
    Configuration conf;

    // Parse args and overrid the config directory if -c option exist
    int parse_result = conf.parse(argc, argv);
    if(parse_result!=1)
        return parse_result;


    std::thread executor(runnable, conf);
    executor.join();
    std::cout << "pia service stopped" << std::endl;
    // TODO SAVE on sqlite

    return signum.load();
}

