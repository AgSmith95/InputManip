/*
1st Route: HOOKS
SetWindowsHookExA
	https://learn.microsoft.com/en-us/windows/win32/api/winuser/nf-winuser-setwindowshookexa

LowLevelMouseProc
	https://learn.microsoft.com/en-us/windows/win32/winmsg/lowlevelmouseproc
	https://gist.github.com/hyrious/bb1080b0316dc83ba930ef12c1dd1ca6

WH_MOUSE_LL
	https://learn.microsoft.com/en-gb/windows/win32/winmsg/about-hooks?redirectedfrom=MSDN#wh_mouse_ll

2nd Route(?): RawInput
RawInput
	https://learn.microsoft.com/en-gb/windows/win32/inputdev/raw-input?redirectedfrom=MSDN

*/

#include <iostream>
#include <atomic>
#include <thread>
#include <chrono>

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif

#include "Windows.h"
#include "WinUser.h"


typedef BOOL (*installFn)();
typedef BOOL (*uninstallFn)();


std::atomic<bool> stop = false;


void installRunUninstall(installFn install, uninstallFn uninstall) {
	// INSTALL HOOK
	if (!install()) {
		std::cout << "install() hook failed. Exiting.\n";
		return;
	}
	std::cout << "install()-ed hook\n";
	// RUN
	MSG msg = { 0 };
	while (!stop.load()) {
		/*	From LowLevelMouseProc documentation:

			This hook is called in the context of the thread that installed it.
			The call is made by sending a message to the thread that installed the hook.
			!!! Therefore, the thread that installed the hook must have a message loop. !!!

			So, in other words, the message loop with GetMessage/PeekMessage is REQUIRED!
		*/
		BOOL peeked = PeekMessageA(
			&msg,
			NULL,
			WM_LBUTTONDOWN,
			WM_LBUTTONUP,
			PM_NOREMOVE
		);
		if (peeked) {
			std::cout << "THREAD: M DOWN/UP (" << msg.pt.x << ',' << msg.pt.y << ")\n";
		}
		// A simple busy loop WON'T WORK!
	}
	// UNINSTALL HOOK
	while (!uninstall()) {
		std::cout << "uninstalling hooks FAILED!\n";
	}
	std::cout << "hook uninstalled\n";
}


int main(int /*argc*/, const char** /*argv*/) {
	stop.store(false);
	// OPEN DLL
	HMODULE MouseDetectorDLL = LoadLibraryA("MouseDetectorDLL.dll");
	if (MouseDetectorDLL == NULL) {
		std::cout << "MouseDetectorDLL wasn't properly loaded. Exiting.\n";
		return 0;
	}
	std::cout << "Loaded MouseDetectorDLL\n";
	// GET FUNCTIONS
	installFn install = (installFn)GetProcAddress(MouseDetectorDLL, "install");
	if (install == NULL) {
		std::cout << "Failed to GetProcAddress install(). Exiting.\n";
		return 0;
	}
	uninstallFn uninstall = (uninstallFn)GetProcAddress(MouseDetectorDLL, "uninstall");
	if (uninstall == NULL) {
		std::cout << "Failed to GetProcAddress uninstall(). Exiting.\n";
		return 0;
	}
	// RUN THREAD
	// Moved message loop to a thread. Read more on this in installRunUninstall comments
	std::thread iruThread(installRunUninstall, install, uninstall);
	// JUST WAIT 5s for now
	std::cout << "SLEEP...\n";
	Sleep(1'000);
	std::cout << "SLEEP...\n";
	Sleep(1'000);
	std::cout << "SLEEP...\n";
	Sleep(1'000);
	std::cout << "SLEEP...\n";
	Sleep(1'000);
	std::cout << "SLEEP...\n";
	Sleep(1'000);
	// STOP THREAD
	stop.store(true);
	iruThread.join();
	// FREE LIBRARY
	FreeLibrary(MouseDetectorDLL);
	return 0;
}
