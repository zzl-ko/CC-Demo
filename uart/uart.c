/************************************************************************************************************* 
* Copyright: (C)2019, All rights reserved.
* Version  : V1.0
* Author   : Kevin Zhou <zzl.ko@outlook.com>
* Date     : 2019.09.05
* Abstact  : None
* History  : Please check the end of this file.
*************************************************************************************************************/

/************************************** INCLUDE *************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <fcntl.h>
#include <unistd.h>

#include <signal.h>
#include <pthread.h>
#include <sys/wait.h>
#include <sys/prctl.h>

#include <sys/time.h>
#include <sys/poll.h>
#include <sys/select.h>
#if defined(__linux__)
#include <linux/serial.h>
#endif

#if 0
#include <termios.h>
#include <sys/ioctl.h>
#else
#include <linux/termios.h>
#include <asm/ioctls.h>
extern int ioctl (int __fd, unsigned long int __request, ...) __THROW;
extern int tcflush (int __fd, int __queue_selector) __THROW;
extern int cfsetospeed (struct termios *__termios_p, speed_t __speed) __THROW;
extern int cfsetispeed (struct termios *__termios_p, speed_t __speed) __THROW;
extern speed_t cfgetospeed (const struct termios *__termios_p) __THROW;
extern speed_t cfgetispeed (const struct termios *__termios_p) __THROW;
extern int tcgetattr (int __fd, struct termios *__termios_p) __THROW;
extern int tcsetattr (int __fd, int __optional_actions, const struct termios *__termios_p) __THROW;
#endif

/************************************** NAMESPACE ***********************************************************/
/************************************** CONST DEFINE ********************************************************/
/************************************** MACRO DEFINE ********************************************************/
#define MAX_BUFFER_SIZE             512
#ifndef UART_RD_WR_FLAG
#define UART_RD_WR_FLAG               2 // 0:only recv, 1:only send, 2: recv and send
#endif

/************************************** EMUN DEFINE *********************************************************/
/************************************** TYPE DEFINE *********************************************************/
/************************************** GLOBAL VARIABLE DECLARE *********************************************/
/************************************** LOCAL VARIABLE DECLARE **********************************************/
static struct termios option_old;
static int    uart_fd = -1;

/************************************** EXTERN FUNCTION PROTOTYPE *******************************************/

/************************************** LOCAL FUNCTION PROTOTYPE ********************************************/
void uart_debug_tx(int fd);
void uart_debug_rx(int fd);

/************************************************************************************************************/
/*                                      LOCAL FUNCTION IMPLEMENT                                            */
/************************************************************************************************************/
/*************************************************************************************************************
* @Param[in]  : options -- 串口属性结构体指针
* @Param[out] : None
* @Return     : None
* @Description: 打印串口各属性值
* @History    :
* -----------------------------------------------------------------------------------------------------------
* @Date        Version    Author        Modify 
* ----------------------------------------------------------------------------------------------------------- 
* 2019-09-05   V1.0       Kevin Zhou    Initial version create
*************************************************************************************************************/
static void uart_print_attr(struct termios *options)
{
    int i = 0;

    printf("c_iflag = %#x\n", options->c_iflag);
    printf("c_oflag = %#x\n", options->c_oflag);
    printf("c_cflag = %#x\n", options->c_cflag);
    printf("c_lflag = %#x\n", options->c_lflag);
    
    for(i=VINTR; i<=VEOL2; i++)
    {
        printf("c_cc[%d] = %d\n", i, options->c_cc[i]);
    }
}

/*************************************************************************************************************
* @Param[in]  : None
* @Param[out] : None
* @Return     : 串口属性结构体指针
* @Description: 初始化默认的串口属性，并返回其申请的属性结构指针
* @History    :
* -----------------------------------------------------------------------------------------------------------
* @Date        Version    Author        Modify 
* ----------------------------------------------------------------------------------------------------------- 
* 2019-09-05   V1.0       Kevin Zhou    Initial version create
*************************************************************************************************************/
static struct termios *uart_default_attr(void)
{
    struct termios *options = NULL;
    
    options = (struct termios *)calloc(1, sizeof(struct termios));
    if(options == NULL)
    {
        perror("calloc");
        _exit(-1);
    }
    /*
        IXON： 使起动/停止输出控制流起作用
        IXOFF：使起动/停止输入控制流起作用
        ICRNL：将输入的CR转换为NL
    */
    options->c_iflag |= IXOFF|IXON|ICRNL;
    /*
        ONLCR：将NL转换为CR-NL
        OPOST：执行输出处理
    */
    options->c_oflag = ONLCR|OPOST;
    /*
        CSIZE：设置数据位时先关闭历史设置
        B115200：波特率为115200
        CS8：数据宽度为8位
        CREAD：启用接收装置，可以接收字符
        CLOCAL：忽略调制解调器状态行，否则一直阻塞到和调制解调器建立连接
    */
    options->c_cflag &= ~CSIZE;    
    options->c_cflag  = B115200|CS8|CREAD|CLOCAL;
    
