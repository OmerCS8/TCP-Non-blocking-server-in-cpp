#include "HTTPRequestsResponder.h"

HTTPRequestsResponder::eLanguage HTTPRequestsResponder::get_lang_query_param(string query_params)
{
	if (query_params.find("lang=he") != string::npos)
		return HTTPRequestsResponder::eLanguage::HE;
	else if (query_params.find("lang=fr") != string::npos)
		return HTTPRequestsResponder::eLanguage::FR;
	else
		return HTTPRequestsResponder::eLanguage::EN;
}

string HTTPRequestsResponder::create_path_to_file(string endpoint, eLanguage lang)
{
	string path_to_relative_root = string(__FILE__).substr(0, string(__FILE__).find_last_of('\\'));
	string full_file_name = endpoint.substr(endpoint.find_last_of('/'));
	string file_name = full_file_name.find('.') != string::npos? full_file_name.substr(0, full_file_name.find('.')) : full_file_name;
	string file_extension = full_file_name.find('.') != string::npos ? full_file_name.substr(full_file_name.find('.')) : "";
	string relativ_file_position = endpoint.substr(0, endpoint.find_last_of('/'));
	std::replace(relativ_file_position.begin(), relativ_file_position.end(), '/', '\\');

	switch (lang)
	{
	case HTTPRequestsResponder::eLanguage::EN:
		path_to_relative_root += relativ_file_position + file_name +"_EN" + file_extension;
		break;
	case HTTPRequestsResponder::eLanguage::HE:
		path_to_relative_root += relativ_file_position + file_name +"_HE" + file_extension;
		break;
	case HTTPRequestsResponder::eLanguage::FR:
		path_to_relative_root += relativ_file_position + file_name +"_FR" + file_extension;
		break;
	}

	return path_to_relative_root;
}

string HTTPRequestsResponder::get_body_from_file_and_return_status(string path_to_file, string& response_status)
{
	string path_to_not_found_file = string(__FILE__).substr(0, string(__FILE__).find_last_of('\\')) + "\\pages\\endpoint_not_exist.html";
	string file_content;
	ifstream file(path_to_file);

	if (file.is_open()) {
		response_status = OK;
		string content((istreambuf_iterator<char>(file)), (istreambuf_iterator<char>()));
		file_content = content;
		file.close();
	}
	else
	{
		file.open(path_to_not_found_file);
		if (file.is_open()) {
			response_status = NOT_FOUND;
			string content((istreambuf_iterator<char>(file)), (istreambuf_iterator<char>()));
			file_content = content;
			file.close();
		}
		else
		{
			response_status = SERVER_ERROR;
		}
	}

	return file_content;
}

void HTTPRequestsResponder::print_body_content(string body_content)
{
	cout << "\n*****************************************\n";
	cout << "Post request body's content:";
	cout << "\n-----------------------------------------\n";
	cout << body_content;
	cout << "\n*****************************************\n\n";
}

string HTTPRequestsResponder::do_request_and_generate_http_response(HTTPRequestInfo httpRequest)
{
	// returned response
	string generated_response;

	// response optional parts
	string http_version = "HTTP/1.1";
	string http_response_status;
	string http_content_Type = "Content-Type: ";
	string http_content_length = "Content-Length: ";
	string http_body;

	// temp variabels
	string path_to_file;
	HTTPRequestsResponder::eLanguage lang;

	// generate response for request
	switch (httpRequest.request_type)
	{
	case eRequestType::GET:
		lang = get_lang_query_param(httpRequest.query_params);
		path_to_file = create_path_to_file(httpRequest.endpoint, lang);
		http_body = get_body_from_file_and_return_status(path_to_file, http_response_status);
		http_content_Type += "text/html";
		http_content_length += http_body.empty() ? "0" : to_string(http_body.size());
		generated_response = http_version + " " + http_response_status + "\n" +
			http_content_Type + "\n" + http_content_length + "\n\n" + http_body;
		break;
	case eRequestType::POST:
		print_body_content(httpRequest.body);


		break;
	case eRequestType::HEAD:

		break;
	case eRequestType::PUT:

		break;
	case eRequestType::TRACE:

		break;
	case eRequestType::DEL:

		break;
	case eRequestType::OPTIONS:

		break;
	default:

		break;
	}

	// return generated response
	return generated_response;
}
