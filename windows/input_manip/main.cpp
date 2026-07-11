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

    MSG msg;
    memset(&msg, 0, sizeof(MSG));

    prepare_keys({0x49, 0x4A, 0x4B, 0x4C});
    while (GetMessage(&msg, NULL, WM_HOTKEY, WM_HOTKEY) != 0) { // or PeekMessage + ", PM_NOREMOVE | PM_NOYIELD"
        switch (msg.wParam) {
            case 1:
                toggle_pressing();
                break;
            case 2:
                stop_pressing();
                std::this_thread::sleep_for(std::chrono::milliseconds(50));
                return 0;
            default:
                break;
        }
    }

    return 0;
}
