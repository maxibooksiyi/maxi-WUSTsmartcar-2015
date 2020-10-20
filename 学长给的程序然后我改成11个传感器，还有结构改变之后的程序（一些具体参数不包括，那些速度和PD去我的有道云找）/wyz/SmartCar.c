#include "STC12C5A60S2.H"      //STC12C5A60S2��ͷ�ļ�
#include "nokia_5110.h"
#include "key.h"
#define uchar unsigned char    //�궨��
#define uint unsigned int

#define T0_HIGH      0xff   //T0��ʱ���Ĵ�����ֵ
#define T0_LOW      0x5f //�������160����160*��1/16����ʱ����10us
                            //Ϊ�˱�֤�������������У���ʱ��������ò�ҪС��80��

#define MOTOR_MAX_PWM_COUNT  100 //PWM���������ֵ 10us*100=1ms,1000Hz
#define SERVO_MAX_PWM_COUNT  1600//1300 63

                              //ע�⣬�����ǹ������ڵ����ϣ��������ϵ磬�����𺦶��

#define ERROR_HISTORY_NUM 5   //��ʷƫ���¼����


sbit LEFT_MOTOR_PWM_PORT=P3^7; // ���PWM���ƶ˿�
sbit RIGHT_MOTOR_PWM_PORT=P3^6;

sbit SERVO_PWM_PORT=P3^5;   //���PWM���ƶ˿�


bit control_period_finished=0; //�������ڽ�����־λ����������ѡ��20ms
                               //Ҳ���Լ�ȷ���������ڣ�����Ҫ��֤�������ڿ��������ڿ���������

char motor_PWM_counter=0;//���ɵ��PWM�ļ�����
char left_motor_PWM=0;   //���ҵ����PWMռ�ձ� //������ֵ��128���ڣ����Զ���Ϊchar
char right_motor_PWM=0;

int servo_PWM_counter=0;//���ɶ��PWM�ļ�����
int servo_PWM=0;//���PWM //����char�ͱ����ķ�Χ

int P2H=0;
sbit end=P1^7;

int error=0;    //����ƫ��
int error_history[ERROR_HISTORY_NUM]={0};  //��ʷƫ��

char line_speed=65;
char bend_speed=55;
unsigned char differ_number=10;
char line_servo_p=25;
char line_servo_d=60;
char bend_servo_p=28;
char bend_servo_d=70;

char realspeed=0;//���ٶ�

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

//�����ֽ�д
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

void flash_store()//����޸Ĳ���д��flash�����������������,��غ�����flash.c
{
del(0x00,0x00);//��������ʼ��
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

void sys_init(void)    //ϵͳ��ʼ��
{
 //��flash�Ѿ��������Ҳ�Ϊ0xff,���ϴ��޸Ĳ��������ݸ�����Ӧ�����������״�flash����������
if(read(0x00,0x00)!=255)   line_speed=read(0x00,0x00);//	servo_P�洢��ַ
if(read(0x00,0x01)!=255)   bend_speed=read(0x00,0x01);//	servo_D�洢��ַ
if(read(0x00,0x02)!=255)   differ_number=read(0x00,0x02);//servo_middle�洢��ַ
if(read(0x00,0x00)!=255)   line_servo_p=read(0x00,0x03);//	servo_P�洢��ַ
if(read(0x00,0x01)!=255)   line_servo_d=read(0x00,0x04);//	servo_D�洢��ַ
if(read(0x00,0x02)!=255)   bend_servo_p=read(0x00,0x05);//servo_middle�洢��ַ
if(read(0x00,0x00)!=255)   bend_servo_d=read(0x00,0x06);//	servo_P�洢��ַ


    //------IO������------------
    P0M1=0x00; 			//�������
    P0M0=0xff;
    P0=0xff;   

    P1M1=0x00; 		// 	 ׼˫���
    P1M0=0x00;
    P1=0xff;   //��ʼ��Ϊ�ߵ�ƽ 
	
	P2M1=0x00;  //ָʾ�ƣ�����Ϊ���
    P2M0=0x00;
    P2=0xff;

    P3M1=0x00;  //ģ��PWM������Ϊ���
    P3M0=0xf8;
     //P3=0x00;
	P3=0x0f;

    LEFT_MOTOR_PWM_PORT=0; //PWM����Ϊ�͵�ƽ�������ת
    RIGHT_MOTOR_PWM_PORT=0;
    left_motor_PWM=0;    //ռ�ձ�����Ϊ0
    right_motor_PWM=0;
    servo_PWM=113;//�����ǵ�����

    //------��ʱ������------------
    AUXR=AUXR | 0x80; //��ʱ��0����Ƶ
    TMOD=0x11;        //��ʱ����ʽ1
    TH0=T0_HIGH;      //��ʱ������ֵ
    TL0=T0_LOW ;
 
   //IT0=1;
//	EX0=1;
    TR0=1;      //��ʱ������
    ET0=1;      //����ʱ���ж�
    EA=1;       //�����ж�

}
//-----------------------------------------------------------------------
int ABS(int i)    //����ֵ����
{
    if (i>=0) return i;
    else return -i;
}
//-----------------------------------------------------------------------
void delay(unsigned int i) //��ʱ����
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
	    P2H=(P1&0x07);//11���ƴ�������������P1.2 P1.1 P1.0 P2.7 P2.6 P2.5 P2.4 P2.3 P2.2 P2.1 P2.0 
        
	//	P2H=(P2H<<3);
			P2H=(P2H<<8);
        P2H=P2H+P2; 
        switch(P2H&0x7ff) 
       //switch(P2H&0x1ff)				  //����֮����Ҫ�Ӵ�worse .
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
		
		case 0x020: error=0;worse/=2;     break;      //    000000100000        ����������м�ֵ             		
		
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
        default: error=error_history[ERROR_HISTORY_NUM-1];worse++; break;    //�������ʱ��������һ��ƫ��
		}

} 

