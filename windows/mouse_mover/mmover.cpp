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

// Debug log macro
#ifdef _DEBUG
	#define DLOG(x) x;
#else // Release - noop
	#define DLOG(x) ;
#endif // _DEBUG

constexpr std::chrono::milliseconds DELAY_BETWEEN_MOVES = 100ms;
constexpr std::chrono::milliseconds DELAY_BETWEEN_CHECKS = 500ms;
#ifdef _DEBUG
const unsigned CYCLES_TO_WAIT = 10;
#else
const unsigned CYCLES_TO_WAIT = 120;
#endif

void reg_hotkey_wrapper(UINT vk, UINT mod = MOD_NOREPEAT) {
    if (RegisterHotKey(NULL, 1, mod, vk))
    {
        DLOG(std::cout << "Hotkey '0x" << std::hex << vk << "' registered, using MOD_NOREPEAT flag\n";)
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
std::atomic<POINT> prev_cursor_pos{POINT{0,0}};
std::atomic_uint counter = 0;

void move_mouse() {
	POINT curr_cursor_pos{0,0};
	GetCursorPos(&curr_cursor_pos); // init current cursor position
	prev_cursor_pos.store(curr_cursor_pos);// init previous cursor position

	while (!exit_condition.load()) {
		std::this_thread::sleep_for(DELAY_BETWEEN_CHECKS);
		GetCursorPos(&curr_cursor_pos); // get current cursor position
		POINT pos = prev_cursor_pos.load(); // get previous cursor position
		bool pos_same = pos.x == curr_cursor_pos.x && pos.y == curr_cursor_pos.y; // compare current and previous positions
		if (pos_same && !random_moving.load()) { // if the mouse hasn't moved by the user - start the countdown
			++counter;
			DLOG(std::cout << "tick " << counter.load() << '\n';)
			if (counter.load() >= CYCLES_TO_WAIT) {
				// if the mouse hasn't moved for a long time - start movinig
				counter = 0;
				random_moving.store(true);
			}
		}
		else if (!pos_same) { // if the mouse moved by the user - stop moving
			DLOG(if (random_moving.load() || counter.load() > 0) std::cout << '\n';)
			random_moving.store(false);
			counter = 0;
		}

		if (random_moving.load()) { // move the mouse
			// 1. SetCursorPos - doesn't work mouse is not really moving
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
			int dx = ud(gen);
			int dy = ud(gen);
			mouse_event(MOUSEEVENTF_MOVE, dx, dy, 0, 0);
			GetCursorPos(&curr_cursor_pos);
			std::this_thread::sleep_for(DELAY_BETWEEN_MOVES);
		}
		prev_cursor_pos.store(curr_cursor_pos);
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

void toggleMoving(bool &movingEnabled)
{
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
	counter = 0;
	movingEnabled = random_moving.load();
	while (!random_moving.compare_exchange_weak(movingEnabled, !movingEnabled)) {}
}

void rightClick()
{
	POINT cursorPos{};
	GetCursorPos(&cursorPos);
	mouse_event(MOUSEEVENTF_RIGHTDOWN, cursorPos.x, cursorPos.y, 0, 0);
	mouse_event(MOUSEEVENTF_RIGHTUP, cursorPos.x, cursorPos.y, 0, 0);
}

int main()
{
    reg_hotkey_wrapper(VK_F10);
    reg_hotkey_wrapper(VK_F1);
    reg_hotkey_wrapper(VK_F2);

    std::thread mover(move_mouse);
    //std::thread clicker(left_click);

    std::cout <<
              "\n\n"
              "===== ===== =====\n"
              "F1  - stop/start moving\n"
              "F2  - right click\n"
              "F10 - EXIT\n"
              "===== ===== =====\n";

    MSG msg = { 0 };
    bool movingEnabled = false;
    while (GetMessage(&msg, NULL, 0, 0) != 0)
    {
        //std::cout << "\nMessage:" << msg.message;
        if (msg.message == WM_HOTKEY)
        {
			DLOG(
            std::cout << "-> WM_HOTKEY 0x" << std::hex << msg.message << " received\n"
                      << "    lParam = " << std::hex << htonl(msg.lParam)
                      <<   "; wParam = " << std::hex << htonl(msg.wParam)
                      << '\n';
			)

            switch (htonl(msg.lParam) >> 8)
            {
                case VK_F10:
				{
                    std::cout << "Exiting\n";
                    exit_condition = true;
                    return 0;
				}
                case VK_F1:
				{
					toggleMoving(movingEnabled);
					break;
				}
				case VK_F2:
				{
					rightClick();
					break;
				}
                default:
                    break;
            }
        }
    }

	exit_condition = true;
	mover.join();
	//clicker.join();
    return 0;
}
