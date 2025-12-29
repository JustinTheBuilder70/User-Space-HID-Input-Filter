#include <fcntl.h>
#include <unistd.h>
#include <iostream>
#include <linux/input.h>
#include <unistd.h>
#include <linux/uinput.h>
#include <chrono>
#include <string.h>


struct Mouse{
    
        int mouse = open("/dev/input/event2", O_RDONLY); //Open the desired devicce;
        int vmouse = open("/dev/uinput", O_WRONLY | O_NONBLOCK); 
        int t;
        struct input_event evst = {0, 0, 0, 0, 0};
        input_event *ev = &evst; //Where the events live;
        struct uinput_user_dev uidev;


        Mouse() {
            setupVdev();
            run();
        }

        
        ~Mouse() {
            destroy();
        }



    int readev() {

        ssize_t h = read(mouse, ev, sizeof(struct input_event));    

        if (h == -1) {
            return -4;
        }

        return 0;

    }



    void setupVdev() { // insert into device open funtion after a check was done

        if (mouse < 0) {
            printf("Mouse Init Error\n");
            return;
        } 

        if (vmouse < 0) {
            printf("Virtual Mouse Init Error\n");
            close(mouse);
            return;
        }


        ioctl(vmouse, UI_SET_EVBIT, EV_KEY);
        ioctl(vmouse, UI_SET_KEYBIT, BTN_LEFT);

        ioctl(vmouse, UI_SET_EVBIT, EV_REL);
        ioctl(vmouse, UI_SET_RELBIT, REL_X);
        ioctl(vmouse, UI_SET_RELBIT, REL_Y);

        
        memset(&uidev, 0, sizeof(uidev));
        snprintf(uidev.name, UINPUT_MAX_NAME_SIZE, "virtual-mouse");
        
        uidev.id.bustype = BUS_USB;
        uidev.id.vendor  = 0x1235;
        uidev.id.product = 0x5678;
        uidev.id.version = 1;

        write(vmouse, &uidev, sizeof(uidev));
        ioctl(vmouse, UI_DEV_CREATE);

        sleep(1); 

        printf("...Setup Complete\n");

    }       

    void check_click() {
    
        if (ev -> code == 273) {          
            //memset(evptr, 0, sizeof(struct input_event));
            ev->type = EV_KEY;
            ev->code = BTN_LEFT;
            ev->value = 1; // press
            write(vmouse, ev, sizeof(struct input_event));
            
            /*I dont think this SYN_REPORT has to be here*/
            /*
            //memset(evptr, 0, sizeof(struct input_event));
            ev->type = EV_SYN;
            ev->code = SYN_REPORT;
            ev->value = 0;
            write(vmouse, ev, sizeof(struct input_event));
            */


            usleep(1); 
            
            //memset(evptr, 0, sizeof(struct input_event));
            ev->type = EV_KEY;
            ev->code = BTN_LEFT;
            ev->value = 0; // release
            write(vmouse, ev, sizeof(struct input_event));


            //memset(evptr, 0, sizeof(struct input_event));
            ev->type = EV_SYN;
            ev->code = SYN_REPORT;
            ev->value = 0;
            write(vmouse, ev, sizeof(struct input_event));
            memset(ev, 0, sizeof(struct input_event));
            
        }
    }


    void destroy() {
        ioctl(vmouse, UI_DEV_DESTROY);
        close(mouse);
        close(vmouse);
    }


    void checks() {
    //determine state, then execute functions if state is matched.
        if (ev -> code == 273) { 
            check_click();
        
        }

        if (ev -> type == EV_REL && (ev -> code == REL_X || ev -> code == REL_Y)) {
            
            acceleration();
            write(vmouse, ev, sizeof(struct input_event));
        }

    }   
    

    void acceleration() {
        
        ev -> value = (int)(2.3 * ev -> value);
        
        //smoothed = alpha * raw + (1 - alpha) * smoothed;
        
    }


    void run() {

        int code = 0;
        while (true) {
            code = readev();

            if (code == -4) {
                printf("Failed to open device\n");
                return;
            }
            
            checks();
            
        }
    }

};


int main() {
    Mouse m1;
   

    
}