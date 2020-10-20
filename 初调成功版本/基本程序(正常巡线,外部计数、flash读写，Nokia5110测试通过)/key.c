#include "key.h"
#include "nokia_5110.h"
#include "STC12C5A60S2.H"   

sbit change_line = P1^0;	
sbit Add_1 = P1^1;	
sbit Sub_1 = P1^2; 

extern char servo_P;   //PIDϵ��
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



/*********************************�޸ı�����ֵ***********************************/
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
                     ����ɨ�躯��
*****************************************************/

void KeyScan (void)
{
 unsigned char num=0;                  
 			 	

  if(~change_line)  //�����⵽�͵�ƽ��˵����������
    {
	  LCD_DLY_ms(8); //��ʱȥ����һ��10-20ms
     if(~change_line)     //�ٴ�ȷ�ϰ����Ƿ��£�û�а������˳�
	     {
       while(~change_line);//���ȷ�ϰ��°����ȴ������ͷţ�û���ͷ���һֱ�ȴ�
	     
//	     if(page_num!=0)
	        LCD_write_english_string(0,line_num," ");
	           
		   if(line_num<2)    //����żӲ���    NOKIA�����ʾ6�У�0~5			   	
	         line_num++;
		   else
		       line_num=0;
		   
//		   if(page_num!=0)
	        LCD_write_english_string(0,line_num,"*");
	     } 
	     
   }
	  
////////////////////////��Ӧ������һ///////////////////////////////////////////////////////
if(~Add_1)  //�����⵽�͵�ƽ��˵����������
    {
	 LCD_DLY_ms(8); //��ʱȥ����һ��10-20ms
     if(~Add_1)     //�ٴ�ȷ�ϰ����Ƿ��£�û�а������˳�
	   {
        while(~Add_1);//���ȷ�ϰ��°����ȴ������ͷţ�û���ͷ���һֱ�ȴ�
        change_value(line_num,1);  
	   }
	  }
/////////////////////////////////��Ӧ������һ///////////////////////////////////////////////
if(~Sub_1)  //�����⵽�͵�ƽ��˵����������
   {
  	 LCD_DLY_ms(8);   //��ʱȥ����һ��10-20ms
     if(~Sub_1)     //�ٴ�ȷ�ϰ����Ƿ��£�û�а������˳�
	   {
        while(~Sub_1);//���ȷ�ϰ��°����ȴ������ͷţ�û���ͷ���һֱ�ȴ�
        change_value(line_num,-1);
	 	 }
	  }
	  }