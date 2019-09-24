// csce463_hw1_zile_830000348.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include "pch.h"
#include "dns.h"
#include "socket.h"
#include "timer.h"
#include "HTMLParserBase.h"
#include "fileHandler.h"

#pragma comment(lib, "ws2_32.lib")

// create queue

std::queue<std::string> Q;

// check if valid url
// takes a char*
std::vector<std::string> validURL(char inputURL[], bool uniqueCheck) {
	//std::cout << "URL: " << inputURL << std::endl << "\t  Parsing URL... ";
	std::vector<std::string> parameters;
	std::string frag;
	std::string query;
	std::string path;
	std::string scheme;
	std::string port;
	std::string host;

	std::string url = inputURL;
	size_t max_len = url.length();

	// find scheme
	size_t scheme_pos = url.find_first_of("://");
	scheme = url.substr(0, scheme_pos);
	if (scheme != "http") {
		//printf("failed with invalid scheme\n");
		if (!uniqueCheck) {
			exit(0);
		}
		return parameters;
	}

	// remove scheme
	std::string trunc_url = url.substr(scheme_pos + 3);

	// find fragment
	size_t frag_pos = trunc_url.find_first_of('#');
	if (frag_pos != std::string::npos) {
		frag = trunc_url.substr(frag_pos+1);
		trunc_url.resize(frag_pos);
	}
	else {
		frag = "";
	}

	// find query
	size_t query_pos = trunc_url.find_first_of('?');
	if (query_pos == std::string::npos) {
		query = "";
	}
	else {
		query = trunc_url.substr(query_pos+1);
		trunc_url.resize(query_pos);
	}

	// find path
	size_t path_pos = trunc_url.find_first_of('/');
	if (path_pos == std::string::npos) {
		// check if path is actually there
		path = '/';
	}
	else {
		path = trunc_url.substr(path_pos);
		trunc_url.resize(path_pos);
	}

	// find host and port
	size_t port_pos = trunc_url.find_first_of(':');
	if (port_pos == std::string::npos) {
		port = "80";
	}
	else {
		port = trunc_url.substr(port_pos+1);
		trunc_url.resize(port_pos);
		if (port == "" or port == "0") {
			//printf("failed with invalid port\n");
			if (!uniqueCheck) {
				exit(0);
			}
			return parameters;
		}
	}

	host = trunc_url;

	if (host.length() > MAX_HOST_LEN) {
		//printf("\nHost name rejected: Host name provided is too long.\n");
		if (!uniqueCheck) {
			exit(0);
		}
		return parameters;
	}

	if ((path.length() + query.length()) > MAX_REQUEST_LEN) {
		//printf("\nRequest rejected: Request provided is too long.\n");
		if (!uniqueCheck) {
			exit(0);
		}
		return parameters;
	}

	//std::cout << "host " << host;
	//std::cout<< ", port " << port << ", request " << path << query << "\n";

	parameters.push_back(scheme);
	parameters.push_back(host);
	parameters.push_back(port);
	parameters.push_back(path);
	parameters.push_back(query);
	parameters.push_back(frag);
	return parameters;
}

void checkIfFalse(bool boolean) {
	if (!boolean) {
		exit(0);
	}
}

