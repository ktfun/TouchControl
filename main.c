#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <errno.h>
#include <termios.h>
#include <signal.h>
#include <pthread.h>
#include <sys/socket.h>
#include <assert.h>
#include <netdb.h>
#include <semaphore.h>
#include <arpa/inet.h>

#include "visual_key_mouse.h"
#include "com_operate.h"
#include "fb_operate.h"

/*****************************************************
    通过给指定UDP端口发送消息来清零自动隐藏界面的定时器
******************************************************/
#define CMD1_CLEAR_TIMER   "clear_timer"  // 清零自动隐藏的定时器
#define CMD2_START_TOUCH   "start_touch"  // 开始处理触摸事件
#define SEND_UDP_PORT     36666   // 发送给QT的端口
#define RECEIVE_UPD_PORT  36667   // 接收消息的端口
#define MAXDATASIZE 1024
void send_udp_message(const char* cmd)
{
    if ( cmd == NULL ) 
    {
        return;
    }
    
    static int sock = -1;
    if ( sock == -1 )
    {
        if ((sock = socket(AF_INET, SOCK_DGRAM, 0)) == -1)   
        {     
            printf("socket failed\n");
            return ;
        }
    }
  
    struct sockaddr_in addrto;  
    bzero(&addrto, sizeof(struct sockaddr_in));  
    addrto.sin_family = AF_INET;
    addrto.sin_addr.s_addr = inet_addr("127.0.0.1"); 
    addrto.sin_port = htons(SEND_UDP_PORT);  
    int nlen = sizeof(addrto);
 
    sendto(sock, cmd, strlen(cmd), 0, (sockaddr*)&addrto, nlen);

    return;
}

// 等待QT发送开始处理事件的消息
void wait_start_touch()
{
    int sock = -1;
    if ( sock == -1 )
    {
        if ((sock = socket(AF_INET, SOCK_DGRAM, 0)) == -1)   
        {     
            printf("socket failed\n");
            return ;
        }
    }

    struct sockaddr_in addrto;  
    bzero(&addrto, sizeof(struct sockaddr_in));  
    addrto.sin_family = AF_INET;
    addrto.sin_addr.s_addr = inet_addr("127.0.0.1"); 
    addrto.sin_port = htons(RECEIVE_UPD_PORT);

    if ( bind(sock, (struct sockaddr*)&addrto, sizeof(struct sockaddr)) == -1 )
    {
        printf("bind failed\n");
        close(sock);
        return;
    }

    int num = 0;
    char buff[MAXDATASIZE] = "";
    struct sockaddr_in client;
    socklen_t addrlen = sizeof(client);

    while ( num = recvfrom(sock, buff, MAXDATASIZE, 0, (struct sockaddr*)&client, &addrlen) )
    {
        if ( num <= 0 )
        {
            continue;
        }

        if ( strncmp(buff, CMD2_START_TOUCH, sizeof(CMD2_START_TOUCH)) == 0 )
        {
            printf("CMD ===> START_TOUCH\n");
            close(sock);
            return ;
        }
        else
        {
            continue;
        }
    }
}

/*****************************************************
    坐标系统
*****************************************************/
#define SIZE_1024    1024
#define TOUCH_X_MAX     1280
#define TOUCH_Y_MAX     800
#define SYSTEM_X_MAX    1280
#define SYSTEM_Y_MAX    720
#define TOUCH_TO_SYSTEM_X(x)  ((x) * (SYSTEM_X_MAX) / (TOUCH_X_MAX))
#define TOUCH_TO_SYSTEM_Y(y)  ((y) * (SYSTEM_Y_MAX) / (TOUCH_Y_MAX))

