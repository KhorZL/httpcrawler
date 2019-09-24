#include "pch.h"
#include "socket.h"
#include "fileHandler.h"
#include "timer.h"
#include <typeinfo>
#include "HTMLParserBase.h"
#include <ctime>
#include <iomanip>
#include <math.h>
#pragma comment(lib, "ws2_32.lib")

bool Socket::sock_init(bool unique) {
	if (unique) {
		Socket::uniqueCheck = true;
	}

	WSADATA wsaData;
	//Initialize WinSock; once per program run
	WORD wVersionRequested = MAKEWORD(2, 2);
	if (WSAStartup(wVersionRequested, &wsaData) != 0) {
		//printf("WSAStartup error %d\n", WSAGetLastError());
		return false;
	}
	// open a TCP socket
	Socket::sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (sock == INVALID_SOCKET)
	{
		//printf("socket() generated error %d\n", WSAGetLastError());
		return false;
	}
	return true;
}

bool Socket::sock_conn(std::vector<std::string> parameters, params* p)
{
	std::string str = parameters[1].c_str();
	// structure used in DNS lookups
	struct hostent* remote;

	// structure for connecting to server
	struct sockaddr_in server;

	if (uniqueCheck) {
		if (p != NULL) {
			WaitForSingleObject(p->mutex, INFINITE);
			if (!fileHand.checkHost(str)) {
				ReleaseMutex(p->mutex);
				return false;
			}
			// update params
			p->h_HostUniqPass += 1;
			ReleaseMutex(p->mutex);

		}
	}

	// first assume that the string is an IP address
	DWORD IP = inet_addr(str.c_str());
	if (IP == INADDR_NONE)
	{
		//printf("\t  Doing DNS... ");
		clock_t time_dns = clock();
		try {
			remote = gethostbyname(str.c_str());
		}
		catch (const std::exception&){
			return false;
		}
		
		// if not a valid IP, then do a DNS lookup
		if (remote == NULL)
		{
			//printf("--failed with %d\n", WSAGetLastError());
			return false;
		}
		else {
			// update params
			if (p != NULL) {
				WaitForSingleObject(p->mutex, INFINITE);
				p->d_DNSlpPass += 1;
				ReleaseMutex(p->mutex);
			}
			
			//std::cout << "done in " << std::setprecision(0) << float(clock() - time_dns) * 1000 / CLOCKS_PER_SEC << " ms";
			memcpy((char*) & (server.sin_addr), remote->h_addr, remote->h_length);
			//printf(", found %s\n", inet_ntoa(server.sin_addr));
			
		}
	}
	else
	{
		// if a valid IP, directly drop its binary version into sin_addr
		server.sin_addr.S_un.S_addr = IP;
		//printf("\t  Doing DNS... done in 0 ms, found %s\n", inet_ntoa(server.sin_addr));
	}
	if (uniqueCheck) {
		if (p != NULL) {
			WaitForSingleObject(p->mutex, INFINITE);
			if (!fileHand.checkIP(inet_ntoa(server.sin_addr))) {
				ReleaseMutex(p->mutex);
				return false;
			}
			// update params
			p->i_IPUniqPass += 1;
			ReleaseMutex(p->mutex);
		}

		// connect on robots
		server.sin_family = AF_INET;
		server.sin_port = htons(80);		// host-to-network flips the byte order

		// connect to the server on port 80
		clock_t time_conn = clock();
		//printf("\t  Connecting on robots... ");
		int connectionStatus = connect(Socket::sock, (struct sockaddr*) & server, sizeof(struct sockaddr_in));
		if (connectionStatus == SOCKET_ERROR)
		{
			//printf("failed with %d\n", WSAGetLastError());
			return false;
		}
		else {
			//std::cout << "done in " << std::setprecision(0) << float(clock() - time_conn) * 1000 / CLOCKS_PER_SEC << " ms\n";
		}
		initialized = true;
		Socket::sendRequest(parameters, false);
		bool robotCheck = Socket::read(new char[8], false, p);
		if (!robotCheck){
			return false;
		}
		// update params
		if (p != NULL){
			WaitForSingleObject(p->mutex, INFINITE);
			p->downloadSize;
			p->r_roboPass += 1;
			ReleaseMutex(p->mutex);
		}
	}
	initialized = false;
	WSACleanup();
	free(Socket::buf);
	Socket::buf = new char[INITAL_BUFF_SIZE];
	bool pass = Socket::sock_init(Socket::uniqueCheck);
	if (!pass) {
		Socket::sock_init(Socket::uniqueCheck);
	}

	// setup the port # and protocol type
	server.sin_family = AF_INET;
	server.sin_port = htons(80);		// host-to-network flips the byte order

	// connect to the server on port 80
	clock_t time_conn = clock();
	//printf("\t* Connecting on page... ");
	int connectionStatus = connect(Socket::sock, (struct sockaddr*) & server, sizeof(struct sockaddr_in));
	if (connectionStatus == SOCKET_ERROR)
	{
		//printf("failed with %d\n", WSAGetLastError());
		return false;
	}
	else {

		//std::cout << "done in " << std::setprecision(0) << float(clock() - time_conn) * 1000 / CLOCKS_PER_SEC << " ms\n";
	}
	
	initialized = true;
	//std::cout << "after headreq memset " << (strlen(Socket::buf) == 0) << std::endl;
	return true;
}
Socket::Socket() {
	buf = new char[INITAL_BUFF_SIZE];
	allocatedSize = INITAL_BUFF_SIZE;
	curPos = 0;
	initialized = false;
	uniqueCheck = false;
}

