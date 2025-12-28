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
int readev(Mouse &evn) {

    ssize_t h = read(evn.mouse, &evn.ev, sizeof(struct input_event));    
    
    if (h == -1) {
        return -4;
    }

    return 0;
    //printf("%d %d\n", evn.ev .value, REL_Y);
}


void initalize(Mouse &evn) {
   // struct Mouse evn;
    //evn.ev = (struct input_event*)malloc(sizeof(struct input_event));
    evn.vmouse = open("/dev/uinput", O_WRONLY | O_NONBLOCK); 
    evn.mouse = open("/dev/input/event2", O_RDONLY);
    
    if (evn.mouse < 0) {
        printf("Mouse Init Error\n");
        return;
    } 

    if (evn.vmouse < 0) {
        printf("Virtual Mouse Init Error\n");
        close(evn.mouse);
        return;
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
    
    printf("Setup Complete...\n");
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


void destroy(Mouse &evn) {
    ioctl(evn.vmouse, UI_DEV_DESTROY);
    close(evn.mouse);
    close(evn.vmouse);
    
    
}

void acceleration(Mouse &evn) {
    if (evn.ev.type == EV_REL && (evn.ev.code == REL_X || evn.ev.code == REL_Y)) {
        //Vvalues less than 1.5 are best.
        evn.ev.value = (int)(2.3 * evn.ev.value);
        write(evn.vmouse, &evn.ev, sizeof(input_event));
        memset(&evn.ev, 0, sizeof(struct input_event));
    }
            
    
}




int main() {
    Mouse mouse;
    initalize(mouse);

    int code;
    code = 0;
    while (true) {
        code = readev(mouse);

        if (code  == -4) {
            printf("Failed to open Device\n");
            return 1;

        }
        
        acceleration(mouse);
    }

    destroy(mouse);

}