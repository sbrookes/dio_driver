#include <stdio.h>
#include <unistd.h>
#include "dio_tx_ops.h"
#include "pmat.h"
#include "dio_user_defs.h"

/* 

   File defines functions that interface with the transmitters
        (transmitter = tx) for the SuperDARN radar arrays.
	Code is being produced as part of an effort to port
	the current QNX-based Radar Operating System to a 
	fully linux-based system.

   Author: Scott Brookes
   date:   2.5.13

   NOTE -- Any reference to 'legacy' code in this file refers to
        the file _get_status.c under the DIO driver on SuperDARN's
	ROS6 QNX software package.

 */

/*

  Code to 'activate' a particular transmitter (tx) given
  the radar array and the transmitter's index.

  Really, this code formats the passed tx_addr and writes
  it to the pmat so that the peripheral hardware knows
  which transmitter we are interested in working with.

 */
int dio_select_tx(int radar, int tx_addr) {

  unsigned char write_msg[DIO_MSG_SIZE],
                 read_msg[DIO_MSG_SIZE];
  int pmat, check = 0;

  /* define the radar being used */
  if (LEG_EAST_RADAR == radar) 
    pmat = dev[EPM1_GRP];
  else  /* client->radar == LEG_WEST_RADAR */
    pmat = dev[WPM1_GRP];

  /* the tx_addr must be written to port C hi */
  write_msg[DIO_MSG_PORT] = PORT_C_HI;
  write_msg[DIO_MSG_DATA] = tx_addr;

  write(pmat, write_msg, DIO_MSG_SIZE);

  /* check for a valid write */
  /* legacy code pauses here */

  read_msg[DIO_MSG_PORT] = PORT_C_HI;  
  read(pmat, read_msg, DIO_MSG_SIZE);

  check |= read_msg[DIO_MSG_DATA];

  if (tx_addr != check ){
    fprintf(stderr, "Write failed in select_tx\n");
    return -2;
  }

  return 0;
} /* end dio_select_tx */

int dio_get_tx_status(int radar, struct tx_status *txstatus) {

  unsigned char read_msg[DIO_MSG_SIZE];
  int pmat, tx, err = 0;

  /* define the radar being used */
  if (LEG_EAST_RADAR == radar) 
    pmat = dev[EPM1_GRP];
  else  /* client->radar == LEG_WEST_RADAR */
    pmat = dev[WPM1_GRP];

  /* find TX statuses */
  read_msg[DIO_MSG_PORT] = PORT_C_LO;
  for ( tx = 0; tx < MAX_TRANSMITTERS; tx++ ) {
    
    /* select tx and check status */
    if ( !(err = dio_select_tx(radar, tx)) ) {
      
      read(pmat, read_msg, DIO_MSG_SIZE);
      
      /* legacy code sets precedent for assignments */
      txstatus->status[tx] = (int) read_msg[DIO_MSG_DATA];
      txstatus->AGC[tx] = (int) (read_msg[DIO_MSG_DATA] & 0x4) >> 2;
      txstatus->LOWPWR[tx] = (int) (read_msg[DIO_MSG_DATA] & 0x2) >> 1;

    }
    else 
      return err;
  }

  return 0;
} /* end dio_get_tx_status */
