#include "fb_operate.h"
#include "fb.h"
#include "ti81xxfb.h"
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <errno.h>
#include <termios.h>
#include <signal.h>
#include <pthread.h>
#include <sys/socket.h>
#include <assert.h>
#include <netdb.h>
#include <semaphore.h>

#define BYTE_4096  0x1000
#define FB1_MEM_BASE 0x48105000

static int fd_mem = -1;
static void* fb1_base_addr = NULL;

// 打开fb
int fb_open(char *dev)
{
    int fd = -1; 
    
    if ( dev == NULL ) 
    {
        printf("dev == NULL\n");
        return -1;
    }
    
	fd = open(dev, O_RDWR);
	if ( fd == -1 )
	{
		printf("failed to open %s \n", dev);
		return -1;
	}
    
	return fd;
}

// 关闭fb
void fb_close(int fd)
{
    if ( fd > 0 ) 
    {
        close(fd);
    }
}

// 这种方式目前存在问题，有一定概率导致编码停止
void alpha_blending(int fd)
{
    if ( fd < 0 )
    {
        return ;
    }
    
#if 1
    struct ti81xxfb_region_params  regp;

    if (ioctl(fd, TIFB_GET_PARAMS, &regp) < 0) 
	{
		printf("failed to TIFB_GET_PARAMS\n");
		return;
    }
    //Set Global Alpha Blending
    regp.blendtype = TI81XXFB_BLENDING_GLOBAL;
    if (ioctl(fd, TIFB_SET_PARAMS, &regp) < 0) 
	{
		printf("failed to TIFB_SET_PARAMS.\n");
		return;
    }
#else
    
#endif
    
}

// 这种方式目前存在问题，有一定概率导致编码停止
void alpha_disable(int fd)
{
    if ( fd < 0 )
    {
        return;
    }
    
#if 1
    struct ti81xxfb_region_params  regp;
    
    if (ioctl(fd, TIFB_GET_PARAMS, &regp) < 0) 
    {
        printf("failed to TIFB_GET_PARAMS\n");
        return;
    }
    //Disable Alpha Blending
    regp.blendtype = TI81XXFB_BLENDING_NO;
    if (ioctl(fd, TIFB_SET_PARAMS, &regp) < 0) 
    {
        printf ("failed to TIFB_SET_PARAMS\n");
        return;
    }
#endif
}


/********************************************************************
** 设置透明色
********************************************************************/
void alpha_global(int fd)
{
    struct ti81xxfb_region_params  regp;
    
    if (ioctl(fd, TIFB_GET_PARAMS, &regp) < 0) 
    {
        printf("failed to TIFB_GET_PARAMS\n");
        return;
    }

    regp.transen = TI81XXFB_FEATURE_ENABLE;
    regp.transcolor = 0xf8f8f8;
    
    if (ioctl(fd, TIFB_SET_PARAMS, &regp) < 0) 
    {
        printf("failed to TIFB_SET_PARAMS\n");
        return;
    }
}

void get_fb_set(int fd)
{
    if ( fd < 0 )
    {
        return;
    }
    
    struct fb_var_screeninfo vinfo;  

    ioctl(fd,FBIOGET_VSCREENINFO,&vinfo);
    printf("The xres is :%d\n",vinfo.xres);  
    printf("The yres is :%d\n",vinfo.yres);  
    printf("bits_per_pixel is :%d\n",vinfo.bits_per_pixel);

    // 1080p
    vinfo.xres = 1280;
    vinfo.yres = 720;
    vinfo.xres_virtual = 1280;
    vinfo.yres_virtual = 720;

    ioctl(fd, FBIOPUT_VSCREENINFO, &vinfo);
}


int mem_mmap()
{
    if ( fd_mem < 0 )
    {
        fd_mem = open("/dev/mem", O_RDWR|O_SYNC);
        if ( fd_mem == -1 )
        {
            printf("open /dev/mem failed\n");
            return -1;
        }
    }

    fb1_base_addr = mmap(NULL, BYTE_4096, PROT_READ|PROT_WRITE, MAP_SHARED, fd_mem, FB1_MEM_BASE);
    if ( fb1_base_addr == NULL )
    {
        return -1;
    }

    return 0;
}

void mem_unmap()
{
    if ( fb1_base_addr == NULL )
    {
        return;
    }
    else
    {
        munmap(fb1_base_addr, BYTE_4096);
    }
    
    if ( fd_mem > 0 )
    {
        close(fd_mem);
        fd_mem = -1;
    }
}

// 显示QT
void alpha_disable_mem_fb1()
{
    if ( fb1_base_addr == NULL )
    {
        return ;
    }

    char *base_addr = (char*)fb1_base_addr;
    unsigned long disable_value = 0x1000a;

    memcpy(base_addr + 0x20c, &disable_value, 4);
}

// 隐藏QT
void alpha_blending_mem_fb1()
{
    if ( fb1_base_addr == NULL )
    {
        return ;
    }

    char *base_addr = (char*)fb1_base_addr;
    unsigned long blending_value = 0x10008;

    memcpy(base_addr + 0x20c, &blending_value, 4);

}

