#pragma once
#include "pch.h"
#include "fileHandler.h"
#include "params.h"

#define INITAL_BUFF_SIZE 8192

class Socket {
public:
	Socket();
	Socket(fileHandler* fileHandler);
	~Socket();
	char* buf;
	bool read(char url[], bool parse, params* p = NULL);
	bool sock_conn(std::vector<std::string> parameters, params* p = NULL);
	void display(void);
	bool sendRequest(std::vector<std::string> parameters, bool forGET=true);
	bool verifyHeader(params* p, bool forRobot=false);
	bool sock_init(bool unique);
	void reset();
	fileHandler fileHand;
private:
	SOCKET sock;
	int curPos;
	bool initialized;
	bool uniqueCheck;
	int allocatedSize;
};