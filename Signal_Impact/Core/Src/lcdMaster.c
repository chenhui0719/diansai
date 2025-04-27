#include "lcdMaster.h"


void lcdMaster_InitUI(){
    //绘制图框UI
	lcd_fill(0, 0, 480, 5, COLOR_CQU);
	lcd_fill(0, 141, 480, 146, COLOR_CQU);
	lcd_fill(0, 280, 480, 285, COLOR_CQU);
	lcd_fill(0, 286, 480, 320, COLOR_NEIE);
}

void lcdMaster_Clear(uint8_t lcdRoom){
	switch(lcdRoom){
	case lcdRoom_wave_t:
		lcd_fill(0, 6, 480, 140, BLACK);
		break;
	case lcdRoom_wave_f:
		lcd_fill(0, 147, 480, 279, BLACK);
		break;
	case lcdRoom_string:
		lcd_fill(0, 286, 359, 320, COLOR_NEIE);
		break;
	case lcdRoom_bf:
		lcd_fill(360, 286, 480, 320, COLOR_NEIE);
	default:
		break;
	}
}






