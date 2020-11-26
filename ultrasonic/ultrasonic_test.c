#include <unistd.h>
#include <fcntl.h>
#include <stdio.h>
#include <sys/ioctl.h>


#define ULTRASONIC_CMD_START		_IOW(0, 1, int)
#define ULTRASONIC_CMD_STOP			_IOW(0, 2, int)
#define ULTRASONIC_CMD_GET_DIS		_IOW(0, 5, int)


int main(int argc, const char *argv[])
{
    int fd;
    int ret;
    long dis_mm;
    long count = 100;
 
    fd = open("/dev/Ultrasonic", O_RDWR | O_NONBLOCK);
    if(fd < 0) {
        perror("open:");
        return -1;
    }
    printf("open successful, fd = %d\n",fd);
    
    ret = ioctl(fd,ULTRASONIC_CMD_START,NULL);
    if(ret < 0) {
		perror("ioctl-START:");
        return -1;
    }

	while(count--) {
		ret = ioctl(fd,ULTRASONIC_CMD_GET_DIS,&dis_mm);
		if(ret < 0){
			perror("ioctl-GET_DIS:");
			return -1;
		}
 
		printf("dis:%ld\n", dis_mm);
		sleep(1);
	}

	ret = ioctl(fd,ULTRASONIC_CMD_STOP,NULL);
    if(ret < 0) {
		perror("ioctl-STOP:");
        return -1;
    }
	close(fd);

    return 0;
}