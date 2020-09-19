#include "helpers.h"

#include <iostream>
#include <atomic>
#include <cstring>
#include <thread>
#include <vector>
#include <mutex>
#include <chrono>

void initiate_pressing(HWND hwnd = nullptr);
void run_thread(HWND hwnd = nullptr);

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

BOOL CALLBACK get_pid_procinfo(_In_ HWND hwnd, _In_ LPARAM lParam) {
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

BOOL CALLBACK enum_all_procs(_In_ HWND hwnd, _In_ LPARAM lParam) {
    if (!IsWindowVisible(hwnd)) {
        return TRUE;
    }

    int length = GetWindowTextLength( hwnd );
    if( 0 == length ) return TRUE;

    TCHAR* buffer;
    buffer = new TCHAR[ length + 1 ];
    memset( buffer, 0, ( length + 1 ) * sizeof( TCHAR ) );

    GetWindowText( hwnd, buffer, length + 1 );
    WORD processId = 0;
    DWORD pid = GetWindowThreadProcessId(hwnd, (LPDWORD)(&processId));
    std::cout << std::dec << (long long)hwnd << " - hwnd, pid = " << pid << ", process = " << processId << ": \"";
    std::cout << buffer << "\"\n";

    delete[] buffer;
}

void printHwndText(HWND hwnd) {
    int length = GetWindowTextLength( hwnd );
    if( 0 != length ) {
        TCHAR *buffer;
        buffer = new TCHAR[ length + 1 ];
        memset( buffer, 0, ( length + 1 ) * sizeof( TCHAR ) );

        GetWindowText( hwnd, buffer, length + 1 );
        std::cout << buffer << '\n';
    }
}

// key pressing stage
std::atomic<bool> run(false);
std::mutex keys_mutex;
std::vector<INPUT> keys;
std::vector<unsigned> codes;

void prepare_keys(const std::vector<unsigned>& keycodes) {
    codes = keycodes;
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
    std::cout << "[ ";
    for (const auto& e: codes) {
        std::cout << e << " ";
    }
    std::cout << "]\n";
    for (const auto& e: keys) {
        std::cout << "INPUT: " << e.type << ", 0x" << std::hex << e.ki.wVk << ", " << e.ki.dwFlags << '\n';
    }
#endif
}

void run_thread(HWND hwnd) {
    if (run) {
        std::thread presser(initiate_pressing, hwnd);
        presser.detach();
#ifdef _DEBUG
        std::cout << "run_thread ==> run is: " << true << "\n";
#endif
    }
#ifdef _DEBUG
    else {
        std::cout << "run_thread ==> run is: " << false << "\n";
    }
#endif
}

void toggle_pressing(HWND hwnd) {
#ifdef _DEBUG
    std::cout << "toggle_pressing ==> hwnd is: " << std::dec << (long long)hwnd << "\n";
#endif
    run = !run;
    run_thread(hwnd);
}

void start_pressing(HWND hwnd) {
    run = true;
    run_thread(hwnd);
}

void stop_pressing() {
    run = false;
}

void initiate_pressing(HWND hwnd) {
    std::lock_guard guard(keys_mutex);
    while(run) {
//        for (auto& key: keys) {
//            SendInput(1, &key, sizeof(INPUT));
//            if (key.ki.dwFlags == 0x0) {
//                std::this_thread::sleep_for(std::chrono::milliseconds(1));
//            }
//        }
        for (const auto& wParam: codes) {
            SendMessage(hwnd, WM_KEYDOWN, wParam, 0);
            std::this_thread::sleep_for(std::chrono::milliseconds(2));
            SendMessage(hwnd, WM_KEYUP, wParam, 0b00000000000000000000000000000011);
            std::this_thread::sleep_for(std::chrono::milliseconds(2));
        }
    }
}