void thread(LPVOID p) {
	params* parameters = (params*)p;
	WaitForSingleObject(parameters->mutex, INFINITE);

	// begin critical region
	parameters->nActiveThreads += 1;
	std::string currURL;

	if (!Q.empty()) {
		currURL.assign(Q.front());
		Q.pop();
	}
	parameters->q_Qsize = Q.size();
	parameters->e_extractedURLs += 1;

	// critical region ends
	ReleaseMutex(parameters->mutex);
	ReleaseSemaphore(parameters->semph, 1, NULL);

	//std::cout << currURL << std::endl;
	
	fileHandler fileH = parameters->fileHand;
	Socket Socket(&fileH);
	std::vector<std::string> val = validURL((char*)currURL.c_str(), true);
	if (val.empty()) {
		free(Socket.buf);
		Socket.buf = new char[INITAL_BUFF_SIZE];
		return;
	}
	std::string baseURL = val[0] + "://" + val[1] + val[3];
	//WSACleanup();
	if (!Socket.sock_init(true)) {
		if (!Socket.sock_init(true)) {
			WaitForSingleObject(parameters->mutex, INFINITE);
			parameters->nActiveThreads -= 1;
			ReleaseMutex(parameters->mutex);
			ReleaseSemaphore(parameters->semph, 1, NULL);
			WSACleanup();
			return;
		}
	}

	bool conn = Socket.sock_conn(val, parameters);
	if (!conn) {
		free(Socket.buf);
		Socket.buf = new char[INITAL_BUFF_SIZE];
		fileH = Socket.fileHand;
		WSACleanup();
		WaitForSingleObject(parameters->mutex, INFINITE);
		parameters->nActiveThreads -= 1;
		ReleaseMutex(parameters->mutex);
		ReleaseSemaphore(parameters->semph, 1, NULL);
		return;
	}
	//std::cout << "after conn " << (strlen(Socket.buf) == 0) << std::endl;
	bool req = Socket.sendRequest(val);
	if (!req) {
		free(Socket.buf);
		Socket.buf = new char[INITAL_BUFF_SIZE];
		fileH = Socket.fileHand;
		WaitForSingleObject(parameters->mutex, INFINITE);
		parameters->nActiveThreads -= 1;
		ReleaseMutex(parameters->mutex);
		ReleaseSemaphore(parameters->semph, 1, NULL);
		WSACleanup();
		return;
	}
	bool read = Socket.read((char*)baseURL.c_str(), true, parameters);
	if (!read) {
		//std::cout << "newSize" << fileH.seenIPs.size() << std::endl;
		free(Socket.buf);
		Socket.buf = new char[INITAL_BUFF_SIZE];
		fileH = Socket.fileHand;
		WaitForSingleObject(parameters->mutex, INFINITE);
		parameters->nActiveThreads -= 1;
		ReleaseMutex(parameters->mutex);
		ReleaseSemaphore(parameters->semph, 1, NULL);
		WSACleanup();
		return;
	}
	else {
		free(Socket.buf);
		Socket.buf = new char[INITAL_BUFF_SIZE];
		fileH = Socket.fileHand;
		WaitForSingleObject(parameters->mutex, INFINITE);
		parameters->nActiveThreads -= 1;
		ReleaseMutex(parameters->mutex);
		ReleaseSemaphore(parameters->semph, 1, NULL);
		WSACleanup();
		return;
	}
	fileH = Socket.fileHand;
	WaitForSingleObject(parameters->mutex, INFINITE);
	parameters->nActiveThreads -= 1;
	ReleaseMutex(parameters->mutex);
	ReleaseSemaphore(parameters->semph, 1, NULL);
	WSACleanup();
	
}

void statsThread(LPVOID p) {
	params* parameters = (params*)p;
	int prevNumURLs = 0;
	int prevDownloadSize = 0;
	float bytes = 0;
	float urls = 0;

	std::chrono::steady_clock::time_point start_t = std::chrono::high_resolution_clock::now();

	while (WaitForSingleObject (parameters->fined, 2000) != 0) {

		std::chrono::steady_clock::time_point end_t = std::chrono::high_resolution_clock::now();
		auto duration = std::chrono::duration_cast<std::chrono::seconds>(end_t - start_t);
		bytes = (float)(parameters->downloadSize - prevDownloadSize) / (duration.count() * 1000000);
		urls = (float) (parameters->c_crawledURLs - prevNumURLs) / duration.count();
		prevDownloadSize = parameters->downloadSize;
		prevNumURLs = parameters->c_crawledURLs;

		printf("[%3d] %d Q %6d E %7d H %6d D %6d I %5d R %5d C %5d L %4dK\n", duration.count(), (int) parameters->nActiveThreads, (int) parameters->q_Qsize, parameters->e_extractedURLs, parameters->h_HostUniqPass, parameters->d_DNSlpPass, parameters->i_IPUniqPass, parameters->r_roboPass, parameters->c_crawledURLs, parameters->l_linksFound/1000);
		printf("\t*** crawling %.2f pps @ %.2f Mbps\n", urls, bytes);
	}

}