Socket::Socket(fileHandler* fileH) {
	buf = new char[INITAL_BUFF_SIZE];
	allocatedSize = INITAL_BUFF_SIZE;
	curPos = 0;
	initialized = false;
	uniqueCheck = false;
	fileHand = *fileH;
}

int parseHTML(char html[], char* url) {
	HTMLParserBase* parser = new HTMLParserBase;
	int numLinks;
	char* linkBuffer = parser->Parse(html, strlen(html), url,strlen(url), &numLinks);
	// check for errors indicated by negative values
	if (numLinks < 0) {
		//std::cout << "parse error" << std::endl;
		numLinks = 0;
	}
	return numLinks;
}

void printError() {
	wchar_t* s = NULL;
	FormatMessageW(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
		NULL, WSAGetLastError(),
		MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
		(LPWSTR)& s, 0, NULL);
	//fprintf(stderr, "%S\n", s);
	LocalFree(s);
}

bool Socket::read(char url[], bool parse, params* p) {
	if (!initialized) {
		return false;
	}
	fd_set sockHold;
	FD_ZERO(&sockHold);
	FD_SET(sock, &sockHold);

	struct timeval timeout;
	timeout.tv_sec = 10;
	timeout.tv_usec = 0;

	//printf("\t  Loading... ");
	clock_t time_load = clock_t();

	while (true) {
		int ret = 0;

		if (parse && (curPos > 2000000)) {
			//std::cout << "download limit exceeded" << std::endl;
			return false;
		}
		if (!parse && (curPos > 16000)) {
			//std::cout << "download limit exceeded" << std::endl;
			return false;
		}

		if (ret = select(0, &sockHold, NULL, NULL, &timeout) > 0) {	
			// new data available; now read the next segment
			int diffPos = allocatedSize - curPos;
			if (buf + allocatedSize == 0) break;
			int recvBytes = recv(sock, (Socket::buf + curPos), diffPos, 0);
			curPos += recvBytes;

			int threshold = ceil(0.01 * allocatedSize);
			int sizeDiff = allocatedSize - curPos;
			if (allocatedSize - curPos < threshold) {
				allocatedSize = allocatedSize << 1;
				char* checker = (char*)realloc(Socket::buf, allocatedSize);
				if (checker == NULL) {
					free(Socket::buf);
					break;
				}
				else {
					buf = checker;
					continue;
				}
			}		
			// recvBytes is zero if closed successfully
			if (recvBytes == 0) {
				buf[curPos] = '\0';
				//std::cout << "done in " << std::setprecision(0) << float(clock() - time_load) * 1000 / CLOCKS_PER_SEC << " ms with " << curPos << " bytes\n";
				bool headerVerified = verifyHeader(p, !parse);

				// update params
				if (p != NULL) {
					WaitForSingleObject(p->mutex, INFINITE);
					p->downloadSize += curPos;
					ReleaseMutex(p->mutex);
				}

				if (headerVerified && parse) {
					//printf("\t+ Parsing page... ");
					clock_t time_parser = clock();
					int numLinks = parseHTML(Socket::buf, url);

					// update params
					if (p != NULL) {
						WaitForSingleObject(p->mutex, INFINITE);
						p->l_linksFound += numLinks;
						p->c_crawledURLs += 1;
						ReleaseMutex(p->mutex);
					}

					//std::cout << "done in " << std::setprecision(0) << float(clock() - time_parser) * 1000 / CLOCKS_PER_SEC << "ms with " << numLinks << " links\n";
				}
				if (!headerVerified) {
					return false;
				}
				return true;
			}
			else if (recvBytes > 0) {
				continue;
			}
			else {
				//printf("failed with %d\n", WSAGetLastError());
				return false;
			}
		}
		else {
			//std::cout << "failed with timeout" << std::endl;
			return false;
		}
		
	}	
	return false;
}

