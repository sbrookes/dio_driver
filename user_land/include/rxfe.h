
#ifndef DEF_GUARD_RXFE_H
#define DEF_GUARD_RXFE_H

#include "rosmsg.h"
#include "rtypes.h"
#include "dio_user_defs.h"

#define IF_MODE 1
#define RF_MODE 0

/* global access to devices */
extern int dev[DIO_NUM_GROUPS];

/* Function Prototypes */
unsigned char build_rxfe_addr(struct RXFESettings *s);
int set_rxfe_addr(struct RXFESettings *s);
void set_standard_rxfe_settings(void);
void set_standard_rxfe_if_settings(void);
void set_standard_rxfe_rf_settings(void);
void set_new_rxfe_settings(struct RXFESettings *iF, 
			   struct RXFESettings *rF);
int export_settings_to_rxfe(void);
void set_if_mode(int new_mode);
int  get_if_mode(void) ;

/* macros to build the rxfe addr */
/* 
   note that the addr is interpreted
   as follows

   bit     value       meaning
   0         0        att1   == 1
   1         0        att2   == 1
   2         0        att3   == 1
   3         0        att4   == 1
   4         1        amp1   == 1
   5         1        amp2   == 1
   6         1        ifmode == 1
   7         1        amp3   == 1
*/

#define NEW_RXFE_ADDR ((char)0x00)
#define ATT1_PART(bit,addr) (bit ? ( addr | 0x00 ) : ( addr | 0x01 ))
#define ATT2_PART(bit,addr) (bit ? ( addr | 0x00 ) : ( addr | 0x02 ))
#define ATT3_PART(bit,addr) (bit ? ( addr | 0x00 ) : ( addr | 0x04 ))
#define ATT4_PART(bit,addr) (bit ? ( addr | 0x00 ) : ( addr | 0x08 ))
#define AMP1_PART(bit,addr) (bit ? ( addr | 0x10 ) : ( addr | 0x00 ))
#define AMP2_PART(bit,addr) (bit ? ( addr | 0x40 ) : ( addr | 0x00 ))
#define AMP3_PART(bit,addr) (bit ? ( addr | 0x80 ) : ( addr | 0x00 ))
#define MODE_PART(bit,addr) (bit ? ( addr | 0x20 ) : ( addr | 0x00 ))

#endif
