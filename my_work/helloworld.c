#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <fcntl.h>
#include <termios.h>
#include <sys/ioctl.h>
#include <linux/serial.h>
#include <arpa/inet.h>
#include <signal.h>
#include <errno.h>

#define PORT 8080
#define SERIAL_PORT "/dev/ttyS3"
#define proce_num   5
#define msg_len     1024
#define send_buffer_len 3
char    rec_buffer[proce_num][msg_len] = {0};
int     idx  = 0;
char    send_buffer[send_buffer_len]   = {0};
int     udpSocket, serial_fd;
unsigned char start_sig[] = {0xb0, 0x07, 0x78, 0xb0, 0x00, 0x00, 0xc0, 0x00};
const char *key = "\"accx\":\"";  // 查找的键，包括冒号和引号，确保准确匹配


float find_accx(const char *input) {
    //printf("sb%s\r\n", input);
    //printf("sb1\r\n");
    char *start = strstr(input, key); // 在输入字符串中查找键
    //printf("sb2\r\n");
    float accx;
    if (start != NULL) {
        // 跳过键本身的长度，即到达值的开始位置
        start += strlen(key);
        // 解析浮点数，%f对应浮点数，%*[^\"], 跳过直到遇到引号的所有字符
        if (sscanf(start, "%f", &accx) == 1) {
            //printf("accx: %f\n", accx); // 如果成功解析，打印结果
        } else {
            //printf("Failed to parse accx.\n");
        }
    } else {
        //printf("Key 'accx' not found.\n");
    }
    return accx;
}

int open_my_serial(int * serial_port) {
    *serial_port = open(SERIAL_PORT, O_RDWR);

    // 检查串口是否打开成功
    if (*serial_port < 0) {
        //printf("Error %i from open: %s\n", errno, strerror(errno));
        return 1;
    }

    // 配置串口参数
    struct termios tty;
    memset(&tty, 0, sizeof(tty));

    if (tcgetattr(*serial_port, &tty) != 0) {
        //printf("Error %i from tcgetattr: %s\n", errno, strerror(errno));
        close(*serial_port);
        return 1;
    }

    tty.c_cflag &= ~PARENB; // 清除奇偶校验位
    tty.c_cflag &= ~CSTOPB; // 使用1个停止位
    tty.c_cflag |= CS8;     // 8位字符
    tty.c_cflag &= ~CRTSCTS; // 禁用RTS/CTS硬件流控制
    tty.c_cflag |= CREAD | CLOCAL; // 打开接收器，忽略调制解调器控制线

    tty.c_lflag &= ~ICANON;
    tty.c_lflag &= ~ECHO; // 禁用回显
    tty.c_lflag &= ~ECHOE; // 禁用回显擦除
    tty.c_lflag &= ~ECHONL; // 禁用换行回显
    tty.c_lflag &= ~ISIG; // 禁用解释特殊字符

    tty.c_iflag &= ~(IXON | IXOFF | IXANY); // 关闭软件流控制
    tty.c_iflag &= ~(IGNBRK | BRKINT | PARMRK | ISTRIP | INLCR | IGNCR | ICRNL);

    tty.c_oflag &= ~OPOST; // 防止特殊字符处理
    tty.c_oflag &= ~ONLCR; // 防止将换行转换为回车换行

    // // 设置输入和输出波特率
    // tty.c_cflag &= ~CBAUD;
    // tty.c_cflag |= BOTHER;
    // tty.__c_ispeed = 31250;
    // tty.__c_ospeed = 31250;

    // // 保存tty设置，同时清空输入和输出缓冲区
    // if (tcsetattr(*serial_port, TCSANOW, &tty) != 0) {
    //     //printf("Error %i from tcsetattr: %s\n", errno, strerror(errno));
    //     close(*serial_port);
    //     return 1;
    // }
        // 设置标准波特率为最接近的值，然后调整为精确值
    cfsetospeed(&tty, B38400);
    cfsetispeed(&tty, B38400);

    if (tcsetattr(*serial_port, TCSANOW, &tty) != 0) {
        perror("tcsetattr");
        return 1;
    }

    // 定制波特率设置
    struct serial_struct serial;
    if (ioctl(*serial_port , TIOCGSERIAL, &serial) == -1) {
        perror("ioctl TIOCGSERIAL");
        return 1;
    }

    serial.flags = (serial.flags & ~ASYNC_SPD_MASK) | ASYNC_SPD_CUST;
    serial.custom_divisor = (serial.baud_base + (31250 / 2)) / 31250;

    if (ioctl(*serial_port , TIOCSSERIAL, &serial) == -1) {
        perror("ioctl TIOCSSERIAL");
        return 1;
    }

    return 0;
}


