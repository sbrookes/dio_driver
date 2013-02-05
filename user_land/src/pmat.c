#include <stdio.h>
#include <unistd.h>
#include <time.h>
#include "pmat.h"
#include "dio_user_defs.h"

/*

  A file to deal with the phasing matrices... that includes
  picking a card to address and a beam.

  Author: Scott Brookes
  Date: 2.4.13

*/

/*

  Select card function to pick a card from the phasing matrix
        based on the client program

  Note: all references within this function to "legacy" refer to 
        _select_file.c as part of SuperDARN's ROS6 QNX based
	software package

  Legacy Comment --
      This code selects a card to adddress. This can be used
      for writing data to the EEPROM, or to verify the output
      of the EEPROM. Tjere are 20 cards in the phasing matrix
      addresses 0-19. A card is addressed when this address
      corresponds to the switches on the phasing card. Card
      address 31 is reserved for programming purposes.

 */
int dio_select_card(struct ControlPRM *client) {

  int err = 0;
  unsigned char write_msg[2], read_msg[2];
  int address, check, pmat;

  /* pause structure */
  struct timespec pause;
  pause.tv_sec  = 0;
  pause.tv_nsec = 5000;
  
  /* start with a blank write message */
  write_msg[DIO_MSG_PORT] = 0x00;
  write_msg[DIO_MSG_DATA] = 0x00;

  /* and a zero check */
  check = 0;

  address = client->tbeam;  /* from legacy code */

  /* address sanity check */
  if ((address < 0 || address > 19) && address != 31) {
    fprintf(stderr, "Client delivered invalid card address %d", address);
    return -1;
  }

  /* define the radar being used */
  if (LEG_EAST_RADAR == client->radar) 
    pmat = dev[EPM0_GRP];
  else  /* client->radar == LEG_WEST_RADAR */
    pmat = dev[WPM0_GRP];
  
  /* must manipulate address based on expectations of the */
  /*      peripheral pmat hardware.                       */
  
  /* 
     EXPECTATIONS OF THE PMAT --
        
          Address is decimal 0-19     (5 bits, bit 5-1)
	  Write enable is binary 0-1  (1 bit , bit 0   )
     
  */

  /* mask out bits not used for addressing cards */
  address &= 0x1f; /* mask out lower 5 bits */ 

  /* write enable is lowest bit... shift left one */
  address <<= 1;

  /* only want to overwrite the address bits in port c */
  /* The top 2 address bits are in port cHI            */
  read_msg[DIO_MSG_PORT] = PORT_C_HI;
  read(pmat, read_msg, DIO_MSG_SIZE);

  /* now read_msg[data] is 0x0[portcHI] */
  address |= ((read_msg[DIO_MSG_DATA] & 0x0c) << 4);
  
  /* repeat procedure to preserve lowest bit of port cLO */
  read_msg[DIO_MSG_PORT] = PORT_C_LO;
  read(pmat, read_msg, DIO_MSG_SIZE);
  
  address |= (read_msg[DIO_MSG_DATA] & 0x01);

  /* now need to write these addresses to the ports */
  /* for C hi, need to isolate the appropriate nibble */
  write_msg[DIO_MSG_PORT] = PORT_C_HI;
  write_msg[DIO_MSG_DATA] = ((address >> 4) & 0x0f);

  write(pmat, write_msg, DIO_MSG_SIZE);

  /* for C lo, just want the lowest nibble */
  write_msg[DIO_MSG_PORT] = PORT_C_LO;
  write_msg[DIO_MSG_DATA] = address & 0x0f;

  write(pmat, write_msg, DIO_MSG_SIZE);

  /* carried from legacy code... unsure if the pause is necessary */
  /* nanosleep(&pause, NULL); */

  /* verify output */
  read_msg[DIO_MSG_PORT] = PORT_C_HI;
  read(pmat, read_msg, DIO_MSG_SIZE);

  check |= read_msg[DIO_MSG_DATA] << 4;  

  read_msg[DIO_MSG_PORT] = PORT_C_LO;
  read(pmat, read_msg, DIO_MSG_SIZE);

  check |= read_msg[DIO_MSG_DATA];

  if ( check != address) {
    fprintf(stderr, "Writing address in select_card failed\n");
    return -2;
  }

  return err;
} /* end dio_select_card function */

/*

  reverse least significant n bits

  implementation differs from legacy program but testing has shown
      common functionality

 */
int reverse_lsn_bits(int in, int n) {

  int out = 0, i, j, bit;

  /* for each bit */
  for ( i = 0; i < n; i++ ) {

    /* make mask for the bit of interest */
    for ( bit = 1, j = 0; j < i; j++)
      bit *= 2;

    if ( i < n / 2 ) /* least significant half of "in" */
      /* mask bit of interest and move left appropriate distance */
      /*      for 32 bits move 31, 29, 27, ... 5, 3, 1           */
      out |= ( in & bit ) << ( n - 1 - 2*i );  
    else  /* most significant half of "in" */
      /* mask bit of interest and move left appropriate distance */
      /*      for 32 bits move 1, 3, 5, ... 27, 29, 31           */
      out |= ( in & bit ) >> ( 2*i + 1 - n ); 

  }

  return out;
} /* end reverse_lsn_bits function */

/*

  Function used to, given a client program, generate 
       the appropriate beam code.

  Note that any comment in this function to "legacy"
       refers to the code from _select_beam.c in the
       SuperDARN ROS6 QNX software package.

  NOTE --> Legacy comment regarding beam number.

       the beam number is 4 bits. This number uses
       (lo) bits 5-6 of port B and (hi) bits 6-7 of 
       port C to output the beam number. Note: the beam
       number is used in addressing the old style
       phasing matrix

  NOTE --> Legacy comment regarding beam code

       The beam code is 13 bits, pAD0 thru pAD12. This 
       code uses bits 0-7 of Port A and bits 0-4 of port B
       to output the beam code. Note: The beam code is an
       address of the EEPROMS in the phasing cards. This code
       is broadcast to ALL phasing cards. If you are writing
       the EEPROM, then this be the beam code you are writing.

 */
int dio_select_beam(struct ControlPRM *client) {

  int err = 0, pmat, beam_num, lo_nib, hi_nib;
  double freq;

  /* retrieve frequency from client */
  freq = client->tfreq*1E3; /* line taken from legacy program */

  /* retrieve beam number */
  beam_num = client->tbeam;

  /* beam number sanity check */
  if ((beam_num > MAX_BEAM) || (beam_num < 0)) {
    fprintf(stderr, "Invalid beam number 0 %d\n", beam_num);

    /* default to beam 0 */
    beam_num = 0;
  }

  /* mask out hi and low sections */
  lo_nib = beam_num & 0x3; /* low  2 bits */
  hi_nib = beam_num & 0xc; /* high 2 bits */
  hi_nib >>= 2;

  /* define the radar being used */
  if (LEG_EAST_RADAR == client->radar) 
    pmat = dev[EPM0_GRP];
  else  /* client->radar == LEG_WEST_RADAR */
    pmat = dev[WPM0_GRP];

  return err;  
} /* end dio_select_beam function */
