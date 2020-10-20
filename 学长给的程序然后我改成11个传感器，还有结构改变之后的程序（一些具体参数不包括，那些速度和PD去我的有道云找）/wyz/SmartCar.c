#include "STC12C5A60S2.H"      //STC12C5A60S2的头文件
#include "nokia_5110.h"
#include "key.h"
#define uchar unsigned char    //宏定义
#define uint unsigned int

#define T0_HIGH      0xff   //T0计时器寄存器初值
#define T0_LOW      0x5f //溢出计数160个，160*（1/16）定时周期10us
                            //为了保证主程序正常运行，定时器计数最好不要小于80个

#define MOTOR_MAX_PWM_COUNT  100 //PWM计数器最大值 10us*100=1ms,1000Hz
#define SERVO_MAX_PWM_COUNT  1600//1300 63

                              //注意，舵机打角过大会打在底盘上，请立即断电，以免损害舵机

#define ERROR_HISTORY_NUM 5   //历史偏差记录个数


sbit LEFT_MOTOR_PWM_PORT=P3^7; // 电机PWM控制端口
sbit RIGHT_MOTOR_PWM_PORT=P3^6;

sbit SERVO_PWM_PORT=P3^5;   //舵机PWM控制端口


bit control_period_finished=0; //控制周期结束标志位，控制周期选择20ms
                               //也可自己确定控制周期，但是要保证主程序在控制周期内可以运行完

char motor_PWM_counter=0;//生成电机PWM的计数器
char left_motor_PWM=0;   //左右电机的PWM占空比 //由于数值在128以内，所以定义为char
char right_motor_PWM=0;

int servo_PWM_counter=0;//生成舵机PWM的计数器
int servo_PWM=0;//舵机PWM //超过char型变量的范围

int P2H=0;
sbit end=P1^7;

int error=0;    //本次偏差
int error_history[ERROR_HISTORY_NUM]={0};  //历史偏差

char line_speed=65;
char bend_speed=55;
unsigned char differ_number=10;
char line_servo_p=25;
char line_servo_d=60;
char bend_servo_p=28;
char bend_servo_d=70;

char realspeed=0;//总速度

int timecount=0;
int frequent=0;
int T0count=0;
int worse=0;

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

void flash_store()//相关修改参数写入flash保存起来，方便调试,相关函数见flash.c
{
del(0x00,0x00);//将扇区初始化
delay_ms(5);
write(0x00,0x00,line_speed);
delay_ms(5);
write(0x00,0x01,bend_speed);
delay_ms(5);
write(0x00,0x02,differ_number);
delay_ms(5);
write(0x00,0x03,line_servo_p);
delay_ms(5);
write(0x00,0x04,line_servo_d);
delay_ms(5);
write(0x00,0x05,bend_servo_p);
delay_ms(5);
write(0x00,0x06,bend_servo_d);
}

void sys_init(void)    //系统初始化
{
 //若flash已经有数据且不为0xff,将上次修改参数的数据赋给相应变量，避免首次flash里面无数据
if(read(0x00,0x00)!=255)   line_speed=read(0x00,0x00);//	servo_P存储地址
if(read(0x00,0x01)!=255)   bend_speed=read(0x00,0x01);//	servo_D存储地址
if(read(0x00,0x02)!=255)   differ_number=read(0x00,0x02);//servo_middle存储地址
if(read(0x00,0x00)!=255)   line_servo_p=read(0x00,0x03);//	servo_P存储地址
if(read(0x00,0x01)!=255)   line_servo_d=read(0x00,0x04);//	servo_D存储地址
if(read(0x00,0x02)!=255)   bend_servo_p=read(0x00,0x05);//servo_middle存储地址
if(read(0x00,0x00)!=255)   bend_servo_d=read(0x00,0x06);//	servo_P存储地址


    //------IO口设置------------
    P0M1=0x00; 			//推挽输出
    P0M0=0xff;
    P0=0xff;   

    P1M1=0x00; 		// 	 准双向口
    P1M0=0x00;
    P1=0xff;   //初始化为高电平 
	
	P2M1=0x00;  //指示灯，设置为输出
    P2M0=0x00;
    P2=0xff;

    P3M1=0x00;  //模拟PWM，设置为输出
    P3M0=0xf8;
     //P3=0x00;
	P3=0x0f;

    LEFT_MOTOR_PWM_PORT=0; //PWM口设为低电平，电机不转
    RIGHT_MOTOR_PWM_PORT=0;
    left_motor_PWM=0;    //占空比设置为0
    right_motor_PWM=0;
    servo_PWM=113;//舵机打角到中心

    //------定时器设置------------
    AUXR=AUXR | 0x80; //定时器0不分频
    TMOD=0x11;        //定时器方式1
    TH0=T0_HIGH;      //定时器赋初值
    TL0=T0_LOW ;
 
   //IT0=1;
//	EX0=1;
    TR0=1;      //定时器运行
    ET0=1;      //开定时器中断
    EA=1;       //开总中断

}
//-----------------------------------------------------------------------
int ABS(int i)    //绝对值函数
{
    if (i>=0) return i;
    else return -i;
}
//-----------------------------------------------------------------------
void delay(unsigned int i) //延时函数
{
    unsigned char j;
    for(;i>0;i--)
        for(j=0;j<250;j++);
}

