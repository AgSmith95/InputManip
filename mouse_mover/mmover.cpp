#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
extern "C" {
#include <winsock.h>
}
#pragma comment(lib, "Ws2_32.lib")

#include <windows.h>

#include <thread>
#include <atomic>

#include <iostream>
#include <random>
#include <chrono>

using namespace std::chrono_literals;

const unsigned CYCLES_TO_WAIT = 480;

void reg_hotkey_wrapper(UINT vk, UINT mod = MOD_NOREPEAT) {
    if (RegisterHotKey(NULL, 1, mod, vk))
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

std::atomic<bool> exit_condition = false;
std::atomic<bool> random_moving = false;
//std::atomic<bool> clicking_flag = false;

std::random_device rd;
std::mt19937 gen(rd());
std::uniform_int_distribution<> ud(-20, 20);
void move_mouse() {
    unsigned counter = 0;
    POINT curr_pos{0,0};
    POINT pos{0,0};
    GetCursorPos(&curr_pos);
    while (!exit_condition) {
        if (random_moving.load()) {
            // 1. SetCursorPos - doesn't work mouse is not really moving
            //GetCursorPos(&cursorPos);
            //SetCursorPos(cursorPos.x + ud(gen), cursorPos.y + ud(gen));

            // 2. SendInput - screen flickers
            //    -- flickering is because you have garbage in struct sometimes
            //    -- zero it out using memset or something
            //INPUT input;
            //input.type = INPUT_MOUSE;
            //input.mi.mouseData = 0;
            //input.mi.dwFlags = MOUSEEVENTF_MOVE;
            //input.mi.dx = ud(gen);
            //input.mi.dy = ud(gen);
            //input.mi.dwExtraInfo = NULL;
            //SendInput(1, &input, sizeof(input));

            // 3. mouse_event
            mouse_event(MOUSEEVENTF_MOVE, ud(gen), ud(gen), 0, 0);
            std::this_thread::sleep_for(20ms);

        }
        else {
            std::this_thread::sleep_for(500ms);
            GetCursorPos(&pos);
            if (pos.x == curr_pos.x && pos.y == curr_pos.y) {
                ++counter;
#ifdef _DEBUG
                std::cout << "tick " << counter << '\n';
#endif
                if (counter >= CYCLES_TO_WAIT) {
                    counter = 0;
                    random_moving.store(true);
                }
            }
            else {
                counter = 0;
            }
            std::swap(curr_pos, pos);
        }
    }
}

//void left_click() {
//    POINT cursorPos;
//
//    while (!exit_condition) {
//        if (clicking_flag) {
//            GetCursorPos(&cursorPos);
//            mouse_event(MOUSEEVENTF_LEFTDOWN, cursorPos.x, cursorPos.y, 0, 0);
//            mouse_event(MOUSEEVENTF_LEFTUP, cursorPos.x, cursorPos.y, 0, 0);
//            std::this_thread::sleep_for(20ms);
//        }
//        else {
//            std::this_thread::sleep_for(200ms);
//        }
//    }
//}

int main()
{
    //reg_hotkey_wrapper(VK_ESCAPE);
    reg_hotkey_wrapper(VK_F1);
    //reg_hotkey_wrapper(VK_F2);

    std::thread mover(move_mouse);
    //std::thread clicker(left_click);

    std::cout <<
              "\n\n"
              "===== ===== =====\n"
              //"ESC - exit\n"
              "F1 - stop/start moving\n"
              //"F2 - stop/start clicking\n"
              "===== ===== =====\n";

    MSG msg = { 0 };
    bool old;
    while (GetMessage(&msg, NULL, 0, 0) != 0)
    {
        //std::cout << "\nMessage:" << msg.message;
        if (msg.message == WM_HOTKEY)
        {
#ifdef _DEBUG
            std::cout << "-> WM_HOTKEY 0x" << std::hex << msg.message << " received\n"
                      << "    lParam = " << std::hex << htonl(msg.lParam)
                      <<   "; wParam = " << std::hex << htonl(msg.wParam)
                      << '\n';
#endif // _DEBUG

            switch (htonl(msg.lParam) >> 8)
            {
                case VK_ESCAPE:
                    std::cout << "Exiting\n";
                    exit_condition = true;
                    mover.join();
                    //clicker.join();
                    return 0;
                case VK_F1:
#ifdef _DEBUG
                    std::cout << "MOVING ";
                    if (!random_moving) {
                        std::cout << "START";
                    }
                    else {
                        std::cout << "STOP";
                    }
                    std::cout << '\n';
#endif // _DEBUG
                    old = random_moving.load();
                    while (!random_moving.compare_exchange_weak(old, !old)) {}
                    break;
//            case VK_F2:
//#ifdef _DEBUG
//                std::cout << "CLICKING\n";
//#endif // _DEBUG
//                clicking_flag = clicking_flag ? false : true;
                default:
                    break;
            }
        }
    }


    return 0;
}