    /*
        ICANON：标准模式
        ISIG：启用终端产生的信号
        IEXTEN：启用扩充的输入字符处理
        ECHOKE：kill的可见擦除
        ECHOCTL：回显控制字符为^(Char) 
        ECHOK：回显kill符
        ECHOE：可见擦除符
        ECHO：回显
    */
    //options->c_lflag = ICANON|ISIG|IEXTEN|ECHOKE|ECHOCTL|ECHOK|ECHOE|ECHO;
    options->c_lflag = ISIG|IEXTEN|ECHOKE|ECHOCTL|ECHOK|ECHOE;
    options->c_cc[VINTR]    = 3;    /* ^c：Ctrl+c，中断信号 */ 
    options->c_cc[VQUIT]    = 28;   /* ^\：Ctrl+\，退出信号 */
    options->c_cc[VERASE]   = 127;  /* ^H：Backspace，向前擦除一个字符 */
    options->c_cc[VKILL]    = 21;   /* ^U：，擦行 */
    options->c_cc[VEOF]     = 4;    /* ^D：Ctrl+d，文件结束 */
    options->c_cc[VTIME]    = 0;    /* 设置超时计时器，单位：deciseconds十分之一秒 */
    options->c_cc[VMIN]     = 1;    /* 串口的缓冲区的大小 */
    options->c_cc[VSWTC]    = 0;    /* '\0' */
    options->c_cc[VSTART]   = 17;   /* ^Q：Ctrl+q，恢复输出 */ 
    options->c_cc[VSTOP]    = 19;   /* ^S：Ctrl+s，停止输出 */
    options->c_cc[VSUSP]    = 26;   /* ^Z：Ctrl+z，挂起信号（SIGTSTP） */
    options->c_cc[VEOL]     = 0;    /* '\0'，行结束 */
    options->c_cc[VREPRINT] = 18;   /* ^R：Ctrl-r，再打印全部输入 */
    options->c_cc[VDISCARD] = 15;   /* ^O：Ctrl-u，擦除全部输出 */
    options->c_cc[VWERASE]  = 23;   /* ^W：Ctrl-w，擦除一个字 */
    options->c_cc[VLNEXT]   = 22;   /* ^V：Ctrl-v，下一个字面字符 */
    options->c_cc[VEOL2]    = 0;    /* 供替换的行结束 */
    
    //uart_print_attr(options);
    return options;
}

/*************************************************************************************************************
* @Param[in]  : baudrate  -- 数值型串口波特率，例如 9600
* @Param[out] : pBaudrate -- 标准串口波特率，例如 B9600
* @Return     : bool      -- 1: 标准波特率，0: 非标准波特率
* @Description: 判断波特率是否为标准波特率
* @History    :
* -----------------------------------------------------------------------------------------------------------
* @Date        Version    Author        Modify 
* ----------------------------------------------------------------------------------------------------------- 
* 2019-09-05   V1.0       Kevin Zhou    Initial version create
*************************************************************************************************************/
static inline int is_standard_baudrate(long baudrate, speed_t *pBaudrate)
{
    int custom_baud = 0;

    switch (baudrate) {
#ifdef B0
    case 0: *pBaudrate = B0; break;
#endif
#ifdef B50
    case 50: *pBaudrate = B50; break;
#endif
#ifdef B75
    case 75: *pBaudrate = B75; break;
#endif
#ifdef B110
    case 110: *pBaudrate = B110; break;
#endif
#ifdef B134
    case 134: *pBaudrate = B134; break;
#endif
#ifdef B150
    case 150: *pBaudrate = B150; break;
#endif
#ifdef B200
    case 200: *pBaudrate = B200; break;
#endif
#ifdef B300
    case 300: *pBaudrate = B300; break;
#endif
#ifdef B600
    case 600: *pBaudrate = B600; break;
#endif
#ifdef B1200
    case 1200: *pBaudrate = B1200; break;
#endif
#ifdef B1800
    case 1800: *pBaudrate = B1800; break;
#endif
#ifdef B2400
    case 2400: *pBaudrate = B2400; break;
#endif
#ifdef B4800
    case 4800: *pBaudrate = B4800; break;
#endif
#ifdef B7200
    case 7200: *pBaudrate = B7200; break;
#endif
#ifdef B9600
    case 9600: *pBaudrate = B9600; break;
#endif
#ifdef B14400
    case 14400: *pBaudrate = B14400; break;
#endif
#ifdef B19200
    case 19200: *pBaudrate = B19200; break;
#endif
#ifdef B28800
    case 28800: *pBaudrate = B28800; break;
#endif
#ifdef B57600
    case 57600: *pBaudrate = B57600; break;
#endif
#ifdef B76800
    case 76800: *pBaudrate = B76800; break;
#endif
#ifdef B38400
    case 38400: *pBaudrate = B38400; break;
#endif
#ifdef B115200
    case 115200: *pBaudrate = B115200; break;
#endif
#ifdef B128000
    case 128000: *pBaudrate = B128000; break;
#endif
#ifdef B153600
    case 153600: *pBaudrate = B153600; break;
#endif
#ifdef B230400
    case 230400: *pBaudrate = B230400; break;
#endif
#ifdef B256000
    case 256000: *pBaudrate = B256000; break;
#endif
#ifdef B460800
    case 460800: *pBaudrate = B460800; break;
#endif
#ifdef B576000
    case 576000: *pBaudrate = B576000; break;
#endif
#ifdef B921600
    case 921600: *pBaudrate = B921600; break;
#endif
#ifdef B1000000
    case 1000000: *pBaudrate = B1000000; break;
#endif
#ifdef B1152000
    case 1152000: *pBaudrate = B1152000; break;
#endif
#ifdef B1500000
    case 1500000: *pBaudrate = B1500000; break;
#endif
#ifdef B2000000
    case 2000000: *pBaudrate = B2000000; break;
#endif
#ifdef B2500000
    case 2500000: *pBaudrate = B2500000; break;
#endif
#ifdef B3000000
    case 3000000: *pBaudrate = B3000000; break;
#endif
#ifdef B3500000
    case 3500000: *pBaudrate = B3500000; break;
#endif
#ifdef B4000000
    case 4000000: *pBaudrate = B4000000; break;
#endif
    default:
        custom_baud = 1;
    }

    return !custom_baud;
}

