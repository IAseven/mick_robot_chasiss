#include "stm32f10x.h"
#include "stm32f10x_gpio.h"
#include "stm32f10x_rcc.h"
#include "misc.h" 
#include "stm32f10x_i2c.h"


#include "AT24C02.h"




/*************************************************************************
*  �������ƣ�AT24C02_I2C_BufferRead
*  ����˵����AT24C02�����ݺ���
*  ����˵����I2Cn             ģ��ţ�I2C1��I2C2��
*            pBuffer          ���ݴ���׵�ַ  ���൱����������
*            ReadAddr         ��ʼ�����ݵĵ�ַ
*            NumByteToRead    ��ȡ�ĸ���
*
*  �������أ���
*  �޸�ʱ�䣺2014-11-1
*  ��    ע��CRP  �Ѳ���
*************************************************************************/
void AT24C02_I2C_BufferRead(I2C_TypeDef* I2Cx,u8* pBuffer, u8 ReadAddr, u16 NumByteToRead)
{
	
   I2C_STM32_Read_Bytes(I2Cx,AT24C02_EEPROM_ADDRESS,ReadAddr,pBuffer,NumByteToRead); 

}
 /*************************************************************************
*  �������ƣ�AT24C02_I2C_BufferWrite
*  ����˵����AT24C02д���ݺ���
*  ����˵����I2Cn             ģ��ţ�I2C1��I2C2��
*            pBuffer          ���ݴ���׵�ַ  ���൱����������
*            WriteAddr        д�����ݵ��׵�ַ
*            NumByteToWrite   д��ĸ���
*
*  �������أ���
*  �޸�ʱ�䣺2014-11-1
*  ��    ע��CRP  �Ѳ���
*************************************************************************/
void AT24C02_I2C_BufferWrite(I2C_TypeDef* I2Cx,u8* pBuffer, u8 WriteAddr, u16 NumByteToWrite)
{
  u8 NumOfPage = 0, NumOfSingle = 0, Addr = 0, count = 0;

  Addr = WriteAddr % I2C_PageSize;
  count = I2C_PageSize - Addr;
  NumOfPage =  NumByteToWrite / I2C_PageSize;
  NumOfSingle = NumByteToWrite % I2C_PageSize;
 
  
  if(Addr == 0) /* If WriteAddr is I2C_PageSize aligned  */
  {
    /* If NumByteToWrite < I2C_PageSize */
    if(NumOfPage == 0) 
    {
      
			I2C_STM32_Write_Bytes(I2Cx,AT24C02_EEPROM_ADDRESS,WriteAddr,pBuffer,NumOfSingle);//д����ֽ�
      AT24C02_I2C_WaitEepromStandbyState();
    }
    /* If NumByteToWrite > I2C_PageSize */
    else  
    {
      while(NumOfPage--)
      {
 
				I2C_STM32_Write_Bytes(I2Cx,AT24C02_EEPROM_ADDRESS,WriteAddr,pBuffer,I2C_PageSize);//д����ֽ�
    	  AT24C02_I2C_WaitEepromStandbyState();
        WriteAddr +=  I2C_PageSize;
        pBuffer += I2C_PageSize;
      }

      if(NumOfSingle!=0)
      {
 
				I2C_STM32_Write_Bytes(I2Cx,AT24C02_EEPROM_ADDRESS,WriteAddr,pBuffer,NumOfSingle);//д����ֽ�
        AT24C02_I2C_WaitEepromStandbyState();
      }
    }
  }
  /* If WriteAddr is not I2C_PageSize aligned  */
  else 
  {
			/* If NumByteToWrite < I2C_PageSize */
			if(NumOfPage== 0) 
			{
				
						if(NumOfSingle > count)
						{
							  I2C_STM32_Write_Bytes(I2Cx,AT24C02_EEPROM_ADDRESS,WriteAddr,pBuffer,count);//д����ֽ�
								AT24C02_I2C_WaitEepromStandbyState();
								pBuffer+=count;
								WriteAddr+=count;				 
							  I2C_STM32_Write_Bytes(I2Cx,AT24C02_EEPROM_ADDRESS,WriteAddr,pBuffer,NumOfSingle-count);//д����ֽ�
								AT24C02_I2C_WaitEepromStandbyState();
						}
						else
						{
							  I2C_STM32_Write_Bytes(I2Cx,AT24C02_EEPROM_ADDRESS,WriteAddr,pBuffer,NumOfSingle);//д����ֽ�
								AT24C02_I2C_WaitEepromStandbyState();
								 
						}

			}
			/* If NumByteToWrite > I2C_PageSize */
			else
			{
							NumByteToWrite -= count;
							NumOfPage =  NumByteToWrite / I2C_PageSize;
							NumOfSingle = NumByteToWrite % I2C_PageSize;	
							
							if(count != 0)
							{  
 
								I2C_STM32_Write_Bytes(I2Cx,AT24C02_EEPROM_ADDRESS,WriteAddr,pBuffer,count);//д����ֽ�
								AT24C02_I2C_WaitEepromStandbyState();
								WriteAddr += count;
								pBuffer += count;
							} 
							
							while(NumOfPage--)
							{
 
								I2C_STM32_Write_Bytes(I2Cx,AT24C02_EEPROM_ADDRESS,WriteAddr,pBuffer,I2C_PageSize);//д����ֽ�
								AT24C02_I2C_WaitEepromStandbyState();
								WriteAddr +=  I2C_PageSize;
								pBuffer += I2C_PageSize;  
							}
							if(NumOfSingle != 0)
							{
								I2C_STM32_Write_Bytes(I2Cx,AT24C02_EEPROM_ADDRESS,WriteAddr,pBuffer,NumOfSingle);//д����ֽ�
								AT24C02_I2C_WaitEepromStandbyState();
							}
			}
   }  
}

 /*************************************************************************
*  �������ƣ�AT24C02_I2C_WaitEepromStandbyState
*  ����˵����AT24C02 �͹���ģʽ   �ڲ�����
*  ����˵���� ��
*
*  �������أ���
*  �޸�ʱ�䣺2014-11-1
*  ��    ע��CRP  �Ѳ���
*************************************************************************/
void AT24C02_I2C_WaitEepromStandbyState(void)      
{
  vu16 SR1_Tmp = 0;

  do
  {
    
    I2C_GenerateSTART(I2C1, ENABLE);/* Send START condition */
    
    SR1_Tmp = I2C_ReadRegister(I2C1, I2C_Register_SR1);/* Read I2C1 SR1 register */
    
    I2C_Send7bitAddress(I2C1, AT24C02_EEPROM_ADDRESS, I2C_Direction_Transmitter);/* Send EEPROM address for write */
  }while(!(I2C_ReadRegister(I2C1, I2C_Register_SR1) & 0x0002));
   
  I2C_ClearFlag(I2C1, I2C_FLAG_AF);/* Clear AF flag */
        
  I2C_GenerateSTOP(I2C1, ENABLE); /* STOP condition */
}