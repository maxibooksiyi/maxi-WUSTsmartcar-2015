#include "key.h"
#include "nokia_5110.h"
#include "STC12C5A60S2.H"   

sbit change_line = P1^0;	
sbit Add_1 = P1^1;	
sbit Sub_1 = P1^2; 

extern char servo_P;   //PID系数
extern char servo_D;
extern unsigned char servo_middle;
extern int frequent;


					
unsigned char line_num;

void pre_show(void)
{
  
       LCD_write_english_string(0,0,"*");
       LCD_write_english_string(10,0,"servo_P:");
	   LCD_write_char(65,0,servo_P);
	   
	   LCD_write_english_string(10,1,"servo_D:");
	   LCD_write_char(65,1,servo_D);

	   LCD_write_english_string(10,2,"servo_mid");
	   LCD_write_char(65,2,servo_middle);
	            
       LCD_write_english_string(10,5,"frequent");
	   LCD_write_char(65,5,frequent);  

}



/*********************************修改变量数值***********************************/
void change_value(unsigned char m,int i)
{  

       switch(m)
       {
       case 0: servo_P+=i;       
               	LCD_clear_l(65,0);
                LCD_write_english_string(0,0,"*");
                LCD_write_char(65,0,servo_P); 
                break;
                 
       case 1:servo_D+=i;
                 LCD_clear_l(65,1);
                 LCD_write_english_string(0,1,"*");
                 LCD_write_char(65,1,servo_D);
                 break;

	   case 2:servo_middle+=i;
                 LCD_clear_l(65,2);
                 LCD_write_english_string(0,2,"*");
                 LCD_write_char(65,2,servo_middle);
                 break;


       }
    
     }
  
/****************************************************
                     按键扫描函数
*****************************************************/

void KeyScan (void)
{
 unsigned char num=0;                  
 			 	

  if(~change_line)  //如果检测到低电平，说明按键按下
    {
	  LCD_DLY_ms(8); //延时去抖，一般10-20ms
     if(~change_line)     //再次确认按键是否按下，没有按下则退出
	     {
       while(~change_line);//如果确认按下按键等待按键释放，没有释放则一直等待
	     
//	     if(page_num!=0)
	        LCD_write_english_string(0,line_num," ");
	           
		   if(line_num<2)    //行序号加操作    NOKIA最多显示6行，0~5			   	
	         line_num++;
		   else
		       line_num=0;
		   
//		   if(page_num!=0)
	        LCD_write_english_string(0,line_num,"*");
	     } 
	     
   }
	  
////////////////////////对应参数加一///////////////////////////////////////////////////////
if(~Add_1)  //如果检测到低电平，说明按键按下
    {
	 LCD_DLY_ms(8); //延时去抖，一般10-20ms
     if(~Add_1)     //再次确认按键是否按下，没有按下则退出
	   {
        while(~Add_1);//如果确认按下按键等待按键释放，没有释放则一直等待
        change_value(line_num,1);  
	   }
	  }
/////////////////////////////////对应参数减一///////////////////////////////////////////////
if(~Sub_1)  //如果检测到低电平，说明按键按下
   {
  	 LCD_DLY_ms(8);   //延时去抖，一般10-20ms
     if(~Sub_1)     //再次确认按键是否按下，没有按下则退出
	   {
        while(~Sub_1);//如果确认按下按键等待按键释放，没有释放则一直等待
        change_value(line_num,-1);
	 	 }
	  }
	  }