Socket::~Socket(){
	free(buf);
}

void Socket::reset() {
	Socket::allocatedSize = INITAL_BUFF_SIZE;
	Socket::curPos = 0;
	char* checker = (char*)realloc(Socket::buf, Socket::allocatedSize);
	if (checker == NULL) {
		free(Socket::buf);
	}
	else {
		Socket::buf = checker;
	}
}

void Socket::display() {
	printf("\n----------------------------------------\n");

	char* header_pos = strstr(buf, "<html>");
	std::string withoutHeader;
	if (header_pos != NULL) {
		// header_pos returns pointer to memaddr of <html>
		// buf returns pointer to memaddr of buffer
		// header_pos - buf gives the len of buffer up till <html>
		withoutHeader = std::string(buf, header_pos - buf);
		std::cout << withoutHeader << std::endl;
	}
	else {
		std::cout << buf << std::endl;
	}
}

bool Socket::sendRequest(std::vector<std::string> parameters, bool forGET){
	std::string request;
	if (forGET) {
		request = "GET " + parameters[3] + parameters[4];
	}
	else {
		request = ("HEAD /robots.txt");
	}
	request += " HTTP/1.0\r\n";
	request += "User-agent: myTAMUcrawler/1.0\r\n";
	request += "Host: " + parameters[1];
	request += "\r\n";
	request += "Connection: close\r\n";
	request += "\r\n";
	int ret = send(Socket::sock, request.c_str(), strlen(request.c_str()), 0);
	//std::cout << request << std::endl;
	if ( ret != request.length()) {
		//printf("failed with %d\n", WSAGetLastError());
		return false;
	}

	//std::cout << "buf is empty == " << (strlen(Socket::buf) == 0) << std::endl;
	return true;
}

bool Socket::verifyHeader(params* p, bool forRobot) {
	std::string header;
	//printf("\t  Verifying header... ");

	if (strlen(Socket::buf) == 0) {
		//std::cout << "buf empty" << std::endl;
		return false;
	}

	char* header_pos = strstr(Socket::buf, "HTTP/");
	if (header_pos == NULL) {
		//std::cout << "header not found" << std::endl;
		return false;
	}
	header = std::string(header_pos + 9, header_pos + 12);

	// to update params
	if (p != NULL) {
		WaitForSingleObject(p->mutex, INFINITE);
		if (header[0] == '2') {
			p->httpTwoxx += 1;
			ReleaseMutex(p->mutex);
			ReleaseMutex(p->mutex);
			if (!forRobot) return true;
		}
		else if (header[0] == '3') {
			p->httpThreexx += 1;
		}
		else if (header[0] == '4') {
			p->httpFourxx += 1;
			ReleaseMutex(p->mutex);
			ReleaseMutex(p->mutex);
			if (forRobot) return true;
		}
		else if (header[0] == '5') {
			p->httpFivexx += 1;
		}
		else {
			p->httpOthers += 1;
		}
		ReleaseMutex(p->mutex);
	}
	else {
		if (!forRobot && header[0] == '2') {
			return true;
		}
		else if (forRobot && header[0] == '4') {
			return true;

		}
	}
	return false;
}
