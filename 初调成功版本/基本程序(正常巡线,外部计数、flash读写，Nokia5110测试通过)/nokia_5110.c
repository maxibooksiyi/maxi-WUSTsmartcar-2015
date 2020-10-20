#include "nokia_5110.h"
#include "code_table.h"
#include "STC12C5A60S2.H"   
sbit LCD_RST = P0^0;
sbit LCD_CE = P0^1;	
sbit LCD_DC = P0^2;	
sbit SDIN = P0^3;	
sbit SCLK = P0^4;   


void delay_1us(void)                 //1us��ʱ����
  {
   unsigned int i;
  for(i=0;i<500;i++);
  }

/*********************LCD ��ʱ1ms************************************/
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
LCD_init          : 3310LCD��ʼ��

��д����          ��2004-8-10 
����޸�����      ��2004-8-10 
-----------------------------------------------------------------------*/
void LCD_init(void)
{

    LCD_RST = 0;// ����һ����LCD��λ�ĵ͵�ƽ����
    delay_1us();   
    LCD_RST = 1;// �ر�LCD
    LCD_CE = 0;
    delay_1us();// ʹ��LCD
    LCD_CE = 1;
    delay_1us();
    
    LCD_write_byte(0x21, 0);	// ʹ����չ��������LCDģʽ
    LCD_write_byte(0xb8, 0);	// ����ƫ�õ�ѹ
    LCD_write_byte(0x06, 0);	// �¶�У��
    LCD_write_byte(0x13, 0);	// 1:48
    LCD_write_byte(0x20, 0);	// ʹ�û�������
    LCD_clear();	        // ����
    LCD_write_byte(0x0c, 0);	// �趨��ʾģʽ��������ʾ        
           
    LCD_CE = 0; // �ر�LCD
}

/*-----------------------------------------------------------------------
LCD_clear         : LCD��������

��д����          ��2004-8-10 
����޸�����      ��2004-8-10 
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
LCD_clear         : LCD��һ�к���

��д����          ��2014-9-19 
����޸�����      ��2014-9-19 
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
LCD_set_XY        : ����LCD���꺯��

���������X       ��0��83
          Y       ��0��5
-----------------------------------------------------------------------*/
void LCD_set_XY(unsigned char X, unsigned char Y)
{
    LCD_write_byte(0x40 | Y, 0);		// column
    LCD_write_byte(0x80 | X, 0);          	// row
}
/*-----------------------------------------------------------------------
LCD_write_one: ��ʾӢ���ַ�

���������c       ����ʾ���ַ���
-----------------------------------------------------------------------*/
void LCD_write_one(unsigned char c)
{
    unsigned char line;
    for (line=0; line<6; line++)
      LCD_write_byte(font6x8[c-32][line], 1);
}

/*-----------------------------------------------------------------------
LCD_write_english_String  : Ӣ���ַ�����ʾ����

���������*s      ��Ӣ���ַ���ָ�룻
          X��Y    : ��ʾ�ַ�����λ��,x 0-83 ,y 0-5		
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
LCD_write_chinese_string: ��LCD����ʾ����

���������X��Y    ����ʾ���ֵ���ʼX��Y���ꣻ
          ch_with �����ֵ���Ŀ��
          num     ����ʾ���ֵĸ�����  
          line    �����ֵ��������е���ʼ����
          row     ��������ʾ���м��
���ԣ�
	LCD_write_chi(0,4,12,7,0,0);	
-----------------------------------------------------------------------*/                        
void LCD_write_chinese_string(unsigned char X, unsigned char Y, 
                   unsigned char ch_with,unsigned char num,
                   unsigned char line,unsigned char row)
{
    unsigned char i,n;
    
    LCD_set_XY(X,Y);                             //���ó�ʼλ��
    
    for (i=0;i<num;)
      {
      	for (n=0; n<ch_with*2; n++)              //дһ������
      	  { 
      	    if (n==ch_with)                      //д���ֵ��°벿��
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
LCD_draw_map      : λͼ���ƺ���

���������X��Y    ��λͼ���Ƶ���ʼX��Y���ꣻ
          *map    ��λͼ�������ݣ�
          Pix_x   ��λͼ���أ�����
          Pix_y   ��λͼ���أ���
-----------------------------------------------------------------------*/
/*void LCD_draw_bmp_pixel(unsigned char X,unsigned char Y,unsigned char *map,
                  unsigned char Pix_x,unsigned char Pix_y)
{
    unsigned int i,n;
    unsigned char row;
    
    if (Pix_y%8==0) row=Pix_y/8;      //����λͼ��ռ����
      else
        row=Pix_y/8+1;
    
    for (n=0;n<row;n++)
      {
      	LCD_set_XY(X,Y);
        for(i=0; i<Pix_x; i++)
          {
            LCD_write_byte(map[i+n*Pix_x], 1);
          }
        Y++;                         //����
      }      
} */

/*-----------------------------------------------------------------------
LCD_write_byte    : ʹ��SPI�ӿ�д���ݵ�LCD

���������data    ��д������ݣ�
          command ��д����/����ѡ��
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
LCD_write_char    : ��ʾ�ַ�����

���������c       ����ʾ�����֣�
-----------------------------------------------------------------------*/
void LCD_write_number(unsigned char c)
{
    unsigned char line;

    for (line=0; line<6; line++)
      LCD_write_byte(font6x8[c+16][line], 1); //������  -32+0x30=16

}   
/*-----------------------------------------------------------------------
LCD_write_char: ��ʾchar�����֣�
-----------------------------------------------------------------------*/
void LCD_write_char(unsigned char X,unsigned char Y,unsigned char i)
{
    LCD_set_XY(X,Y);                             //���ó�ʼλ��
    LCD_write_number((char)(i/100%10));
    LCD_write_number((char)(i%100/10));
    LCD_write_number((char)(i%10));
} 
/*-----------------------------------------------------------------------
LCD_write_int: ��ʾint�����֣�
-----------------------------------------------------------------------*/
void LCD_write_int(unsigned char X,unsigned char Y,int i)
{
    LCD_set_XY(X,Y);                             //���ó�ʼλ��
    LCD_write_number((char)(i/10000));
    LCD_write_number((char)(i/1000%10));
    LCD_write_number((char)(i/100%10));
    LCD_write_number((char)(i%100/10));
    LCD_write_number((char)(i%10));
}  
/*-----------------------------------------------------------------------
LCD_write_float: ��ʾfloat�����֣�
-----------------------------------------------------------------------*/
void LCD_write_float(unsigned char X,unsigned char Y,float i)
{
    LCD_set_XY(X,Y);                             //���ó�ʼλ��
	LCD_write_number((int)i/100%10);
	LCD_write_number((int)i%100/10);
	LCD_write_number((int)i%10);
	LCD_write_one('.');
 	LCD_write_number((int)(i*10)%10);
  	LCD_write_number((int)(i*100)%10);
}

/***************������������ʾ8*16һ��int�ͱ���	��ʾ�����꣨x,y����yΪҳ��Χ0��3****************/
void LCD_write_Hex(unsigned char X,unsigned char Y,unsigned char i)
{
    LCD_set_XY(X,Y);                             //���ó�ʼλ��

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