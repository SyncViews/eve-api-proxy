#pragma once
#include <string>
class Socket;

void send_crest_get_request(Socket *socket, const std::string &path);
