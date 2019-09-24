#include "pch.h"
#include "dns.h"
#include "windows.h"
#include <iostream>
#include <vector>
#include "HTMLParserBase.h"

void dns::nslookup(std::string hostname) {
	WSADATA wsaData;
	int iResult;

	DWORD dwError;
	int i = 0;

	struct in_addr addr;

	char** pAlias;

	iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
	if (iResult != 0) {
		
		("WSAStartup failed: %d\n", iResult);
		//return;
	}

	struct hostent* remoteHost;
	const char* chost = hostname.c_str();
	remoteHost = gethostbyname(chost);

	if (remoteHost == NULL) {
		dwError = WSAGetLastError();
		if (dwError != 0) {
			if (dwError == WSAHOST_NOT_FOUND) {
				//printf("Host not found\n");

			}
			else if (dwError == WSANO_DATA) {
				//printf("No data record found\n");

			}
			else {
				//printf("Function failed with error: %ld\n", dwError);

			}
		}
	}
	else {
		printf("Function returned:\n");
		printf("\tOfficial name: %s\n", remoteHost->h_name);
		for (pAlias = remoteHost->h_aliases; *pAlias != 0; pAlias++) {
			printf("\tAlternate name #%d: %s\n", ++i, *pAlias);
		}
		printf("\tAddress type: ");
		switch (remoteHost->h_addrtype) {
		case AF_INET:
			printf("AF_INET\n");
			break;
		case AF_NETBIOS:
			printf("AF_NETBIOS\n");
			break;
		default:
			printf(" %d\n", remoteHost->h_addrtype);
			break;
		}
		printf("\tAddress length: %d\n", remoteHost->h_length);

		i = 0;
		if (remoteHost->h_addrtype == AF_INET)
		{
			while (remoteHost->h_addr_list[i] != 0) {
				addr.s_addr = *(u_long*)remoteHost->h_addr_list[i++];
				printf("\tIP Address #%d: %s\n", i, inet_ntoa(addr));

			}
		}
		else if (remoteHost->h_addrtype == AF_NETBIOS)
		{
			printf("NETBIOS address was returned\n");
		}
	}

	
}