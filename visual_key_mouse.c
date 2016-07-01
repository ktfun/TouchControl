#include "visual_key_mouse.h"

// qt -> showfullscreen
#define MAX_X    1280    // 
#define MAX_Y    720    // 

void send_move_event_abs_first();

/* Globals */
static int uinp_fd = -1;
struct uinput_user_dev uinp; // uInput device structure
struct input_event event; // Input device structure
/* Setup the uinput device */
int setup_uinput_device()
{
// Temporary variable
    int i=0;

// Open the input device
    uinp_fd = open("/dev/uinput", O_WRONLY | O_NONBLOCK);

    if (uinp_fd == -1)
    {
        //Dashboard January 2007 Issue
        printf("Unable to open /dev/uinput");
        return -1;
    }

    memset(&uinp,0,sizeof(uinp)); // Intialize the uInput device to NULL
    strncpy(uinp.name, "ETAH Virtual Keyboard", UINPUT_MAX_NAME_SIZE);
    uinp.id.version = 0x4;
    uinp.id.bustype = BUS_VIRTUAL;
// ETAH + 1 -> beyboard
    uinp.id.vendor = 0x4554;
    uinp.id.product = 0x414a;

    uinp.absmin[ABS_X] = 0;  
	uinp.absmax[ABS_X] = 2048;
	uinp.absmin[ABS_Y] = 0;  
	uinp.absmax[ABS_Y] = 1536;

// Setup the uinput device
    ioctl(uinp_fd, UI_SET_EVBIT, EV_SYN);
    ioctl(uinp_fd, UI_SET_EVBIT, EV_KEY);
	ioctl(uinp_fd, UI_SET_EVBIT, EV_MSC);
	ioctl(uinp_fd, UI_SET_EVBIT, EV_REP);
	ioctl(uinp_fd, UI_SET_MSCBIT, MSC_SCAN);

    // ¾ø¶Ô×ø±ê
	ioctl(uinp_fd, UI_SET_EVBIT, EV_ABS);
	ioctl(uinp_fd, UI_SET_ABSBIT, ABS_X);
	ioctl(uinp_fd, UI_SET_ABSBIT, ABS_Y);
	ioctl(uinp_fd, UI_SET_RELBIT, ABS_WHEEL);

	// key - button
	ioctl(uinp_fd, UI_SET_KEYBIT, BTN_MOUSE);
	ioctl(uinp_fd, UI_SET_KEYBIT, BTN_LEFT);
	ioctl(uinp_fd, UI_SET_KEYBIT, BTN_RIGHT); 
	ioctl(uinp_fd, UI_SET_KEYBIT, BTN_MIDDLE); 
	ioctl(uinp_fd, UI_SET_KEYBIT, BTN_SIDE); 
	ioctl(uinp_fd, UI_SET_KEYBIT, BTN_EXTRA);

// key
    for(i = 0; i < 194; i++)
    {
		if (i != 84 &&
			i != KEY_LINEFEED &&
			i != KEY_MACRO &&
			i != KEY_SCALE &&
			i != KEY_KPEQUAL &&
			i != KEY_MENU &&
			i != KEY_SETUP &&
			i != KEY_WAKEUP &&
			i != KEY_FILE &&
			i != KEY_SENDFILE &&
			i != KEY_DELETEFILE &&
			i != KEY_XFER &&
			i != KEY_PROG1 &&
			i != KEY_PROG2 &&
			i != KEY_MSDOS &&
			i != KEY_DIRECTION &&
			i != KEY_CYCLEWINDOWS &&
			i != KEY_MAIL &&
			i != KEY_BOOKMARKS &&
			i != KEY_COMPUTER &&
			i != KEY_CLOSECD &&
			i != KEY_EJECTCLOSECD &&
			i != KEY_RECORD &&
			i != KEY_REWIND &&
			i != KEY_PHONE &&
			i != KEY_ISO &&
			i != KEY_CONFIG &&
			//
			i != KEY_HOMEPAGE &&
			i != KEY_EXIT &&
			i != KEY_MOVE &&
			//
			i != KEY_KPLEFTPAREN &&
			i != KEY_KPRIGHTPAREN &&
			i != KEY_NEW &&
			i != KEY_REDO
			

			) 

		{
			ioctl(uinp_fd, UI_SET_KEYBIT, i);
		}
    }

    write(uinp_fd, &uinp, sizeof(uinp));

    if (ioctl(uinp_fd, UI_DEV_CREATE))
    {
        printf("Unable to create UINPUT device.");
        return -1;
    }

    sleep(10);

    send_move_event_abs_first();

    return 1;
}

void close_uinput_device()
{
    ioctl(uinp_fd, UI_DEV_DESTROY);
    close(uinp_fd);
}

static void send_sync()
{
	gettimeofday(&event.time, NULL);
    event.type = EV_SYN;
    event.code = SYN_REPORT;
    event.value = 0;
    write(uinp_fd, &event, sizeof(event));
}

