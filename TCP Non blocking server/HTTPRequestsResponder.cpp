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
	string path_to_file;
	string full_file_name = endpoint.substr(endpoint.find_last_of('/') + 1);
	string file_name = full_file_name.find('.') != string::npos? full_file_name.substr(0, full_file_name.find('.')) : full_file_name;
	string file_extension = full_file_name.find('.') != string::npos ? full_file_name.substr(full_file_name.find('.')) : "";
	string relativ_file_position = endpoint.substr(0, endpoint.find_last_of('/') + 1);
	std::replace(relativ_file_position.begin(), relativ_file_position.end(), '/', '\\');

	switch (lang)
	{
	case HTTPRequestsResponder::eLanguage::EN:
		path_to_file = ROOT_FOLDER + relativ_file_position + file_name +"_EN" + file_extension;
		break;
	case HTTPRequestsResponder::eLanguage::HE:
		path_to_file = ROOT_FOLDER + relativ_file_position + file_name +"_HE" + file_extension;
		break;
	case HTTPRequestsResponder::eLanguage::FR:
		path_to_file = ROOT_FOLDER + relativ_file_position + file_name +"_FR" + file_extension;
		break;
	}

	return path_to_file;
}

string HTTPRequestsResponder::get_body_from_file_and_update_status_and_type(string path_to_file, string& response_status, string& content_type)
{
	string file_content;
	ifstream file(path_to_file);

	if (file.is_open() && file.good()) {
		response_status = OK;
		string content((istreambuf_iterator<char>(file)), (istreambuf_iterator<char>()));
		file_content = content;
		content_type += get_extention_to_content_type(path_to_file);
		file.close();
	}
	else
	{
		file_content = set_status_404_and_get_404_content(response_status, content_type);
	}

	return file_content;
}

string HTTPRequestsResponder::get_extention_to_content_type(string file_path)
{
	string file_extention = file_path.find_last_of('.') != string::npos ? file_path.substr(file_path.find_last_of('.')) : "";
	string content_type;

	if (file_extention == ".html")
		content_type = "text/html";
	else if (file_extention == ".json")
		content_type = "text/json";
	else if (file_extention == ".xml")
		content_type = "application/xml";
	else if (file_extention == ".js")
		content_type = "application/javascript";
	else
		content_type = "text/plain";

	return content_type;
}

string HTTPRequestsResponder::set_status_404_and_get_404_content(string& response_status, string& content_type)
{
	string file_content;
	string path_to_not_found_file = ROOT_FOLDER;
	path_to_not_found_file += PAGE404;

	ifstream file(path_to_not_found_file);

	if (file.is_open() && file.good()) {
		response_status = NOT_FOUND;
		string content((istreambuf_iterator<char>(file)), (istreambuf_iterator<char>()));
		file_content = content;
		content_type += "text/html";
		file.close();
	}
	else
	{
		response_status = SERVER_ERROR;
		content_type += "text/plain";
	}

	return file_content;
}

string HTTPRequestsResponder::print_body_content_and_generate_response_body(string body_content)
{
	cout << "\n**********************************************\n";
	cout << "Post request body's content:";
	cout << "\n----------------------------------------------\n";
	cout << body_content;
	cout << "\n**********************************************\n\n";

	string response_body;
	response_body = "the next lines were printed in the server's console:\n";
	response_body += body_content;
	
	return response_body;
}

string HTTPRequestsResponder::create_or_replace_file_content_with_body_and_return_response_body(
	string path_to_file, string request_body, string& response_status)
{
	string response_body;
	string file_name = path_to_file.substr(path_to_file.find_last_of('\\') + 1);
	string file_path = path_to_file.substr(0, path_to_file.find_last_of('\\') + 1);

	//create dirs
	try {
		experimental::filesystem::create_directories(file_path);
	}
	catch (std::exception& e) {
		cout << "couldent create dirs." << std::endl;
	}

	ifstream ifile(path_to_file);
	if (ifile.is_open() && ifile.good()) // file exist
		response_body = "File \"" + file_name + "\" in path \"" + file_path + "\" was modified to: \"" + request_body + "\"";
	else // new file
		response_body = "File \"" + file_name + "\" was created in path \"" + file_path + "\" with the content: \"" + request_body + "\"";
	ifile.close();

	ofstream file(path_to_file);
	if (file.is_open() && file.good()) {
		file << request_body;
		file.close();
		response_status = OK;
	}
	else {
		response_body = "Problem with opening the file.";
		response_status = SERVER_ERROR;
	}

	return response_body;
}

