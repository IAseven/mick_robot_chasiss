/*
*            
*
* update 2019-10-02
* maker: crp
*/
#include "stm32f10x.h"
#include "stdio.h"//�������ڷ��͵�FILE����
#include "stdlib.h"

#include "stm32f10x_Delay.h"
 
// #include "speed_cntr.h"
#include "speed_control.h"
#include "bsp_uart.h"
#include "bsp_can.h"
#include "DBUS.h"
#include "DJI_Motor.h"
/*********************************************************/
/**********************�궨��****************************/
#define EnableInterrupt 	__set_PRIMASK(0) //���ж� д��0�� �����ж�   ��1�������ж�
#define DisableInterrupt 	__set_PRIMASK(1) //���ж� д��0�� �����ж�   ��1�������ж�
uint32_t TimingDelay; //����delay���Ƶľ�ȷ��ʱ
uint16_t ADC_ConvertedValue; 
 
#define DEBUUG 1
#define DEBUUG_MATRIX_KEYSACN 0

#define LED1_FLIP  GPIO_Flip_level(GPIOE,GPIO_Pin_5) 
#define LED2_FLIP  GPIO_Flip_level(GPIOE,GPIO_Pin_6) 
#define LED3_FLIP  GPIO_Flip_level(GPIOF,GPIO_Pin_8) 
 
/*********************************************************/
/******************ȫ�ֱ�������***************************/

volatile uint8_t UART1_DMA_Flag=0x00;
volatile uint8_t UART2_Flag=0x00;
volatile uint8_t CAN1_Flag=0x00;

volatile uint32_t Timer2_Counter1=0; //�ֱ�������ǽ��������Ƿ񳬹������Ʒ�Χ
volatile uint32_t Timer2_Counter2=0;
volatile uint32_t Timer2_Counter3=0; 


extern int set_v,set_spd[4];  //�����ĸ����Ŀ���ٶ�
extern command_t recived_cmd; //���̽�����λ������ṹ��

extern CanRxMsg RxMessage;				 //���ջ�����
extern uint8_t USART_RX_BUF[USART_REC_LEN];     //���ջ���,���USART_REC_LEN���ֽ�
extern rc_info_t dbus_rc;               //DBUS ����
extern moto_measure_t moto_chassis[4] ; //CAN��ȡ���״̬���ݱ��� 
extern moto_measure_t moto_info;
/*********************************************************/
/**********************��������***************************/
	
 
 
