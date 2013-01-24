#ifndef _DIO_USER_DEFS_H
#define _DIO_USER_DEFS_H

/* 

   Header file to define macros for use in accessing 
   the ACCESS-120-DIO card from userland.

   Userland programming using the DIO driver for linux
   written by Scott Brookes should take care to use 
   linux syscalls open, read, write, and close directly
   rather than C library calls fopen, fread, fwrite and 
   fclose. 

 */

/* start with port offsets... defined previously in */
/*       the dio driver (kernel module) source      */
#define PORT_A     0x00
#define PORT_B     0x01
#define PORT_C_HI  0x02
#define PORT_C_LO  0x12
#define PORT_CNTRL 0x03

/* define masks for setting up control register settings */
/*
  As per documentation with the DIO card re: control byte
  
  b0    = Port C Lo      --> 1 = input, 0 = output
  b1    = Port B         --> 1 = input, 0 = output
  b2    = Mode Selection --> 1 = mode1, 0 = mode 0
  b3    = Port C Hi      --> 1 = input, 0 = output
  b4    = Port A         --> 1 = input, 0 = output
  b5/b6 = Mode selection --> 01 = mode1, 00 = mode0
  b7    = mode set flag  --> 1X = mode2, 1  = active
          & tristate                          & tristate

  Note that although this card supports buffered outputs, 
  they will not be used here.

  The basic control byte will configure a group for all 
  non-buffered outputs.

 */
#define CONTROL_BASIC    0x80
#define PORT_C_LO_IN     0x01
#define PORT_C_HI_IN     0x08
#define PORT_B_IN        0x02
#define PORT_A_IN        0x10

/* definitions to relate char devices to radar components */
/*     this is effectively choosing a group from the card */
#define RXFE       "/dev/dio4"
#define EAST_PMAT0 "/dev/dio0"
#define EAST_PMAT1 "/dev/dio1"
#define WEST_PMAT0 "/dev/dio2"
#define WEST_PMAT1 "/dev/dio3"

/* commands that the control program can send */
#define DIO_CNTRL_PROG_READY  '1'
#define DIO_GET_TX_STATUS     'S'
#define DIO_PRETRIGGER        '3'
#define DIO_CLRFREQ           'C'
#define DIO_RXFE_SETTINGS     'R'
#define DIO_RXFE_RESET        'r'

/* Note that the following are defined in old headers but  */
/*      are not implemented in the QNX version of this prg */
#define DIO_TRIGGER           '4'
#define DIO_CNTRL_PROG_END    '@'

#endif
