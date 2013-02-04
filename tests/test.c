#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include "dio_user_defs.h"

int dev[DIO_NUM_GROUPS];

int get_input(void) {

  int grp, i, single = 0;
  unsigned char temp[DIO_MSG_SIZE];
  char in[20];

  fputs("input group (0-4) > ", stdout);
  fflush(stdout);
  
  fgets(in, 20, stdin);
  if (in[0] == 'q')
    return -9;
  grp = atoi(&in[0]);
  if (!grp && in[0] != '0')
    return 1;
  memset(in, '\0', 20);

  if (grp < 0 || grp > 4)
    return 1;

  fputs("input port (a,b,c,d)  > ", stdout);
  fflush(stdout);

  fgets(in, 20, stdin);
  memset(&in[1], '\0', 19);

  switch(in[0]) {
  case 'a':
    temp[DIO_MSG_PORT] = 0x00;
    break;
  case 'b':
    temp[DIO_MSG_PORT] = 0x01;
    break;
  case 'c':
    temp[DIO_MSG_PORT] = 0x02;
    break;
  case 'd':
    temp[DIO_MSG_PORT] = 0x12;
    break;
  case 'q':
    return -3;
  default:
    return 1;
  }
  
  fputs("input hex data > ", stdout);
  fflush(stdout);

  temp[DIO_MSG_DATA] = 0x00;
  fgets(in, 20, stdin);

  for (i = 1; i >= 0; i--) {

    switch (in[i]) {

    case '0':
      temp[DIO_MSG_DATA] |= (0x00 >> i*4);
      break;
    case '1':
      temp[DIO_MSG_DATA] |= (0x10 >> i*4);
      break;
    case '2':
      temp[DIO_MSG_DATA] |= (0x20 >> i*4);
      break;
    case '3':
      temp[DIO_MSG_DATA] |= (0x30 >> i*4);
      break;
    case '4':
      temp[DIO_MSG_DATA] |= (0x40 >> i*4);
      break;
    case '5':
      temp[DIO_MSG_DATA] |= (0x50 >> i*4);
      break;
    case '6':
      temp[DIO_MSG_DATA] |= (0x60 >> i*4);
      break;
    case '7':
      temp[DIO_MSG_DATA] |= (0x70 >> i*4);
      break;
    case '8':
      temp[DIO_MSG_DATA] |= (0x80 >> i*4);
      break;
    case '9':
      temp[DIO_MSG_DATA] |= (0x90 >> i*4);
      break;
    case 'A':
    case 'a':
      temp[DIO_MSG_DATA] |= (0xa0 >> i*4);
      break;
    case 'B':
    case 'b':
      temp[DIO_MSG_DATA] |= (0xb0 >> i*4);
      break;
    case 'C':
    case 'c':
      temp[DIO_MSG_DATA] |= (0xc0 >> i*4);
      break;
    case 'D':
    case 'd':
      temp[DIO_MSG_DATA] |= (0xd0 >> i*4);
      break;
    case 'E':
    case 'e':
      temp[DIO_MSG_DATA] |= (0xe0 >> i*4);
      break;
    case 'F':
    case 'f':
      temp[DIO_MSG_DATA] |= (0xf0 >> i*4);
      break;
    case 'q':
      return -1;
    default:
      if (i) {
	single = 1;
	continue;
      }
      return 1;
    }
    
    if (single)
      temp[DIO_MSG_DATA] >>= 4;
    
  }
  
  memset(in, '\0', 20);

  write(dev[grp], temp, 2);
  
  return 0;
}

int main(void) { 
  
  int grp, bad = 0;
  unsigned char write_msg[2];

  dev[0] = open("/dev/dio0", O_RDWR);
  dev[1] = open("/dev/dio1", O_RDWR);
  dev[2] = open("/dev/dio2", O_RDWR);
  dev[3] = open("/dev/dio3", O_RDWR);
  dev[4] = open("/dev/dio4", O_RDWR);

  for(grp = 0; grp < DIO_NUM_GROUPS; grp++) {
    if (!dev[grp]) {
      printf("Well that went badly for dev %d\n", grp);
      bad = 1;
    }
  }
  
  if (bad) 
    return 2;

  /* config */
  write_msg[DIO_MSG_PORT] = PORT_CNTRL;
  write_msg[DIO_MSG_DATA] = CONTROL_BASIC;
  for (grp = 0; grp < DIO_NUM_GROUPS; grp++)
    write(dev[grp], write_msg, DIO_MSG_SIZE);

  while (1) {
    
    if (get_input() < 0)
      break;

  }

  for(grp = 0; grp < 5; grp++)
    close(dev[grp]);

  return 0;
}
