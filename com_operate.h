#ifndef _COM_OPERATE_H_
#define _COM_OPERATE_H_

typedef unsigned char    BYTE;
typedef unsigned short   WORD;
typedef signed int       LONG;
typedef unsigned int     DWORD;

typedef struct tagCOMM_ATTR
{
    DWORD	baudrate;
    BYTE	databits;
    BYTE	parity;
    BYTE	stopbits;
    BYTE	reserved;
} COMM_ATTR;


enum comm_stopbits_t
{
    COMM_ONESTOPBIT,
    COMM_ONE5STOPBITS,
    COMM_TWOSTOPBITS,
};

enum comm_parity_t
{
    COMM_NOPARITY,
    COMM_ODDPARITY,
    COMM_EVENPARITY,
    COMM_MARK,
    COMM_SPACE,
};


#define COMM_PURGE_TXABORT			0x0001
#define COMM_PURGE_RXABORT			0x0002
#define COMM_PURGE_TXCLEAR			0x0004
#define COMM_PURGE_RXCLEAR			0x0008


#define dprintfbin(buf, size) 					\
	do {											\
		int i; 										\
		for (i = 0; i < size - 1; i++){ 				\
			if (0 == i % 16){ 						\
				if (0 != i)							\
					printf("\n");					\
				printf("0x%04x: ", i); 			\
			}										\
			printf("%02x ", ((char*)buf)[i]);		\
		}											\
		printf("%02x\n", ((char*)buf)[i]); 			\
	} while(0)

#define MAX(a, b)		(a > b? a: b)

#define ERR_PRINT		perror
#define LIBDVR_PRINT		printf


int CommOpen(char *DEV_COM);
int CommDestory(void);
int SetAttribute(COMM_ATTR *pattr);
int GetAttribute(void);
int CommRead(void *pdata, DWORD nbytes);
int CommWrite(void *pdata, DWORD len);
int CommPurge(DWORD dw_flags);

#endif
