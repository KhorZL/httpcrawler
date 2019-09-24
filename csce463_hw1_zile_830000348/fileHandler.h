#pragma once
#include <unordered_set>
#include <queue>

class fileHandler
{
public:
	fileHandler();
	int fileSize;
	std::unordered_set<std::string> seenIPs;
	std::unordered_set<std::string> seenHosts;
	char* allURL;
	char* extractText(char filename[]);
	bool checkIP(std::string);
	bool checkHost(std::string);
	void printAll();
	std::queue<std::string> getQueued(char filename[], std::queue<std::string> q);
};

