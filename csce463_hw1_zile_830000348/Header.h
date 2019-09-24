#pragma once
#include "pch.h'


class Socket {
public:
	Socket();
	~Socket();
private:
	SOCKET sock;
	char* buf;
	int allocatedSize;
	int curPos;
	bool initialized;
};