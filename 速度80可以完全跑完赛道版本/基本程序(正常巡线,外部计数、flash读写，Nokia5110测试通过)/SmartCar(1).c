//晶振 12MHz 1T模式  总线频率12MHz，一个指令1/12us
#include "STC12C5A60S2.H"      //STC12C5A60S2的头文件
#include "nokia_5110.h"
#include "key.h"
#include "flash.h"
//#define uchar unsigned char    //宏定义
#define uint unsigned int

#define T0_HIGH      0xff   //T0计时器寄存器初值
#define T0_LOW      0x83 //溢出计数160个，160*（1/16）定时周期10us
                            //为了保证主程序正常运行，定时器计数最好不要小于80个

#define MOTOR_MAX_PWM_COUNT  100 //PWM计数器最大值 10us*100=1ms,1000Hz
#define SERVO_MAX_PWM_COUNT  1010//1000=20ms,50HZ //舵机控制高电平0.5~2.5ms对应PWM的50~250
                                 //舵机控制频率在50~100Hz之间，这里选50Hz

                              //注意，舵机打角过大会打在底盘上，请立即断电，以免损害舵机

#define ERROR_HISTORY_NUM 5   //历史偏差记录个数


sbit LEFT_MOTOR_PWM_PORT=P3^7; // 电机PWM控制端口
sbit RIGHT_MOTOR_PWM_PORT=P3^6;
//	 sbit LEFT_MOTOR_PWM_PORT=P3^1; // 电机PWM控制端口
 //    sbit RIGHT_MOTOR_PWM_PORT=P3^0;
sbit SERVO_PWM_PORT=P3^5;   //舵机PWM控制端口



bit control_period_finished=0; //控制周期结束标志位，控制周期选择20ms
                               //也可自己确定控制周期，但是要保证主程序在控制周期内可以运行完
								 //一直在最后的中断那里可以找到

char motor_PWM_counter=0;//生成电机PWM的计数器
char left_motor_PWM=0;   //左右电机的PWM占空比 //由于数值在128以内，所以定义为char
char right_motor_PWM=0;
 //char left_motor_PWM=50;   //左右电机的PWM占空比 //由于数值在128以内，所以定义为char
//char right_motor_PWM=50;

int servo_PWM_counter=0;//生成舵机PWM的计数器
int servo_PWM=0;//舵机PWM //超过char型变量的范围
//unsigned char servo_middle=130;
			  unsigned char servo_middle=140;	  //调这个可以改变舵机其停止时的角度。

int P2H=0;
sbit end=P1^3;
sbit boma1=P1^4;
int error=0;    //本次偏差
int error_history[ERROR_HISTORY_NUM]={0};  //历史偏差

unsigned char servo_P=28;   //PID系数
unsigned char servo_D=55 ;
																										  
char speed_expect=80;  //给定速度			 ,90太快了，85比较快。

int timecount=0;
int frequent=0;
int T0count=0;
int T1count=0;

int count=0;
int temp=0;
int worse=0;					//把worse加大!!!!!

int SUM=0;

