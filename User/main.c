#include "stm32f10x.h"                  // Device header
#include "Delay.h"
#include "LED.h"
#include "Key.h"
#include "OLED.h"
#include "LightSensor.h"
#include "PWM.h"
#include "Sound.h"

uint8_t KeyNum;
uint8_t mode = 0;   /* 0=自动调光 1=手动最亮 */
uint8_t enable = 1; /* 0=关闭输出 1=开启 */
uint16_t sound_timer = 0;   /* 声音触发后延时保持,单位100ms */
uint16_t last_sound = 0;    /* 上一次声音值,用于检测边沿 */

int main(void)
{
	OLED_Init();
	LightSensor_Init();
	Key_Init();
	PWM_Init();
	Sound_Init();

	OLED_ShowString(0, 0, "模式：", OLED_8X16);
	OLED_ShowString(0, 16, "灯的状态：", OLED_8X16);

	OLED_Update();

	while (1)
	{
		uint16_t adc = LightSensor_Get();
		uint16_t sound = Sound_Get();

		/* 边沿触发:声音从大变小(>=2000 -> <2000)时才启动5秒计时 */
		if (last_sound >= 2000 && sound < 2000)
			sound_timer = 50;
		else if (sound_timer > 0)
			sound_timer--;
		last_sound = sound;

		KeyNum = Key_GetNum();
		if (KeyNum == 1)          /* K1: 开关输出 */
		{
			enable = !enable;
		}
		if (KeyNum == 2)          /* K2: 切换自动/手动模式 */
		{
			mode++;
			if (mode > 1) mode = 0;
		}

		uint16_t duty;
		if (!enable)
			duty = 0;                         /* 全局关闭 */
		else if (mode == 1)
			duty = 100;                       /* 手动最亮 */
		else if (sound_timer > 0)
			duty = 100;                       /* 声音触发后延时保持5秒 */
		else if (adc > 2500)
			duty = adc * 100 / 4095;         /* 光敏调光 */
		else
			duty = 0;                         /* 都不满足,灭 */

		PWM_SetCompare(duty);

		OLED_ShowString(40, 0, mode ? "手动" : "自动", OLED_8X16);
		OLED_ShowString(80, 16, duty ? "开启" : "关闭", OLED_8X16);
		OLED_Update();
		Delay_ms(100);
	}
}
