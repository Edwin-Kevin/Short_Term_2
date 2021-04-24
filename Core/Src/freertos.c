/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * File Name          : freertos.c
  * Description        : Code for freertos applications
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright (c) 2021 STMicroelectronics.
  * All rights reserved.</center></h2>
  *
  * This software component is licensed by ST under Ultimate Liberty license
  * SLA0044, the "License"; You may not use this file except in compliance with
  * the License. You may obtain a copy of the License at:
  *                             www.st.com/SLA0044
  *
  ******************************************************************************
  */
/* USER CODE END Header */

/* Includes ------------------------------------------------------------------*/
#include "FreeRTOS.h"
#include "task.h"
#include "main.h"
#include "cmsis_os.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "gpio.h"
#include "GUI.h"
#include <stdio.h>
#include "tim.h"

/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
#define WS_LOGO 0
#define WS_GUI1 1
#define WS_GUI2 2
#define WS_GUI3 3
#define WS_GUI4 4
extern GUI_FLASH const GUI_FONT GUI_FontHZ_KaiTi_20;  //����������ͼƬ
extern GUI_FLASH const GUI_FONT GUI_FontHZ_KaiTi_16;
extern GUI_CONST_STORAGE GUI_BITMAP bmh;
extern GUI_CONST_STORAGE GUI_BITMAP bmz;

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
/* USER CODE BEGIN Variables */
int g_ws = WS_LOGO;    //��ʾ��ǰ��UI����
uint32_t intick = 0;   //���ڼ�¼ʱ���

volatile float temp = 0;

uint32_t beeptick = 0;

/* USER CODE END Variables */
/* Definitions for MainTask */
osThreadId_t MainTaskHandle;
const osThreadAttr_t MainTask_attributes = {
  .name = "MainTask",
  .stack_size = 512 * 4,
  .priority = (osPriority_t) osPriorityNormal,
};
/* Definitions for KeyTask */
osThreadId_t KeyTaskHandle;
const osThreadAttr_t KeyTask_attributes = {
  .name = "KeyTask",
  .stack_size = 256 * 4,
  .priority = (osPriority_t) osPriorityNormal,
};
/* Definitions for UartTask */
osThreadId_t UartTaskHandle;
const osThreadAttr_t UartTask_attributes = {
  .name = "UartTask",
  .stack_size = 256 * 4,
  .priority = (osPriority_t) osPriorityNormal,
};
/* Definitions for GUITask */
osThreadId_t GUITaskHandle;
const osThreadAttr_t GUITask_attributes = {
  .name = "GUITask",
  .stack_size = 256 * 4,
  .priority = (osPriority_t) osPriorityNormal,
};
/* Definitions for DataTask */
osThreadId_t DataTaskHandle;
const osThreadAttr_t DataTask_attributes = {
  .name = "DataTask",
  .stack_size = 128 * 4,
  .priority = (osPriority_t) osPriorityNormal,
};

/* Private function prototypes -----------------------------------------------*/
/* USER CODE BEGIN FunctionPrototypes */
void WSLogo(void);
void DrawLogo(void);
void DrawGUI1(void);
void Beep(int time,int tune);
void BeepDone(void);
void DispSeg(uint8_t num[4],uint8_t dot);

/* USER CODE END FunctionPrototypes */

void StartMainTask(void *argument);
void StartKeyTask(void *argument);
void StartUartTask(void *argument);
void StartGUITask(void *argument);
void StartDataTask(void *argument);

void MX_FREERTOS_Init(void); /* (MISRA C 2004 rule 8.1) */

/**
  * @brief  FreeRTOS initialization
  * @param  None
  * @retval None
  */
