#include "nokia_5110.h"
#include "code_table.h"
#include "STC12C5A60S2.H"   
sbit LCD_RST = P0^0;
sbit LCD_CE = P0^1;	
sbit LCD_DC = P0^2;	
sbit SDIN = P0^3;	
sbit SCLK = P0^4;   


void delay_1us(void)                 //1us延时函数
  {
   unsigned int i;
  for(i=0;i<500;i++);
  }

/*********************LCD 延时1ms************************************/
void LCD_DLY_ms(unsigned int ms)
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
/*-----------------------------------------------------------------------
LCD_init          : 3310LCD初始化

编写日期          ：2004-8-10 
最后修改日期      ：2004-8-10 
-----------------------------------------------------------------------*/
void LCD_init(void)
{

    LCD_RST = 0;// 产生一个让LCD复位的低电平脉冲
    delay_1us();   
    LCD_RST = 1;// 关闭LCD
    LCD_CE = 0;
    delay_1us();// 使能LCD
    LCD_CE = 1;
    delay_1us();
    
    LCD_write_byte(0x21, 0);	// 使用扩展命令设置LCD模式
    LCD_write_byte(0xb8, 0);	// 设置偏置电压
    LCD_write_byte(0x06, 0);	// 温度校正
    LCD_write_byte(0x13, 0);	// 1:48
    LCD_write_byte(0x20, 0);	// 使用基本命令
    LCD_clear();	        // 清屏
    LCD_write_byte(0x0c, 0);	// 设定显示模式，正常显示        
           
    LCD_CE = 0; // 关闭LCD
}

/*-----------------------------------------------------------------------
LCD_clear         : LCD清屏函数

编写日期          ：2004-8-10 
最后修改日期      ：2004-8-10 
-----------------------------------------------------------------------*/
void LCD_clear(void)
{
    unsigned int i;

    LCD_write_byte(0x0c, 0);			
    LCD_write_byte(0x80, 0);			

    for (i=0; i<504; i++)
      LCD_write_byte(0, 1);			
}

/*-----------------------------------------------------------------------
LCD_clear         : LCD清一行函数

编写日期          ：2014-9-19 
最后修改日期      ：2014-9-19 
-----------------------------------------------------------------------*/
void LCD_clear_l( unsigned char X,unsigned char Y)
{
    unsigned int i;

    LCD_write_byte(0x0c, 0);			
    LCD_write_byte(0x80, 0);			

    for (i=(Y-1)*84+X; i<Y*84; i++)
      LCD_write_byte(0, 1);			
}


/*-----------------------------------------------------------------------
LCD_set_XY        : 设置LCD坐标函数

输入参数：X       ：0－83
          Y       ：0－5
-----------------------------------------------------------------------*/
void LCD_set_XY(unsigned char X, unsigned char Y)
{
    LCD_write_byte(0x40 | Y, 0);		// column
    LCD_write_byte(0x80 | X, 0);          	// row
}
/*-----------------------------------------------------------------------
LCD_write_one: 显示英文字符

输入参数：c       ：显示的字符；
-----------------------------------------------------------------------*/
void LCD_write_one(unsigned char c)
{
    unsigned char line;
    for (line=0; line<6; line++)
      LCD_write_byte(font6x8[c-32][line], 1);
}

