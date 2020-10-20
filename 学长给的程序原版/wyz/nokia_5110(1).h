#ifndef __NOKIA5110_H__
#define __NOKIA5110_H__



void LCD_init(void);
void LCD_clear(void);
void LCD_clear_l(unsigned char X, unsigned char Y);
void LCD_move_chinese_string(unsigned char X, unsigned char Y, unsigned char T); 
void LCD_write_english_string(unsigned char X,unsigned char Y,char *s);
void LCD_write_chinese_string(unsigned char X, unsigned char Y,unsigned char ch_with,unsigned char num,unsigned char line,unsigned char row);
void chinese_string(unsigned char X, unsigned char Y, unsigned char T);                 
void LCD_write_one(unsigned char c);
void LCD_write_number(unsigned char c);
void LCD_draw_bmp_pixel(unsigned char X,unsigned char Y,unsigned char *map, unsigned char Pix_x,unsigned char Pix_y);
void LCD_write_byte(unsigned char dat, unsigned char dc);
void LCD_set_XY(unsigned char X, unsigned char Y);
void delay_1us(void);  
void LCD_DLY_ms(unsigned int ms);
void LCD_write_number(unsigned char c);
void LCD_write_int(unsigned char X,unsigned char Y,int i);
void LCD_write_char(unsigned char X,unsigned char Y,unsigned char i);
void LCD_write_float(unsigned char X,unsigned char Y,float i);
void LCD_write_Hex(unsigned char x,unsigned char y,unsigned char i);


#endif               
 