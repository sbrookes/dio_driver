#include "dio_user_defs.h"
#include <unistd.h>
#include <stdio.h>
#include <fcntl.h>
#include <time.h>

#define DURATION 30 /* duration of test in seconds */

void print(int spin);

int main(void) {

  int group[DIO_NUM_GROUPS], i, j, err = 0, dur = 0, spin = 0;

  unsigned char zero_byte = 0x00, full_byte = 0xff;
  unsigned char write_msg[DIO_MSG_SIZE], port[PORTS_PER_GROUP];

  struct timespec pause;
  pause.tv_sec  = 0;
  pause.tv_nsec = 500000000;

  /* access port offsets */
  port[0] = PORT_A;
  port[1] = PORT_B;
  port[2] = PORT_C_HI;
  port[3] = PORT_C_LO;

  /* open groups from DIO board */
  group[EPM0_GRP] = open(EAST_PMAT0, O_RDWR);
  group[EPM1_GRP] = open(EAST_PMAT1, O_RDWR);
  group[WPM0_GRP] = open(WEST_PMAT0, O_RDWR);
  group[WPM1_GRP] = open(WEST_PMAT1, O_RDWR);
  group[RXFE_GRP] = open(RXFE, O_RDWR);

  /* initialize DIO board */
  write_msg[DIO_MSG_PORT] = PORT_CNTRL;

  /* all outputs */
  write_msg[DIO_MSG_DATA] = CONTROL_BASIC;

  write(group[EPM0_GRP], write_msg, DIO_MSG_SIZE);
  write(group[EPM1_GRP], write_msg, DIO_MSG_SIZE);
  write(group[WPM0_GRP], write_msg, DIO_MSG_SIZE);
  write(group[WPM1_GRP], write_msg, DIO_MSG_SIZE);
  write(group[RXFE_GRP], write_msg, DIO_MSG_SIZE);

  for ( i = 0; i < DIO_NUM_GROUPS; i++ ) {
    if (!group[i]) {
      fprintf(stdout, "Device %d failed to open\n", i);
      err = -1;
    }
  }  

  if (err) 
    return err;

  fprintf(stdout, "Checking\n");
  fprintf(stdout, "->  ");
  fflush(stdout);

  while (1) {

    /* test routine */
    for ( i = 0; i < DIO_NUM_GROUPS; i++) {
      
      /* first clear lights */
      write_msg[DIO_MSG_DATA] = zero_byte;
      
      for ( j = 0; j < PORTS_PER_GROUP; j++) {
	write_msg[DIO_MSG_PORT] = port[j];
	write(group[i], write_msg, 2);
      }
    }

    nanosleep(&pause, NULL);
    print(spin++);

    for ( i = 0; i < DIO_NUM_GROUPS; i++) {
      
      /* first clear lights */
      write_msg[DIO_MSG_DATA] = full_byte;
      
      for ( j = 0; j < PORTS_PER_GROUP; j++) {
	write_msg[DIO_MSG_PORT] = port[j];
	write(group[i], write_msg, 2);
      }
    }
    
    nanosleep(&pause, NULL);
    print(spin++);

    /* run for 30 seconds */
    if (dur++ >= DURATION)
      break;
  } /* end while loop */
  
  
  fprintf(stdout, "\bDone!\n");
  fflush(stdout);

  for (i = 0; i < DIO_NUM_GROUPS; i++)
    close(group[i]);
  
  return 0;

} /* end main */

void print(int spin) {

    switch (spin%4) {
    case 0: 
      fprintf(stdout, "\b|");
      fflush(stdout);
      break;
    case 1:
      fprintf(stdout, "\b/");
      fflush(stdout);
      break;
    case 2: 
      fprintf(stdout, "\b-");
      fflush(stdout);
      break;
    case 3: 
      fprintf(stdout, "\b\\");
      fflush(stdout);
      break;
    }

    return;

} /* end print */
