#include "pch.h"
#include "fileHandler.h"

fileHandler::fileHandler() {
	int fileSize;
	std::unordered_set<std::string> seenIPs;
	std::unordered_set<std::string> seenHosts;
}

std::queue<std::string> fileHandler::getQueued(char filename[], std::queue<std::string> q) {
	char* fileBuffed;
	char* context = NULL;
	fileBuffed = fileHandler::extractText(filename);
	char* token = NULL;

	token = strtok_s(fileBuffed, "\r\n", &context);

	while (token != NULL) {
		std::string tokenStr = std::string(token);
		q.push(token);
		token = strtok_s(NULL, "\r\n", &context);
	}
	return q;

	}

char* fileHandler::extractText(char filename[]) {
	HANDLE hFile = CreateFile(filename, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING,
		FILE_ATTRIBUTE_NORMAL, NULL);
	// process errors
	if (hFile == INVALID_HANDLE_VALUE)
	{
		//printf("CreateFile failed with %d\n", GetLastError());
		exit(0);
	}
	LARGE_INTEGER li;
	BOOL sizeError = GetFileSizeEx(hFile, &li);
	if (sizeError == 0) {
		//printf("GetFileSizeEx error %d\n", GetLastError());
		exit(0);
	}

	//read file into buffer
	fileSize = (DWORD)li.QuadPart;
	DWORD bytesRead;

	// allocate buffer
	char* allURL = new char[fileSize];

	//read into buffer
	BOOL readError = ReadFile(hFile, allURL, fileSize, &bytesRead, NULL);
	if (readError == 0 || bytesRead != fileSize) {
		//printf("ReadFile failed with %d\n", GetLastError());
		return 0;
	}

	// close file
	CloseHandle(hFile);
	//std::cout << fileSize << std::endl;
	return allURL;
}

bool fileHandler::checkIP(std::string IP) {
	int prevSize = fileHandler::seenIPs.size();
	fileHandler::seenIPs.insert(IP);
	//printf("\t  Checking IP uniqueness... ");
	if (seenIPs.size() > prevSize) {
		// new IP
		//printf("passed\n");
		return true;
	}
	//printf("failed\n");
	return false;
}

bool fileHandler::checkHost(std::string Host) {
	int prevSize = fileHandler::seenHosts.size();
	fileHandler::seenHosts.insert(Host);
	
	("\t  Checking host uniqueness... ");
	if (seenHosts.size() > prevSize) {
		// new Host
		//printf("passed\n");
		return true;
	}
	//printf("failed\n");
	return false;
}

void fileHandler::printAll() {
	for (auto const& i : seenIPs) {
		std::cout << "." << i << std::endl;
	}
	for (auto const& i : seenHosts) {
		std::cout << "." << i << std::endl;
	}
}