
#ifndef DEF_GUARD_DIO_CONTROL_H
#define DIO_GUARD_DIO_CONTROL_H

#include "rosmsg.h"
#include "dio_user_defs.h"

/* global device access */
extern int dev[DIO_NUM_GROUPS];

/* basic init and shutdown functions */
int init_dio_sys(int if_mode);
int shutdown_dio_sys(int err);

/* *** Functions replacing legacy switch statement *** */

/* implementation of legacy */
/*     case DIO_RXFE_RESET: */
int reset_rxfe_dio(void);

/* implementation of legacy */
/*  case DIO_RXFE_SETTINGS: */
int set_up_rxfe_dio(struct RXFESettings *iF, 
		    struct RXFESettings *rF);

#endif
