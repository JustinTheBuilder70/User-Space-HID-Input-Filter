#include <fcntl.h>
#include <unistd.h>
#include <iostream>
#include <linux/input.h>
#include <unistd.h>
#include <linux/uinput.h>
#include <chrono>
#include <string.h>


struct Mouse{
    int mouse;
    int vmouse; 
    struct input_event ev;
    struct uinput_user_dev uidev;
};

//open("/dev/uinput", O_WRONLY | O_NONBLOCK); 
//open("/dev/input/event2", O_RDONLY);
static inline void readev(Mouse &evn) {

    ssize_t h = read(evn.mouse, &evn.ev, sizeof(struct input_event));    
    //printf("%d %d\n", evn.ev .value, REL_Y);
}


void initalize(Mouse &evn) {
   // struct Mouse evn;
    //evn.ev = (struct input_event*)malloc(sizeof(struct input_event));
    evn.vmouse = open("/dev/uinput", O_WRONLY | O_NONBLOCK); 
    evn.mouse = open("/dev/input/event17", O_RDONLY);
    
    if (evn.mouse <= 0 || evn.vmouse <= 0) {
        printf("Mouse Init Failed");
    } 

    ioctl(evn.vmouse, UI_SET_EVBIT, EV_KEY);
    ioctl(evn.vmouse, UI_SET_KEYBIT, BTN_LEFT);
    ioctl(evn.vmouse, UI_SET_EVBIT, EV_REL);
    ioctl(evn.vmouse, UI_SET_RELBIT, REL_X);
    ioctl(evn.vmouse, UI_SET_RELBIT, REL_Y);

    memset(&evn.uidev, 0, sizeof(evn.uidev));
    snprintf(evn.uidev.name, UINPUT_MAX_NAME_SIZE, "virtual-mouse");
    evn.uidev.id.bustype = BUS_USB;
    evn.uidev.id.vendor  = 0x1235;
    evn.uidev.id.product = 0x5678;
    evn.uidev.id.version = 1;
    write(evn.vmouse, &evn.uidev, sizeof(evn.uidev));
    ioctl(evn.vmouse, UI_DEV_CREATE);
    

}
    

void check_click(Mouse &evn) {

    if (evn.ev.code == 273) {          
        //memset(evptr, 0, sizeof(struct input_event));
        evn.ev.type = EV_KEY;
        evn.ev.code = BTN_LEFT;
        evn.ev.value = 1; // Press
        write(evn.vmouse, &evn.ev, sizeof(input_event));

        //memset(evptr, 0, sizeof(struct input_event));
        evn.ev.type = EV_SYN;
        evn.ev.code = SYN_REPORT;
        evn.ev.value = 0;
        write(evn.vmouse, &evn.ev, sizeof(input_event));

        usleep(1); 
        
        //memset(evptr, 0, sizeof(struct input_event));
        evn.ev.type = EV_KEY;
        evn.ev.code = BTN_LEFT;
        evn.ev.value = 0; // release
        write(evn.vmouse, &evn.ev, sizeof(input_event));


        //memset(evptr, 0, sizeof(struct input_event));
        evn.ev.type = EV_SYN;
        evn.ev.code = SYN_REPORT;
        evn.ev.value = 0;
        write(evn.vmouse, &evn.ev, sizeof(input_event));
        memset(&evn.ev, 0, sizeof(struct input_event));
        
    }
}


static inline void destroy(Mouse &evn) {
    ioctl(evn.vmouse, UI_DEV_DESTROY);
    close(evn.mouse);
    close(evn.vmouse);
    
    
}


static inline void acceleration(Mouse &evn, const float ph = 2.00) {
    int type_rel = evn.ev.type == EV_REL;
    evn.ev.value = evn.ev.value * (type_rel * ph + !type_rel);
    write(evn.vmouse, &evn.ev, sizeof(input_event));
    memset(&evn.ev, 0, sizeof(struct input_event));
    
}




int main() {
    Mouse mouse;
    initalize(mouse);

    while (true) {
        readev(mouse);
        acceleration(mouse);
    }

    destroy(mouse);

}