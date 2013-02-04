/* 

   Author: Scott Brookes
   Date  : 1.25.13

 */

#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include "rxfe.h"
#include "dio_user_defs.h"

int fmode;

struct RXFESettings if_settings;
struct RXFESettings rf_settings;

/*

  Given a pointer to the RXFE Settings desired, This 
  function generates the appropriate "address".

  Although the implementation has deviated from the 
  original version of the function on the QNX6 platform, 
  it has been tested with a variety of inputs such that 
  the functionality has been confirmed to be equal.

 */
unsigned char build_rxfe_addr(struct RXFESettings *s) {

  char addr;
  
  addr = NEW_RXFE_ADDR;

  /* apply all necessary masks */
  /* macros defined in rxfe.h  */
  addr = ATT1_PART(s->att1, addr);
  addr = ATT2_PART(s->att2, addr);
  addr = ATT3_PART(s->att3, addr);
  addr = ATT4_PART(s->att4, addr);
  addr = AMP1_PART(s->amp1, addr);
  addr = AMP2_PART(s->amp2, addr);
  addr = AMP3_PART(s->amp3, addr);
  addr = MODE_PART(s->ifmode, addr);

  return addr;
  
}

/* 

   Function used to actually write the appropriate
   settings to the RXFE...

 */
int set_rxfe_addr(int dev, struct RXFESettings *s) {

  unsigned char write_message[DIO_MSG_SIZE];

  write_message[DIO_MSG_DATA] = build_rxfe_addr(s);
  write_message[DIO_MSG_PORT] = PORT_B;
  
  write(dev, write_message, DIO_MSG_SIZE);

  return 0;
}

/*

  Restore standard settings for the RF mode

 */
void set_standard_rxfe_rf_settings(void) {

  rf_settings.amp1 = 0;
  rf_settings.amp2 = 0;
  rf_settings.amp3 = 0;
  rf_settings.att1 = 1;
  rf_settings.att2 = 1;
  rf_settings.att3 = 1; 
  rf_settings.att4 = 1;
  rf_settings.ifmode = 0;
  
  return;
} /* end set_standard_rxfe_rf_settings */

/*

  Restore standard settings for the IF mode

 */
void set_standard_rxfe_if_settings(void) {

  if_settings.amp1 = 0;
  if_settings.amp2 = 0;
  if_settings.amp3 = 0;
  if_settings.att1 = 1;
  if_settings.att2 = 1;
  if_settings.att3 = 1; 
  if_settings.att4 = 1;
  if_settings.ifmode = 1;

  return;
} /* end set_standard_rxfe_if_settings */

/*

  restores both RF and IF standard settings

 */
void set_standard_rxfe_settings(void) {

  set_standard_rxfe_if_settings();
  set_standard_rxfe_rf_settings();

  return;

} /* end set_standard_rxfe_settings */

/* 

   Given pointers to new structures, replace the global
   settings with the new ones. If NULL, do not change
   the corresponding argument.

 */
void set_new_rxfe_settings(struct RXFESettings *iF, 
			   struct RXFESettings *rF) {

  if (iF) {
    if_settings.amp1 = iF->amp1;
    if_settings.amp2 = iF->amp2;
    if_settings.amp3 = iF->amp3;
    if_settings.att1 = iF->att1;
    if_settings.att2 = iF->att2;
    if_settings.att3 = iF->att3;
    if_settings.att4 = iF->att4;
    if_settings.ifmode = iF->ifmode;
  }

  if (rF) {
    rf_settings.amp1 = rF->amp1;
    rf_settings.amp2 = rF->amp2;
    rf_settings.amp3 = rF->amp3;
    rf_settings.att1 = rF->att1;
    rf_settings.att2 = rF->att2;
    rf_settings.att3 = rF->att3;
    rf_settings.att4 = rF->att4;
    rf_settings.ifmode = rF->ifmode;
  }

} /* end set_new_rxfe_standards */

/*

  Write the correct set of settings to the RXFE

 */
int export_settings_to_rxfe(int rxfe) {
 
  int err = 0;
 
  if (fmode)
    err = set_rxfe_addr(rxfe, &if_settings);
  else
    err = set_rxfe_addr(rxfe, &rf_settings);

  return err;
} /* end export_settings_to_rxfe */

/*

  set a new ifmode

 */
void set_if_mode(int new_mode) {

  fmode = new_mode;

  return;
} /* end set_if_mode */