void getposition(void)

{
       
	  //P2H=(P2&0xff);
	  //P2H=(P2H<<1);
	  //P2H=P2H+(P1&0x01); 
	    P2H=(P1&0x07);//11个灯从左到右依次连接P1.2 P1.1 P1.0 P2.7 P2.6 P2.5 P2.4 P2.3 P2.2 P2.1 P2.0 
        
	//	P2H=(P2H<<3);
			P2H=(P2H<<8);
        P2H=P2H+P2; 
        switch(P2H&0x7ff) 
       //switch(P2H&0x1ff)				  //改了之后需要加大worse .
	    {
		case 0x400:	error=-11;worse/=2;	  break;	//		010000000000
		case 0x600:	error=-9;worse/=2;	  break;	//		011000000000
		case 0x200:	error=-7;worse/=2;	  break;	    //  001000000000
		case 0x300: error=-6;worse/=2;    break;

		case 0x100: error=-5;worse/=2;    break;
		case 0x180: error=-4;worse/=2;    break;   //-7
		case 0x080: error=-3;worse/=2;    break;	//6
		case 0x0c0: error=-2;worse/=2;    break;	//5	
		case 0x040: error=-1;worse/=2;    break;    //4      //left
        case 0x060: error=0;worse/=2;     break;                    				
		case 0x020: error=0;worse/=2;     break;                   		
		case 0x030: error=0;worse/=2;     break;
		
		            
        case 0x010: error=1;worse/=2;     break;
		
		          
        case 0x018: error=2;worse/=2;     break;            
        case 0x008: error=3;worse/=2;     break;           
        case 0x00c: error=4;worse/=2;     break;                  
        case 0x004: error=5;worse/=2;     break;              
        case 0x006: error=6;worse/=2;     break;       // right
        case 0x002: error=7;worse/=2;     break;            
        case 0x003: error=9;worse/=2;     break;       //7      
        case 0x001: error=11;worse/=2;    break;       //9    
		
		/*
		case 0x400:	error=-25;worse/=2;	  break;	//		010000000000
		case 0x600:	error=-17;worse/=2;	  break;	//		011000000000
		case 0x200:	error=-13;worse/=2;	  break;	    //  001000000000
		case 0x300: error=-8;worse/=2;    break;        //  001100000000
		case 0x100: error=-6;worse/=2;    break;			000100000000
		case 0x180: error=-5;worse/=2;    break;   //-7		000110000000
		case 0x080: error=-4;worse/=2;    break;	//6		000010000000
		case 0x0c0: error=-3;worse/=2;    break;	//5		000011000000
		case 0x040: error=-2;worse/=2;    break;    //4     000001000000                //left
        case 0x060: error=0;worse/=2;     break;            000001100000        				
		
		case 0x020: error=0;worse/=2;     break;      //    000000100000        这个现在是中间值             		
		
		case 0x030: error=0;worse/=2;     break;
		case 0x010: error=2;worse/=2;     break;
		case 0x018: error=3;worse/=2;     break;            
        case 0x008: error=4;worse/=2;     break;           
        case 0x00c: error=5;worse/=2;     break;                  
        case 0x004: error=6;worse/=2;     break;              
        case 0x006: error=8;worse/=2;     break;       // right
        case 0x002: error=13;worse/=2;    break;            
        case 0x003: error=17;worse/=2;    break;       //7      
        case 0x001: error=25;worse/=2;    break;       //9    
														   */
		case 0x000: error=error_history[ERROR_HISTORY_NUM-1];break;       //000000000 0x000  
        default: error=error_history[ERROR_HISTORY_NUM-1];worse++; break;    //其他情况时，保持上一次偏差
		}

} 

