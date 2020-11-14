#include <iostream>
#include <cpprest/http_client.h>

#include <iostream>
#include <cpprest/http_client.h>
#include <cpprest/filestream.h>
#include <cpprest/uri.h>
#include <cpprest/json.h>

using namespace utility;
using namespace web;
using namespace web::http;
using namespace web::http::client;
using namespace concurrency::streams;

int main(int argc, char *argv[])
{
    std::cout << "PIA Service" << std::endl;
    http_client_config config;
    config.set_validate_certificates(false);
    // Make a GET request.
	auto requestJson = http_client(U("https://api.ipify.org?format=json"), config)
		.request(methods::GET,uri_builder().append_query(U("format"), "json").to_string())
    // Get the response.
	.then([](http_response response) {
		// Check the status code.
		if (response.status_code() != 200) {
			throw std::runtime_error("Returned " + std::to_string(response.status_code()));
		}

		// Convert the response body to JSON object.
		return response.extract_json();
	})
	// Parse the user details.
	.then([](json::value jsonObject) {
		std::cout << jsonObject[U("ip")].as_string() << std::endl;
	});
    // Wait for the concurrent tasks to finish.
	try {
		requestJson.wait();
	} catch (const std::exception &e) {
		printf("Error exception:%s\n", e.what());
	}

    return 0;
}

