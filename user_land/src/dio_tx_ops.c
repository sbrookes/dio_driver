#include <stdio.h>
#include "dio_tx_ops.h"
#include "pmat.h"
#include "dio_user_defs.h"

int dio_get_tx_status(int radar, struct tx_status *txstatus) {

  int pmat;

  /* define the radar being used */
  if (LEG_EAST_RADAR == radar) 
    pmat = dev[EPM1_GRP];
  else  /* client->radar == LEG_WEST_RADAR */
    pmat = dev[WPM1_GRP];

  return 0;
} /* end dio_get_tx_status */
