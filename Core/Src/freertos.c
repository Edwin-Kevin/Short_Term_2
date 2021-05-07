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
#include "DS_18B20.h"
#include "MPU6050.h"
#include "string.h"

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
#define MAX_DATA_LEN 77
extern GUI_FLASH const GUI_FONT GUI_FontHZ_KaiTi_20;  //导入字体库和图片
extern GUI_FLASH const GUI_FONT GUI_FontHZ_KaiTi_16;
extern GUI_FLASH const GUI_FONT GUI_FontHZ_SimSun_12;
extern GUI_CONST_STORAGE GUI_BITMAP bmh;
extern GUI_CONST_STORAGE GUI_BITMAP bmz;

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
/* USER CODE BEGIN Variables */
int g_ws = WS_LOGO;    //表示当前的UI界面
uint32_t intick = 0;   //用于记录时间戳

volatile float temp = 0;

uint32_t beeptick = 0;
uint8_t tempwarn = 0;
uint8_t mpuwarn = 0;

uint8_t pageidx = 0;
uint32_t warntick = 0;
uint32_t K1234_tick = 0;
uint8_t g_fax_data[MAX_DATA_LEN];
uint8_t g_fay_data[MAX_DATA_LEN];
uint8_t g_faz_data[MAX_DATA_LEN];


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
void DrawGUI2(void);
void DrawGUI3(void);
void DrawGUI4(void);
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
		
		if(tempwarn || mpuwarn)
		{
			if(warntick == 0)
				warntick = osKernelGetTickCount();
			else if(osKernelGetTickCount() >= warntick + 30000)
			{
				tempwarn = mpuwarn = 0;
				warntick = 0;
			}
			else
			{
				uint32_t tic = warntick + 30000 - osKernelGetTickCount();
				num[0] = (tic / 10000) % 10;
				num[1] = (tic / 1000) % 10;
				num[2] = (tic / 100) % 10;
				num[3] = (tic / 10) % 10;
				
				if((num[2] == 1 || num[2] == 3 || num[2] == 5) && mpuwarn == 1)
					Beep(100,num[2]);
				if((num[2] == 2 || num[2] == 4 || num[2] == 6) && tempwarn == 1)
					Beep(100,num[2]);
			}
		}
		else
			num[0] = num[1] = num[2] = num[3] = ' ';
		
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
		uint8_t key = ScanKey();  //获取按键值
		if (key > 0)
			printf("%02X\n",key);   //在串口上输出按键值
		
		
		if((mpuwarn || tempwarn) == 0)
		{
			switch(g_ws)
			{
				case WS_LOGO:
					if(key == KEY5)       //按下KEY5就直接进入主界面，计时器归零
					{
						g_ws = WS_GUI1;
						intick = 0;
					}
					break;
				case WS_GUI1:
					if(key == KEY1)
						g_ws = WS_GUI4;
					else if(key == KEY4)
						g_ws = WS_GUI2;
					else if(key == KEY2)
					{
						if(pageidx > 0)
							--pageidx;
					}
					else if(key == KEY3)
					{
						if(pageidx < 2)
							++pageidx;
					}
					else if(key == KEY6)
						g_ws = WS_LOGO;
					break;
				case WS_GUI2:
					if(key == KEY1)
						g_ws = WS_GUI1;
					else if(key == KEY4)
						g_ws = WS_GUI3;
					else if(key == KEY6)
						g_ws = WS_LOGO;					
					break;
				case WS_GUI3:
					if(key == KEY1)
						g_ws = WS_GUI2;
					else if(key == KEY4)
						g_ws = WS_GUI4;
					else if(key == KEY6)
						g_ws = WS_LOGO;
					break;
				case WS_GUI4:
					if(key == KEY1)
						g_ws = WS_GUI3;
					else if(key == KEY4)
						g_ws = WS_GUI1;
					else if(key == KEY6)
						g_ws = WS_LOGO;
					break;
				default:
					if(key == KEY6)       //按下KEY6返回启动界面
						g_ws = WS_LOGO;
					break;
				}
		}
		else
		{
			if(key == (KEY1 | KEY2 | KEY3 | KEY4))
			{
				
				if(K1234_tick == 0)
					K1234_tick = osKernelGetTickCount();
				else
					if(osKernelGetTickCount() >= K1234_tick + 3000)
					{
						mpuwarn = tempwarn = 0;
						K1234_tick = 0;
						warntick = 0;
					}
			}
			else
			{
				if(key > 0)
					K1234_tick = 0;
			}
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
				case WS_LOGO:
					DrawLogo();
					break;
				case WS_GUI1:
					DrawGUI1();
					break;
				case WS_GUI2:
					DrawGUI2();
					break;
				case WS_GUI3:
					DrawGUI3();
					break;
				case WS_GUI4:
					DrawGUI4();
					break;
				default:
					break;
			}
      osDelay(100);
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
	osDelay(1000);
	
	ds18b20_init();
	
	
	uint8_t mpuok = MPU_init();
	uint8_t cnt = 0;
	uint8_t idx = 0;
	while(cnt++ < 3 && !mpuok)
	{
		osDelay(500);
		mpuok = mpu_init();
	}
	
	uint32_t dstick = 0;
	uint32_t mputick = 0;
	
	int warcnt = 0;
	
  /* Infinite loop */
  for(;;)
  {
		if(osKernelGetTickCount() >= dstick + 1000)
		{
			dstick = osKernelGetTickCount();
			float ft = ds18b20_read();
			if(ft < 125)
			{
				temp = ft;
				
				if(temp >= 50)
				{
					tempwarn = 1;
				}
			}
		}
		
		if(mpuok)
		{
			if(osKernelGetTickCount() >= mputick + 50)
			{
				mputick = osKernelGetTickCount();
				MPU_getdata();
				g_fax_data[idx] = 32 - fAX * 20 / 90;
				g_fay_data[idx] = 32 - fAY * 20 / 180;
				g_faz_data[idx] = 32 - fAZ * 20 / 180;
				++idx;
				if(idx >= MAX_DATA_LEN)
				{
					memcpy(g_fax_data,g_fax_data + 1,MAX_DATA_LEN - 1);
					memcpy(g_fay_data,g_fay_data + 1,MAX_DATA_LEN - 1);
					memcpy(g_faz_data,g_faz_data + 1,MAX_DATA_LEN - 1);
					
					idx = MAX_DATA_LEN - 1;
				}
				
				if(ax * ax + ay * ay + az * az > 400000000)
				{
					if(++warcnt >= 3)
					{
						mpuwarn = 1;
					}
				}
				else
					warcnt = 0;
			}
		}
			
		
    osDelay(20);
  }
  /* USER CODE END StartDataTask */
}