/*************************************************************************************************************
* @Param[in]  : fd       -- 串口设备文件描述符
* @Param[in]  : Baudrate -- 波特率宏值
* @Param[in]  : options  -- 串口属性结构体指针
* @Param[out] : None
* @Return     : bool     -- 1:成功，0:失败
* @Description: 设置标准串口波特率
* @History    :
* -----------------------------------------------------------------------------------------------------------
* @Date        Version    Author        Modify 
* ----------------------------------------------------------------------------------------------------------- 
* 2019-09-05   V1.0       Kevin Zhou    Initial version create
*************************************************************************************************************/
static inline int uart_set_standard_baudrate(int fd, speed_t Baudrate, struct termios *options)
{
#ifdef __linux__
        // try to clear custom baud rate, using termios v2
        struct termios2 tio2;
        if (ioctl(fd, TCGETS2, &tio2) != -1) {
            if (tio2.c_cflag & BOTHER) {
                tio2.c_cflag &= ~BOTHER;
                tio2.c_cflag |= CBAUD;
                ioctl(fd, TCSETS2, &tio2);
            }
        }

        // try to clear custom baud rate, using serial_struct (old way)
        struct serial_struct serial;
        memset(&serial, 0, sizeof(serial));
        if (ioctl(fd, TIOCGSERIAL, &serial) != -1) {
            if (serial.flags & ASYNC_SPD_CUST) {
                serial.flags &= ~ASYNC_SPD_CUST;
                serial.custom_divisor = 0;
                // we don't check on errors because a driver can has not this feature
                ioctl(fd, TIOCSSERIAL, &serial);
            }
        }
#endif

#ifdef _BSD_SOURCE
        if (cfsetspeed(options, Baudrate) < 0) {
            return 0;
        }
#else
        if (cfsetispeed(options, Baudrate) < 0) {
            return 0;
        }
        if (cfsetospeed(options, Baudrate) < 0) {
            return 0;
        }
#endif

    return 1;
}

/*************************************************************************************************************
* @Param[in]  : fd       -- 串口设备文件描述符
* @Param[in]  : baudrate -- (非标)波特率数值
* @Param[out] : None
* @Return     : bool     -- 1:成功，0:失败
* @Description: 设置非标准串口波特率
* @History    :
* -----------------------------------------------------------------------------------------------------------
* @Date        Version    Author        Modify 
* ----------------------------------------------------------------------------------------------------------- 
* 2019-09-05   V1.0       Kevin Zhou    Initial version create
*************************************************************************************************************/
static inline int uart_set_custom_baudrate(int fd, unsigned long baudrate)
{
    struct termios2 tio2;
    // https://blog.csdn.net/u013372900/article/details/96484914
    if (-1 != ioctl(fd, TCGETS2, &tio2) ) {
        tio2.c_cflag &= ~CBAUD;
        tio2.c_cflag |= BOTHER;

        tio2.c_ispeed = baudrate;
        tio2.c_ospeed = baudrate;

        tcflush(fd,TCIFLUSH); 

        if (fcntl(fd, F_SETFL, FNDELAY)) {
            return 0;
        }

        if(-1 != ioctl(fd, TCSETS2, &tio2)
        && -1 != ioctl(fd, TCSETS2, &tio2) ) {
            return 1;
        }
    }

    return 0;
}

