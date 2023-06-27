#include <stdio.h>
#include <assert.h>
#include <X11/Xlib.h>
#include <X11/extensions/XInput2.h>

/* 
a portal has two points (rx,ry) and (lx,ly) and is oriented so that it can only be entered from one side
convoluting this with vector math saves 1 multiply operation per portal check or something ridiculous idk I wrote it in 2017
*/
struct portal {
	//line info
	int diffx;//rx-lx
	int diffy;//ry-ly
	float diff2;//distance between points squared
	float diffm;//distance between points
	int lDot;//dot(diff,l)
	int rDot;//dot(diff,r)
	int dDot;//dot(diff,(r+l)/2)
	//last state information (would not be needed if mouse intent was available)
	int pDot;//cross(vec3(cursor,0),vec3(diff,0)).z also= dot(cursor,vec2(diffy,-diffx))
	int cDot;//dot(cursor,diff)
	bool mouseState = true;//false=behind true=infront
	bool mouseAlignment = false;//is the mouse between the left and right points
	//which portal will the mouse teleport to when this portal is crossed
	int teleportTarget;
};

int main(int argc, char const *argv[])
{
	Display *display = XOpenDisplay(0);
	assert(display);

	Window *root_window = XRootWindow(display, 0);
	assert(root_window);

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
		// int pDot = x*portalList[i].diffy - y*portalList[i].diffx;
		// int cDot = x*portalList[i].diffx + y*portalList[i].diffy;
		// bool mouseState = pDot >= portalList[i].dDot;//dot(Cursor,turn(diff))>=dDot
		// bool mouseAlignment = portalList[i].lDot <= cDot&&cDot <= portalList[i].rDot;//lDot<dot(Cursor,diff)<rDot
		// bool condition = mouseState && !portalList[i].mouseState&&portalList[i].mouseAlignment;
		// if(condition){
			// int j = portalList[i].teleportTarget;
			// float tcDot = portalList[j].rDot + float(portalList[j].lDot - portalList[j].rDot)*float(portalList[i].cDot - portalList[i].lDot) / float(portalList[i].rDot - portalList[i].lDot);
			// float tpDot = portalList[j].dDot - max(0.0f, portalList[j].diffm*float(portalList[i].pDot - portalList[i].dDot)) / portalList[i].diffm;
			// int dest_x = int((tcDot*portalList[j].diffx + tpDot*portalList[j].diffy) / portalList[j].diff2) - sgn(portalList[j].diffy);
			// int dest_y = int((tcDot*portalList[j].diffy - tpDot*portalList[j].diffx) / portalList[j].diff2) + sgn(portalList[j].diffx);
		// 	// Display *display;
		// 	// Window src_w, dest_w;
		// 	// int src_x, src_y;
		// 	// unsigned int src_width, src_height;
		// 	XWarpPointer(display, e.window, e.window, e.x, e.y, src_width, src_height, dest_x, dest_y)
		// }
	}
	XCloseDisplay(display);
	return 0;
}