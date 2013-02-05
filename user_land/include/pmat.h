#ifndef DEF_GUARD_PMAT_H
#define DEF_GUARD_PMAT_H

#include "rtypes.h"
#include "rosmsg.h"
#include "dio_user_defs.h"

/* constants defined by legacy program */
#define LEG_EAST_RADAR 1
#define LEG_WEST_RADAR 2

/*
  This definition included multiple times with different 
      values in the QNX source. This one taken from 
      dio_tcp_driver/include/plx_defines.h 
 */
#define MAX_BEAM 23

/* retrieve global access to devices */
extern int dev[DIO_NUM_GROUPS]; 

int reverse_lsn_bits(int in, int n);
int dio_select_card(struct ControlPRM *client);
int dio_select_beam(struct ControlPRM *client);

#endif