int main(int argc, char *argv[])
{	
	if (argc == 2) {
		std::string URL(argv[1]);
		std::vector <std::string> val = validURL(argv[1], false);
		Socket Socket;
		bool initCheck = Socket.sock_init(false);
		checkIfFalse(initCheck);
		bool connCheck = Socket.sock_conn(val);
		checkIfFalse(connCheck);
		bool sendCheck = Socket.sendRequest(val);
		checkIfFalse(sendCheck);
		std::string baseURL = val[0] + "://" + val[1];
		bool readCheck = Socket.read((char*)baseURL.c_str(), true);
		checkIfFalse(readCheck);
		Socket.display();
	}

	else if (argc == 3) {
		// init shared queue
		if (*argv[1] < '1') {
			printf("Invalid number of threads. Only one thread is allowed.");
			exit(0);
		}
		else if (!std::experimental::filesystem::exists(argv[2])) {
			printf("File designated by input path does not exist.");
			exit(0);
		}

		//TODO CHECK PARSER
		fileHandler fileH;
		char* textBuf = fileH.extractText(argv[2]);
		bool endOfFile = false;
		Q = fileH.getQueued(argv[2], Q);
		int nURLS = Q.size();
		int nThreads = atoi(argv[1]);

		params p;
		p.mutex = CreateMutex(NULL, 0, NULL);
		p.semph = CreateSemaphore(NULL, 0, nThreads, NULL);
		p.fined = CreateEvent(NULL, true, false, NULL);
		p.c_crawledURLs = 0;
		p.d_DNSlpPass = 0;
		p.e_extractedURLs = 0;
		p.h_HostUniqPass = 0;
		p.i_IPUniqPass = 0;
		p.l_linksFound = 0;
		p.nActiveThreads = 0;
		p.q_Qsize = 0;
		p.r_roboPass = 0;
		p.httpTwoxx = 0;
		p.httpThreexx = 0;
		p.httpFourxx = 0;
		p.httpFivexx = 0;
		p.httpOthers = 0;
		p.fileHand = fileH;

		timer timer;
		timer.start();

		HANDLE* handles = new HANDLE[nThreads];
		CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)statsThread, &p, 0, NULL);

		while (Q.size() != 0) {
			if (Q.size() <= nThreads) {
				nThreads = Q.size();
			}
			for (int i = 0; i < nThreads; i++) {
				handles[i] = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)thread, &p, 0, NULL);
				if (handles[i] == NULL) {
					ExitProcess(3);
				}
			}
			if (WaitForMultipleObjects(nThreads, handles, TRUE, INFINITE) == WAIT_FAILED) 
				continue;
			for (int i = 0; i < nThreads; i++) {
				CloseHandle(handles[i]);
			}
		}
		SetEvent(p.fined);

		int duration = timer.end();
		if (duration < 1) return 0;
		try {
			std::cout << "Extracted " << nURLS << " URLS @ " << (nURLS / duration) << "/s" << std::endl;
			std::cout << "Looked up " << p.h_HostUniqPass << " DNS names @ " << (p.h_HostUniqPass / duration) << "/s" << std::endl;
			std::cout << "Downloaded " << p.r_roboPass << " robots @ " << (p.r_roboPass / duration) << "/s" << std::endl;
			std::cout << "Crawled " << p.c_crawledURLs << " pages @ " << (p.c_crawledURLs / duration) << "/s" << std::endl;
			std::cout << "Parsed " << p.l_linksFound << " links @ " << (p.l_linksFound / duration) << "/s" << std::endl;
			std::cout << "HTTP codes: 2xx = " << p.httpTwoxx << " 3xx = " << p.httpThreexx << " 4xx = " << p.httpFourxx << " 5xx = " << p.httpFivexx << " other = " << p.httpOthers << std::endl;
		}
		catch (const std::exception&) {}
		exit(0);



		while (!endOfFile) {
			Socket Socket(&fileH);
			//std::cout << "prevSize" << fileH.seenIPs.size() << std::endl;
			Socket.sock_init(true);
			char* endLine = strstr(textBuf, "\r\n");
			if (endLine != NULL) {
				std::string newLine = std::string(textBuf, endLine);
				if (newLine.empty()) {
					endOfFile = true;
				}
				// remove the \n as well
				textBuf = endLine + 2;
				std::vector<std::string> val = validURL((char*)newLine.c_str(), true);
				//Socket.display();
				if (val.empty()) {
					free(Socket.buf);
					Socket.buf = new char[INITAL_BUFF_SIZE];
					fileH = Socket.fileHand;
					continue;
				}
				std::string baseURL = val[0] + "://" + val[1] + val[3];
				WSACleanup();
				Socket.sock_init(true);
				bool conn = Socket.sock_conn(val);
				if (!conn) {
					free(Socket.buf);
					Socket.buf = new char[INITAL_BUFF_SIZE];
					fileH = Socket.fileHand;
					continue;
				}
				//std::cout << "after conn " << (strlen(Socket.buf) == 0) << std::endl;
				bool req = Socket.sendRequest(val);
				if (!req) {
					free(Socket.buf);
					Socket.buf = new char[INITAL_BUFF_SIZE];
					fileH = Socket.fileHand;
					continue;
				}
				bool read = Socket.read((char*) baseURL.c_str(), true);
				if (!read) {

					//std::cout << "newSize" << fileH.seenIPs.size() << std::endl;
					free(Socket.buf);
					Socket.buf = new char[INITAL_BUFF_SIZE];
					fileH = Socket.fileHand;
					continue;
				}
				else {
					free(Socket.buf);
					Socket.buf = new char[INITAL_BUFF_SIZE];
					fileH = Socket.fileHand;
				}
				fileH = Socket.fileHand;
			}
			else {
				endOfFile = true;
				if ((textBuf != NULL) && (textBuf[0] != '\0')) {
					std::string newLine = std::string(textBuf);
					if (newLine.empty()) {
						endOfFile = true;
					}
					// remove the \n as well
					std::vector<std::string> val = validURL((char*)newLine.c_str(), true);
					if (val.empty()) {
						Socket.reset();
						//free(Socket.buf);
						//Socket.buf = new char[INITAL_BUFF_SIZE];
						continue;
					}
					WSACleanup();
					Socket.sock_init(true);
					std::string baseURL = val[0] + "://" + val[1] + val[3];
					bool conn = Socket.sock_conn(val);
					if (!conn) {
						Socket.reset();
						//free(Socket.buf);
						//Socket.buf = new char[INITAL_BUFF_SIZE];
						continue;
					}
					bool req = Socket.sendRequest(val);
					if (!req) {
						Socket.reset();
						//free(Socket.buf);
						//Socket.buf = new char[INITAL_BUFF_SIZE];
						continue;
					}
					bool read = Socket.read((char*)baseURL.c_str(), true);
					if (!read) {
						Socket.reset();
						//free(Socket.buf);
						//Socket.buf = new char[INITAL_BUFF_SIZE];
						continue;
					}
				}
			}
		}
	}
	
	else {
		printf("Invalid number of arguments.\nPlease input only one URL argument for simple webcrawler.\nOr input <number of threads> <path to file> to crawl multiple URLs for robots.txt");
		exit(0);
	}
}


