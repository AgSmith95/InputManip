#include <fcntl.h>
#include <linux/uinput.h>
#include <unistd.h>

#include <cstring>
#include <iostream>

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

// just click 20 times for now to test basic functions
int main(int argc, char** argv) {
	// 1. Open the uinput device
	int fd = open("/dev/uinput", O_WRONLY | O_NONBLOCK);
	if (fd < 0) {
		std::cerr
			<< "Error: Could not open /dev/uinput. Are you running as root?"
			<< std::endl;
		return 1;
	}

	// 2. Configure the virtual device to support keys/buttons, specifically the
	// Left Mouse Button
	ioctl(fd, UI_SET_EVBIT, EV_KEY);
	ioctl(fd, UI_SET_KEYBIT, BTN_LEFT);

	// 3. Setup the device details
	struct uinput_setup usetup;
	memset(&usetup, 0, sizeof(usetup));
	usetup.id.bustype = BUS_USB;
	usetup.id.vendor  = 0x1234;  // Arbitrary vendor ID
	usetup.id.product = 0x5678;  // Arbitrary product ID
	strcpy(usetup.name, "Generic C++ AutoClicker");

	ioctl(fd, UI_DEV_SETUP, &usetup);

	// 4. Create the device
	if (ioctl(fd, UI_DEV_CREATE) < 0) {
		std::cerr << "Error: Could not create uinput device." << std::endl;
		close(fd);
		return 1;
	}

	// Give the desktop environment a second to recognize the new "hardware"
	// TODO: figure out how to detect this recognition instead of simply sleeping
	std::cout << "Virtual device created. Waiting 2 seconds..." << std::endl;
	usleep(2'000'000);

	// 5. Simulate the click
	std::cout << "Clicking!" << std::endl;
	for (int i = 1; i < 21; ++i) {
		// Press the left button down
		emit(fd, EV_KEY, BTN_LEFT, 1);
		emit(fd, EV_SYN, SYN_REPORT, 0); // Sync event tells the OS the event is complete
		// Release the left button
		emit(fd, EV_KEY, BTN_LEFT, 0);
		emit(fd, EV_SYN, SYN_REPORT, 0); // Sync event tells the OS the event is complete
		std::cout << i << ' ' << std::flush; // doesn't print one by one without flush. understandable.
		usleep(100'000); // sleep 0.1s; TODO: loop doesn't work without sleep; figur eout why?
	}
	std::cout << '\n';

	// 6. Clean up
	ioctl(fd, UI_DEV_DESTROY);
	close(fd);

	return 0;
}
