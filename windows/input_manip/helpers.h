#ifndef INPUTMANIP_HELPERS_H
#define INPUTMANIP_HELPERS_H

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
extern "C" {
#include <winsock.h>
}

#pragma comment(lib, "Ws2_32.lib")

#include <windows.h>
#include <winuser.h>
#define MOD_NOREPEAT 0x4000 // because for some reason it is not defined

#include <vector>

void reg_hotkey_wrapper(UINT vk, int id = 1, UINT mod = MOD_NOREPEAT);
void toggle_pressing();
void stop_pressing();
void prepare_keys(const std::vector<unsigned>& keycodes);

#endif //INPUTMANIP_HELPERS_H
