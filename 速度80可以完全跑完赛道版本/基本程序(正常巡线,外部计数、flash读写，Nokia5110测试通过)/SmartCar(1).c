//���� 12MHz 1Tģʽ  ����Ƶ��12MHz��һ��ָ��1/12us
#include "STC12C5A60S2.H"      //STC12C5A60S2��ͷ�ļ�
#include "nokia_5110.h"
#include "key.h"
#include "flash.h"
//#define uchar unsigned char    //�궨��
#define uint unsigned int

#define T0_HIGH      0xff   //T0��ʱ���Ĵ�����ֵ
#define T0_LOW      0x83 //�������160����160*��1/16����ʱ����10us
                            //Ϊ�˱�֤�������������У���ʱ��������ò�ҪС��80��

#define MOTOR_MAX_PWM_COUNT  100 //PWM���������ֵ 10us*100=1ms,1000Hz
#define SERVO_MAX_PWM_COUNT  1010//1000=20ms,50HZ //������Ƹߵ�ƽ0.5~2.5ms��ӦPWM��50~250
                                 //�������Ƶ����50~100Hz֮�䣬����ѡ50Hz

                              //ע�⣬�����ǹ������ڵ����ϣ��������ϵ磬�����𺦶��

#define ERROR_HISTORY_NUM 5   //��ʷƫ���¼����


sbit LEFT_MOTOR_PWM_PORT=P3^7; // ���PWM���ƶ˿�
sbit RIGHT_MOTOR_PWM_PORT=P3^6;
//	 sbit LEFT_MOTOR_PWM_PORT=P3^1; // ���PWM���ƶ˿�
 //    sbit RIGHT_MOTOR_PWM_PORT=P3^0;
sbit SERVO_PWM_PORT=P3^5;   //���PWM���ƶ˿�



bit control_period_finished=0; //�������ڽ�����־λ����������ѡ��20ms
                               //Ҳ���Լ�ȷ���������ڣ�����Ҫ��֤�������ڿ��������ڿ���������
								 //һֱ�������ж���������ҵ�

char motor_PWM_counter=0;//���ɵ��PWM�ļ�����
char left_motor_PWM=0;   //���ҵ����PWMռ�ձ� //������ֵ��128���ڣ����Զ���Ϊchar
char right_motor_PWM=0;
 //char left_motor_PWM=50;   //���ҵ����PWMռ�ձ� //������ֵ��128���ڣ����Զ���Ϊchar
//char right_motor_PWM=50;

int servo_PWM_counter=0;//���ɶ��PWM�ļ�����
int servo_PWM=0;//���PWM //����char�ͱ����ķ�Χ
//unsigned char servo_middle=130;
			  unsigned char servo_middle=140;	  //��������Ըı�����ֹͣʱ�ĽǶȡ�

int P2H=0;
sbit end=P1^3;
sbit boma1=P1^4;
int error=0;    //����ƫ��
int error_history[ERROR_HISTORY_NUM]={0};  //��ʷƫ��

unsigned char servo_P=28;   //PIDϵ��
unsigned char servo_D=55 ;
																										  
char speed_expect=80;  //�����ٶ�			 ,90̫���ˣ�85�ȽϿ졣

int timecount=0;
int frequent=0;
int T0count=0;
int T1count=0;

int count=0;
int temp=0;
int worse=0;					//��worse�Ӵ�!!!!!

int SUM=0;

