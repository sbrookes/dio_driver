/* 

   Author: Scott Brookes
   Date:   1.24.13

   This is a program being written in the effort to port
        the superDARN "Radar Operating System" from QNX
	to Linux. This program runs in user land and 
	translates the routine that a control program 
	would like to execute into the necessary DIO signals.
	The signals are actually sent by the DIO driver, 
	written earlier in this effort.

 */

#include <stdio.h> 
#include <unistd.h>
#include <stdlib.h>
#include <fcntl.h>
#include "dio_control.h"
#include "dio_user_defs.h"
#include "dio_tx_ops.h"
#include "rxfe.h"
#include "pmat.h"

int dev[DIO_NUM_GROUPS];

int init_dio_sys(int if_mode) {

  int err = 0, i;
  unsigned char write_msg[DIO_MSG_SIZE];

  /* open groups from DIO board */
  dev[EPM0_GRP] = open(EAST_PMAT0, O_RDWR);
  dev[EPM1_GRP] = open(EAST_PMAT1, O_RDWR);
  dev[WPM0_GRP] = open(WEST_PMAT0, O_RDWR);
  dev[WPM1_GRP] = open(WEST_PMAT1, O_RDWR);
  dev[RXFE_GRP] = open(RXFE, O_RDWR);

  for ( i = 0; i < DIO_NUM_GROUPS; i++) {
    if (!dev[i]) {
      fprintf(stderr, "DIO Group %d failed to open\n", i);
      err = -1;
    }
  }
  if (err)
    shutdown_dio_sys(err);

  /* initialize DIO board */
  write_msg[DIO_MSG_PORT] = PORT_CNTRL;

  /* all outputs */
  write_msg[DIO_MSG_DATA] = CONTROL_BASIC;

  write(dev[EPM0_GRP], write_msg, DIO_MSG_SIZE);
  write(dev[WPM0_GRP], write_msg, DIO_MSG_SIZE);
  write(dev[RXFE_GRP], write_msg, DIO_MSG_SIZE);

  /* A, B, and Clo Input */
  write_msg[DIO_MSG_DATA] = CONTROL_BASIC |
                            PORT_C_LO_IN  |
                            PORT_B_IN     |
                            PORT_A_IN     ;

  write(dev[EPM1_GRP], write_msg, DIO_MSG_SIZE);
  write(dev[WPM1_GRP], write_msg, DIO_MSG_SIZE);

  /* set up initial RXFE settings */
  set_standard_rxfe_settings();
  set_if_mode(if_mode);
  err = export_settings_to_rxfe();

  return err;

}

int shutdown_dio_sys(int err) {
  
  int i;
  
  for ( i = 0; i < DIO_NUM_GROUPS; i++ ) {
    if (dev[i])
      close(dev[i]);
  }

  exit(err);
  
}

/* case DIO_CtrlProg_READY */
/* *********************** */

/* case DIO_GET_TX_STATUS */
/* ********************** */

/* case DIO_PRETRIGGER */
/* ******************* */

/* case DIO_CLRFREQ */
int clear_freq_dio(struct ControlPRM *client) {

  int err = 0;
  
  err += dio_select_card(client);
  err += dio_select_beam(client);

  /* clear frequency search can only be done in RF mode */
  if (get_if_mode() == IF_MODE) {
    set_if_mode(RF_MODE);
    err += export_settings_to_rxfe();
    set_if_mode(IF_MODE);
  }
  
  return err;
}

/* case DIO_RXFE_RESET */
int reset_rxfe_dio(void) {

  return export_settings_to_rxfe();

}

/* case DIO_RXFE_SETTINGS */
int set_up_rxfe_dio(struct RXFESettings *iF, 
		    struct RXFESettings *rF) {
 
  set_new_rxfe_settings(iF, rF);
  return export_settings_to_rxfe();

}

/* 

   Main included here only for testing purposes

   later, the tcp driver or (eventually) the control
   program itself will compile with this file to 
   call these functions directly from its own
   main...

 */
int main(void) {

  int err = 0;

  err = init_dio_sys(IF_MODE);

  dio_select_tx(2, 3);

  shutdown_dio_sys(err);

  /* never reached */
  return err;
}
