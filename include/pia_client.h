#ifndef PIA_CLIENT_H
#define PIA_CLIENT_H

#include <string>
#include <cpprest/http_client.h>
#include <functional>
#include <cpprest/json.h>
#include <pplx/pplxtasks.h>
class PiaClient
{
    public:
        PiaClient();
        virtual ~PiaClient();
        std::string get_public_ip();
        bool auth(const std::string &username, const std::string &password);
        bool generate_signature();
        bool bind_port();
        int get_port() const;
        bool isExpired();
        std::string currentISO8601TimeUTC();
        long stringToTimestamp(const std::string &time);
    protected:

    private:
        web::http::client::http_client_config config;
        // private access internet host
        std::string pia_host;
        std::string token;
        std::string signature;
        std::string payload;
        std::string pia_request_uri;

        void process_request(const pplx::task<web::http::http_response> &task,std::function<void(const web::json::value &jsonObject)> handler_func);



};

#endif // PIA_CLIENT_H
