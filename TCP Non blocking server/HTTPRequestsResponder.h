#pragma once
using namespace std;
#define _SILENCE_EXPERIMENTAL_FILESYSTEM_DEPRECATION_WARNING 
#include <algorithm>
#include <iostream>
#include <fstream>
#include <experimental/filesystem>
#include <string>

#define NOT_FOUND "404 Not Found"
#define SERVER_ERROR "500 Internal Server Error"
#define OK "200 OK"
#define NOT_IMPLEMENTED "501 Not Implemented"
#define ROOT_FOLDER "C:\\temp"
#define PAGE404 "\\pages\\page404.html"

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
	static string get_body_from_file_and_update_status_and_type(string path_to_file, string& response_status, string& content_type);
	static string get_extention_to_content_type(string file_path);
	static string set_status_404_and_get_404_content(string& response_status, string& content_type);
	static string print_body_content_and_generate_response_body(string body_content);
	static string create_or_replace_file_content_with_body_and_return_response_body(
		string path_to_file, string request_body, string& response_status);
	static string try_delete_file_and_get_response(string path_to_file, string& response_status, string& content_type);


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