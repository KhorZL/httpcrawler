#pragma once
#include "pch.h"

class params {
public:
	HANDLE mutex;
	HANDLE semph;
	HANDLE fined;
	fileHandler fileHand;

	int nActiveThreads;
	size_t q_Qsize;
	int e_extractedURLs;
	int h_HostUniqPass;
	int d_DNSlpPass;
	int i_IPUniqPass;
	int r_roboPass;
	int c_crawledURLs;
	int l_linksFound;
	int downloadSize;

	int httpTwoxx;
	int httpThreexx;
	int httpFourxx;
	int httpFivexx;
	int httpOthers;
};