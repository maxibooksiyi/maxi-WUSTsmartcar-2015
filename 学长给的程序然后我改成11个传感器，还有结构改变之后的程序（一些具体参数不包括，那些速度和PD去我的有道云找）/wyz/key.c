#include "key.h"
#include "nokia_5110.h"
#include "STC12C5A60S2.H"   

sbit change_line = P1^4;	
sbit Add_1 = P1^5;	
sbit Sub_1 = P1^6; 

extern char line_speed;
extern char bend_speed;
extern unsigned char differ_number;
extern char line_servo_p;
extern char line_servo_d;
extern char bend_servo_p;
extern char bend_servo_d;

						
			

unsigned char line_num;

void pre_show(void)
{
  
       LCD_write_english_string(0,0,"*");
       LCD_write_english_string(10,0,"L_V:");
	   LCD_write_char(65,0,line_speed);
	   
	   LCD_write_english_string(10,1,"B_V:");
	   LCD_write_char(65,1,bend_speed);
	   
	   LCD_write_english_string(10,2,"D_N:");
	   LCD_write_char(65,2,differ_number);
 
           
       LCD_write_english_string(10,3,"Line:");
	   LCD_write_english_string(55,3,"Bend:");
	   LCD_write_english_string(0,4,"P:");
	   LCD_write_english_string(0,5,"D:");


	   LCD_write_char(20,4,line_servo_p);      
	   LCD_write_char(65,4,bend_servo_p);
    
	   LCD_write_char(20,5,line_servo_d);
       LCD_write_char(65,5,bend_servo_d);

}


/*********************************修改变量数值***********************************/
void change_value(unsigned char m,int i)
{  

       switch(m)
       {
                case 0: line_speed+=i;       
               	LCD_clear_l(65,0);
                LCD_write_english_string(0,0,"*");
                LCD_write_char(65,0,line_speed); 
                break;

    	        case 1: bend_speed+=i;       
               	LCD_clear_l(65,1);
                LCD_write_english_string(0,1,"*");
                LCD_write_char(65,1,bend_speed); 
                break;

	            case 2: differ_number+=i;       
               	LCD_clear_l(65,2);
                LCD_write_english_string(0,2,"*");
                LCD_write_char(65,2,differ_number); 
                break;


				case 4: line_servo_p+=i;       
               	LCD_clear_l(10,4);
                LCD_write_english_string(10,4,"*");
                LCD_write_char(20,4,line_servo_p); 
                break;

			    case 5: bend_servo_p+=i;       
               	LCD_clear_l(55,4);
                LCD_write_english_string(55,4,"*");
                LCD_write_char(65,4,bend_servo_p); 
                break;

				case 6: line_servo_d+=i;       
               	LCD_clear_l(10,5);
                LCD_write_english_string(10,5,"*");
                LCD_write_char(20,5,line_servo_d); 
                break;

			    case 7: bend_servo_d+=i;       
               	LCD_clear_l(55,5);
                LCD_write_english_string(55,5,"*");
                LCD_write_char(65,5,bend_servo_d); 
                break;

				default: break;
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
	  LCD_DLY_ms(10); //延时去抖，一般10-20ms
     if(~change_line)     //再次确认按键是否按下，没有按下则退出
	     {
       while(~change_line);//如果确认按下按键等待按键释放，没有释放则一直等待
	        LCD_write_english_string(0,line_num," ");     
		   
		   if(line_num<7)    //行序号加操作    NOKIA最多显示6行，0~5
	         {
			 if(line_num==2) line_num=line_num+2;
			 else line_num++;
			 }
		   else
		       {line_num=0;LCD_clear();}
		if(line_num<=2)	  LCD_write_english_string(0,line_num,"*");
		else if(line_num==4) {LCD_clear_l(0,4);LCD_write_english_string(0,4,"P:");LCD_write_english_string(10,4,"*");LCD_write_char(20,4,line_servo_p);LCD_write_char(65,4,bend_servo_p);}
	    else if(line_num==5) {LCD_clear_l(0,4);LCD_write_english_string(0,4,"P:");LCD_write_english_string(55,4,"*");LCD_write_char(20,4,line_servo_p);LCD_write_char(65,4,bend_servo_p);}
		else if(line_num==6) {LCD_clear_l(0,5);LCD_write_english_string(0,5,"D:");LCD_write_english_string(10,5,"*");LCD_write_char(20,5,line_servo_d);LCD_write_char(65,5,bend_servo_d);}
		else                 {LCD_clear_l(0,5);LCD_write_english_string(0,5,"D:");LCD_write_english_string(55,5,"*");LCD_write_char(20,5,line_servo_d);LCD_write_char(65,5,bend_servo_d);}
		 } 
	   	   
   }
	  
////////////////////////对应参数加一///////////////////////////////////////////////////////
if(~Add_1)  //如果检测到低电平，说明按键按下
    {
	 LCD_DLY_ms(10); //延时去抖，一般10-20ms
     if(~Add_1)     //再次确认按键是否按下，没有按下则退出
	   {
        while(~Add_1);//如果确认按下按键等待按键释放，没有释放则一直等待
        change_value(line_num,1);  
	   }
	  }
/////////////////////////////////对应参数减一///////////////////////////////////////////////
if(~Sub_1)  //如果检测到低电平，说明按键按下
   {
  	 LCD_DLY_ms(10);   //延时去抖，一般10-20ms
     if(~Sub_1)     //再次确认按键是否按下，没有按下则退出
	   {
        while(~Sub_1);//如果确认按下按键等待按键释放，没有释放则一直等待
        change_value(line_num,-1);
	 	 }
	  }
	  }