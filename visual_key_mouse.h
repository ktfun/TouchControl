#ifndef _VISUAL_KEY_MOUSE_H_
#define _VISUAL_KEY_MOUSE_H_

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <linux/input.h>
#include <linux/uinput.h>
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/mman.h>

int setup_uinput_device();

void send_press_event(int key);

void send_release_event(int key);

void send_move_event_abs(int x, int y);

void send_mouse_press_event();

void send_mouse_release_event();

#endif