int main(int argc, char *argv[])
{
    int fd1 = -1;
    int fd0 = -1;

    // 参数检测
    if (argc < 2)
    {
        printf("argc < 2!\n  func: %s line: %d\n", __FUNCTION__, __LINE__);
        return -1;
    }
    
/*********************************************************
* 1. 设置fb
*********************************************************/
    fd1 = fb_open((char*)"/dev/fb1");
    fd0 = fb_open((char*)"/dev/fb0");
    if ( fd1 < 0 || fd0 < 0 )
    {
        printf("fb_open() -> failed!\n  func: %s line: %d\n", __FUNCTION__, __LINE__);
        return -1;
    }

    alpha_disable(fd1);
    alpha_disable(fd0);

    // 设置透明色
    alpha_global(fd1);
    alpha_global(fd0);

    // 设置分辨率为720p
    get_fb_set(fd1);

    fb_close(fd1);
    fb_close(fd0);

/*********************************************************
* 2. 创建虚拟鼠标
*********************************************************/
    if (setup_uinput_device() < 0)
    {
        printf("setup_uinput_device() -> failed!\n  func: %s line: %d\n", __FUNCTION__, __LINE__);
        return -1;
    }

/*********************************************************
* 3. 开启串口，进入监听轮训
*********************************************************/
    // 打开串口
	if(CommOpen(argv[1]) < 0)
	{
        printf("CommOpen() -> failed!\n  func: %s line: %d\n", __FUNCTION__, __LINE__);
		return -1;
	}

	COMM_ATTR attr = {0 };
	attr.baudrate = 115200;
	attr.databits = 8;			// 8位数据位
	attr.parity = COMM_NOPARITY;		// 无奇偶效验	
	attr.stopbits = COMM_ONESTOPBIT;	// 1位停止位
	if(SetAttribute(&attr) !=0)
	{
		printf("set com attr failed\n");
	}

    struct	timeval time_preview;
	struct	timeval time_next;
    int first_time = 0; //第一次

    wait_start_touch();  // 等待QT发送开始的消息

	while(1)
	{
        int ret = 0;
        int int_unknown = 0;
        int touch_x = 0;
        int touch_y = 0;
        int pressure = 0;
        int index = 0;
		
		char buffer[SIZE_1024] = "";
        
		ret = CommRead(buffer, SIZE_1024);
        if (ret < 0)
        {
            printf("CommRead() -> failed!\n  func: %s line: %d\n", __FUNCTION__, __LINE__);
            continue;
        }
		
		printf("ret = %d buffer -> %s", ret, buffer);

        #if 1
        // 这里过滤还是需要加上，不加的话，容易出现点一次触摸屏，实际出现多次点击的情况
        // 加上的话，会把一直按着屏幕，一直发送点击信息的这种情况给屏蔽掉

        // 第一次
        if (first_time == 0)
        {
            gettimeofday(&time_preview,NULL);
            first_time = 1;
        }
        else
        {
            int diff = 0;
            
            gettimeofday(&time_next,NULL);
            diff = 1000 * (time_next.tv_sec - time_preview.tv_sec)+ (time_next.tv_usec - time_preview.tv_usec) / 1000;
            time_preview = time_next;
            // diff < 100ms
            if (diff < 50)
            {
                //printf("diff -> %d continue\n", diff);
                continue;
            }
            else
            {
                //printf("diff -> %d\n", diff);
            }
        }

        #endif

        // 按键
        if (strncmp(buffer, "keyvalue:", strlen("keyvalue:")) == 0)
        {
            // 单片机来处理，这里不再需要处理
            //send_press_event(KEY_HOME);
            //send_release_event(KEY_HOME);
        }
        // 触摸屏
        else
        {
            for(index = 0; index < SIZE_1024; index++)
            {
                if (buffer[index] == ':')
                    break;
            }
            if (index == SIZE_1024)
            {
                printf("CommRead() -> failed!\n  func: %s line: %d\n", __FUNCTION__, __LINE__);
                continue;
            }

            // 发送UDP消息
            send_udp_message(CMD1_CLEAR_TIMER);

            sscanf( &(buffer[index]), ": %d %dx%d %d", &int_unknown, &touch_x, &touch_y, &pressure);
            printf("int_unknown -> %d touch_x -> %d, touch_y -> %d pressure -> %d\n", int_unknown, touch_x, touch_y, pressure);

            send_move_event_abs(TOUCH_TO_SYSTEM_X(touch_x), TOUCH_TO_SYSTEM_Y(touch_y));
            send_mouse_press_event();
            send_mouse_release_event();
        }
	}

    // 清理资源
    mem_unmap();
	CommDestory();
	return 0;
}

