#include "messages.hpp"

#include <iostream>

#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <signal.h>
#include <linux/fb.h>

// Pointer to framebuffer memory. 480*320 16bpp 
char* framebuffer;
int fd_framebuffer = -1;

pid_t lidar_pid = 0;

uint16_t convert_color(uint8_t r, uint8_t g, uint8_t b)
{
	uint16_t r1 = ((uint16_t)r * 31) / 255 ;
	uint16_t g1 = ((uint16_t)g * 63) / 255 ;
	uint16_t b1 = ((uint16_t)b * 31) / 255 ;
	return (r1 << 11) | (g1 << 5) | b1;
};

void open_framebuffer()
{
    int fd = -1;
    fd = open("/dev/fb1", O_RDWR);
    struct fb_var_screeninfo vinfo;
    struct fb_fix_screeninfo finfo;
    // Get fixed screen information
    if (ioctl(fd, FBIOGET_FSCREENINFO, &finfo) == -1) {
        perror("Error reading fixed information");
        exit(2);
    }

    // Get variable screen information
    if (ioctl(fd, FBIOGET_VSCREENINFO, &vinfo) == -1) {
        perror("Error reading variable information");
        exit(3);
    }

    int screensize = vinfo.xres * vinfo.yres * vinfo.bits_per_pixel / 8;
    framebuffer = (char *)mmap(0, screensize, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
	fd_framebuffer = fd;
};

void draw_score(int score)
{

};

void draw_indicator(int x, int y, uint16_t color)
{
	for(int iy = y; iy < y+16; iy++)
	{
		for(int ix = x; ix < x + 16; ix++)
		{
			*(uint16_t*)(framebuffer + (ix + (iy * 480)) * 2) = color;
		}
	}
	
	for(int i = 0; i < 16; i++)
	{
		*(uint16_t*)(framebuffer + (x+i + (y * 480)) * 2) = 0xf00f;
		*(uint16_t*)(framebuffer + (x+i + ((y + 15) * 480)) * 2) = 0xf00f;
		*(uint16_t*)(framebuffer + (x + ((y +i)* 480)) * 2) = 0xf00f;
		*(uint16_t*)(framebuffer + (x + 15 + ((y +i)* 480)) * 2) = 0xf00f;
	};
};

void send_robot_config()
{
	
	
}

extern void* zmq_comm_pub_socket;

void zmq_process_message(uint16_t type, void* data, size_t data_size)
{
	// Match state change
	if(type == 15)
	{
		int state = *(uint8_t*)data;
		int side = *(uint8_t*)(data+1);
		std::cout << "match_state: " << state << "side: " << side << std::endl;

		
	};
	//std::cout << type;
	//gpio
	if(type == 20)
	{
		uint32_t gpio_state = *(uint32_t*)(data);
		std::cout << gpio_state << std::endl;
		
		for(int i = 0; i < 8; i++)
		{
			draw_indicator(i*16,0, (gpio_state & (1 << i)) ? 0xffff : 0);
		};
	};
	
	//gpio
	if(type == 21)
	{
		uint32_t gpio_state = *(uint32_t*)(data);

		for(int i = 0; i < 8; i++)
		{
			// gpio=1: team B purple
			draw_indicator(i*16,32, (gpio_state & (1 << i)) ? 0xffff : 0);
		};
		
		if( gpio_state & (1 << 4))
		{
			draw_indicator(256,16, convert_color(132, 76, 130));
		} else
		{
			// gpio=0: team A yellow
			draw_indicator(256,16, convert_color(247, 81, 0));
			
		}
		
	};
	zmq_send_message(zmq_comm_pub_socket,42,"foo",0);
	// Adversary detection
};


static int s_interrupted = 0;
static void s_signal_handler (int signal_value)
{
    s_interrupted = 1;
}

static void s_catch_signals (void)
{
    struct sigaction action;
    action.sa_handler = s_signal_handler;
    action.sa_flags = 0;
    sigemptyset (&action.sa_mask);
    sigaction (SIGINT, &action, NULL);
    sigaction (SIGTERM, &action, NULL);
}

int main()
{
	s_catch_signals();
	
	open_framebuffer();
	zmq_init();	
    
    while(!s_interrupted)
    {
		zmq_check_messages();
    }
	zmq_cleanup();
	close(fd_framebuffer);
	
	return 0;
}
