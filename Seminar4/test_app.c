#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>

#define SIZE 100

int main()
{
    char input[SIZE] = "Test string sent from user space app";
    char output[SIZE];

    printf("initially output is: %s\n", output);
    int fd = open("/dev/test_mod01", O_RDWR);

    if(fd == -1){
        printf("ERROR\n");
        return -1;
    }

    printf("Start write:\n %s\n\n", input);
    int ret = write(fd,input,SIZE);

    if(ret < 0){
        printf("WRITE ERROR\n");
        return -1;
    }

    printf("Reading from device:\n");
    for(int i=0; i<SIZE; i+=10){
        memset(output, 0, strlen(output));
        read(fd, output, 10);
        printf("%s\n", output);
    }

    close(fd);
    return 0;
}