/*************************************************************************************************************
* @Param[in]  : flowctrl -- 流控设置，2-硬件流控，1-软件流控，0-无
* @Param[out] : options  -- 串口属性结构体指针
* @Return     : None
* @Description: 设置串口流控
* @History    :
* -----------------------------------------------------------------------------------------------------------
* @Date        Version    Author        Modify 
* ----------------------------------------------------------------------------------------------------------- 
* 2019-09-05   V1.0       Kevin Zhou    Initial version create
*************************************************************************************************************/
static inline void uart_set_flowctrl(int flowctrl, struct termios *options)
{
    switch (flowctrl) {
    case 2:
        options->c_cflag |= CRTSCTS;
        options->c_iflag &= ~(IXON | IXOFF | IXANY);
        break;
    case 1:
        options->c_cflag &= ~CRTSCTS;
        options->c_iflag |= IXON | IXOFF | IXANY;
        break;
    case 0:
    default:
        options->c_cflag &= ~CRTSCTS;
        options->c_iflag &= ~(IXON | IXOFF | IXANY);
        break;
    }
}

/*************************************************************************************************************
* @Param[in]  : fd        -- 串口设备文件描述符
* @Param[in]  : speed     -- 串口波特率（支持包括标准的9600,115200等或非标准的214285）
* @Param[in]  : check     -- 校验位（可选:'N'、'O'、'E'；分别对应无奇偶校验、奇校验、偶校验）
* @Param[in]  : data_bits -- 数据位（可选5、6、7、8）
* @Param[in]  : stop_bits -- 停止位（可选1、2）
* @Param[in]  : flow_ctrl -- 流控位（可选: 2-硬件流控，1-软件流控，0-无）
* @Param[out  : options   -- 串口属性结构体指针
* @Return     : 串口属性结构体指针
* @Description: 设置串口属性
* @History    :
* -----------------------------------------------------------------------------------------------------------
* @Date        Version    Author        Modify 
* ----------------------------------------------------------------------------------------------------------- 
* 2019-09-05   V1.0       Kevin Zhou    Initial version create
*************************************************************************************************************/
struct termios *uart_set_attr(int fd,
                    long speed,
                    char check,
                    int data_bits,
                    int stop_bits,
                    int flow_ctrl,
                    struct termios *options)
{
    if(options == NULL)
    {
        options = uart_default_attr();
    }

    /***********波特率选择****************/
    speed_t Baudrate;
    int isStandardBaudrate = is_standard_baudrate(speed, &Baudrate);
    if  (isStandardBaudrate) {
        uart_set_standard_baudrate(fd, Baudrate, options);
    } else {
        uart_set_custom_baudrate(fd, speed);
    }
    /***********校验位选择****************/
    switch(check)
    {
        case 'O':
            options->c_cflag |= PARENB;         //允许输出产生奇偶信息以及输入的奇偶校验
            options->c_iflag |= (INPCK|ISTRIP); //INPCK:启用输入奇偶检测;ISTRIP:去掉第八位(传输时只传7位)
            options->c_cflag |= PARODD;         //输入和输出是奇校验(ODD)
            break;
        case 'E':
            options->c_cflag |= PARENB;
            options->c_iflag |= (INPCK|ISTRIP);
            options->c_cflag &= ~PARODD;        //输入和输出是偶校验(ECC)
            break;
        case 'N':
        default :
            options->c_cflag &= ~PARENB;        //无奇偶校验位
            break;
    }
    /***********数据位选择****************/
    switch(data_bits)
    {
        case 5: options->c_cflag |= CS5; break;
        case 6: options->c_cflag |= CS6; break;
        case 7: options->c_cflag |= CS7; break;
        case 8:
        default: 
                options->c_cflag |= CS8; break;
    }
    /***********停止位选择****************/
    switch(stop_bits)
    {
        case 2: options->c_cflag &= ~CSTOPB; break;
        case 1:
        default: options->c_cflag |= CSTOPB; break;
    }
    /***********流控位设置****************/
    uart_set_flowctrl(flow_ctrl, options);

    tcflush(fd, TCIFLUSH);
    // fcntl(fd, F_SETFL, O_NONBLOCK);           //read时不阻塞
    fcntl(fd, F_SETFL, 0);                       //read时阻塞

