#include <Bela.h>
#include <stdlib.h>

#include "usb_backpack.h"

float *ch0, *ch1;
char port[] = "/dev/ttyACM0";

display_20x4_lcd *lcd;

AuxiliaryTask SerialLcdFuncs;
void *arg;

void SerialLCD_BG_Task(void*)
{
	while(!gShouldStop)
	{
		lcd_level_meter_write(lcd, 2);
		usleep(2000);
		lcd_level_meter_write(lcd, 3);
		usleep(2000);
		lcd_level_meter_write(lcd, 4);
		usleep(2000);
		lcd_level_meter_write(lcd, 5);
		usleep(20000);

	}
}

bool setup(BelaContext *context, void *userData)
{

	ch0 = (float*) malloc(sizeof(float)*context->audioFrames);
	ch1 = (float*) malloc(sizeof(float)*context->audioFrames);
	
	for(int i = 0; i < context->audioFrames; i++)
	{
		ch0[i] = 0.0;
		ch0[i] = 0.0;
	}
	
	lcd = make_20x4_lcd(lcd, port, false, context->audioSampleRate);
	lcd_color(lcd, 180, 0, 250);
	
	SerialLcdFuncs= Bela_createAuxiliaryTask(&SerialLCD_BG_Task, 50, "bela-serial-lcd", arg);
	Bela_scheduleAuxiliaryTask(SerialLcdFuncs);
	return true;
}

void render(BelaContext *context, void *userData)
{

	for(unsigned int n = 0; n < context->audioFrames; n++) 
	{
		 ch0[n] = audioRead(context, n, 0);
		 ch0[n] *= 2.0;
		 ch1[n] = audioRead(context, n, 1);
		 ch1[n] *= 3.0;
		 
	}
	lcd_level_meter(lcd, ch0, context->audioFrames, 4);
	lcd_level_meter(lcd, ch1, context->audioFrames, 2);
	
	for(unsigned int n = 0; n < context->audioFrames; n++) 
	{
		ch0[n] *= 2.0;
		ch1[n] *= 3.0;
		 audioWrite(context, n, 0, ch0[n]);
		 audioWrite(context, n, 1, ch1[n]);
	}
	lcd_level_meter(lcd, ch0, context->audioFrames, 5);
	lcd_level_meter(lcd, ch1, context->audioFrames, 3);

}

void cleanup(BelaContext *context, void *userData)
{

}