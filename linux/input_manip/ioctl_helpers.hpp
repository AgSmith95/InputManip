#pragma once

#include "helper_macros.h"

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
// get device name
std::string get_dev_name(int fd) {
	char name[256] = "Unknown Device";
	ioctl(fd, EVIOCGNAME(sizeof(name)), name); // fills the buffer with the device name
	return name;
}
