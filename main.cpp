#include "helpers.h"

#include <iostream>
#include <cstring>

#include <thread>
#include <chrono>

int main() {
    std::cout << "InputManip " BUILD_TYPE "\n\n";
    std::cout << "Press F2 to stop/start pressing\n";
    std::cout << "Press F10 to exit\n";
    reg_hotkey_wrapper(VK_F2, 1);
    reg_hotkey_wrapper(VK_F10, 2);
    reg_hotkey_wrapper(VK_F3, 3);

    MSG msg;
    memset(&msg, 0, sizeof(MSG));
    GUITHREADINFO info;
    memset(&info, 0, sizeof(GUITHREADINFO));

    prepare_keys({0x49, 0x4A, 0x4B, 0x4C});
    long long llHwnd = 0;
    HWND hwnd = nullptr;
    char cont = 'y';
    while (GetMessage(&msg, NULL, WM_HOTKEY, WM_HOTKEY) != 0) { // or PeekMessage + ", PM_NOREMOVE | PM_NOYIELD"
        switch (msg.wParam) {
            case 1:
                stop_pressing();
                //EnumWindows(get_pid_procinfo, 11196);
                EnumDesktopWindows(nullptr, enum_all_procs, 0);
                do {
                    std::cout << "\n===> Enter desired window hwnd:   ";
                    std::cin >> llHwnd;
                    hwnd = (HWND) llHwnd;
                    std::cout << "You've selected window: ";
                    printHwndText(hwnd);
                    std::cout << "It's children are: \n\n";
                    EnumChildWindows(hwnd, enum_all_procs, 0);
                    std::cout << "Continue? (n - to stop)";
                    std::cin >> cont;
                } while (cont != 'n');
                //toggle_pressing(hwnd);
                break;
            case 2:
                stop_pressing();
                std::this_thread::sleep_for(std::chrono::milliseconds(50));
                return 0;
            case 3:
                std::cout << "\n ===== PRESSED F3 =====\n";
                toggle_pressing(hwnd);
            default:
                break;
        }
    }

    return 0;
}
