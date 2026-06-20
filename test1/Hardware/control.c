#include "control.h"
#include "sr04.h"
#include "encoder.h"
#include "motor.h"

extern int Encoder_Left,Encoder_Right;
extern float distance;
extern TIM_HandleTypeDef htim1,htim2,htim3,htim4;
extern uint8_t Fore,Back,Left,Right,Stop,Mode;
extern uint8_t Fore2,Back2,Left2,Right2;

int speed_left=0,speed_right=0,turn_left=0,turn_right=0;
int MOTO1,MOTO2;

int constraint(int enco_l,int enco_r)
{
	int temp;
	temp = 0.5*(enco_l - enco_r);
	return temp;
}

void Control(void)
{
	GET_Distance();
	Encoder_Left = Read_Speed(&htim2);
	Encoder_Right = -Read_Speed(&htim4);
	
	if(HAL_GPIO_ReadPin(GPIOA,GPIO_PIN_15) == 0)Stop=1;
	
	if(distance < 10.0){
		Stop=1;
		if(Back){speed_left = -1000;speed_right = -1000;Stop=0;
			if(Back2){Back=0;Back2=0;}}
	}
	
	if(Stop)//启动LED显示
	{
		HAL_GPIO_WritePin(GPIOB,GPIO_PIN_0,GPIO_PIN_SET);
		HAL_GPIO_WritePin(GPIOB,GPIO_PIN_1,GPIO_PIN_RESET);
	}
	else
	{
		HAL_GPIO_WritePin(GPIOB,GPIO_PIN_1,GPIO_PIN_SET);
		HAL_GPIO_WritePin(GPIOB,GPIO_PIN_0,GPIO_PIN_RESET);
	}
	
	if(Mode)//模式一
	{
		if(Stop == 0 && speed_left == 0 && speed_right == 0)//速度为0且为行进状态时
		{
			if(Fore){speed_left = 1000;speed_right = 1000;}
			if(Back){speed_left = -1000;speed_right = -1000;}
		}
		if(Stop == 0)//行进状态
		{
			if(Fore){speed_left += 100;speed_right += 100;
				if(Fore2){Fore=0;Fore2=0;}}
			if(Back){speed_left -= 100;speed_right -= 100;
				if(Back2){Back=0;Back2=0;}}
			
			if(speed_left > 0 && speed_right > 0){speed_left -= 10;speed_right-=10;}
			else if(speed_left < 0 && speed_right < 0){speed_left += 10;speed_right+=10;}
			
			if(Left){turn_left += 1;turn_right -=1;
				if(Left2){Left=0;Left2=0;}}
			if(Right){turn_left -= 1;turn_right +=1;
				if(Right2){Right=0;Right2=0;}}
		}
	}
	
	if(!Mode)//模式二
	{
		if(Stop == 0)
		{
			if(Fore){speed_left += 1000;speed_right += 1000;Fore=0;}
			if(Back){speed_left -= 1000;speed_right -= 1000;Back=0;}
			if(Left){turn_left += 1;turn_right -=1;
				if(Left2){Left=0;Left2=0;}}
			if(Right){turn_left -= 1;turn_right +=1;
				if(Right2){Right=0;Right2=0;}}
		}
	}
	
	if(Stop)//停车
	{
		speed_left = speed_right = turn_left = turn_right = 0;
		Fore = Back = Left = Right = 0;
		Fore2 = Back2 = Left2 = Right2 = 0;
		Stop = 0;
	}
	
	if(turn_left > 0 && turn_right < 0)
	{
		turn_left--;turn_right++;
		MOTO1 = speed_left - 0.3*speed_left;
		MOTO2 = speed_right + 0.3*speed_right;
	}
	else if(turn_left < 0 && turn_right > 0)
	{
		turn_left++;turn_right--;
		MOTO1 = speed_left + 0.3*speed_left;
		MOTO2 = speed_right - 0.3*speed_right;
	}
	else
	{
		turn_left=0;turn_right=0;
		MOTO1 = speed_left + constraint(Encoder_Left,Encoder_Right);
		MOTO2 = speed_right - constraint(Encoder_Left,Encoder_Right);
	}
	
	Limit(&MOTO1,&MOTO2);
	Load(MOTO1,MOTO2);
}


