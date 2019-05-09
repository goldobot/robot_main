#include <zmq.h>
#include <iostream>

#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <sys/mman.h>

#include <linux/fb.h>

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

printf("%dx%d, %dbpp\n", vinfo.xres, vinfo.yres, vinfo.bits_per_pixel);
int screensize = vinfo.xres * vinfo.yres * vinfo.bits_per_pixel / 8;
framebuffer = (char *)mmap(0, screensize, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
for(int i = 0; i < screensize;i++)
{
framebuffer[i] = i;
}
};

void draw_score(int score)
{

};

int main()
{
	open_framebuffer();
    void* zmq_context;
    void* socket;
    void* pull_socket;

    zmq_context = zmq_init(1);
    socket = zmq_socket(zmq_context, ZMQ_SUB);
    zmq_connect(socket, "tcp://localhost:3001");
    zmq_setsockopt(socket,ZMQ_SUBSCRIBE, "", 0); 
    while(1)
    {
    int64_t more;
    size_t more_size = sizeof(more);
    zmq_msg_t msg;
    auto rc = zmq_msg_init (&msg);
//    assert (rc == 0);
    rc = zmq_recvmsg(socket, &msg, 0);
    uint16_t message_id = *(uint16_t*)zmq_msg_data (&msg);
  //  assert (rc == 0);
    rc = zmq_getsockopt (socket, ZMQ_RCVMORE, &more, &more_size);
   if(more && message_id != 8 && message_id != 9 && message_id != 5 && message_id != 21)
    std::cout << "message:" << message_id <<  std::endl;
    zmq_msg_close (&msg);
    }
	return 0;
}
