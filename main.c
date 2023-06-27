#include <stdio.h>
#include <assert.h>
#include <X11/Xlib.h>
#include <X11/extensions/XInput2.h>


int main(int argc, char const *argv[])
{
	Display *display = XOpenDisplay(0);
	assert(display);

    Window *root_window = XRootWindow(&display, 0);
	assert(root_window);
	
	for(;;) {
		XEvent e;
		XNextEvent(display, &e);
		if (e.type == MotionNotify)
			printf("%d,%d", e.x, e.y);
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
	}
	XCloseDisplay(display);
	return 0;
}