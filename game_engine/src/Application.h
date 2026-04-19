#ifndef APPLICATION_H
#define APPLICATION_H


#include <thread>
#include <chrono>
#include "Helper.h"

class Application {
public:
	static void quit() {
		exit(0);
	}

	static int get_frame() {
		return Helper::GetFrameNumber();
	}

	static void sleep(int ms) {
		std::this_thread::sleep_for(std::chrono::milliseconds(ms));
	}

	static void open_url(std::string url) {
#ifdef _WIN32
		std::string a = "start " + url;
		std::system(a.c_str());
#else
#ifdef __APPLE__
		std::string b = "open " + url;
		std::system(b.c_str());
#else
		std::string c = "xdg-open " + url;
		std::system(c.c_str());
#endif
#endif
	}
};

#endif
