#include <stdio.h>
#include <unistd.h>
#include <time.h>
#include "pmat.h"
#include "dio_user_defs.h"

/*

  A file to deal with the phasing matrices... that includes
  picking a card to address and a beam. This is part of the
  suite of userland software to support the function of the 
  ACCESS-DIO-120 card as it supports the SuperDARN Radar 
  arrays, being developed in an attempt to port the QNX
  based hardware-control to a Linux system.

  PMAT = Phasing Matrix

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

  unsigned char write_msg[DIO_MSG_SIZE], read_msg[DIO_MSG_SIZE];
  int address, check, pmat;
  
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

  /* legacy code had a brief pause here but */
  /* testing has suggested it is not needed */

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

  return 0;
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

    if ( i < (n / 2) ) /* least significant half of "in" */
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
       the appropriate beam code. It writes the 
       generated code to the appropriate PMAT.

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

  int pmat, beam_code, beam_num, check = 0;
  unsigned char write_msg[DIO_MSG_SIZE], read_msg[DIO_MSG_SIZE];

  /* retrieve beam number */
  beam_num = client->tbeam;

  /* beam number sanity check */
  if ((beam_num > MAX_BEAM) || (beam_num < 0)) {
    fprintf(stderr, "Invalid beam number 0 %d\n", beam_num);

    /* default to beam 0 */
    beam_num = 0;
  }

  /* define the radar being used */
  if (LEG_EAST_RADAR == client->radar) 
    pmat = dev[EPM0_GRP];
  else  /* client->radar == LEG_WEST_RADAR */
    pmat = dev[WPM0_GRP];

  /* generate beam code */
  beam_code = reverse_lsn_bits(beam_num, BEAM_CODE_SIZE);

  /* write code to pmat... */
  /* low 8 bits to port A  */
  write_msg[DIO_MSG_PORT] = PORT_A;
  write_msg[DIO_MSG_DATA] = beam_code & 0xff; 

  write(pmat, write_msg, DIO_MSG_SIZE);

  /* high 5 bits to port B */
  write_msg[DIO_MSG_PORT] = PORT_B;
  write_msg[DIO_MSG_DATA] = (beam_code & 0x1f00) >> 8;

  write(pmat, write_msg, DIO_MSG_SIZE);

  /* legacy code had a brief pause here but */
  /* testing has suggested it is not needed */

  /* verify output */
  read_msg[DIO_MSG_PORT] = PORT_A;
  read(pmat, read_msg, DIO_MSG_SIZE);

  check |= read_msg[DIO_MSG_DATA];  

  read_msg[DIO_MSG_PORT] = PORT_B;
  read(pmat, read_msg, DIO_MSG_SIZE);

  check |= read_msg[DIO_MSG_DATA] << 8;

  if ( check != beam_code) {
    fprintf(stderr, "Writing address in select_beam failed\n");
    return -2;
  }

  return 0;  
} /* end dio_select_beam function */
