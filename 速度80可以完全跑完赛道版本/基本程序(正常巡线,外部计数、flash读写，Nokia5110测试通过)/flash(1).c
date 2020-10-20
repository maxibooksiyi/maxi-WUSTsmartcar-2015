#include "flash.h"
#include "STC12C5A60S2.H"
bit flag_2=0;//扇区字节读
void delay_ms(unsigned int ms)
{
    unsigned int a;
    while(ms)
    {
    a=1000;
    while(a--){};
    ms--;
    }
    return;
}

unsigned char read(unsigned char adder_h,unsigned char adder_l)
{
 unsigned char dat;
 IAP_ADDRH=adder_h;
 IAP_ADDRL=adder_l;
 IAP_CONTR=0x82;
 IAP_CMD=0x01;
 IAP_TRIG=0x5a;
 IAP_TRIG=0xa5;
// nop;
 dat=IAP_DATA;
 return dat;
}
//扇区字节写
void write(unsigned char adder_h,unsigned char adder_l,unsigned char dat)
{
 IAP_DATA=dat;
 IAP_ADDRH=adder_h;
 IAP_ADDRL=adder_l;
 IAP_CONTR=0x82;
 IAP_CMD=0x02;
 IAP_TRIG=0x5a;
 IAP_TRIG=0xa5;
// nop;
}
void del(unsigned char adder_h,unsigned char adder_l)
{
 IAP_ADDRH=adder_h;
 IAP_ADDRL=adder_l;
 IAP_CONTR=0x82;
 IAP_CMD=0x03;
 IAP_TRIG=0x5a;
 IAP_TRIG=0xa5;
}