/* Private application code --------------------------------------------------*/
/* USER CODE BEGIN Application */
void WSLogo(void)
{
	static uint32_t tick = 0;
	static uint8_t leds_sta = 0x00;
	
	if(intick == 0)                          //如果计时器为0，说明没有初始化
		intick = osKernelGetTickCount();       //计时器读取当前时间戳
	else
	{
		if(osKernelGetTickCount() >= intick + 8000)  //如果距离启动已经过去8秒，进入主界面
		{
			intick = 0;                          //复位计时器
			g_ws = WS_GUI1;
			Beep(500,3);                           //蜂鸣器鸣叫
		}
	}
	
	SetLeds(leds_sta);                       //设置LED灯
	if(osKernelGetTickCount() >= tick + 500) //每过1秒翻转一次LED
	{
		leds_sta = ~leds_sta;
		tick = osKernelGetTickCount();         //更新另一个计时器tick的值
	}
}

void DrawLogo(void)
{	
	if(intick == 0)
		intick = osKernelGetTickCount();
	else
	{
		if(osKernelGetTickCount() <= intick + 2000)  //0-1秒末
		{
			GUI_Clear();          //清屏
			GUI_SetFont(&GUI_FontHZ_KaiTi_20);   //设置字体
			GUI_DispStringHCenterAt("防火、防盗远\n程监测器设计",64,12);   //屏幕为128*64
			GUI_Update();        //刷新屏幕来显示内容
		}
		else if((osKernelGetTickCount() <= intick + 5000))  //2秒初-4秒末
		{
			GUI_Clear();
			GUI_SetFont(&GUI_FontHZ_KaiTi_16);
			GUI_DispStringHCenterAt("成员1:周楷文",64,0);
			GUI_DispStringHCenterAt("18041838",96,16);
			GUI_DispStringHCenterAt("成员2:胡彦成",64,32);
			GUI_DispStringHCenterAt("18041814",96,50);
			GUI_Update();			
		}
		else if((osKernelGetTickCount() <= intick + 8000))  //5秒初-7秒末
		{
			GUI_Clear();
			GUI_DrawBitmap(&bmz,0,0);     //绘图，本代码未验证，请下载到单片机测试一下
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
	GUI_SetFont(&GUI_FontHZ_SimSun_12);
	GUI_SetColor(GUI_COLOR_BLACK);
	GUI_DispStringAt("实时监测",0,0);
	GUI_SetColor(GUI_COLOR_WHITE);
	GUI_DispStringAt("数据曲线",0,13);
	GUI_DispStringAt("无线通信",0,26);
	GUI_DispStringAt("参数设置",0,39);
	GUI_DispStringAt("K1︽  K2《 K3》  K4︾",0,52);
	
	GUI_DrawHLine(52,0,128);
	GUI_DrawVLine(48,0,52);
	
	char buf[20];
	
	if(pageidx == 0)
	{
		GUI_DispStringAt("当前温度:",50,0);
		GUI_DispStringAt("震动报警:",50,26);
		sprintf(buf,"%.1f℃",temp);
		GUI_DispStringAt(buf,90,13);
		GUI_DispStringAt(mpuwarn ? "是" : "否",90,39);
	}
	else if(pageidx == 1)
	{
		sprintf(buf,"ax:%6d",ax);
		GUI_DispStringAt(buf,50,0);
		if(ax > 0)
			GUI_FillRect(70,13,70 + ax * 55 /32768,16);
		else if(ax < 0)
			GUI_DrawRect(70,30,70 + ax * 55 /32768,33);
		sprintf(buf,"ay:%6d",ay);
		GUI_DispStringAt(buf,50,17);
		if(ay > 0)
			GUI_FillRect(70,30,70 + ay * 55 /32768,33);
		else if(ay < 0)
			GUI_DrawRect(70,13,70 + ay * 55 /32768,16);
		sprintf(buf,"az:%6d",az);
		GUI_DispStringAt(buf,50,34);
		if(az > 0)
			GUI_FillRect(70,47,70 + az * 55 /32768,50);
		else if(az < 0)
			GUI_DrawRect(70,47,70 + az * 55 /32768,50);
	}
	else if(pageidx == 2)
	{
		sprintf(buf,"gx:%6d",gx);
		GUI_DispStringAt(buf,50,0);
		if(gx > 0)
			GUI_FillRect(70,13,70 + gx * 55 /32768,16);
		else if(gx < 0)
			GUI_DrawRect(70,30,70 + gx * 55 /32768,33);
		sprintf(buf,"gy:%6d",gy);
		GUI_DispStringAt(buf,50,17);
		if(gy > 0)
			GUI_FillRect(70,30,70 + gy * 55 /32768,33);
		else if(gy < 0)
			GUI_DrawRect(70,13,70 + gy * 55 /32768,16);
		sprintf(buf,"gz:%6d",gz);
		GUI_DispStringAt(buf,50,34);
		if(gz > 0)
			GUI_FillRect(70,47,70 + gz * 55 /32768,50);
		else if(gz < 0)
			GUI_DrawRect(70,47,70 + gz * 55 /32768,50);		
	}
	GUI_Update();
}

void DrawGUI2(void)
{
	GUI_Clear();
	GUI_SetFont(&GUI_FontHZ_SimSun_12);
	GUI_DispStringAt("实时监测",0,0);
	GUI_SetColor(GUI_COLOR_BLACK);
	GUI_DispStringAt("数据曲线",0,13);
	GUI_SetColor(GUI_COLOR_WHITE);
	GUI_DispStringAt("无线通信",0,26);
	GUI_DispStringAt("参数设置",0,39);
	GUI_DispStringAt("K1︽  K2《  K3》  K4︾",0,52);
	
	GUI_DrawHLine(52,0,128);
	GUI_DrawVLine(48,0,52);
	
	GUI_Update();
}

void DrawGUI3(void)
{
	GUI_Clear();
	GUI_SetFont(&GUI_FontHZ_SimSun_12);
	GUI_DispStringAt("实时监测",0,0);
	GUI_DispStringAt("数据曲线",0,13);
	GUI_SetColor(GUI_COLOR_BLACK);
	GUI_DispStringAt("无线通信",0,26);
	GUI_SetColor(GUI_COLOR_WHITE);
	GUI_DispStringAt("参数设置",0,39);
	GUI_DispStringAt("K1︽  K2《  K3》  K4︾",0,52);
	
	GUI_DrawHLine(52,0,128);
	GUI_DrawVLine(48,0,52);
	
	GUI_Update();	
}

void DrawGUI4(void)
{
	GUI_Clear();
	GUI_SetFont(&GUI_FontHZ_SimSun_12);
	GUI_DispStringAt("实时监测",0,0);
	GUI_DispStringAt("数据曲线",0,13);
	GUI_DispStringAt("无线通信",0,26);
	GUI_SetColor(GUI_COLOR_BLACK);
	GUI_DispStringAt("参数设置",0,39);
	GUI_SetColor(GUI_COLOR_WHITE);
	GUI_DispStringAt("K1︽  K2《  K3》  K4︾",0,52);
	
	GUI_DrawHLine(52,0,128);
	GUI_DrawVLine(48,0,52);
	
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