int steer_value=0; 
int steer_kp,steer_kd;  
char line_count=0;    
void servo_control(void) //�������
{
    char i=0;
      if(ABS(error)>=3) {steer_kp=bend_servo_p;steer_kd=bend_servo_d;}
      else {steer_kp=line_servo_p;steer_kd=line_servo_d;}//����ƫ��������Ƕ����

    if(ABS(error-error_history[4])>=9) //������ε�ƫ������򱣳���һ�ε�ƫ��
        error=error_history[4];      //��ֹ��ȷ������������ܳ���ܵ�����ɵ�ƫ������
     steer_value=(      steer_kp*error    //����
                   + steer_kd*(error-error_history[4])//΢��
               )/10;   
//����ƫ��������Ƕ����
     servo_PWM = 113+steer_value ;

    if(servo_PWM>=158)      //�޷�
        servo_PWM=158;     //��ֹ�����������ֵ
    if(servo_PWM<=83)
        servo_PWM=83;

  for(i=0;i<ERROR_HISTORY_NUM-1;i++)    //��¼ƫ��
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



void motor_control(void) //�������
{
    if(line_count>=8) realspeed=line_speed;
	else 
	{
	if(ABS(error)<=2) realspeed=bend_speed;
	else  realspeed=bend_speed-8;
	}
	
	left_motor_PWM=realspeed+(int)((float)(differ_number/10.0)*steer_value);;   //�ٶȿ���
    right_motor_PWM=realspeed-(int)((float)(differ_number/10.0)*steer_value);;  //����ֱ�Ӹ�����Ҳ���Ը��������������ó�

    //if(worse>=30)		   //�ĳ�11�������ˣ���Ӵ�worse
	if(worse>=75)	
	{
	left_motor_PWM=0;
    right_motor_PWM=0;
	}

    if(left_motor_PWM>100)   //�޷�
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
		if(~end)  //�����⵽�͵�ƽ��˵����������
     {
  	 LCD_DLY_ms(10);   //��ʱȥ����һ��10-20ms
       if(~end)     //�ٴ�ȷ�ϰ����Ƿ��£�û�а������˳�
	     {
        while(~end);//���ȷ�ϰ��°����ȴ������ͷţ�û���ͷ���һֱ�ȴ�
        end=0;
		flash_store();
	 	 }
	  }

    }
    while(1)
    {	
	    
        getposition();    //�жϺ���λ��
	    P2=0xff;
        servo_control();
		motor_control();         //���Ƶ��	
    }
	    


}

//void exint0() interrupt 0  //interrupt 0 (location at 0003H)
//{
//   T0count++;  
//}
		

void TIME_BASE(void) interrupt 1 using 1   //��ʱ������PWM  //������Ŀ��Բο�PWM�Ķ���
{
    TH0=T0_HIGH;        //����ֵ
    TL0=T0_LOW ;

    motor_PWM_counter++;  //������+1 ,ÿ+1��ʾ10us
    servo_PWM_counter=servo_PWM_counter+1;

    if(motor_PWM_counter>100)   //PWM�Ƚϡ�����
    {
        motor_PWM_counter=1; //��1��ʼ����
    }
    if(servo_PWM_counter>SERVO_MAX_PWM_COUNT)   //PWM�Ƚϡ�����
    {

        servo_PWM_counter=1; //��1��ʼ����
        control_period_finished=1;  //20ms�������ڵ����־λ��1����������˶����������20ms��50Hz����
                            //�������������������ڣ��ɶ���һ��������servo_PWM_counter�ļ�������ʱ
	  //  timecount++;
    }

    if(left_motor_PWM>=motor_PWM_counter)   //����PWM
        LEFT_MOTOR_PWM_PORT=1;  //�˿�����ߵ�ƽ
    else
        LEFT_MOTOR_PWM_PORT=0;  //�˿�����͵�ƽ


    if(right_motor_PWM>=motor_PWM_counter)   //����PWM
        RIGHT_MOTOR_PWM_PORT=1;  //�˿�����ߵ�ƽ
    else
        RIGHT_MOTOR_PWM_PORT=0;  //�˿�����͵�ƽ

    if(servo_PWM>=servo_PWM_counter)//���PWM
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