    return options;
}

/*************************************************************************************************************
* @Param[in]  : dev_name -- 设备名称，例如： "/dev/ttyS0"
* @Param[out] : None
* @Return     : UART设备的文件描述符
* @Description: 打开UART设备，并返回其文件描述符
* @History    :
* -----------------------------------------------------------------------------------------------------------
* @Date        Version    Author        Modify 
* ----------------------------------------------------------------------------------------------------------- 
* 2019-09-05   V1.0       Kevin Zhou    Initial version create
*************************************************************************************************************/
static int uart_open(char *dev_name)
{
    int fd = -1;
    
    fd = open(dev_name, O_RDWR | O_NOCTTY );
    if(fd < 0)
    {
        perror("open_dev");
        _exit(-1);
    }

    return fd;
}

/************************************************************************************************************/
/*                                      GLOBAL FUNCTION IMPLEMENT                                           */
/************************************************************************************************************/

/*************************************************************************************************************
* @Param[in]  : dev_name -- 串口设备名称
* @Param[out] : None
* @Return     : None
* @Description: 串口初始化
* @History    :
* -----------------------------------------------------------------------------------------------------------
* @Date        Version    Author        Modify 
* ----------------------------------------------------------------------------------------------------------- 
* 2019-09-05   V1.0       Kevin Zhou    Initial version create
*************************************************************************************************************/
int uart_init(char *dev_name, long speed, char check, int data_bits, int stop_bits, int flow_ctrl)
{
    int fd = -1;
    struct termios *options = NULL;

    printf("dev:%s, baudrate=%ld,check=%c,data=%d,stop=%d,flow=%d\n", dev_name, speed, check, data_bits, stop_bits, flow_ctrl);
    fd = uart_open(dev_name);
    tcgetattr(fd, &option_old);               //保存串口属性
    
#if 0
    struct termios option_new;
    tcgetattr(fd, &option_new);
    cfsetispeed(&option_new, B115200);        //波特率为115200
    cfsetospeed(&option_new, B115200);        //波特率为115200
    option_new.c_cflag &= ~CSIZE;             //设置数据位时先关闭历史设置
    option_new.c_cflag |= CS8;                //数据位为8位
    option_new.c_cflag &= ~CSTOPB;            //1位停止位
    option_new.c_cflag &= ~PARENB;            //无奇偶校验位
    option_new.c_lflag &= ~(ICANON);          //非标准模式
    // option_new.c_lflag &= ~ECHO;           //关回显，在使用GPRS模组时需关回显
    option_new.c_lflag |= ECHO;               //开回显
    tcsetattr(fd, TCSANOW, &option_new);
#else
    options = uart_set_attr(fd, speed, check, data_bits, stop_bits, flow_ctrl, NULL);
    options->c_cc[VMIN]=1;
    options->c_cc[VTIME]=1;
    // https://blog.csdn.net/u012996559/article/details/101375099
    options->c_iflag &= ~(BRKINT | INLCR | ICRNL | IGNCR | INPCK | ISTRIP | IXON | IXOFF | IXANY);
    options->c_oflag &= ~(ONLCR | OCRNL);
    options->c_lflag &= ~(ICANON | ECHO | ECHOE | ISIG);
    
    // options->c_lflag |= ECHO;              //开回显
    // options->c_lflag &= ~ECHO;             //关回显
    tcsetattr(fd, TCSANOW, options);     //设置串口属性
    free(options);
#endif
    
    return fd;
}

/*************************************************************************************************************
* @Param[in]  : fd -- 串口设备文件描述符
* @Param[out] : None
* @Return     : None
* @Description: 串口反初始化
* @History    :
* -----------------------------------------------------------------------------------------------------------
* @Date        Version    Author        Modify 
* ----------------------------------------------------------------------------------------------------------- 
* 2019-09-05   V1.0       Kevin Zhou    Initial version create
*************************************************************************************************************/
void uart_uninit(int fd)
{
    if(fd < 0) return;

    /* 还原串口属性 */
    tcsetattr(fd, TCSANOW, &option_old);
    
    /* 关闭串口 */
    close(fd);
}

/*************************************************************************************************************
* @Param[in]  : fd   -- 串口设备文件描述符
* @Param[in]  : tot  -- 发送超时时间(单位：ms)
* @Param[in]  : len  -- 待发送的内容长度
* @Param[in]  : pBuf -- 待发送的内容
* @Param[out] : None
* @Return     : 实际发送的字符数
* @Description: 串口发送API
* @History    :
* -----------------------------------------------------------------------------------------------------------
* @Date        Version    Author        Modify 
* ----------------------------------------------------------------------------------------------------------- 
* 2019-09-05   V1.0       Kevin Zhou    Initial version create
*************************************************************************************************************/
int uart_send(int fd, long tot, int len, char *pBuf)
{
    (void)tot;

    int ret = write(fd, pBuf, len);
    if (ret < 0)
    {
        perror("write");
    }

    return ret;
}

