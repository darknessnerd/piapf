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
    std::cout << "PIA Service running: " << std::endl;
    // Get the ip from the vpn interface
    PiaClient piaClient;
    std::cout << "Public ip: " << piaClient.get_public_ip() << std::endl;


    while(alive.load())
    {
        bool auth, sign, binding = false;
        if(piaClient.isExpired())
        {
            auth = piaClient.auth(conf.get_pia_username(), conf.get_pia_password());
            sign = piaClient.generate_signature();
        }
        if( auth && sign)
        {
            binding = piaClient.bind_port();
            // Set the port on deluge client
            // generate the shell command
            // example: connect ip:port username password; config --set listen_ports (port,port)
            if( binding )
            {
                std::ostringstream oss;
                oss << "deluge-console \"connect " << conf.get_deulged_host()  <<":" << conf.get_deluged_port();
                oss << " " <<  conf.get_deluged_username() << " " <<  conf.get_deluged_password();
                oss <<  "; config --set listen_ports ("<< piaClient.get_port()<<","<<piaClient.get_port()<<")\"";
                // Run the deluge command line
                std::cout << shell::cmd{oss.str()}.str() << std::endl;
            }
        }
        else
        {
            std::cerr << "Binding failed due too auth=" << auth << ", sign=" << sign << ", bind=" << binding << std::endl;
        }
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