void sys_init(void)    //ϵͳ��ʼ��
{
//��flash�Ѿ��������Ҳ�Ϊ0xff,���ϴ��޸Ĳ��������ݸ�����Ӧ�����������״�flash����������
if(read(0x00,0x00)!=255) servo_P=read(0x00,0x00);//	servo_P�洢��ַ
if(read(0x00,0x01)!=255) servo_D=read(0x00,0x01);//	servo_D�洢��ַ
if(read(0x00,0x02)!=255) servo_middle=read(0x00,0x02);//servo_middle�洢��ַ
if(read(0x00,0x03)!=255) speed_expect=read(0x00,0x03);//speed_expect�洢��ַ�����Լ��ӵ�

	//------IO������------------
    P0M1=0x00; 	//�������
    P0M0=0xff;
    P0=0xff;   

    P1M1=0x00; //׼˫���
    P1M0=0x00;
    P1=0xff; //��ʼ��Ϊ�ߵ�ƽ 
	
	P2M1=0x00;//ָʾ�ƣ�����Ϊ���
    P2M0=0x00;
    P2=0xff;

    P3M1=0x00;//P3�ڸ���λ��Ϊ�����������PWM������λ���ó�׼˫��ڣ������ⲿ�ж��������
    P3M0=0xf0;
	P3=0x0f;

    LEFT_MOTOR_PWM_PORT=0; //PWM����Ϊ�͵�ƽ�������ת
    RIGHT_MOTOR_PWM_PORT=0;
    //left_motor_PWM=0;    //ռ�ձ�����Ϊ0
    //right_motor_PWM=0;
	left_motor_PWM=80;    //ռ�ձ�����Ϊ0
    right_motor_PWM=80;
    servo_PWM=servo_middle;//�����ǵ�����

    //------��ʱ������------------
    AUXR=AUXR | 0x80; //��ʱ��0����Ƶ
    TMOD=0x11;        //��ʱ����ʽ1
    TH0=T0_HIGH;      //��ʱ������ֵ
    TL0=T0_LOW ;
 /*****************��û���ٶȱջ��������ֿ���ע�͵�*********************/
 /*
 //������·�ⲿ�жϣ����ڱ������������
    IT0=1;//�½��ش���
	EX0=1; 
	IT1=1;//�½��ش���
	EX1=1;
	IPH=0x04;
	IP=0x04;//PX1=1;//���ⲿ�ж�1���ȼ���Ϊ���
	*/												//��û�б�����������ע�͵�
/**********************��������ɲ���STC12�Դ���·Ӳ�����岶׽����ʵ�֣�����CCAPM0\CCAPM1***********************************************/   
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
void getposition(void)//ǰ��9������Թܣ�Ҳ���Խ���Լ�����Թܸ������Ų���������ƫ��
{
       
	   P2H=(P2&0xff);//������1-8������Թ�״̬������ѹ�Ƚ�����������P2.7-P2.0
	   P2H=(P2H<<1);//����һλ
	   P2H=P2H+(P1&0x80);//������һλ���ֵ��λ���ϴ����ҵ�9������Թ�(����ѹ�Ƚ�����������P1.7)��ֵ��P1&0x80�ǽ�P1�ڵ����λȡ�� 
	   //P2H=001000000	;//���Լ��ӵĲ���һ���ҵ�Ƭ���Ľӿ��Ƿ��ˡ�
       switch(P2H&0x1ff)//����P2H�ĵ;�λ��ÿһλ��״̬������ǰ�����Թ��յ������ϵ������
	   //����ѹ�Ƚ�������ߵ�ƽ�����·�Ϊ����(��Ӧ��һλΪ1)�����ڰ�ɫ�ܵ���Ϊ�͵�ƽ(��Ӧ��һλΪ0)
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
        default: error=error_history[ERROR_HISTORY_NUM-1];worse++; break;    //�������ʱ��������һ��ƫ��
		}
 //worse��ʾ�����źţ�������0x000������������ź�ʱ��
 //��ʾ�źŲ�����,�ۼӼ������ﵽһ������ʱ�����϶����Ѿ����磬����ת��Ϊ0������ײ��
} 

    
void servo_control(void) //�������
{
    char i=0;

    if(ABS(error-error_history[4])>=11) //������ε�ƫ������򱣳���һ�ε�ƫ��
        error=error_history[4];      //��ֹ��ȷ������������ܳ���ܵ�����ɵ�ƫ������
    //����ƫ��������Ƕ����
     servo_PWM = servo_middle+(servo_P*error    //����
                   + servo_D*(error-error_history[4])//΢��
              )/10;   //����ʹ�ó���һ�����ϵ���ٳ���һ�����ķ���
                       //��С������ת�����������㣬��*0.1��ת��Ϊ*10/100���Լ��ᵥƬ���ĸ���
  //servo_PWM  =100	;
  //servo_PWM= servo_middle+(servo_P*error    //����
      //             + servo_D*(error-error_history[4]-50)//΢��
       //      )/3;   //����ʹ�ó���һ�����ϵ���ٳ���һ�����ķ���
                       //��С������ת�����������㣬��*0.1��ת��Ϊ*10/100���Լ��ᵥƬ���ĸ���


  // if(servo_PWM<95)      //�޷�
			 if(servo_PWM<90)      //�޷�
    //    servo_PWM=95;     //��ֹ�����������ֵ
			servo_PWM=90;     //��ֹ�����������ֵ
   // if(servo_PWM>165)
	 if(servo_PWM>170)
     //   servo_PWM=165;
		 servo_PWM=170;
  for(i=0;i<ERROR_HISTORY_NUM-1;i++)    //��¼ƫ��
    {
        error_history[i]=error_history[i+1];
    }
    error_history[ERROR_HISTORY_NUM-1]=error;

}



