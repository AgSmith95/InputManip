#include "helpers.h"

#include <iostream>
#include <atomic>
#include <cstring>
#include <thread>
#include <vector>
#include <mutex>
#include <chrono>

void initiate_pressing();

void reg_hotkey_wrapper(UINT vk, int id, UINT mod) {
    if (RegisterHotKey(NULL, id, mod, vk))
    {
#ifdef _DEBUG
        std::cout << "Hotkey '0x" << std::hex << vk << "' registered, using MOD_NOREPEAT flag\n";
#endif // _DEBUG
    }
    else
    {
        std::cout << "Error code " << GetLastError();
        exit(1);
    }
}

BOOL CALLBACK EnumWindowsProc(
        _In_ HWND hwnd,
        _In_ LPARAM lParam
) {
    if (lParam == 0) {
        return 0;
    }

    int length = GetWindowTextLength( hwnd );
    if( 0 == length ) return TRUE;

    TCHAR* buffer;
    buffer = new TCHAR[ length + 1 ];
    memset( buffer, 0, ( length + 1 ) * sizeof( TCHAR ) );

    GetWindowText( hwnd, buffer, length + 1 );
    DWORD pid = GetWindowThreadProcessId(hwnd, NULL);
    if (lParam == pid) {
        std::cout << hwnd << ": " << buffer << '\n';
    }

    delete[] buffer;
}

// key pressing stage
std::atomic<bool> run(false);
std::mutex keys_mutex;
std::vector<INPUT> keys;

void prepare_keys(const std::vector<unsigned>& keycodes) {
    std::lock_guard guard(keys_mutex);
    keys.clear();
    keys.resize(keycodes.size() * 2);
    memset(&keys[0], 0, sizeof(INPUT) * keycodes.size());
    for (size_t i = 0; i < keycodes.size(); ++i) {
        keys[i*2].type = INPUT_KEYBOARD;
        keys[i*2].ki.wVk = keycodes[i];
        keys[i*2 + 1].type = INPUT_KEYBOARD;
        keys[i*2 + 1].ki.wVk = keycodes[i];
        keys[i*2 + 1].ki.dwFlags = KEYEVENTF_KEYUP;
    }
#ifdef _DEBUG
    for (const auto& e: keys) {
        std::cout << "INPUT: " << e.type << ", 0x" << std::hex << e.ki.wVk << ", " << e.ki.dwFlags << '\n';
    }
#endif
}

void run_thread() {
    if (run) {
        std::thread presser(initiate_pressing);
        presser.detach();
    }
}

void toggle_pressing() {
    run = !run;
    run_thread();
}

void stop_pressing() {
    run = false;
}

void initiate_pressing() {
    std::lock_guard guard(keys_mutex);
    while(run) {
        for (auto& key: keys) {
            SendInput(1, &key, sizeof(INPUT));
            if (key.ki.dwFlags == 0x0) {
                std::this_thread::sleep_for(std::chrono::milliseconds(1));
            }
        }
    }
}