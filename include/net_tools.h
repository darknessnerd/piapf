#ifndef NET_TOOLS_H
#define NET_TOOLS_H

#include <string>
#include <vector>
namespace net_tools
{
std::string get_ip_address(const std::string &iface);
std::vector<std::string> dns_lookup(const std::string &host_name, int ipv=4);
}

#endif // NET_TOOLS_H
