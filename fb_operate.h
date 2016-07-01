#ifndef _FB_OPERATE_H_
#define _FB_OPERATE_H_

int  fb_open(char *dev);
void fb_close(int fd);
void alpha_blending(int fd); // 隐藏QT
void alpha_disable(int fd);  // 显示QT
void alpha_global(int fd);   // 设置透明色
void get_fb_set(int fd);

// 操作寄存器来实现QT显示和隐藏
int mem_mmap();
void mem_unmap();
void alpha_disable_mem_fb1();  // 显示QT
void alpha_blending_mem_fb1(); // 隐藏QT

#endif