/*************************************************************************************************************
* @Param[in]  : fd   -- 串口设备文件描述符
* @Param[in]  : tot  -- 接收超时时间(单位：ms)
* @Param[in]  : len  -- 存放数据的内存空间的大小
* @Param[out] : pBuf -- 存放数据的内存的首地址
* @Return     : 实际读到的字符数
* @Description: 串口读一行字符串，在设定的时间内读不到数据则函数返回
* @History    :
* -----------------------------------------------------------------------------------------------------------
* @Date        Version    Author        Modify 
* ----------------------------------------------------------------------------------------------------------- 
* 2019-09-05   V1.0       Kevin Zhou    Initial version create
*************************************************************************************************************/
int uart_recv_select(int fd, long tot, int len, char *pBuf)
{
    char *pTmp = pBuf;
    char chr = '\0';
    int  cnt = 0;
    int  ret = 0;

    fd_set fds;
    struct timeval tv;
    tv.tv_sec = 0;
    tv.tv_usec = tot * 1000;

    memset(pBuf, 0, len);
    for(cnt=0; cnt<len; cnt+=ret){
        FD_ZERO(&fds);
        FD_SET(fd, &fds);
        ret = select(/*FD_SETSIZE*/ 1, &fds, NULL, NULL, &tv);
        if(ret < 0){
            perror("seclect err");
            ret = 0;
            continue;
        } else if(ret == 0) {
            perror("seclect tot");
            break;
        } else {
            ret = read(fd, &chr, 1);
            if(ret > 0) {
                if(chr == '\n') break; // 指定特殊结束条件
                *pTmp++ = chr;
                //printf("c=%c\n", c);
            } else {
                perror("read");
                ret = 0;
                continue;
            }
        }
    }

    return cnt;
}

/*************************************************************************************************************
* @Param[in]    : fd   -- 串口设备文件描述符
* @Param[in]    : tot  -- 接收超时时间(单位：ms)
* @Param[in&out]: pLen -- in: 存放数据的缓冲区的大小, out: 实际读到的字符数
* @Param[out]   : pBuf -- 存放数据的缓冲区
* @Return       : 0成功, 非0失败
* @Description  : 串口读指定长度的数据，在设定的时间内读不到数据则函数返回
* @History      :
* -----------------------------------------------------------------------------------------------------------
* @Date        Version    Author        Modify 
* ----------------------------------------------------------------------------------------------------------- 
* 2019-09-05   V1.0       Kevin Zhou    Initial version create
*************************************************************************************************************/
int uart_recv_poll(int fd, int tot, int *pLen, char *pBuf)
{
    char *pTmp = pBuf;
    char chr = '\0';
    int  len = *pLen;
    int  res = 0;

    struct pollfd fds[1];
    fds[0].fd = fd;
    fds[0].events = POLLIN | POLLERR | POLLHUP;

    memset(pTmp, 0, len);
    for (*pLen = 0; *pLen < len; *pLen += res) {
        res = poll(fds, 1, tot);
        if (res < 0) {
            perror("poll err");
            return -1;
        } else if (res == 0) {
            perror("poll tot");
            return -2;
        } else {
            if( fds[0].revents & POLLIN )
            {
                res = read(fd, &chr, sizeof(chr));
                if(res < 0){
                    perror("read");
                    return -3;
                } else if(res == 0){
                    continue;
                } else {
                    /* 指定特殊结束条件
                    if(chr == '\n')
                    {
                        break;
                    } */

                    *pTmp++ = chr;
                    //printf("c=%c\n", c);
                }
            }

            if( fds[0].revents & POLLERR )
            {
                perror("POLLERR");
                return -4;
            }
            if( fds[0].revents & POLLHUP )
            {
                perror("POLLHUP");
                return -5;
            }
        }
    }

    return 0;
}


/*=============================================== Split Line =================================================*/

