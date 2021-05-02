#include "vin.h"

/**
  * @��  ��  VIN �����ѹ����ʼ��
  * @��  ��  ��
  * @����ֵ  ��
  */
void Vin_Init(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;
	ADC_InitTypeDef ADC_InitStructure;

	//����GPIO
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);

	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AIN;
	GPIO_Init(GPIOB, &GPIO_InitStructure);

	//**����ADC******
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_ADC2, ENABLE);

	//ADCģʽ����
	ADC_InitStructure.ADC_Mode = ADC_Mode_Independent;
	ADC_InitStructure.ADC_ScanConvMode = DISABLE;
	ADC_InitStructure.ADC_ContinuousConvMode = DISABLE;
	ADC_InitStructure.ADC_ExternalTrigConv = ADC_ExternalTrigConv_None;
	ADC_InitStructure.ADC_DataAlign = ADC_DataAlign_Right;
	ADC_InitStructure.ADC_NbrOfChannel = 1;
	ADC_Init(ADC2, &ADC_InitStructure);	

	//����ADC��Ƶ����6 72M/6=12,ADC���ʱ�䲻�ܳ���14M
	RCC_ADCCLKConfig(RCC_PCLK2_Div6);

	//����ADCͨ����ת��˳��Ͳ���ʱ��
	ADC_RegularChannelConfig(ADC2, ADC_Channel_8, 1, ADC_SampleTime_239Cycles5);

	//ʹ��ADC
	ADC_Cmd(ADC2, ENABLE);

	//��ʼ��ADC У׼�Ĵ���  
	ADC_ResetCalibration(ADC2);

	//�ȴ�У׼�Ĵ�����ʼ�����
	while(ADC_GetResetCalibrationStatus(ADC2));

	//ADC��ʼУ׼
	ADC_StartCalibration(ADC2);

	//�ȴ�У׼���
	while(ADC_GetCalibrationStatus(ADC2)); 	
}

/**
  * @��  ��  VIN ��������ѹ
  * @��  ��  ��	  
  * @����ֵ  ��ѹֵ������100��������7.2V���Ϊ720��
  */
uint16_t Vin_GetVol_X100(void)
{
	uint16_t Vin_Vol,temp;

	//��������ת������ 
	ADC_SoftwareStartConvCmd(ADC2, ENABLE);  

	//�ȴ�ת������ 
	while(!ADC_GetFlagStatus(ADC2, ADC_FLAG_EOC ));

	temp = ADC_GetConversionValue(ADC2);
	Vin_Vol = (uint16_t)((3630 * temp)/4095);

	return Vin_Vol;
}