void MX_FREERTOS_Init(void) {
  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* USER CODE BEGIN RTOS_MUTEX */
  /* add mutexes, ... */
  /* USER CODE END RTOS_MUTEX */

  /* USER CODE BEGIN RTOS_SEMAPHORES */
  /* add semaphores, ... */
  /* USER CODE END RTOS_SEMAPHORES */

  /* USER CODE BEGIN RTOS_TIMERS */
  /* start timers, add new ones, ... */
  /* USER CODE END RTOS_TIMERS */

  /* USER CODE BEGIN RTOS_QUEUES */
  /* add queues, ... */
  /* USER CODE END RTOS_QUEUES */

  /* Create the thread(s) */
  /* creation of MainTask */
  MainTaskHandle = osThreadNew(StartMainTask, NULL, &MainTask_attributes);

  /* creation of KeyTask */
  KeyTaskHandle = osThreadNew(StartKeyTask, NULL, &KeyTask_attributes);

  /* creation of UartTask */
  UartTaskHandle = osThreadNew(StartUartTask, NULL, &UartTask_attributes);

  /* creation of GUITask */
  GUITaskHandle = osThreadNew(StartGUITask, NULL, &GUITask_attributes);

  /* creation of DataTask */
  DataTaskHandle = osThreadNew(StartDataTask, NULL, &DataTask_attributes);

  /* USER CODE BEGIN RTOS_THREADS */
  /* add threads, ... */
  /* USER CODE END RTOS_THREADS */

  /* USER CODE BEGIN RTOS_EVENTS */
  /* add events, ... */
  /* USER CODE END RTOS_EVENTS */

}

/* USER CODE BEGIN Header_StartMainTask */
/**
  * @brief  Function implementing the MainTask thread.
  * @param  argument: Not used
  * @retval None
  */
/* USER CODE END Header_StartMainTask */
void StartMainTask(void *argument)
{
  /* USER CODE BEGIN StartMainTask */
	uint8_t num[4] = {0};
  /* Infinite loop */
  for(;;)
  {
		switch(g_ws)
		{
			case WS_LOGO:
				WSLogo();
			  break;
			default:
				SetLeds(0);
			  break;
		}
		
		
		DispSeg(num,2);
		BeepDone();
		
    osDelay(1);
  }
  /* USER CODE END StartMainTask */
}

/* USER CODE BEGIN Header_StartKeyTask */
/**
* @brief Function implementing the KeyTask thread.
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_StartKeyTask */
void StartKeyTask(void *argument)
{
  /* USER CODE BEGIN StartKeyTask */
  /* Infinite loop */
  for(;;)
  {
		uint8_t key = ScanKey();
		if (key > 0)
			printf("%02X\n",key);   //�ڴ������������ֵ
		
		switch(g_ws)
		{
			case WS_LOGO:
				if(key == KEY5)       //����KEY5��ֱ�ӽ��������棬��ʱ������
				{
					g_ws = WS_GUI1;
					intick = 0;
				}
				break;
			default:
				if(key == KEY6)       //����KEY6������������
					g_ws = WS_LOGO;
				break;
		}
    osDelay(1);
  }
  /* USER CODE END StartKeyTask */
}

/* USER CODE BEGIN Header_StartUartTask */
/**
* @brief Function implementing the UartTask thread.
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_StartUartTask */
void StartUartTask(void *argument)
{
  /* USER CODE BEGIN StartUartTask */
  /* Infinite loop */
  for(;;)
  {
    osDelay(1);
  }
  /* USER CODE END StartUartTask */
}

/* USER CODE BEGIN Header_StartGUITask */
/**
* @brief Function implementing the GUITask thread.
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_StartGUITask */
void StartGUITask(void *argument)
{
  /* USER CODE BEGIN StartGUITask */
  /* Infinite loop */
	GUI_Init();
    for(;;)
    {
			switch(g_ws)
			{
				case WS_LOGO:     //��������
					DrawLogo();
						break;
				case WS_GUI1:     //������
					DrawGUI1();
						break;
				default:
					break;
			}			
      osDelay(1);
    }
  /* USER CODE END StartGUITask */
}

