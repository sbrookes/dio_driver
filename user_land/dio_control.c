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
#include "control_program.h"
#include "dio_user_defs.h"