int open_socket(int* udpSocket) {
    struct sockaddr_in serverAddr;
        // 创建UDP socket
    *udpSocket = socket(PF_INET, SOCK_DGRAM, 0);
    if (*udpSocket < 0) {
        //printf("Error %i from create socket: %s\n", errno, strerror(errno));
        return -1;
    }

    // 配置地址结构
    memset(&serverAddr, 0, sizeof(serverAddr));
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(PORT);
    serverAddr.sin_addr.s_addr = INADDR_ANY;

    // 绑定socket到地址
    if (bind(*udpSocket, (struct sockaddr *) &serverAddr, sizeof(serverAddr)) < 0) {
        //printf("Error %i from Bind failed: %s\n", errno, strerror(errno));
        close(*udpSocket);
        return -1;
    }

     // 设置非阻塞模式
    int flags = fcntl(*udpSocket, F_GETFL, 0);
    if (flags == -1) {
        perror("fcntl get failed");
        exit(EXIT_FAILURE);
    }
    flags |= O_NONBLOCK;
    if (fcntl(*udpSocket, F_SETFL, flags) != 0) {
        perror("fcntl set failed");
        exit(EXIT_FAILURE);
    }
    return 0;
}


int socket_recve_data(int* udpSocket, char* buffer) {
    struct sockaddr_in clientAddr;
    int recvLen;
    memset(&clientAddr, 0, sizeof(clientAddr));
    socklen_t addr_size = sizeof(clientAddr);
    recvLen = recvfrom(*udpSocket, buffer, msg_len, 0, (struct sockaddr *) &clientAddr, &addr_size);
    if (recvLen == -1) {
        if (errno == EWOULDBLOCK) {
            // //printf("No data available\n");
            return -1;
        } else {
            //printf("idx%d rec_buffer[*idx] %d\r\n",idx, sizeof(buffer));
            //printf("Error %i from bad receive: %s\n", errno, strerror(errno));
        }
    }
    buffer[recvLen] = '\0'; // 确保字符串以null终止
    //printf("rec bufffer len%d %s\r\n", recvLen, buffer);
    return 0;
}


void sigint_handler(int sig) {
    close(serial_fd);
    close(udpSocket);
    exit(0);
}

int main() {
    signal(SIGINT, sigint_handler);
    if (open_my_serial(&serial_fd) != 0) {
        //printf("open serial erro, return\r\n");
        return -1;
    }
    start_midi(serial_fd);
    send_buffer[0] = 0x90;
    send_buffer[1] = '-';
    send_buffer[2] = 'Z';
    if (open_socket(&udpSocket) != 0) {
        //printf("open serial erro, return\r\n");
        return -1;
    }
    while(1) {
        if (socket_recve_data(&udpSocket, rec_buffer[idx]) == 0) {
            idx++;
        }
        if (idx == proce_num) {
            float x, y;
                for (int i = 0; i < proce_num; i++) {
                x += find_accx(rec_buffer[i]);
            }
            send_buffer[1] = x;
            if(send_buffer[send_buffer_len - 1] & 0xF0 <= 0xB0) {
                send_buffer[send_buffer_len - 1] = 127;
            }
            idx = 0;
            if (write(serial_fd, send_buffer, sizeof(send_buffer)) < 0) {
                perror("Failed to write to the port");
            }   
        }
        sleep(0.01);
    }
    // 关闭socket和串行端口
    return 0;
}


void start_midi(int serial_fd) {
    if (write(serial_fd, start_sig, sizeof(start_sig)) < 0) {
        perror("Failed to begin midi");
    } 
}