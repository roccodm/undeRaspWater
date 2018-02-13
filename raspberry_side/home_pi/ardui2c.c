#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <linux/i2c-dev.h>
#include <sys/ioctl.h>

#define ADDRESS 0x04
#define DEVICE "/dev/i2c-1"


int main(int argc, char** argv) {
   if (argc != 2) {
      printf("Missing parameter. Aborting");
      exit(1);
   }
   int file;
   if ((file = open(DEVICE, O_RDWR)) < 0) {

      printf("Fatal: unable to device (do you have privileges?)\n");
      exit(1);
   }
   if (ioctl(file, I2C_SLAVE, ADDRESS) < 0) {
      printf("Fatal: unable to access i2c device  0x%x\n", ADDRESS);
      exit(1);
   }

   int r1,r2,i;
   r1=write(file, argv[1], strlen(argv[1]));
   usleep(10000);
   char buf[20];
   r2=read(file, buf, 20);
   for (i=0;i<20;i++){
      if (buf[i]==255){
         buf[i]='\0';
         break;
      }
   }
   printf("%s\t(%d,%d)\n",buf,r1,r2);
   usleep(10000);
   close(file);
   return (EXIT_SUCCESS);
}