void control(void);
void Main_Delay(unsigned int delayvalue);
void Main_Delay_us(unsigned int delayvalue);
/*********************************************************/
/*****************������**********************************/
int main(void)
{	
	   unsigned int i=0;
     
	   
		 SysTick_Init();//��ʼ���δ�ʱ��
	
	   //����ʹ����PB3���ţ���˽�JTAG�������ض�����
	   //RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA|RCC_APB2Periph_AFIO,ENABLE);
	   //GPIO_PinRemapConfig(GPIO_Remap_SWJ_JTAGDisable, ENABLE); 
	
	   My_GPIO_Init(GPIOE,GPIO_Pin_5,GPIO_Mode_Out_OD, GPIO_Speed_10MHz);//��ʼ��LED�˿�
	   My_GPIO_Init(GPIOE,GPIO_Pin_6,GPIO_Mode_Out_OD, GPIO_Speed_10MHz);//��ʼ��LED�˿�
	   //My_GPIO_Init(GPIOF,GPIO_Pin_8,GPIO_Mode_Out_OD, GPIO_Speed_10MHz);//��ʼ��LED�˿�
	
	   My_Config_USART_Init(USART2,115200,1);	    
	   UART_send_string(USART2,"this is a test project�� \n");//�ַ�������
	
		 USART_DMA_Rec_Config(USART1,100000); //��������1 DMA���շ�ʽ

     CAN_Config();
		 DJI_Motor_Init();
     //CAN_DJI_C620_DataSend(10,10,10,10);
		 Mecanum_Wheel_Speed_Model(0,0,0);
		 	   
		 Timer_2to7_counter_Generalfuncation(TIM2,1000);//1ms
		 Timer_2to7_Generalfuncation_start(TIM2);
		 
		 //Timer_2to7_counter_Generalfuncation(TIM3,50000);//500ms
		 //Timer_2to7_Generalfuncation_start(TIM3);
     UART_send_string(USART2,"Init Successful ....\n");//�ַ�������
		 while(1)
			{
					
 
						if(UART1_DMA_Flag) //ң����������������߼�
						{									
							UART1_DMA_Flag=0x00;
							Timer2_Counter1=0; //��ն�ʱ������
							control();
								LED1_FLIP;
						  //rc_show_message();
						}
						if(UART2_Flag) //��λ��ָ���·��ӿ�
						{
							   
							  Timer2_Counter2=0; //��ն�ʱ������
							  UART2_Flag=0x00;
						}
					
						if(CAN1_Flag) //��ӡCAN�жϽ��յ�����
						{
							CAN1_Flag=0x00;  
							if(Timer2_Counter3 == 20) //1ms*20  50HZ ��ӡƵ��
							{
								LED2_FLIP;
								 Timer2_Counter3=0;
								DJI_Motor_Upload_Message();
								//DJI_Motor_Show_Message();
							}
							else if(Timer2_Counter3 > 20)  
							{
								 Timer2_Counter3=0;
							}
//							for(i=0;i<4;i++)
//							{
//								// ��ӡ�ĸ������ת�١�ת�ǡ��¶ȵ���Ϣ
//								UART_send_string(USART2,"M"); UART_send_data(USART2,i);UART_send_string(USART2,"�� ");
//								UART_send_string(USART2,"v:");UART_send_floatdat(USART2,moto_chassis[i].speed_rpm);UART_send_char(USART2,'\t');	
//								UART_send_string(USART2,"t_a:");UART_send_floatdat(USART2,moto_chassis[i].total_angle);UART_send_char(USART2,'\t');	
//								UART_send_string(USART2,"n:");UART_send_floatdat(USART2,moto_chassis[i].round_cnt);UART_send_char(USART2,'\t');	
//								UART_send_string(USART2,"a:");UART_send_floatdat(USART2,moto_chassis[i].angle);UART_send_char(USART2,'\n');	
//							}
//						  UART_send_char(USART2,'\n');
						}
						
						if((Timer2_Counter2>100*10) ) // �����ʱ����������4s��û�б����㣬˵��ͨѶ�������ж�
						{			     
									recived_cmd.flag =0; //��ǽ������ݲ�����
									Timer2_Counter2=0;
							    //Mecanum_Wheel_Speed_Model(0,0,0);
						}
						if((Timer2_Counter1>100*10) ) // �����ʱ����������4s��û�б����㣬˵��ͨѶ�������ж�
						{			     
									dbus_rc.sw1  =5; //��ǽ������ݲ�����
							    Timer2_Counter1=0;
									Mecanum_Wheel_Speed_Model(0,0,0);
						}
				
      }
      Mecanum_Wheel_Speed_Model(0,0,0);
 // exit from main() function
}
// �������߼�
void control(void)
{
	if(dbus_rc.sw1 !=1 )
	{
		if(dbus_rc.sw1 ==3)
		{
			//Mecanum_Wheel_Speed_Model((dbus_rc.ch4-1024)*5,(1027-dbus_rc.ch1)*5,(1024-dbus_rc.ch3)*2);
			Mecanum_Wheel_Speed_Model((dbus_rc.ch2-dbus_rc.ch2_offset)*3,(dbus_rc.ch1_offset-dbus_rc.ch1)*3,(dbus_rc.ch3_offset-dbus_rc.ch3)*1);
		}
		else if(dbus_rc.sw1 ==2)
		{
			//Mecanum_Wheel_Speed_Model((dbus_rc.ch4-1024)*5,(1027-dbus_rc.ch1)*5,(1024-dbus_rc.ch3)*2);
			Mecanum_Wheel_Speed_Model((dbus_rc.ch2-dbus_rc.ch2_offset)*6,(dbus_rc.ch1_offset-dbus_rc.ch1)*5,(dbus_rc.ch3_offset-dbus_rc.ch3)*2);
		}
		else
			Mecanum_Wheel_Speed_Model(0,0,0);
	}
	else if(dbus_rc.sw1 ==1 )
	{
		if(recived_cmd.flag) //���ڽ��������ݹ���
		{
			if(recived_cmd.cmd == 0xF1)
			{	
					Mecanum_Wheel_Rpm_Model(recived_cmd.tag_rpm1,recived_cmd.tag_rpm2,recived_cmd.tag_rpm3,recived_cmd.tag_rpm4);
			}
			else if(recived_cmd.cmd == 0xF2)
			{
					Mecanum_Wheel_Speed_Model(recived_cmd.tag_speed_x,recived_cmd.tag_speed_y,recived_cmd.tag_speed_z);
			}
			else if(recived_cmd.cmd == 0xE1) //��̼�����
			{
					 DJI_Motor_Clear_Odom();
			}
			else;
			//Delay_10us(50000);
			//DJI_Motor_Show_Message();
			//recived_cmd.flag =0;//ʹ��һ���Ժ���������
		}
		else
			Mecanum_Wheel_Speed_Model(0,0,0);
			
	}
	else;
//	{
//			Mecanum_Wheel_Speed_Model(0,0,0);
//	}

}
 
 //��ʱ���� 6.3ms
void Main_Delay(unsigned int delayvalue)
{
	unsigned int i;
	while(delayvalue-->0)
	{	
		i=5000;
		while(i-->0);
	}
}
void Main_Delay_us(unsigned int delayvalue)
{
//	unsigned int i;
	while(delayvalue-->0)
	{	
		;
	}
}
// --------------------------------------------------------------//
/***************************END OF FILE**********************/