int steer_value=0; 
int steer_kp,steer_kd;  
char line_count=0;    
void servo_control(void) //舵机控制
{
    char i=0;
      if(ABS(error)>=3) {steer_kp=bend_servo_p;steer_kd=bend_servo_d;}
      else {steer_kp=line_servo_p;steer_kd=line_servo_d;}//根据偏差计算舵机角度输出

    if(ABS(error-error_history[4])>=9) //如果两次的偏差过大，则保持上一次的偏差
        error=error_history[4];      //防止不确定情况（如红外管冲出跑道）造成的偏差跳变
     steer_value=(      steer_kp*error    //比例
                   + steer_kd*(error-error_history[4])//微分
               )/10;   
//根据偏差计算舵机角度输出
     servo_PWM = 113+steer_value ;

    if(servo_PWM>=158)      //限幅
        servo_PWM=158;     //防止舵机超过极限值
    if(servo_PWM<=83)
        servo_PWM=83;

  for(i=0;i<ERROR_HISTORY_NUM-1;i++)    //记录偏差
    {
        error_history[i]=error_history[i+1];
    }
    error_history[ERROR_HISTORY_NUM-1]=error;

      
   if(ABS(steer_value)<=6)  line_count++;
   else line_count=0;

   if(line_count>=10) line_count=10;
	   
   if(steer_value>=35)  steer_value=35;
   if(steer_value<=-35) steer_value=-35;


}



void motor_control(void) //电机控制
{
    if(line_count>=8) realspeed=line_speed;
	else 
	{
	if(ABS(error)<=2) realspeed=bend_speed;
	else  realspeed=bend_speed-8;
	}
	
	left_motor_PWM=realspeed+(int)((float)(differ_number/10.0)*steer_value);;   //速度控制
    right_motor_PWM=realspeed-(int)((float)(differ_number/10.0)*steer_value);;  //可以直接给定，也可以根据赛道情况计算得出

    //if(worse>=30)		   //改成11个管子了，须加大worse
	if(worse>=75)	
	{
	left_motor_PWM=0;
    right_motor_PWM=0;
	}

    if(left_motor_PWM>100)   //限幅
        left_motor_PWM=100;
    if(left_motor_PWM<0)
        left_motor_PWM=0;

    if(right_motor_PWM>100)
        right_motor_PWM=100;
    if(right_motor_PWM<0)
        right_motor_PWM=0;
}

//-----------------------------------------------------------------------
void main(void)
{	
    sys_init();
    LCD_init();
	pre_show();
    P1=0xff;
    
	while(end)
    {
     	KeyScan();
		pre_show();
		if(~end)  //如果检测到低电平，说明按键按下
     {
  	 LCD_DLY_ms(10);   //延时去抖，一般10-20ms
       if(~end)     //再次确认按键是否按下，没有按下则退出
	     {
        while(~end);//如果确认按下按键等待按键释放，没有释放则一直等待
        end=0;
		flash_store();
	 	 }
	  }

    }
    while(1)
    {	
	    
        getposition();    //判断黑线位置
	    P2=0xff;
        servo_control();
		motor_control();         //控制电机	
    }
	    


}

//void exint0() interrupt 0  //interrupt 0 (location at 0003H)
//{
//   T0count++;  
//}
		

void TIME_BASE(void) interrupt 1 using 1   //定时器生成PWM  //不清楚的可以参考PWM的定义
{
    TH0=T0_HIGH;        //赋初值
    TL0=T0_LOW ;

    motor_PWM_counter++;  //计数器+1 ,每+1表示10us
    servo_PWM_counter=servo_PWM_counter+1;

    if(motor_PWM_counter>100)   //PWM比较、生成
    {
        motor_PWM_counter=1; //从1开始计数
    }
    if(servo_PWM_counter>SERVO_MAX_PWM_COUNT)   //PWM比较、生成
    {

        servo_PWM_counter=1; //从1开始计数
        control_period_finished=1;  //20ms控制周期到达，标志位置1，这里借用了舵机控制周期20ms（50Hz），
                            //如果更改主程序控制周期，可定义一个类似于servo_PWM_counter的计数器计时
	  //  timecount++;
    }

    if(left_motor_PWM>=motor_PWM_counter)   //左右PWM
        LEFT_MOTOR_PWM_PORT=1;  //端口输出高电平
    else
        LEFT_MOTOR_PWM_PORT=0;  //端口输出低电平


    if(right_motor_PWM>=motor_PWM_counter)   //左右PWM
        RIGHT_MOTOR_PWM_PORT=1;  //端口输出高电平
    else
        RIGHT_MOTOR_PWM_PORT=0;  //端口输出低电平

    if(servo_PWM>=servo_PWM_counter)//舵机PWM
        SERVO_PWM_PORT=1;
    else
        SERVO_PWM_PORT=0;
		
	
//if(timecount==5) 
  //  { 
  ////   frequent=T0count;
  //   T0count=0;
 //    timecount=0;
	 //backspeed=frequent*20;
     //}
   }	
