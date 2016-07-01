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
    ͨ����ָ��UDP�˿ڷ�����Ϣ�������Զ����ؽ���Ķ�ʱ��
******************************************************/
#define CMD1_CLEAR_TIMER   "clear_timer"  // �����Զ����صĶ�ʱ��
#define CMD2_START_TOUCH   "start_touch"  // ��ʼ�������¼�
#define SEND_UDP_PORT     36666   // ���͸�QT�Ķ˿�
#define RECEIVE_UPD_PORT  36667   // ������Ϣ�Ķ˿�
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

// �ȴ�QT���Ϳ�ʼ�����¼�����Ϣ
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
    ����ϵͳ
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

    // �������
    if (argc < 2)
    {
        printf("argc < 2!\n  func: %s line: %d\n", __FUNCTION__, __LINE__);
        return -1;
    }
    
/*********************************************************
* 1. ����fb
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

    // ����͸��ɫ
    alpha_global(fd1);
    alpha_global(fd0);

    // ���÷ֱ���Ϊ720p
    get_fb_set(fd1);

    fb_close(fd1);
    fb_close(fd0);

/*********************************************************
* 2. �����������
*********************************************************/
    if (setup_uinput_device() < 0)
    {
        printf("setup_uinput_device() -> failed!\n  func: %s line: %d\n", __FUNCTION__, __LINE__);
        return -1;
    }

/*********************************************************
* 3. �������ڣ����������ѵ
*********************************************************/
    // �򿪴���
	if(CommOpen(argv[1]) < 0)
	{
        printf("CommOpen() -> failed!\n  func: %s line: %d\n", __FUNCTION__, __LINE__);
		return -1;
	}

	COMM_ATTR attr = {0 };
	attr.baudrate = 115200;
	attr.databits = 8;			// 8λ����λ
	attr.parity = COMM_NOPARITY;		// ����żЧ��	
	attr.stopbits = COMM_ONESTOPBIT;	// 1λֹͣλ
	if(SetAttribute(&attr) !=0)
	{
		printf("set com attr failed\n");
	}

    struct	timeval time_preview;
	struct	timeval time_next;
    int first_time = 0; //��һ��

    wait_start_touch();  // �ȴ�QT���Ϳ�ʼ����Ϣ

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
        // ������˻�����Ҫ���ϣ����ӵĻ������׳��ֵ�һ�δ�������ʵ�ʳ��ֶ�ε�������
        // ���ϵĻ������һֱ������Ļ��һֱ���͵����Ϣ��������������ε�

        // ��һ��
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

        // ����
        if (strncmp(buffer, "keyvalue:", strlen("keyvalue:")) == 0)
        {
            // ��Ƭ�����������ﲻ����Ҫ����
            //send_press_event(KEY_HOME);
            //send_release_event(KEY_HOME);
        }
        // ������
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

            // ����UDP��Ϣ
            send_udp_message(CMD1_CLEAR_TIMER);

            sscanf( &(buffer[index]), ": %d %dx%d %d", &int_unknown, &touch_x, &touch_y, &pressure);
            printf("int_unknown -> %d touch_x -> %d, touch_y -> %d pressure -> %d\n", int_unknown, touch_x, touch_y, pressure);

            send_move_event_abs(TOUCH_TO_SYSTEM_X(touch_x), TOUCH_TO_SYSTEM_Y(touch_y));
            send_mouse_press_event();
            send_mouse_release_event();
        }
	}

    // ������Դ
    mem_unmap();
	CommDestory();
	return 0;
}