enum {
    BB_FATAL_SIGS = (int)(0
        + (1LL << SIGINT)    // 程序终止(interrupt)信号, 在用户键入INTR字符(通常是Ctrl-C)时发出，用于通知前台进程组终止进程
        + (1LL << SIGHUP)    // 挂断。本信号在用户终端连接(正常或非正常)结束时发出, 通常是在终端的控制进程结束时, 通知同一session内的各个作业, 这时它们与控制终端不再关联
        + (1LL << SIGQUIT)   // 和SIGINT类似, 但由QUIT字符(通常是Ctrl-\)来控制. 进程在因收到SIGQUIT退出时会产生core文件, 在这个意义上类似于一个程序错误信号
        + (1LL << SIGTERM)   // 程序结束(terminate)信号, 与SIGKILL不同的是该信号可以被阻塞和处理。通常用来要求程序自己正常退出，shell命令kill缺省产生这个信号
//      + (1LL << SIGTSTP)   // 停止进程的运行, 但该信号可以被处理和忽略. 用户键入SUSP字符时(通常是Ctrl+Z)发出这个信号
//      + (1LL << SIGSTOP)   // 暂停(stopped)进程的执行. 注意它和terminate以及interrupt的区别:该进程还未结束, 只是暂停执行. 本信号不能被阻塞, 处理或忽略
//      + (1LL << SIGCONT)   // 让一个停止(stopped)的进程继续执行. 本信号不能被阻塞. 可以用一个handler来让程序在由stopped状态变为继续执行时完成特定的工作
        + (1LL << SIGPIPE)   // Write to pipe with no readers
        + (1LL << SIGABRT)   // Abort signal from abort(3)
        + (1LL << SIGALRM)   // Timer signal from alarm(2)
        + (1LL << SIGVTALRM) // Virtual alarm clock
        + (1LL << SIGXCPU)   // CPU time limit exceeded
        + (1LL << SIGXFSZ)   // File size limit exceeded
        + (1LL << SIGUSR1)   // Yes kids, these are also fatal!
        + (1LL << SIGUSR2)
        + 0),
};

void bb_signals(int sigs, void (*f)(int))
{
    int sig_no = 0;
    int bit = 1;

    while (sigs) {
        if (sigs & bit) {
            sigs -= bit;
            signal(sig_no, f);
        }
        sig_no++;
        bit <<= 1;
    }
}

void exit_signal_handlr(int sig)
{
    printf("pid=%d, sig=%d: process exit\n", getpid(), sig);

    uart_uninit(uart_fd);
    exit(0);
}

/*----------------------------------------------- Split Line -------------------------------------------------*/

void *thread_handlr_tx(void *arg)
{
    prctl(PR_SET_NAME, "tx_thread"); // 设定线程名为 tx_thread
    printf("in thread_handlr_tx, ppid=%d, pid=%d, tid=%ld\n", getppid(), getpid(), pthread_self());
    uart_debug_tx(uart_fd);
    return NULL;
}

void *thread_handlr_rx(void *arg)
{
    prctl(PR_SET_NAME, "rx_thread"); // 设定线程名为 rx_thread
    printf("in thread_handlr_rx, ppid=%d, pid=%d, tid=%ld\n", getppid(), getpid(), pthread_self());
    uart_debug_rx(uart_fd);
    return NULL;
}

typedef void* (*pfunc_t)(void *arg);

void multi_pthread(void)
{
    pfunc_t fun[] = {
        thread_handlr_tx,
        thread_handlr_rx,
    };
    int i=0,  ret=-1;
    pthread_t tid[sizeof(fun)/sizeof(fun[0])];

    for(i=0; i<sizeof(tid)/sizeof(tid[0]); i++)
    {
        ret = pthread_create(&tid[i], NULL, fun[i], NULL);
        if(0 != ret)
        {
            perror("pthread_create");
            return;
        }
        printf("tid%d=%ld\n", i, tid[i]);
    }

    for(i=0; i<sizeof(tid)/sizeof(tid[0]); i++)
    {
        //pthread_detach(tid[i]); 
        pthread_join(tid[i], NULL);
    }
}

/*----------------------------------------------- Split Line -------------------------------------------------*/

void child_handlr(void)
{
    printf("in child_handlr, pid=%d\n", getpid());
    multi_pthread();
}

void parent_handlr(void)
{
    int status= -1;
    pid_t pid = 0;

    printf("in parent_handlr, pid=%d\n", getpid());
    multi_pthread();
    while(0 < (pid = waitpid(-1, &status, WNOHANG)) )
    {
        printf("Wait Child Success pid[%d],status[%d]\n", pid, status);
    }
}

void process_exit_handlr(void)
{
    printf("pid=%d, process exit\n", getpid());
}

void child_exit_handlr(int sig)
{
    printf("pid=%d, sig=%d, child exit\n", getpid(), sig);
}

void multi_process(void)
{
    pid_t pid = -1;

    atexit(process_exit_handlr);
    /* SIGCHLD: 
     * 子进程（child）结束时, 父进程会收到这个信号。如果父进程没有处理这个信号，也没有等待(wait)子进程，
     * 则子进程虽然终止，但是还会在内核进程表中占有表项，这时的子进程称为僵尸进程 */
    signal(SIGCHLD, child_exit_handlr);

    pid = fork(); //vfork();
    if (pid < 0)
    {
        perror("vfork");
        return;
    }
    else if(0 == pid)
    {
        printf("I am Child, my pid[%d], Parent pid[%d]\n", getpid(), getppid());
        prctl(PR_SET_PDEATHSIG, SIGCHLD);
        child_handlr();
    }
    else
    {
        printf("I am Parent, my pid[%d], Child pid[%d]\n", getpid(), pid);
        parent_handlr();
    }

    printf("This Common code area: pid=%d\n", getpid());
}

