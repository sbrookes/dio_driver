#include "../dio_user_defs.h"
#include <unistd.h>
#include <stdio.h>
#include <fcntl.h>
#include <time.h>

int main(void) {

  int group[DIO_NUM_GROUPS], i, j, k, err = 0, bits, spin = 0;

  unsigned char data_byte = 0x01, zero_byte = 0x00, full_byte = 0xff;
  unsigned char write_msg[DIO_MSG_SIZE], port[PORTS_PER_GROUP];

  /* for pause between bit tests */
  struct timespec pause;
  pause.tv_sec  = 0;
  pause.tv_nsec = 250000000;

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

  /* check devices opened */
  for ( i = 0; i < DIO_NUM_GROUPS; i++ ) {
    if (!group[i]) {
      fprintf(stdout, "Device %d failed to open\n", i);
      err = -1;
    }
  }  
  if (err) 
    return err;

  /* first - zero the whole rig */
  for ( i = 0; i < DIO_NUM_GROUPS; i++) {

    write_msg[DIO_MSG_DATA] = zero_byte;

    for ( j = 0; j < PORTS_PER_GROUP; j++) {
      write_msg[DIO_MSG_PORT] = port[j];
      write(group[i], write_msg, 2);
    }
  }

  fprintf(stdout, "Checking\n");
  fprintf(stdout, "->  ");
  fflush(stdout);

  /* for each group */
  for ( i = 0; i < DIO_NUM_GROUPS; i++) {

    /* for each port */
    for ( j = 0; j < PORTS_PER_GROUP; j++) {

      /* address the port */
      write_msg[DIO_MSG_PORT] = port[j];
      
      /* determine 4 or 8 bit port */
      if ( j < PORTS_PER_GROUP/2)
	bits = 8;
      else 
	bits = 4;

      /* write 1 bit each time, moving it along */
      for (k = 0; k < bits; k++) {
	
	write_msg[DIO_MSG_DATA] = (data_byte << k);

	write(group[i], write_msg, 2);

	/* Fancy spinner output */
	switch(spin++%4) {
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
	
	/* pause */
	nanosleep(&pause, NULL);
      }
      
      /* zero port we just tested */
      write_msg[DIO_MSG_DATA] = zero_byte;
      write(group[i], write_msg, 2);

    }

    /* done, so write all high */ 
    write_msg[DIO_MSG_DATA] = full_byte;

    for ( j = 0; j < PORTS_PER_GROUP; j++) {
      write_msg[DIO_MSG_PORT] = port[j];
      write(group[i], write_msg, 2);
    }

  } /* end master for loop*/

  fprintf(stdout, "\bDone!\n");
  fflush(stdout);

  /* close devices */
  for (i = 0; i < DIO_NUM_GROUPS; i++)
    close(group[i]);
  
  return 0;

} /* end void */
