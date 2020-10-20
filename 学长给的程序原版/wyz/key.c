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


/*********************************�޸ı�����ֵ***********************************/
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
                     ����ɨ�躯��
*****************************************************/

void KeyScan (void)
{
 unsigned char num=0;                  
 if(~change_line)  //�����⵽�͵�ƽ��˵����������
    {
	  LCD_DLY_ms(10); //��ʱȥ����һ��10-20ms
     if(~change_line)     //�ٴ�ȷ�ϰ����Ƿ��£�û�а������˳�
	     {
       while(~change_line);//���ȷ�ϰ��°����ȴ������ͷţ�û���ͷ���һֱ�ȴ�
	        LCD_write_english_string(0,line_num," ");     
		   
		   if(line_num<7)    //����żӲ���    NOKIA�����ʾ6�У�0~5
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
	  
////////////////////////��Ӧ������һ///////////////////////////////////////////////////////
if(~Add_1)  //�����⵽�͵�ƽ��˵����������
    {
	 LCD_DLY_ms(10); //��ʱȥ����һ��10-20ms
     if(~Add_1)     //�ٴ�ȷ�ϰ����Ƿ��£�û�а������˳�
	   {
        while(~Add_1);//���ȷ�ϰ��°����ȴ������ͷţ�û���ͷ���һֱ�ȴ�
        change_value(line_num,1);  
	   }
	  }
/////////////////////////////////��Ӧ������һ///////////////////////////////////////////////
if(~Sub_1)  //�����⵽�͵�ƽ��˵����������
   {
  	 LCD_DLY_ms(10);   //��ʱȥ����һ��10-20ms
     if(~Sub_1)     //�ٴ�ȷ�ϰ����Ƿ��£�û�а������˳�
	   {
        while(~Sub_1);//���ȷ�ϰ��°����ȴ������ͷţ�û���ͷ���һֱ�ȴ�
        change_value(line_num,-1);
	 	 }
	  }
	  }