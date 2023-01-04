#pragma once
using namespace std;
#include <algorithm>
#include <iostream>
#include <fstream>
#include <string>

#define NOT_FOUND "404 Not Found";
#define SERVER_ERROR "500 Internal Server Error";
#define OK "200 OK";
#define NOT_IMPLEMENTED "501 Not Implemented";

class HTTPRequestsResponder
{
private:
	enum class eLanguage {
		HE,
		FR,
		EN
	};

	static eLanguage get_lang_query_param(string query_params);
	static string create_path_to_file(string endpoint, eLanguage lang);
	static string get_body_from_file_and_return_status(string path_to_file, string& response_status);
	static string print_body_content_and_generate_response_body(string body_content);

public:
	enum class eRequestType {
		GET,
		POST,
		HEAD,
		PUT,
		TRACE,
		DEL,
		OPTIONS,
		ILEGALREQUEST
	};

	struct HTTPRequestInfo
	{
		string full_request;
		eRequestType request_type;
		string endpoint;
		string query_params;
		string headers;
		string body;
	};

	static string do_request_and_generate_http_response(HTTPRequestInfo httpRequest);

};