string HTTPRequestsResponder::try_delete_file_and_get_response(string path_to_file, string& response_status, string& content_type)
{
	string response_body;
	string file_name = path_to_file.substr(path_to_file.find_last_of('\\') + 1);
	string path = path_to_file.substr(0, path_to_file.find_last_of('\\') + 1);
	std::replace(path.begin(), path.end(), '\\', '/');

	ifstream ifile(path_to_file);
	if (!(ifile.is_open() && ifile.good())) // file does not exist
	{
		response_body = set_status_404_and_get_404_content(response_status, content_type);
	}
	else { // file exist
		ifile.close();
		if (remove(path_to_file.c_str()) == 0) {
			response_status += OK;
			content_type += "application / json";
			response_body = "{\"success\":\"true\",\n\"deleted_file\":\"" + file_name + "\",\n\"path\":\"" + path + "\"}";
		}
		else
		{
			response_status += SERVER_ERROR;
			content_type += "application/json";
			response_body = "{\"error\":\"File could not be deleted\"}";
		}
	}

	return response_body;
}

string HTTPRequestsResponder::do_request_and_generate_http_response(HTTPRequestInfo httpRequest)
{
	// returned response
	string generated_response;

	// response optional parts
	string http_version = "HTTP/1.1";
	string http_response_status;
	string http_content_type = "Content-Type: ";
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
		http_body = get_body_from_file_and_update_status_and_type(path_to_file, http_response_status, http_content_type);
		http_content_length += http_body.empty() ? "0" : to_string(http_body.size());
		generated_response = http_version + " " + http_response_status + "\n" +
			http_content_type + "\n" + http_content_length + "\n\n" + http_body;
		break;
	case eRequestType::POST:
		http_body = print_body_content_and_generate_response_body(httpRequest.body);
		http_response_status = OK;
		http_content_type += "text/plain";
		http_content_length += http_body.empty() ? "0" : to_string(http_body.size());
		generated_response = http_version + " " + http_response_status + "\n" +
			http_content_type + "\n" + http_content_length + "\n\n" + http_body;
		break;
	case eRequestType::HEAD:
		lang = get_lang_query_param(httpRequest.query_params);
		path_to_file = create_path_to_file(httpRequest.endpoint, lang);
		http_body = get_body_from_file_and_update_status_and_type(path_to_file, http_response_status, http_content_type);
		http_content_length += http_body.empty() ? "0" : to_string(http_body.size());
		generated_response = http_version + " " + http_response_status + "\n" +
			http_content_type + "\n" + http_content_length + "\n\n";
		break;
	case eRequestType::PUT:
		lang = get_lang_query_param(httpRequest.query_params);
		path_to_file = create_path_to_file(httpRequest.endpoint, lang);
		http_body = create_or_replace_file_content_with_body_and_return_response_body(
			path_to_file, httpRequest.body, http_response_status);
		http_content_type += "text/plain";
		http_content_length += http_body.empty() ? "0" : to_string(http_body.size());
		generated_response = http_version + " " + http_response_status + "\n" +
			http_content_type + "\n" + http_content_length + "\n\n" + http_body;
		break;
	case eRequestType::TRACE:
		http_response_status = OK;
		http_content_type += "message/http";
		http_body = httpRequest.full_request;
		http_content_length += http_body.empty() ? "0" : to_string(http_body.size());
		generated_response = http_version + " " + http_response_status + "\n" +
			http_content_type + "\n" + http_content_length + "\n\n" + http_body;
		break;
	case eRequestType::DEL:
		lang = get_lang_query_param(httpRequest.query_params);
		path_to_file = create_path_to_file(httpRequest.endpoint, lang);
		http_body = try_delete_file_and_get_response(path_to_file, http_response_status, http_content_type);
		http_content_length += http_body.empty() ? "0" : to_string(http_body.size());
		generated_response = http_version + " " + http_response_status + "\n" +
			http_content_type + "\n" + http_content_length + "\n\n" + http_body;
		break;
	case eRequestType::OPTIONS:

		break;
	default:

		break;
	}

	// return generated response
	return generated_response;
}
