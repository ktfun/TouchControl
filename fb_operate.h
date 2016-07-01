#ifndef _FB_OPERATE_H_
#define _FB_OPERATE_H_

int  fb_open(char *dev);
void fb_close(int fd);
void alpha_blending(int fd); // ����QT
void alpha_disable(int fd);  // ��ʾQT
void alpha_global(int fd);   // ����͸��ɫ
void get_fb_set(int fd);

// �����Ĵ�����ʵ��QT��ʾ������
int mem_mmap();
void mem_unmap();
void alpha_disable_mem_fb1();  // ��ʾQT
void alpha_blending_mem_fb1(); // ����QT

#endif
