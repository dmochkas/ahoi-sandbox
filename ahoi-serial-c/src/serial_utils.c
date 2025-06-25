#include "../include/ahoi_serial.h"
#include <fcntl.h>
#include <termios.h>
#include <unistd.h>

int open_serial_port(const char *port, int baudrate) {
    int fd = open(port, O_RDWR | O_NOCTTY | O_NDELAY);
    if (fd == -1) {
        perror("Error opening serial port");
        return -1;
    }

    struct termios options;
    tcgetattr(fd, &options);

    cfsetispeed(&options, baudrate);
    cfsetospeed(&options, baudrate);
    options.c_cflag &= ~PARENB;
    options.c_cflag &= ~CSTOPB;
    options.c_cflag &= ~CSIZE;
    options.c_cflag |= CS8;
    options.c_cflag |= (CLOCAL | CREAD);
    options.c_lflag &= ~(ICANON | ECHO | ECHOE | ISIG);
    options.c_iflag &= ~(IXON | IXOFF | IXANY);
    options.c_oflag &= ~OPOST;

    tcsetattr(fd, TCSANOW, &options);
    return fd;
}