void send_move_event_abs_first()
{    
    /**************************************
        1. x, y -> 1, 1
    ***************************************/
    // x -> 1
	gettimeofday(&event.time, NULL); 
	event.type = EV_ABS; 
	event.code = ABS_X;
	event.value = 1;
	write(uinp_fd, &event, sizeof(event));

    // y -> 1
	event.type = EV_ABS; 
	event.code = ABS_Y; 
	event.value = 1; 
	write(uinp_fd, &event, sizeof(event)); 

    printf("### move -> x = %d, y = %d\n", 1, 1);
    // sync
    send_sync();

    
    /**************************************
        2. x, y -> max, max
    ***************************************/
    // x -> max
    gettimeofday(&event.time, NULL); 
	event.type = EV_ABS; 
	event.code = ABS_X;
	event.value = MAX_X;
	write(uinp_fd, &event, sizeof(event));

    // y -> max
	event.type = EV_ABS; 
	event.code = ABS_Y; 
	event.value = MAX_Y; 
	write(uinp_fd, &event, sizeof(event)); 

    printf("### move -> x = %d, y = %d\n", MAX_X, MAX_Y);

    // sync
	send_sync();

    /**************************************
        3. x, y -> 1, 1
    ***************************************/
    
    // x -> 1
    gettimeofday(&event.time, NULL); 
	event.type = EV_ABS; 
	event.code = ABS_X;
	event.value = 1;
	write(uinp_fd, &event, sizeof(event));

    // y -> 1
	event.type = EV_ABS; 
	event.code = ABS_Y; 
	event.value = 1;
	write(uinp_fd, &event, sizeof(event)); 

    // sync
	send_sync();
    printf("### move -> x = %d, y = %d\n", 1, 1);
	
}

static int key_mapto_scan(int key)
{
	switch(key)
	{
		case KEY_HOME : 
			return 458826;
		case KEY_SPACE : 
			return 458796;
		case KEY_ESC : 
			return 458793;
		case KEY_UP : 
			return 458834;
		case KEY_LEFT : 
			return 458832;
		case KEY_DOWN : 
			return 458833;
		case KEY_RIGHT : 
			return 458831;
		default :
			return 0;
	}
}

static int threshold_x = 12;
static int threshold_y = 12;


void send_move_event_abs(int x, int y) 
{
	int ret = 0;
    //static int is_first_time = 1;
    static int history_x = 0;
    static int history_y = 0;
    int abs_x = 0;
    int abs_y = 0;
    
	memset(&event, 0, sizeof(event)); 
	gettimeofday(&event.time, NULL); 
	event.type = EV_ABS; 
	event.code = ABS_X;

    printf("move -> hx = %d, hy = %d x = %d, y = %d\n", 
        history_x, history_y, x, y);

    abs_x = abs(x - history_x);
	if (x < 0)
	{
		x = 0;
        history_x = x;
	}
	else if (x >= MAX_X)
	{
		x = MAX_X;
        history_x = x;
	}
    else if (abs_x <= threshold_x)
    {
        x = history_x;
    }
    else
    {
        history_x = x;
    }

	event.value = x;
    printf("event.value.x -> %d\n", event.value);
	ret = write(uinp_fd, &event, sizeof(event));

	event.type = EV_ABS; 
	event.code = ABS_Y;
    abs_y = abs(y - history_y);
	if (y < 0)
	{
		y = 0;
        history_y = y;
	}
	else if (y >= MAX_Y)
	{
		y = MAX_Y;
        history_y = y;
	}
    else if (abs_y <= threshold_y)
    {
        y = history_y;
    }
    else
    {
        history_y = y;
    }

	event.value = y;
    printf("event.value.y -> %d\n", event.value);
	ret = write(uinp_fd, &event, sizeof(event)); 
	send_sync();
}

void send_mouse_press_event()
{
	int ret = 0;
	// Report BUTTON CLICK - PRESS event 
	gettimeofday(&event.time, NULL); 
	event.type = EV_KEY; 
	event.code = BTN_LEFT; 
	event.value = 1; 
	ret = write(uinp_fd, &event, sizeof(event)); 
	//printf("ret = %d func: %s line: %d\n", ret, __FUNCTION__, __LINE__);
	send_sync();
}

void send_mouse_release_event()
{
    int ret = 0;
    // Report BUTTON CLICK - RELEASE event 
	gettimeofday(&event.time, NULL); 
	event.type = EV_KEY; 
	event.code = BTN_LEFT; 
	event.value = 0; 
	ret = write(uinp_fd, &event, sizeof(event)); 
	//printf("ret = %d func: %s line: %d\n", ret, __FUNCTION__, __LINE__);
	send_sync();
}

void send_press_event(int key)
{
    int ret = 0;

    printf("key -> %d func: %s line: %d\n", key, __FUNCTION__, __LINE__);
// 1. scan
	gettimeofday(&event.time, NULL);
    event.type = EV_MSC;
    event.code = MSC_SCAN;
    event.value = key_mapto_scan(key);
    ret = write(uinp_fd, &event, sizeof(event));
	
// 2. press
    gettimeofday(&event.time, NULL);
    event.type = EV_KEY;
    event.code = key;
    event.value = 1;
    ret = write(uinp_fd, &event, sizeof(event));
    printf("ret = %d func: %s line: %d\n", ret, __FUNCTION__, __LINE__);
// 3. sync
    send_sync();
	
}

void send_release_event(int key)
{
	int ret = 0;
// 4. scan

    printf("key -> %d func: %s line: %d\n", key, __FUNCTION__, __LINE__);
	gettimeofday(&event.time, NULL);
    event.type = EV_MSC;
    event.code = MSC_SCAN;
    event.value = key_mapto_scan(key);
    ret = write(uinp_fd, &event, sizeof(event));

// 5. release
    gettimeofday(&event.time, NULL);
    event.type = EV_KEY;
    event.code = key;
    event.value = 0;
    ret = write(uinp_fd, &event, sizeof(event));
    printf("ret = %d func: %s line: %d key = %d\n", ret, __FUNCTION__, __LINE__, key);
// 6. sync
    send_sync();

}

