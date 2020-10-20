#ifndef __FLASH_H__
#define __FLASH_H__

unsigned char read(unsigned char adder_h,unsigned char adder_l);
void write(unsigned char adder_h,unsigned char adder_l,unsigned char dat);
void del(unsigned char adder_h,unsigned char adder_l);

void delay_ms(unsigned int ms);
#endif