/* USER CODE BEGIN Header_StartDataTask */
/**
* @brief Function implementing the DataTask thread.
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_StartDataTask */
void StartDataTask(void *argument)
{
  /* USER CODE BEGIN StartDataTask */
  /* Infinite loop */
  for(;;)
  {
    osDelay(1);
  }
  /* USER CODE END StartDataTask */
}

/* Private application code --------------------------------------------------*/
/* USER CODE BEGIN Application */
void WSLogo(void)
{
	static uint32_t tick = 0;
	static uint8_t leds_sta = 0x00;
	
	if(intick == 0)                          //�����ʱ��Ϊ0��˵��û�г�ʼ��
		intick = osKernelGetTickCount();       //��ʱ����ȡ��ǰʱ���
	else
	{
		if(osKernelGetTickCount() >= intick + 8000)  //������������Ѿ���ȥ8�룬����������
		{
			intick = 0;                          //��λ��ʱ��
			g_ws = WS_GUI1;
			Beep(500,3);                           //����������
		}
	}
	
	SetLeds(leds_sta);                       //����LED��
	if(osKernelGetTickCount() >= tick + 500) //ÿ��1�뷭תһ��LED
	{
		leds_sta = ~leds_sta;
		tick = osKernelGetTickCount();         //������һ����ʱ��tick��ֵ
	}
}

void DrawLogo(void)
{	
	if(intick == 0)
		intick = osKernelGetTickCount();
	else
	{
		if(osKernelGetTickCount() <= intick + 2000)  //0-1��ĩ
		{
			GUI_Clear();          //����
			GUI_SetFont(&GUI_FontHZ_KaiTi_20);   //��������
			GUI_DispStringHCenterAt("���𡢷���Զ\n�̼�������",64,12);   //��ĻΪ128*64
			GUI_Update();        //ˢ����Ļ����ʾ����
		}
		else if((osKernelGetTickCount() <= intick + 5000))  //2���-4��ĩ
		{
			GUI_Clear();
			GUI_SetFont(&GUI_FontHZ_KaiTi_16);
			GUI_DispStringHCenterAt("��Ա1:�ܿ���",64,0);
			GUI_DispStringHCenterAt("18041838",96,16);
			GUI_DispStringHCenterAt("��Ա2:�����",64,32);
			GUI_DispStringHCenterAt("18041814",96,50);
			GUI_Update();			
		}
		else if((osKernelGetTickCount() <= intick + 8000))  //5���-7��ĩ
		{
			GUI_Clear();
			GUI_DrawBitmap(&bmz,0,0);     //��ͼ��������δ��֤�������ص���Ƭ������һ��
			GUI_DrawBitmap(&bmh,64,0);
			GUI_Update();
		}
		else
			intick = 0;
	}
}

void DrawGUI1(void)
{
	GUI_Clear();
	GUI_SetFont(&GUI_FontHZ_KaiTi_20);
	GUI_DispStringHCenterAt("GUI1",64,12);
	GUI_Update();
}

void Beep(int time,int tune)
{
	static uint16_t TAB[] = {494,523,588,660,698,784,880,988};
	HAL_TIM_Base_Start(&htim3);
	
	if(tune >= 1 && tune <= 7)
	{
		int pre = 1000000 / TAB[tune];
		__HAL_TIM_SET_AUTORELOAD(&htim3,pre);
		__HAL_TIM_SET_COMPARE(&htim3,TIM_CHANNEL_1,pre / 2);
		
		beeptick = osKernelGetTickCount() + time;
		HAL_TIM_PWM_Start(&htim3,TIM_CHANNEL_1);
	}
}

void BeepDone(void)
{
	if(beeptick > 0 && osKernelGetTickCount() >= beeptick)
	{
		beeptick = 0;
		HAL_TIM_PWM_Stop(&htim3,TIM_CHANNEL_1);
	}
}


void DispSeg(uint8_t num[4],uint8_t dot)
{
	for(int i = 0;i < 4;++i)
	{
		Write595(i,num[i],(dot == (i + 1)) ? 1 : 0);
		osDelay(1);
	}
}
/* USER CODE END Application */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
