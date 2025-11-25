#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>  //added ioctl header

#define SIZE 100
#define BUF_SIZE 256

#define ECE_IOC_MAGIC   'E'     //ECE driver magic ASCII number
#define ECE_IOCTL_RST_R _IO(ECE_IOC_MAGIC, 0)   //reset read pointer
#define ECE_IOCTL_RST_W _IO(ECE_IOC_MAGIC, 1)    //reset write pointer 

static void print_menu(void) {
    printf("\n=== User Commands ===\n");
    printf("w  : write text to device\n");
    printf("r  : read text from device\n");
    printf("rr : [ioctl] reset READ pointer\n");
    printf("rw : [ioctl] reset WRITE pointer\n");
    printf("q  : quit\n");
    fflush(stdout);
}

int main()
{
    char input[100] = "Test string sent from user space app";
    char output[SIZE] = {0};
    char cmd[SIZE]; //command line input by user
    char buf[BUF_SIZE + 1];

    printf("initially output is: %s\n", output);
    int fd = open("/dev/test_mod01", O_RDWR);

    if(fd == -1){
        printf("ERROR\n");
        return -1;
    }
    print_menu();
    printf("Enter command: ");
    while(1){
    printf("Enter command: ");
        if(!fgets(cmd, sizeof(cmd), stdin)){
            perror("Error reading command");
            continue;
        }

        cmd[strcspn(cmd, "\n")] = '\0'; //end (\0) the string when \n is inputted into command line
        
        //quit
        if(strcmp(cmd, "q") == 0){
            printf("Closing Module\n");
            close(fd);
            break;
        } 
        //reset read pointer
        else if(strcmp(cmd, "rr") == 0){
            printf("[IOCTL] Resetting read pointer *\n");
            if (ioctl(fd, ECE_IOCTL_RST_R) < 0){
                perror("ioctl RST_R");
            }
        } 
        //output the string buffer  
        else if(strcmp(cmd, "r") == 0){
            printf("Reading from device...\n");
            for(int i=0; i<BUF_SIZE; i+=10){
                //memset(output, 0, strlen(output));
                ssize_t n = read(fd, output, 10);
                if (n <= 0){
                    break;
                }
                output[n] = '\0';
                printf("%s\n", output);
            }
        }
        //reset the write pointer
        else if(strcmp(cmd, "rw") == 0){        
        printf("[IOCTL] Resetting write pointer *\n");
        if (ioctl(fd, ECE_IOCTL_RST_W) < 0){
            perror("ioctl RST_W");
        }
        }   
        //write to device
        else if(strcmp(cmd, "w") == 0){
            printf("Write to device...\n");
            printf("Enter Text:\n");
            fflush(stdout);

            if (!fgets(buf, sizeof(buf), stdin)) {
                printf("Error reading user input\n");
                continue;
            }

            buf[strcspn(buf, "\n")] = '\0';

            // Save into output variable
            strcpy(output, buf);

            // Write to device starting at current ece_offset_w
            ssize_t ret = write(fd, output, strlen(output));
            if (ret < 0) {
                perror("write");
            }
        }

    }
    return 0;
}



    