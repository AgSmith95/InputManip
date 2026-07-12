#include <fcntl.h>
#include <linux/input.h>
#include <linux/uinput.h>
#include <unistd.h>

#include <atomic>
#include <chrono>
#include <condition_variable>
#include <cstring>
#include <filesystem>
#include <iostream>
#include <mutex>
#include <thread>
#include <vector>

// macros to test bits within an array
#define BITS_PER_LONG (sizeof(unsigned long) * 8)
#define NBITS(x) ((((x)-1)/BITS_PER_LONG)+1)
#define OFF(x)  ((x)%BITS_PER_LONG)
#define BIT(x)  ((x)/BITS_PER_LONG)
#define LONG(x) ((x)/BITS_PER_LONG)
#define TEST_BIT(bit, array) ((array[LONG(bit)] >> OFF(bit)) & 1)

// Debug log macro
#ifdef _DEBUG
	#define DLOG(x) x;
#else // Release - noop
	#define DLOG(x) ;
#endif // _DEBUG

namespace fs = std::filesystem;

// Synchronization primitives
std::atomic<bool> is_clicking(false);
std::mutex cv_m;
std::condition_variable cv;


// Helper function to emit input events to the kernel
void emit(int fd, int type, int code, int val) {
	struct input_event ie;
	memset(&ie, 0, sizeof(ie));
	ie.type = type;
	ie.code = code;
	ie.value = val;
	// Write the event to the virtual device file descriptor
	if (write(fd, &ie, sizeof(ie)) < 0) {
		std::cerr << "Failed to write event." << std::endl;
	}
}

// listens for a keypress
void keyboard_listener(int fd, std::string device_path) {
	struct input_event ie;
	while (true) {
		// This is a blocking read. The OS puts this thread to sleep
		// until the kernel delivers a new input event. 0% CPU usage.
		if (read(fd, &ie, sizeof(ie)) > 0) {
			if (ie.type == EV_KEY && ie.code == KEY_F2 && ie.value == 1) {
				is_clicking = !is_clicking;
				DLOG(std::cout << "\nF2 is pressed on '" << device_path << '\'';)
				if (is_clicking) {
					std::cout << "\n[+] Clicking STARTED" << std::endl;
					// "Interrupt" the main thread to wake it up
					cv.notify_one();
				} else {
					std::cout << "\n[-] Clicking STOPPED" << std::endl;
				}
			}
		}
	}
}

// checks if a device is a keyboard that supports F2 key
bool is_keyboard_with_F2(int fd) {
	// unsigned long evbit[NBITS(EV_MAX)];
	// memset(evbit, 0, sizeof(evbit));
	// // Get supported event types (EV_SYN, EV_KEY, EV_REL, etc.)
	// if (ioctl(fd, EVIOCGBIT(0, sizeof(evbit)), evbit) < 0) {
	// 	DLOG(std::cerr << "ERROR: ioctl EVIOCGBIT(0)\n";)
	// 	return false;
	// }
	// // Check if EV_KEY type is supported, if not - not a keyboard
	// if (!TEST_BIT(EV_KEY, evbit)) {
	// 	return false;
	// }
	// Check if F2 button is on the keyboard
	unsigned long keybit[NBITS(KEY_MAX)];
	memset(keybit, 0, sizeof(keybit));
	if (ioctl(fd, EVIOCGBIT(EV_KEY, sizeof(keybit)), keybit) < 0) {
		DLOG(std::cerr << "ERROR: ioctl EVIOCGBIT(EV_KEY)\n";)
		return false;
	}
	return TEST_BIT(KEY_F2, keybit);
}

// Toggle clicking by pressing F2
// NOTE: this has to be called with sudo, because of ioctl and /dev/uinput
int main(int argc, char** argv) {
	// Spawn listener threads
	std::vector<std::thread> listener_threads;
	for (const auto& entry : fs::directory_iterator("/dev/input")) {
		std::string path = entry.path().string();
		if (path.find("event") != std::string::npos) {// only check /dev/input/*event*
			int ev_fd = open(path.c_str(), O_RDONLY); // O_RDONLY makes read() blocking
			bool with_ev_key = is_keyboard_with_F2(ev_fd);
			DLOG(std::cout << '\'' << path << "' supports EV_KEY? - " << with_ev_key << std::endl)
			if (ev_fd >= 0) {
				if (with_ev_key) {
					listener_threads.emplace_back(keyboard_listener, ev_fd, path);
				} else {
					close(ev_fd);
				}
			}
		}
	}
	if (listener_threads.size() == 0) {
		std::cout << "No suitable keyboards detected!\n";
		return 1;
	}
	std::cout << "Listening on " << listener_threads.size() << " keyboards\n";

	// SET UP VIRTUAL MOUSE | START
	// Open the uinput device
	int fd = open("/dev/uinput", O_WRONLY | O_NONBLOCK);
	if (fd < 0) {
		std::cerr
			<< "Error: Could not open /dev/uinput. Are you running as root?"
			<< std::endl;
		return 1;
	}

	// Configure the virtual device to support keys/buttons, specifically the
	// Left Mouse Button
	ioctl(fd, UI_SET_EVBIT, EV_KEY);
	ioctl(fd, UI_SET_KEYBIT, BTN_LEFT);

	// Setup the device details
	struct uinput_setup usetup;
	memset(&usetup, 0, sizeof(usetup));
	usetup.id.bustype = BUS_USB;
	usetup.id.vendor  = 0x1234;  // Arbitrary vendor ID
	usetup.id.product = 0x5678;  // Arbitrary product ID
	strcpy(usetup.name, "Generic C++ AutoClicker");

	ioctl(fd, UI_DEV_SETUP, &usetup);

	// Create the device
	if (ioctl(fd, UI_DEV_CREATE) < 0) {
		std::cerr << "Error: Could not create uinput device." << std::endl;
		close(fd);
		return 1;
	}

	// Give the desktop environment a second to recognize the new "hardware"
	// TODO: figure out how to detect this recognition instead of simply sleeping
//	std::cout << "Virtual device created. Waiting 2 seconds..." << std::endl;
//	usleep(2'000'000);
	// SET UP VIRTUAL MOUSE | FINISH

	std::cout << "Setup complete. Press F2 to toggle clicking, Ctrl+C to exit" << std::endl;

	// MAIN CLICKING LOOP
	while (true) {
		{
			std::unique_lock<std::mutex> lk(cv_m);
			// If is_clicking is false, the thread suspends here indefinitely.
			// When cv.notify_one() is called, it wakes up, checks is_clicking,
			// and proceeds.
			cv.wait(lk, [] { return is_clicking.load(); });
		} // The mutex is unlocked here so we don't hold it during the click delay

		// Reaching this point means is_clicking is true
		emit(fd, EV_KEY, BTN_LEFT, 1);
		emit(fd, EV_SYN, SYN_REPORT, 0); // Sync event tells the OS the event is complete
		// Release the left button
		emit(fd, EV_KEY, BTN_LEFT, 0);
		emit(fd, EV_SYN, SYN_REPORT, 0); // Sync event tells the OS the event is complete

		usleep(25'000); // sleep short amount of time; TODO: loop doesn't work without sleep; figure out why?
		// TODO: lowering from 0.1 to 0.01s makes the whole thing stop working; 0.025s seems fine on my device
		// TODO: figure out how to check when the event actually finishes instead of rapid-firing as fast as possible
	}

	// Clean up
	ioctl(fd, UI_DEV_DESTROY);
	close(fd);

	return 0;
}
