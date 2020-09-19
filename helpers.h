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
/**
 * A callback to be passed into EnumWindows
 *
 * @param hwnd - window handler
 * @param lParam - passed from EnumWindows(this_callback, lParam)
 *                 here - pid
 * @return
 */
BOOL CALLBACK get_pid_procinfo(_In_ HWND   hwnd, _In_ LPARAM lParam);
/**
 * See above
 * @param hwnd - see above
 * @param lParam - not used here
 * @return
 */
BOOL CALLBACK enum_all_procs(_In_ HWND hwnd, _In_ LPARAM lParam);
void printHwndText(HWND hwnd);

void start_pressing(HWND hwnd = nullptr);
void stop_pressing();
void toggle_pressing(HWND hwnd = nullptr);
void prepare_keys(const std::vector<unsigned>& keycodes);

#endif //INPUTMANIP_HELPERS_H