/*----------------------------------------------- Split Line -------------------------------------------------*/

#include <sched.h>
int set_realtime_sched()
{
    struct sched_param param;
    int maxpri = sched_get_priority_max(SCHED_FIFO); //»ñÈ¡×î´óÖµ

    if(maxpri == -1) {
        perror("sched_get_priority_max() failed");
        return -1;
    }

    param.sched_priority = maxpri;
    if (sched_setscheduler(getpid(), SCHED_FIFO, &param) == -1) //设置优先级
    { 
        perror("sched_setscheduler() failed"); 
        return -1;
    }

    return 0;
}

/*----------------------------------------------- Split Line -------------------------------------------------*/

static inline long get_timestamp(int units)
{
    struct timeval cctv;
    gettimeofday(&cctv, NULL);

    long timestamp = -1;

    if (0 == units) {
        timestamp = cctv.tv_sec + cctv.tv_usec / 1000 / 1000; // sec
    } else if (1 == units) {
        timestamp = cctv.tv_sec / 1000 + cctv.tv_usec * 1000; // msec
    } else {
        timestamp = cctv.tv_sec * 1000 * 1000 + cctv.tv_usec; // usec
    }

    return timestamp;
}

/*=============================================== Split Line =================================================*/

void uart_debug_tx(int fd)
{

    char buf[1] = {0x55};
    int  len = sizeof(buf);
    int  ret = uart_send(fd, 0, len, buf);

    long cnt = 0;
    char chr = 0;

    long curr_time = get_timestamp(0);

    while(1)
    {
        ret = uart_send(fd, 0, sizeof(chr), &chr);
        printf("@@@@---%ld, chr=%02X, ret=%d---@@@@\n", cnt++, (unsigned char)chr++, ret);
        // usleep(10000);

        if(60 < (get_timestamp(0) - curr_time)) break;
    }
}

void uart_debug_rx(int fd)
{
    int res = -1;
    char buf[MAX_BUFFER_SIZE];

    long start_time = get_timestamp(0);

    while(1)
    {
        res = uart_recv_select(fd, 5000, sizeof(buf), buf);

        for(int i=0; i<res; i++)
            printf(" %02X ", buf[i]);

        printf("start=%ld, now=%ld, res=%d\n", start_time, get_timestamp(0), res);

        if(60 < (get_timestamp(0) - start_time)) break;
    }
}

void uart_debug(int argc, char *argv[])
{
    /* 打开并初始化串口 */
#if   UART_RD_WR_FLAG == 1 || 2 == UART_RD_WR_FLAG
#define UART_DEV_NAME "/dev/ttyUSB1"
#elif UART_RD_WR_FLAG == 0
#define UART_DEV_NAME "/dev/ttyUSB0"
#else
#undef UART_RD_WR_FLAG
#endif

#ifdef UART_RD_WR_FLAG
    #define UART_BAUDRATE 115200

    char *dev_name = UART_DEV_NAME;
    long  baudrate = UART_BAUDRATE;

    if(argc > 1) dev_name = argv[1];
    if(argc > 2) sscanf(argv[2], "%li", &baudrate);

    bb_signals(BB_FATAL_SIGS, exit_signal_handlr);

    uart_fd = uart_init(dev_name, baudrate, 'N', 8, 1, 0);

#if   UART_RD_WR_FLAG == 0
    uart_debug_rx(uart_fd);
#elif UART_RD_WR_FLAG == 1
    uart_debug_tx(uart_fd);
#elif UART_RD_WR_FLAG == 2
    multi_pthread();
#else
    multi_process();
#endif

    uart_uninit(uart_fd);
#endif
}

int main(int argc, char *argv[])
{
    uart_debug(argc, argv);

    return 0;
}

/* Complie: gcc uart.c -DUART_RD_WR_FLAG=0 -o rx_uart; gcc uart.c -DUART_RD_WR_FLAG=1 -o tx_uart
   Execute: ./rx_uart "/dev/pts/0" 115200;             ./tx_uart "/dev/pts/1" 115200
 */
/*************************************************************************************************************
* Date         Version    Author        Abstact
*-------------------------------------------------------------------------------------------------------------
* 2019-09-05   V1.0       Kevin Zhou    Initial version create
*************************************************************************************************************/
