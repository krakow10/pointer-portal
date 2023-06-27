#include <stdio.h>
#include <assert.h>
#include <X11/Xlib.h>
#include <X11/extensions/XInput2.h>


int main(int argc, char const *argv[])
{
	Display *display = XOpenDisplay(0);
	assert(display);

	Window *root_window = XRootWindow(display, 0);
	assert(root_window);
			// typedef struct {
			// 	int type;		/* MotionNotify */
			// 	unsigned long serial;	/* # of last request processed by server */
			// 	Bool send_event;	/* true if this came from a SendEvent request */
			// 	Display *display;	/* Display the event was read from */
			// 	Window window;		/* ``event'' window reported relative to */
			// 	Window root;		/* root window that the event occurred on */
			// 	Window subwindow;	/* child window */
			// 	Time time;		/* milliseconds */
			// 	int x, y;		/* pointer x, y coordinates in event window */
			// 	int x_root, y_root;	/* coordinates relative to root */
			// 	unsigned int state;	/* key or button mask */
			// 	char is_hint;		/* detail */
			// 	Bool same_screen;	/* same screen flag */
			// } XMotionEvent;
			// if(POINTER_DELTA_CROSSES_PORTAL_BOUNDARY){
			// 	//math
			// 	// Display *display;
			// 	// Window src_w, dest_w;
			// 	// int src_x, src_y;
			// 	// unsigned int src_width, src_height;
			// 	// int dest_x, dest_y;
			// 	XWarpPointer(display, e.window, e.window, e.x, e.y, src_width, src_height, dest_x, dest_y)
			// }

	//https://askubuntu.com/questions/1348943/c-programming-detect-mouse-pointer-movement
	/* check XInput */
	int xi_opcode, event, error;
	if (!XQueryExtension(display, "XInputExtension", &xi_opcode, &event, &error)) {
		fprintf(stderr, "Error: XInput extension is not supported!\n");
		return 1;
	}

	/* Check XInput 2.0 */
	int major = 2;
	int minor = 0;
	int retval = XIQueryVersion(display, &major, &minor);
	if (retval != Success) {
		fprintf(stderr, "Error: XInput 2.0 is not supported (ancient X11?)\n");
		return 1;
	}
	/*
	 * Set mask to receive XI_RawMotion events. Because it's raw,
	 * XWarpPointer() events are not included, you can use XI_Motion
	 * instead.
	 */
	unsigned char mask_bytes[(XI_LASTEVENT + 7) / 8] = {0};  /* must be zeroed! */
	XISetMask(mask_bytes, XI_RawMotion);

	/* Set mask to receive events from all master devices */
	XIEventMask evmasks[1];
	/* You can use XIAllDevices for XWarpPointer() */
	evmasks[0].deviceid = XIAllMasterDevices;
	evmasks[0].mask_len = sizeof(mask_bytes);
	evmasks[0].mask = mask_bytes;
	XISelectEvents(display, root_window, evmasks, 1);

	XEvent xevent;
	while (1) {
		XNextEvent(display, &xevent);

		if (xevent.xcookie.type != GenericEvent || xevent.xcookie.extension != xi_opcode) {
			/* not an XInput event */
			continue;
		}
		XGetEventData(display, &xevent.xcookie);
		if (xevent.xcookie.evtype != XI_RawMotion) {
			/*
			 * Not an XI_RawMotion event (you may want to detect
			 * XI_Motion as well, see comments above).
			 */
			XFreeEventData(display, &xevent.xcookie);
			continue;
		}
		XFreeEventData(display, &xevent.xcookie);

		Window root_return, child_return;
		int root_x_return, root_y_return;
		int win_x_return, win_y_return;
		unsigned int mask_return;
		/*
		 * We need:
		 *     child_return - the active window under the cursor
		 *     win_{x,y}_return - pointer coordinate with respect to root window
		 */
		int retval = XQueryPointer(display, root_window, &root_return, &child_return,
								   &root_x_return, &root_y_return,
								   &win_x_return, &win_y_return,
								   &mask_return);
		if (!retval) {
			/* pointer is not in the same screen, ignore */
			continue;
		}
		printf("%d,%d", win_x_return, win_y_return);
	}
	XCloseDisplay(display);
	return 0;
}