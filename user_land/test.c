#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>

int main(void) { 
  
  int dev;
  //FILE *dev;
  unsigned char temp[2], iread[2];

  temp[0] = 0x01;
  temp[1] = 0xa7;

  iread[0] = 0x01;
  iread[1] = 0xff;

  dev = open("/dev/dio0", O_RDWR);

  if (!dev) {
    printf("well that went badly\n");
    return 2;
  }

  write(dev, temp, 2);

  read(dev, iread, 2);

  printf("read 0x%x%x\n", iread[0], iread[1]);

  close(dev);

  return 0;
}