void sys_init(void)    //系统初始化
{
//若flash已经有数据且不为0xff,将上次修改参数的数据赋给相应变量，避免首次flash里面无数据
if(read(0x00,0x00)!=255) servo_P=read(0x00,0x00);//	servo_P存储地址
if(read(0x00,0x01)!=255) servo_D=read(0x00,0x01);//	servo_D存储地址
if(read(0x00,0x02)!=255) servo_middle=read(0x00,0x02);//servo_middle存储地址
if(read(0x00,0x03)!=255) speed_expect=read(0x00,0x03);//speed_expect存储地址，我自己加的

	//------IO口设置------------
    P0M1=0x00; 	//推挽输出
    P0M0=0xff;
    P0=0xff;   

    P1M1=0x00; //准双向口
    P1M0=0x00;
    P1=0xff; //初始化为高电平 
	
	P2M1=0x00;//指示灯，设置为输出
    P2M0=0x00;
    P2=0xff;

    P3M1=0x00;//P3口高四位设为推挽输出产生PWM，低四位设置成准双向口，用以外部中断脉冲计数
    P3M0=0xf0;
	P3=0x0f;

    LEFT_MOTOR_PWM_PORT=0; //PWM口设为低电平，电机不转
    RIGHT_MOTOR_PWM_PORT=0;
    //left_motor_PWM=0;    //占空比设置为0
    //right_motor_PWM=0;
	left_motor_PWM=80;    //占空比设置为0
    right_motor_PWM=80;
    servo_PWM=servo_middle;//舵机打角到中心

    //------定时器设置------------
    AUXR=AUXR | 0x80; //定时器0不分频
    TMOD=0x11;        //定时器方式1
    TH0=T0_HIGH;      //定时器赋初值
    TL0=T0_LOW ;
 /*****************若没有速度闭环，本部分可以注释掉*********************/
 /*
 //开启两路外部中断，用于编码器脉冲计数
    IT0=1;//下降沿触发
	EX0=1; 
	IT1=1;//下降沿触发
	EX1=1;
	IPH=0x04;
	IP=0x04;//PX1=1;//将外部中断1优先级设为最高
	*/												//我没有编码器，所以注释掉
/**********************本部分亦可采用STC12自带两路硬件脉冲捕捉功能实现，参照CCAPM0\CCAPM1***********************************************/   
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
void getposition(void)//前面9个红外对管，也可以结合自己红外对管个数和排布自行量化偏差
{
       
	   P2H=(P2&0xff);//从左到右1-8个红外对管状态，经电压比较器后依次连P2.7-P2.0
	   P2H=(P2H<<1);//左移一位
	   P2H=P2H+(P1&0x80);//将左移一位后的值按位与上从左到右第9个红外对管(经电压比较器后连接至P1.7)的值，P1&0x80是将P1口的最高位取出 
	   //P2H=001000000	;//我自己加的测试一下我单片机的接口是否坏了。
       switch(P2H&0x1ff)//最终P2H的低九位，每一位的状态代表了前面红外对管照到黑线上的情况，
	   //经电压比较器输出高电平代表下方为黑线(对应那一位为1)，照在白色跑道上为低电平(对应那一位为0)
	    {
		case 0x100: error=-12;worse/=2;    break;
		case 0x180: error=-11;worse/=2;    break;
		case 0x080: error=-9;worse/=2;    break;
		case 0x0c0: error=-7;worse/=2;    break;		
		case 0x040: error=-5;worse/=2;    break;          //left
        case 0x060: error=-3;worse/=2;    break;                    				
		case 0x020: error=-2;worse/=2;     break;                   		
		case 0x030: error=-1;worse/=2;     break;
		
		            
        case 0x010: error=0;worse/=2;     break;
		
		          
        case 0x018: error=1;worse/=2;     break;            
        case 0x008: error=2;worse/=2;     break;           
        case 0x00c: error=3;worse/=2;     break;                  
        case 0x004: error=5;worse/=2;     break;              
        case 0x006: error=7;worse/=2;     break;       // right
        case 0x002: error=9;worse/=2;     break;            
        case 0x003: error=11;worse/=2;     break;             
        case 0x001: error=12;worse/=2;    break;           

		case 0x000: error=error_history[ERROR_HISTORY_NUM-1];break;       //000000000 0x000  
        default: error=error_history[ERROR_HISTORY_NUM-1];worse++; break;    //其他情况时，保持上一次偏差
		}
 //worse表示错误信号，当出现0x000或者其他情况信号时，
 //表示信号不正常,累加计数，达到一定数量时可以认定车已经出界，令电机转速为0，避免撞坏
} 

    
void servo_control(void) //舵机控制
{
    char i=0;

    if(ABS(error-error_history[4])>=11) //如果两次的偏差过大，则保持上一次的偏差
        error=error_history[4];      //防止不确定情况（如红外管冲出跑道）造成的偏差跳变
    //根据偏差计算舵机角度输出
     servo_PWM = servo_middle+(servo_P*error    //比例
                   + servo_D*(error-error_history[4])//微分
              )/10;   //这里使用乘以一个大的系数再除以一个数的方法
                       //将小数计算转换成整数计算，如*0.1可转换为*10/100。以减轻单片机的负担
  //servo_PWM  =100	;
  //servo_PWM= servo_middle+(servo_P*error    //比例
      //             + servo_D*(error-error_history[4]-50)//微分
       //      )/3;   //这里使用乘以一个大的系数再除以一个数的方法
                       //将小数计算转换成整数计算，如*0.1可转换为*10/100。以减轻单片机的负担


  // if(servo_PWM<95)      //限幅
			 if(servo_PWM<90)      //限幅
    //    servo_PWM=95;     //防止舵机超过极限值
			servo_PWM=90;     //防止舵机超过极限值
   // if(servo_PWM>165)
	 if(servo_PWM>170)
     //   servo_PWM=165;
		 servo_PWM=170;
  for(i=0;i<ERROR_HISTORY_NUM-1;i++)    //记录偏差
    {
        error_history[i]=error_history[i+1];
    }
    error_history[ERROR_HISTORY_NUM-1]=error;

}



void motor_control(void) //电机控制
{
    left_motor_PWM=speed_expect;   //速度控制
    right_motor_PWM=speed_expect;  //可以直接给定，也可以根据赛道情况计算得出

	if(worse>=30)//前面传感器信号不正常，累计N次判定为冲出跑道，防止车撞坏，电机速度给0		  //worse需要加大！！！！！
	{
	left_motor_PWM=0;
    right_motor_PWM=0;
	}
	
	 
    if(left_motor_PWM>MOTOR_MAX_PWM_COUNT)   //限幅
        left_motor_PWM=MOTOR_MAX_PWM_COUNT;
    if(left_motor_PWM<0)
        left_motor_PWM=0;

    if(right_motor_PWM>MOTOR_MAX_PWM_COUNT)
        right_motor_PWM=MOTOR_MAX_PWM_COUNT;
    if(right_motor_PWM<0)
        right_motor_PWM=0;
}

void flash_store()//相关修改参数写入flash保存起来，方便调试,相关函数见flash.c
{
del(0x00,0x00);//将扇区初始化
delay_ms(5);
write(0x00,0x00,servo_P);
delay_ms(5);
write(0x00,0x01,servo_D);
delay_ms(5);
write(0x00,0x02,servo_middle);
}


 /**********************************************************
函数名：servo_test
说明：测试舵机是否能够左右正常摆动
入口：无
出口：无
备注：将boma1用杜邦线连至P1.4口，拨动拨码开关1可选择正常模式或舵机测试模式

***********************************************************/
void servo_test()
{
  static uint i=0;
  if(i<120)
    i=i+1;
  else 
    i=0;
  if(i<=30||(i>90&&i<=120))
  	servo_PWM-=1;
  else if(i>30&&i<=90)
  	servo_PWM+=1;
}							//叶神新加的,测试了一下，是可以的。


//-----------------------------------------------------------------------
void main(void)
{	
    sys_init();
    LCD_init();
	pre_show();
    P1=0xff;
	/*
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
	 	 }
	  }
	 flash_store();

    }				  */			 //把这个注释掉之后初试成功！！！坚持真理！！！！
    while(1)
    {	
			 //我将键盘扫描的程序移动到这下面来。
	   /*
	    	KeyScan();
		pre_show();
		if(~end)  //如果检测到低电平，说明按键按下
     {
  	 LCD_DLY_ms(10);   //延时去抖，一般10-20ms
       if(~end)     //再次确认按键是否按下，没有按下则退出
	     {
        while(~end);//如果确认按下按键等待按键释放，没有释放则一直等待
        end=0;
	 	 }
	  }
	 flash_store();		 //我担心这的就会对舵机的检测时间有影响导致反应不及时，影响控制周期！
	       */
		//
		if(!boma1)			 //选择模式-舵机测试或正常循迹
		servo_test();		 //舵机测试
		else
		{
        getposition();  //判断黑线位置
        servo_control();//控制舵机
		//servo_PWM=90;						//把上面那个循环注释掉之后这个才开始起作用！！！！
		motor_control();//控制电机
		}
        while(control_period_finished==0);  //等待控制周期20ms到达,保证每次控制的时间间隔都相同
        {
		control_period_finished=0; //标志位清零
	    }	
    }
}

void exint0() interrupt 0 //外部中断0测速,中断号0
{
   T0count++;  
}
void exint1() interrupt 2 //外部中断1测速，中断号2
{
   T1count++;  
}
		

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
    if(servo_PWM_counter>1010)   //PWM比较、生成
    {

        servo_PWM_counter=1; //从1开始计数
        control_period_finished=1;  //20ms控制周期到达，标志位置1，这里借用了舵机控制周期20ms（50Hz），
                            //如果更改主程序控制周期，可定义一个类似于servo_PWM_counter的计数器计时
	    timecount++;
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
		
	
if(timecount>=5)//20ms*5=100ms，每100ms统计一次脉冲个数 
    { 
     frequent=(T0count+T1count)/2;
     T0count=0;//置0重新计数
	 T1count=0;//置0重新计数
     timecount=0;
    }
}	