/*-----------------------------------------------------------------------
LCD_write_english_String  : 英文字符串显示函数

输入参数：*s      ：英文字符串指针；
          X、Y    : 显示字符串的位置,x 0-83 ,y 0-5		
-----------------------------------------------------------------------*/
void LCD_write_english_string(unsigned char X,unsigned char Y,char *s)
{
    LCD_set_XY(X,Y);
    while (*s) 
      {
    	 LCD_write_one(*s);
    	 s++;
      }
}
/*-----------------------------------------------------------------------
LCD_write_chinese_string: 在LCD上显示汉字

输入参数：X、Y    ：显示汉字的起始X、Y坐标；
          ch_with ：汉字点阵的宽度
          num     ：显示汉字的个数；  
          line    ：汉字点阵数组中的起始行数
          row     ：汉字显示的行间距
测试：
	LCD_write_chi(0,4,12,7,0,0);	
-----------------------------------------------------------------------*/                        
void LCD_write_chinese_string(unsigned char X, unsigned char Y, 
                   unsigned char ch_with,unsigned char num,
                   unsigned char line,unsigned char row)
{
    unsigned char i,n;
    
    LCD_set_XY(X,Y);                             //设置初始位置
    
    for (i=0;i<num;)
      {
      	for (n=0; n<ch_with*2; n++)              //写一个汉字
      	  { 
      	    if (n==ch_with)                      //写汉字的下半部分
      	      {
      	        if (i==0) LCD_set_XY(X,Y+1);
      	        else
      	           LCD_set_XY((X+(ch_with+row)*i),Y+1);
              }
      	    LCD_write_byte(write_chinese[line+i][n],1);
      	  }
      	i++;
      	LCD_set_XY((X+(ch_with+row)*i),Y);
      }
}
/*-----------------------------------------------------------------------
LCD_draw_map      : 位图绘制函数

输入参数：X、Y    ：位图绘制的起始X、Y坐标；
          *map    ：位图点阵数据；
          Pix_x   ：位图像素（长）
          Pix_y   ：位图像素（宽）
-----------------------------------------------------------------------*/
/*void LCD_draw_bmp_pixel(unsigned char X,unsigned char Y,unsigned char *map,
                  unsigned char Pix_x,unsigned char Pix_y)
{
    unsigned int i,n;
    unsigned char row;
    
    if (Pix_y%8==0) row=Pix_y/8;      //计算位图所占行数
      else
        row=Pix_y/8+1;
    
    for (n=0;n<row;n++)
      {
      	LCD_set_XY(X,Y);
        for(i=0; i<Pix_x; i++)
          {
            LCD_write_byte(map[i+n*Pix_x], 1);
          }
        Y++;                         //换行
      }      
} */

/*-----------------------------------------------------------------------
LCD_write_byte    : 使用SPI接口写数据到LCD

输入参数：data    ：写入的数据；
          command ：写数据/命令选择；
-----------------------------------------------------------------------*/
void LCD_write_byte(unsigned char dat, unsigned char command)
{
    unsigned char i;
    LCD_CE = 0;
    
    if (command == 0)
     LCD_DC = 0;
    else
     LCD_DC = 1;
		for(i=0;i<8;i++)
		{
			if(dat&0x80)
				SDIN = 1;
			else
				SDIN = 0;
			SCLK = 0;
			dat = dat << 1;
			SCLK = 1;
		}
     LCD_CE = 1;
}

/*-----------------------------------------------------------------------
LCD_write_char    : 显示字符数字

输入参数：c       ：显示的数字；
-----------------------------------------------------------------------*/
void LCD_write_number(unsigned char c)
{
    unsigned char line;

    for (line=0; line<6; line++)
      LCD_write_byte(font6x8[c+16][line], 1); //这里是  -32+0x30=16

}   
/*-----------------------------------------------------------------------
LCD_write_char: 显示char型数字；
-----------------------------------------------------------------------*/
void LCD_write_char(unsigned char X,unsigned char Y,unsigned char i)
{
    LCD_set_XY(X,Y);                             //设置初始位置
    LCD_write_number((char)(i/100%10));
    LCD_write_number((char)(i%100/10));
    LCD_write_number((char)(i%10));
} 
/*-----------------------------------------------------------------------
LCD_write_int: 显示int型数字；
-----------------------------------------------------------------------*/
void LCD_write_int(unsigned char X,unsigned char Y,int i)
{
    LCD_set_XY(X,Y);                             //设置初始位置
    LCD_write_number((char)(i/10000));
    LCD_write_number((char)(i/1000%10));
    LCD_write_number((char)(i/100%10));
    LCD_write_number((char)(i%100/10));
    LCD_write_number((char)(i%10));
}  
/*-----------------------------------------------------------------------
LCD_write_float: 显示float型数字；
-----------------------------------------------------------------------*/
void LCD_write_float(unsigned char X,unsigned char Y,float i)
{
    LCD_set_XY(X,Y);                             //设置初始位置
	LCD_write_number((int)i/100%10);
	LCD_write_number((int)i%100/10);
	LCD_write_number((int)i%10);
	LCD_write_one('.');
 	LCD_write_number((int)(i*10)%10);
  	LCD_write_number((int)(i*100)%10);
}

/***************功能描述：显示8*16一组int型变量	显示的坐标（x,y），y为页范围0～3****************/
void LCD_write_Hex(unsigned char X,unsigned char Y,unsigned char i)
{
    LCD_set_XY(X,Y);                             //设置初始位置

	if((i>>4)<10)
	{
		LCD_write_one((i>>4)+48);
	}
	else
	{
		LCD_write_one((i>>4)+55);
	}
	if((i & 0x0f)<10)
	{
		LCD_write_one((i & 0x0f)+48);
	}
	else
	{
		LCD_write_one((i & 0x0f)+55);
	}

}