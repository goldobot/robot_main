#include "messages.hpp"

#include <iostream>

#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <sys/mman.h>

#include <linux/fb.h>

// Pointer to framebuffer memory. 480*320 16bpp 
char* framebuffer;

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
};

void send_robot_config()
{
	
	
}

void start_lidar()
{
	const char* lidar_path = "/home/pi/goldo/src/robotik/coupe2019/soft/strat/Strat/tools/rplidar_raspi_2018/sdk/output/Linux/Release/ultra_simple";
	auto pid = fork();
	if(pid == 0)
	{		
		execl(lidar_path, "ultra_simple",nullptr);
	};
};


void zmq_process_message(uint16_t type, void* data, size_t data_size)
{
	if(type == 20)
	{
		uint32_t gpio_state = *(uint32_t*)(data);
		std::cout << gpio_state << std::endl;
		for(int i = 0; i < 6; i++)
		{
			//draw_indicator(i*16,0, (gpio_state & (1 << i)) ? 0xffff : 0);
		};
	};
};

int main()
{
	open_framebuffer();
	draw_indicator(0,0, 0xffff);
	start_lidar();
	for(int i= 0; i < 2000; i++)
	{
		framebuffer[i] = 0xff;
	};
	zmq_init();
    
    while(1)
    {
		zmq_check_messages();
    }
	return 0;
}
