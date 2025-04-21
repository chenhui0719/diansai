#ifndef INC_LCDMASTER_H_
#define INC_LCDMASTER_H_

#include "../../Drivers/BSP/LCD/lcd.h"

#define lcdRoom_wave_t 1
#define lcdRoom_wave_f 2
#define lcdRoom_string 3
#define lcdRoom_bf 4


void lcdMaster_InitUI();
void lcdMaster_Clear(uint8_t lcdRoom);



#endif /* INC_LCDMASTER_H_ */
