#ifndef DEF_GUARD_DIO_TX_OPS_H
#define DEF_GUARD_DIO_TX_OPS_H

#include "dio_user_defs.h"
#include "rosmsg.h"

extern int dev[DIO_NUM_GROUPS];

int dio_get_tx_status(int radar, struct tx_status *txstatus);
int dio_select_tx(int radar, int address);

#endif
