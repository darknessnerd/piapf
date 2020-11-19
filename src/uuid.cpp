#include "uuid.h"

#include <random>
#include <string>
#include <sstream>

using std::string;
namespace UUID
{

// anonymous namespace all private outside
namespace
{
unsigned char random_char()
{
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dis(0, 255);
    return static_cast<unsigned char>(dis(gen));
} // End namespace

string generate_hex(const unsigned int len)
{
    std::stringstream ss;
    for (auto i = 0; i < len; i++)
    {
        const auto rc = random_char();
        std::stringstream hexstream;
        hexstream << std::hex << int(rc);
        auto hex = hexstream.str();
        ss << (hex.length() < 2 ? '0' + hex : hex);
    }
    return ss.str();
}
}

string generate_uuid()
{
    std::stringstream ss;
    for (auto i = 0; i < 4; ++i)
    {
        ss << generate_hex(5) << "-";
    }
    ss << generate_hex(5);
    return ss.str();
}
} // namespace