void motor_control(void) //�������
{
    left_motor_PWM=speed_expect;   //�ٶȿ���
    right_motor_PWM=speed_expect;  //����ֱ�Ӹ�����Ҳ���Ը��������������ó�

	if(worse>=30)//ǰ�洫�����źŲ��������ۼ�N���ж�Ϊ����ܵ�����ֹ��ײ��������ٶȸ�0		  //worse��Ҫ�Ӵ󣡣�������
	{
	left_motor_PWM=0;
    right_motor_PWM=0;
	}
	
	 
    if(left_motor_PWM>MOTOR_MAX_PWM_COUNT)   //�޷�
        left_motor_PWM=MOTOR_MAX_PWM_COUNT;
    if(left_motor_PWM<0)
        left_motor_PWM=0;

    if(right_motor_PWM>MOTOR_MAX_PWM_COUNT)
        right_motor_PWM=MOTOR_MAX_PWM_COUNT;
    if(right_motor_PWM<0)
        right_motor_PWM=0;
}

void flash_store()//����޸Ĳ���д��flash�����������������,��غ�����flash.c
{
del(0x00,0x00);//��������ʼ��
delay_ms(5);
write(0x00,0x00,servo_P);
delay_ms(5);
write(0x00,0x01,servo_D);
delay_ms(5);
write(0x00,0x02,servo_middle);
}


 /**********************************************************
��������servo_test
˵�������Զ���Ƿ��ܹ����������ڶ�
��ڣ���
���ڣ���
��ע����boma1�öŰ�������P1.4�ڣ��������뿪��1��ѡ������ģʽ��������ģʽ

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
}							//Ҷ���¼ӵ�,������һ�£��ǿ��Եġ�


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
		if(~end)  //�����⵽�͵�ƽ��˵����������
     {
  	 LCD_DLY_ms(10);   //��ʱȥ����һ��10-20ms
       if(~end)     //�ٴ�ȷ�ϰ����Ƿ��£�û�а������˳�
	     {
        while(~end);//���ȷ�ϰ��°����ȴ������ͷţ�û���ͷ���һֱ�ȴ�
        end=0;
	 	 }
	  }
	 flash_store();

    }				  */			 //�����ע�͵�֮����Գɹ��������������������
    while(1)
    {	
			 //�ҽ�����ɨ��ĳ����ƶ�������������
	   /*
	    	KeyScan();
		pre_show();
		if(~end)  //�����⵽�͵�ƽ��˵����������
     {
  	 LCD_DLY_ms(10);   //��ʱȥ����һ��10-20ms
       if(~end)     //�ٴ�ȷ�ϰ����Ƿ��£�û�а������˳�
	     {
        while(~end);//���ȷ�ϰ��°����ȴ������ͷţ�û���ͷ���һֱ�ȴ�
        end=0;
	 	 }
	  }
	 flash_store();		 //�ҵ�����ľͻ�Զ���ļ��ʱ����Ӱ�쵼�·�Ӧ����ʱ��Ӱ��������ڣ�
	       */
		//
		if(!boma1)			 //ѡ��ģʽ-������Ի�����ѭ��
		servo_test();		 //�������
		else
		{
        getposition();  //�жϺ���λ��
        servo_control();//���ƶ��
		//servo_PWM=90;						//�������Ǹ�ѭ��ע�͵�֮������ſ�ʼ�����ã�������
		motor_control();//���Ƶ��
		}
        while(control_period_finished==0);  //�ȴ���������20ms����,��֤ÿ�ο��Ƶ�ʱ��������ͬ
        {
		control_period_finished=0; //��־λ����
	    }	
    }
}

void exint0() interrupt 0 //�ⲿ�ж�0����,�жϺ�0
{
   T0count++;  
}
void exint1() interrupt 2 //�ⲿ�ж�1���٣��жϺ�2
{
   T1count++;  
}
		

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
    if(servo_PWM_counter>1010)   //PWM�Ƚϡ�����
    {

        servo_PWM_counter=1; //��1��ʼ����
        control_period_finished=1;  //20ms�������ڵ����־λ��1����������˶����������20ms��50Hz����
                            //�������������������ڣ��ɶ���һ��������servo_PWM_counter�ļ�������ʱ
	    timecount++;
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
		
	
if(timecount>=5)//20ms*5=100ms��ÿ100msͳ��һ��������� 
    { 
     frequent=(T0count+T1count)/2;
     T0count=0;//��0���¼���
	 T1count=0;//��0���¼���
     timecount=0;
    }
}	

