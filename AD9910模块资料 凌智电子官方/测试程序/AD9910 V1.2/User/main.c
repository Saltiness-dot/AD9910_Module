//-----------------------------------------------------------------
// 程序描述: AD9910测试程序
// 作　　者: 凌智电子
// 开始日期: 2020-05-01
// 完成日期: 2020-05-05
// 当前版本: V1.2，增加方波和三角波输出功能
// 历史版本:
//		v1.0:初始版本
//		v1.1:增加并行调制模式
// 调试工具: 凌智STM32核心开发板、2.8寸液晶、LZE_ST_LINK2
// 说　　明: 
//				
//-----------------------------------------------------------------
//												AD9910与STM32F1连接
//						  AD9910                    		STM32F1
//								MRT														PB15
//								CSN														PB14
//								SCK														PB13
//								SDI														PB12
//								IUP														PB11
//								DRH														PB10
//								DRC														PB9
//								PF0														PB8
//								PF1														PA7
//								PF2														PA6
//								OSK														PC7
//
//								GND														JGND
//
//			其中 	DRG功能也可以外接控制信号来控制    控制引脚为 DRC	   DRH
//			 			OSK功能也可以外界控制信号来控制    控制引脚为 OSK
// 如果需要进行并行调制模式, 则还需要对模块的P1接口进行连接
// 不需要并行调制模式, 不需要连接以下P1接口（模式的）

//								F0														PA5
//								F1														PA4
//								D0														PA3
//								D1														PA2
//								D2														PA1
//								D3														PA0
//								D4														PC5
//								D5														PC4
//								D6														PC3
//								D7														PC2
//								D8														PC1
//								D9														PC0
//								D10														PC8
//								D11														PC9
//								D12														PC10
//								D13														PC11
//								D14														PC12
//								D15														PC13
//
//
//-----------------------------------------------------------------
// 头文件包含
//-----------------------------------------------------------------
#include <stm32f10x.h>
#include "Delay.h"
#include "string.h"
#include "PeripheralInit.h"
#include "AD9910.h"
#include "key.h" 
#include "lze_lcd.h"
#include "stdio.h"
#include "fonts.h"

unsigned char static DataStr[]=__DATE__;
unsigned char static TimeStr[]=__TIME__;

u8 key_numb;											// 按键标志位
u8 numb=0;												// 标志位
u8 NB=0;													// AD9910功能标志位
u8 buf[10];												// 显示数组
int i;														// 循环标志位（验证用的）

// 初始化的单频频率
uint32_t fre_data[] = {0, 1, 10, 100, 500, 1000, 5000, 10000, 50000, 100000, 500000, 1000000, 5000000, 10000000, 50000000, 100000000, 200000000, 300000000, 380000000, 400000000, 420000000};
// 初始化的单频幅度
int amp_data[] = {0,1023,2047,4095,8191,9215,10239,11263,12287,13311,14335,15359,16383};
// 初始化的单频相位
int pha_data[] = {0,90,180,270,360};

// 并口的偏移量
int offset_fre[64] = {0, 1000, 2000, 4000, 8000, 10000, 20000, 40000, 60000};
// 并口的增益
int gain[64] = {0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15};
	
//-----------------------------------------------------------------
// int get_data(int pos_fre, int num[])
//-----------------------------------------------------------------
//
//  函数功能: 将num中的数值合成一个数
//  入口参数: pos_fre数组有效位数，num数据存放的数组
//  返回参数: 返回数据值
//  注意事项: 无
//
//-----------------------------------------------------------------
int get_data(int pos_fre, int num[])
{
	int i = 0, data;
	
	data = num[i];
	i++;
	while(i <= pos_fre)
	{
		data = data*10 + num[i];	
		i++;
	}
	
	return data;	
}
//-----------------------------------------------------------------
// void  clr_data(int pos_fre, int num[])
//-----------------------------------------------------------------
//
//  函数功能: 清除数组里面的数
//  入口参数: pos_fre数组有效位数，num数据存放的数组
//  返回参数: 无
//  注意事项: 无
//
//-----------------------------------------------------------------
void  clr_data(int pos_fre, int num[])
{
	int i = 0;
	
	for(; i <= pos_fre; i++)
	{
		num[i] = 0;
	}
}
//9910扫频参数
typedef struct dds_ad9910
{
	int start_parm;
	int stop_parm;
	int rising_step;
	int falling_step;
	int rising_time;
	int falling_time;
}DDS;
//-----------------------------------------------------------------
// 主程序
//-----------------------------------------------------------------
int main(void)
{
	int flag = 0;
	int num[10];
	DDS ad9910;
	uint8_t  tx_buf[64];
	int pos_fre = 0;
	int pos_f = 0,pos_a = 0, pos_p = 0;
	int pos_1 = 0,pos_2 = 0, pos_3 = 0, pos_4 = 0, pos_5 = 0, pos_6 = 0, pos_7 = 0;
	int posx = 0, posy = 40, posx1 = 100, posy1 = 40;
	int fre = 0;
	int amp = 0, pha = 0;
	PeripheralInit();																		// 外设初始化
	AD9910_Init();																			// AD9910时钟的设置和辅助DAC设置 以及AD9910复位
	LCD_Clear(LCD_COLOR_WHITE); // 设置背景色

	LCD_SetFont(&Font8, &LCD_DEFAULT_FONT_CN); 
	LCD_WriteString (   230,    150,  LCD_COLOR_RED,  LCD_COLOR_WHITE,  (uint8_t *)"HARDWARE_VERSION" );
	LCD_WriteString (   230,    170,  LCD_COLOR_RED,  LCD_COLOR_WHITE,  (uint8_t *)"FIRMWARE_VERSION" );
	sprintf((char *)tx_buf, "BUILD TIME: %s  %s", DataStr, TimeStr);
	LCD_WriteString(    150,    190,  LCD_COLOR_RED,  LCD_COLOR_WHITE,  tx_buf);
	LCD_SetFont(&LCD_DEFAULT_FONT, &LCD_DEFAULT_FONT_CN); 
	
	LCD_SetFont(&Font16, &Font16_CN);
	LCD_WriteString (     40,   220,  LCD_COLOR_RED,  LCD_COLOR_WHITE,  (uint8_t *)"k1向下  k2向上  k3确定  k4返回" );
	LCD_SetFont(&LCD_DEFAULT_FONT, &LCD_DEFAULT_FONT_CN);
	
	while(1)
	{
		LCD_SetFont(&Font8, &LCD_DEFAULT_FONT_CN); 
		LCD_WriteString (   230,    150,  LCD_COLOR_RED,  LCD_COLOR_WHITE,  (uint8_t *)"HARDWARE_VERSION" );
		LCD_WriteString (   230,    170,  LCD_COLOR_RED,  LCD_COLOR_WHITE,  (uint8_t *)"FIRMWARE_VERSION" );
		sprintf((char *)tx_buf, "BUILD TIME: %s  %s", DataStr, TimeStr);
		LCD_WriteString(    150,    190,  LCD_COLOR_RED,  LCD_COLOR_WHITE,  tx_buf);
		LCD_SetFont(&LCD_DEFAULT_FONT, &LCD_DEFAULT_FONT_CN); 
		
		LCD_WriteString (    60,    10,  LCD_COLOR_RED,  LCD_COLOR_WHITE,  (uint8_t *)"AD9910模式选择" );
		LCD_WriteString (  posx,  posy,  LCD_COLOR_RED,  LCD_COLOR_WHITE,  (uint8_t *)"->" );
		LCD_WriteString (    30,    40,  LCD_COLOR_RED,  LCD_COLOR_WHITE,  (uint8_t *)"1.单频模式" );
		LCD_WriteString (    30,    70,  LCD_COLOR_RED,  LCD_COLOR_WHITE,  (uint8_t *)"2.RAM模式" );
		LCD_WriteString (    30,   100,  LCD_COLOR_RED,  LCD_COLOR_WHITE,  (uint8_t *)"3.数字斜坡调制模式" );
		LCD_WriteString (    30,   130,  LCD_COLOR_RED,  LCD_COLOR_WHITE,  (uint8_t *)"4.OSK模式" );
		LCD_WriteString (    30,   160,  LCD_COLOR_RED,  LCD_COLOR_WHITE,  (uint8_t *)"5.并口调制" );
		
		 
		
		key_numb = Key_Scan();															// 按键扫描
		switch(key_numb)
		{
			case 1: 
				LCD_WriteString (  posx,  posy,  LCD_COLOR_RED,  LCD_COLOR_WHITE,  (uint8_t *)"   " );
				if(posy == 160)
				{
					posy = 10;
				}
				posy += 30;
				break;
			case 2:
				LCD_WriteString (  posx,  posy,  LCD_COLOR_RED,  LCD_COLOR_WHITE,  (uint8_t *)"   " );
				if(posy == 40)
				{
					posy = 190;
				}
				posy -= 30;
				break;
			case 3: 
				LCD_WriteString (    60,    10,  LCD_COLOR_RED,  LCD_COLOR_WHITE,  (uint8_t *)"                            " );
				LCD_WriteString (  posx,  posy,  LCD_COLOR_RED,  LCD_COLOR_WHITE,  (uint8_t *)"                            " );
				LCD_WriteString (    30,    40,  LCD_COLOR_RED,  LCD_COLOR_WHITE,  (uint8_t *)"                            " );
				LCD_WriteString (    30,    70,  LCD_COLOR_RED,  LCD_COLOR_WHITE,  (uint8_t *)"                            " );
				LCD_WriteString (    30,    100,  LCD_COLOR_RED,  LCD_COLOR_WHITE,  (uint8_t *)"                           " );
				LCD_WriteString (    30,    130,  LCD_COLOR_RED,  LCD_COLOR_WHITE,  (uint8_t *)"                           " );
				LCD_WriteString (    30,    160,  LCD_COLOR_RED,  LCD_COLOR_WHITE,  (uint8_t *)"                           " );
				while(key_numb != 4)
				{
					LCD_WriteString (   230,    150,  LCD_COLOR_RED,  LCD_COLOR_WHITE,  (uint8_t *)"                " );
					LCD_WriteString (   230,    170,  LCD_COLOR_RED,  LCD_COLOR_WHITE,  (uint8_t *)"                " );
					LCD_WriteString (   150,    190,  LCD_COLOR_RED,  LCD_COLOR_WHITE,  (uint8_t *)"          " );
					switch(posy/10)
					{
						case 4:  //单频模式
							posx = 0;
							posy = 40;
							posx1 = 100; posy1 = 40; 
							LCD_WriteString (   100,    10,  LCD_COLOR_RED,  LCD_COLOR_WHITE,  (uint8_t *)"单频模式" );
					  	LCD_WriteString (    40,    40,  LCD_COLOR_RED,  LCD_COLOR_WHITE,  (uint8_t *)"频率:" );
						  LCD_WriteString (    40,    70,  LCD_COLOR_RED,  LCD_COLOR_WHITE,  (uint8_t *)"幅度:" );
						  LCD_WriteString (    40,   100,  LCD_COLOR_RED,  LCD_COLOR_WHITE,  (uint8_t *)"相位:" );
							LCD_WriteString (  posx,  posy,  LCD_COLOR_RED,  LCD_COLOR_WHITE,  (uint8_t *)"->" );

						
							LCD_WriteString (    100,    40,  LCD_COLOR_RED,  LCD_COLOR_WHITE,  (uint8_t *)"          " );
							LCD_WriteNumLong(    100,    40,  LCD_COLOR_RED,  LCD_COLOR_WHITE,  fre_data[pos_f]);
							LCD_WriteString (    100,    70,  LCD_COLOR_RED,  LCD_COLOR_WHITE,  (uint8_t *)"          " );
							LCD_WriteNumLong(    100,    70,  LCD_COLOR_RED,  LCD_COLOR_WHITE,  amp_data[pos_a]);
							LCD_WriteString (    100,    100,  LCD_COLOR_RED,  LCD_COLOR_WHITE,  (uint8_t *)"          " );
							LCD_WriteNumLong(    100,    100,  LCD_COLOR_RED,  LCD_COLOR_WHITE,  pha_data[pos_p]);
							while(key_numb != 4)
							{
								key_numb = Key_Scan();															// 按键扫描	
								switch(key_numb)
								{
									case 1:	
										if(posy1 == 40)
										{
											if(pos_f == (sizeof(fre_data)/sizeof(uint32_t)-1))
											{
												pos_f = 0;
											}
											pos_f++;
											LCD_WriteString (    posx1,    posy1,  LCD_COLOR_RED,  LCD_COLOR_WHITE,  (uint8_t *)"          " );
											LCD_WriteNumLong(    posx1,    posy1,  LCD_COLOR_RED,  LCD_COLOR_WHITE,  fre_data[pos_f]);
										}else if(posy1 == 70)
										{
											if(pos_a == (sizeof(amp_data)/sizeof(int)-1))
											{
												pos_a = 0;
											}
											pos_a++;
											LCD_WriteString (    posx1,    posy1,  LCD_COLOR_RED,  LCD_COLOR_WHITE,  (uint8_t *)"          " );
											LCD_WriteNumLong(    posx1,    posy1,  LCD_COLOR_RED,  LCD_COLOR_WHITE,  amp_data[pos_a]);
										}else if(posy1 == 100)
										{
											if(pos_p == (sizeof(pha_data)/sizeof(int)-1))
											{
												pos_p = 0;
											}
											pos_p++;
											LCD_WriteString (    posx1,    posy1,  LCD_COLOR_RED,  LCD_COLOR_WHITE,  (uint8_t *)"          " );
											LCD_WriteNumLong(    posx1,    posy1,  LCD_COLOR_RED,  LCD_COLOR_WHITE,  pha_data[pos_p]);
										}		
										break;
									case 2:	
									  if(posy1 == 40)
										{		
											if(pos_f == 0)
											{
												pos_f = sizeof(fre_data)/sizeof(uint32_t);
											}
											pos_f--;
											LCD_WriteString (    posx1,    posy1,  LCD_COLOR_RED,  LCD_COLOR_WHITE,  (uint8_t *)"          " );
											LCD_WriteNumLong(    posx1,    posy1,  LCD_COLOR_RED,  LCD_COLOR_WHITE,  fre_data[pos_f]);
									  }else if(posy1 == 70)
										{
											if(pos_a == 0)
											{
												pos_a = sizeof(amp_data)/sizeof(int);
											}
											pos_a--;
											LCD_WriteString (    posx1,    posy1,  LCD_COLOR_RED,  LCD_COLOR_WHITE,  (uint8_t *)"          " );
											LCD_WriteNumLong(    posx1,    posy1,  LCD_COLOR_RED,  LCD_COLOR_WHITE,  amp_data[pos_a]);
										}else if(posy1 == 100)
										{
											if(pos_p == 0)
											{
												pos_p = sizeof(pha_data)/sizeof(int)-1;
											}		
											pos_p--;
											LCD_WriteString (    posx1,    posy1,  LCD_COLOR_RED,  LCD_COLOR_WHITE,  (uint8_t *)"          " );
											LCD_WriteNumLong(    posx1,    posy1,  LCD_COLOR_RED,  LCD_COLOR_WHITE,  pha_data[pos_p]);								
										}
										break;
									case 3:		
										if(posy1 == 40)
										{
											fre = fre_data[pos_f];
										}else if(posy1 == 70)
										{
											amp = amp_data[pos_a];
										}else if(posy1 == 100)
										{
											pha = pha_data[pos_p];	
											AD9910_Singal_Profile_Init();																	// 单频PROFILE初始化
											AD9910_Singal_Profile_Set(0, fre, amp, pha);									// 设置正弦波信息 （ 通道（0~7） ， 频率 ，幅度 ， 相位 ）
										}	
										LCD_WriteString (  posx,  posy,  LCD_COLOR_RED,  LCD_COLOR_WHITE,  (uint8_t *)"  " );
										posy += 30;
										LCD_WriteString (  posx,  posy,  LCD_COLOR_RED,  LCD_COLOR_WHITE,  (uint8_t *)"->" );
										if(posy >= 130)
										{
											LCD_WriteString (  posx,  posy,  LCD_COLOR_RED,  LCD_COLOR_WHITE,  (uint8_t *)"  " );
										}	
										posy1 += 30;
										break;
									case 4:
										LCD_WriteString (    30,    40,  LCD_COLOR_RED,  LCD_COLOR_WHITE,  (uint8_t *)"                                            " );
										LCD_WriteString (    30,    70,  LCD_COLOR_RED,  LCD_COLOR_WHITE,  (uint8_t *)"                                            " );
										LCD_WriteString (    30,    100,  LCD_COLOR_RED,  LCD_COLOR_WHITE,  (uint8_t *)"                                           " );
										LCD_WriteString (    30,    130,  LCD_COLOR_RED,  LCD_COLOR_WHITE,  (uint8_t *)"                                           " );
										LCD_WriteString (  posx,  posy,  LCD_COLOR_RED,  LCD_COLOR_WHITE,  (uint8_t *)"  " );
										
									  posx = 0; posy = 40; posx1 = 100; posy1 = 40;	
										break;
									default:
										break;
								}
							}
							break;
						case 7:    //RAM模式
							posx = 0;
  						posy = 40;		
							while(key_numb != 4)
							{	
								flag = 0;
								LCD_WriteString (   100,    10,  LCD_COLOR_RED,  LCD_COLOR_WHITE,  (uint8_t *)"RAM模式" );
								LCD_WriteString (  posx,  posy,  LCD_COLOR_RED,  LCD_COLOR_WHITE,  (uint8_t *)"->" );						
								LCD_WriteString (    35,    40,  LCD_COLOR_RED,  LCD_COLOR_WHITE,  (uint8_t *)"1.直接转换模式" );
								LCD_WriteString (    35,    70,  LCD_COLOR_RED,  LCD_COLOR_WHITE,  (uint8_t *)"2.上斜坡模式" );
								LCD_WriteString (    35,   100,  LCD_COLOR_RED,  LCD_COLOR_WHITE,  (uint8_t *)"3.双向斜坡模式" );
								LCD_WriteString (    35,   130,  LCD_COLOR_RED,  LCD_COLOR_WHITE,  (uint8_t *)"4.连续双向斜坡模式" );
								LCD_WriteString (    35,   160,  LCD_COLOR_RED,  LCD_COLOR_WHITE,  (uint8_t *)"5.连续循环模式" );
								LCD_WriteString (    35,   190,  LCD_COLOR_RED,  LCD_COLOR_WHITE,  (uint8_t *)"6.三角波和方波" );
								while(flag == 0 && key_numb != 4)
								{
									key_numb = Key_Scan();															// 按键扫描
									switch(key_numb)
									{
										case 1:		
											LCD_WriteString (  posx,  posy,  LCD_COLOR_RED,  LCD_COLOR_WHITE,  (uint8_t *)"  " );
											if(posy == 190)
											{
												posy = 10;
											}
											posy += 30;
											LCD_WriteString (  posx,  posy,  LCD_COLOR_RED,  LCD_COLOR_WHITE,  (uint8_t *)"->" );
											break;
										case 2:
											LCD_WriteString (  posx,  posy,  LCD_COLOR_RED,  LCD_COLOR_WHITE,  (uint8_t *)"  " );
											if(posy == 40)
											{
												posy = 220;
											}
											posy -= 30;
											LCD_WriteString (  posx,  posy,  LCD_COLOR_RED,  LCD_COLOR_WHITE,  (uint8_t *)"->" );
											break;
										case 3:
											LCD_WriteString (  posx,  posy,  LCD_COLOR_RED,  LCD_COLOR_WHITE,  (uint8_t *)"  " );
											LCD_WriteString (   100,    10,  LCD_COLOR_RED,  LCD_COLOR_WHITE,  (uint8_t *)"               " );					
											LCD_WriteString (    30,    40,  LCD_COLOR_RED,  LCD_COLOR_WHITE,  (uint8_t *)"               " );
											LCD_WriteString (    30,    70,  LCD_COLOR_RED,  LCD_COLOR_WHITE,  (uint8_t *)"               " );
											LCD_WriteString (    30,   100,  LCD_COLOR_RED,  LCD_COLOR_WHITE,  (uint8_t *)"              " );
											LCD_WriteString (    30,   130,  LCD_COLOR_RED,  LCD_COLOR_WHITE,  (uint8_t *)"              " );
											LCD_WriteString (    30,   160,  LCD_COLOR_RED,  LCD_COLOR_WHITE,  (uint8_t *)"               " );
											LCD_WriteString (    30,   190,  LCD_COLOR_RED,  LCD_COLOR_WHITE,  (uint8_t *)"           " );
											if(posy == 40)
											{														
												while(key_numb != 5)
												{		
													LCD_WriteString (   100,    10,  LCD_COLOR_RED,  LCD_COLOR_WHITE,  (uint8_t *)"直接转换模式" );
													LCD_WriteString (  posx,  posy,  LCD_COLOR_RED,  LCD_COLOR_WHITE,  (uint8_t *)"->" );						
													LCD_WriteString (    30,    40,  LCD_COLOR_RED,  LCD_COLOR_WHITE,  (uint8_t *)"1.频率" );
													LCD_WriteString (    30,    70,  LCD_COLOR_RED,  LCD_COLOR_WHITE,  (uint8_t *)"2.幅度" );
													LCD_WriteString (    30,   100,  LCD_COLOR_RED,  LCD_COLOR_WHITE,  (uint8_t *)"3.相位" );
													key_numb = Key_Scan();															// 按键扫描	
													switch(key_numb)
													{
														case 1:
															LCD_WriteString (  posx,  posy,  LCD_COLOR_RED,  LCD_COLOR_WHITE,  (uint8_t *)"  " );	
															posy += 30;
															if(posy == 130)
															{
																posy = 40;
															}
															LCD_WriteString (  posx,  posy,  LCD_COLOR_RED,  LCD_COLOR_WHITE,  (uint8_t *)"->" );	
															break;
														case 2:
															LCD_WriteString (  posx,  posy,  LCD_COLOR_RED,  LCD_COLOR_WHITE,  (uint8_t *)"  " );	
															posy -= 30;
															if(posy == 10)
															{
																posy = 100;
															}
															LCD_WriteString (  posx,  posy,  LCD_COLOR_RED,  LCD_COLOR_WHITE,  (uint8_t *)"->" );	
															break;
														case 3:
															switch(posy/10)
															{
																case 4:
																	AD9910_RAM_Init();																							// RAM功能初始化
																	AD9910_RAM_Fre_W();																							// 写频率的RAM 写入profie0-4分别2M 4M 6M 8M 10M 			
																	AD9910_RAM_DIR_Fre_R();																					// 回放RAM——————直接转换模式-----频率
																	break;
																case 7:
																	AD9910_RAM_Init();																							// RAM功能初始化
																	AD9910_RAM_ZB_Fre_Init();																				// 载波频率初始化
																	AD9910_RAM_ZB_Fre_Set(1000000);																	// 设置载波频率
																	AD9910_RAM_AMP_W();																							// 写幅度的RAM 写入profile0 740mv左右  profile1 90mv左右 profile2 180mv左右  profile3 370mv左右 profile4 740mv左右
																	AD9910_RAM_DIR_AMP_R();																					// 回放RAM——————直接转换模式-----幅度
																	break;
																case 10:
																	AD9910_RAM_Init();																							// RAM功能初始化
																	AD9910_RAM_ZB_Fre_Init();                                       // 载波频率初始化
																	AD9910_RAM_ZB_Fre_Set(1000000);                                 // 设置载波频率
																	AD9910_RAM_Pha_W();                                             // 写相位的RAM
																	AD9910_RAM_DIR_PHA_R();                                         // 回放RAM——————直接转换模式-----相位				
																	break;
																default:
																	break;
															}
															Set_Profile(0);     //读取profile0的数据  profile的数据在AD9910.c文件中  
															Delay_50ms(10);
															Set_Profile(1);			//读取profile1的数据
															Delay_50ms(10);
															Set_Profile(2);			//读取profile2的数据
															Delay_50ms(10);
															Set_Profile(3);			//读取profile3的数据
															Delay_50ms(10);
															Set_Profile(4);			//读取profile4的数据
															Delay_50ms(10);
															Set_Profile(0);     //读取profile0的数据  profile的数据在AD9910.c文件中  
															break;
														case 4:
															LCD_WriteString (  posx,  posy,  LCD_COLOR_RED,  LCD_COLOR_WHITE,  (uint8_t *)"  " );	
															LCD_WriteString (   100,    10,  LCD_COLOR_RED,  LCD_COLOR_WHITE,  (uint8_t *)"              " );
															LCD_WriteString (  posx,  posy,  LCD_COLOR_RED,  LCD_COLOR_WHITE,  (uint8_t *)"        " );						
															LCD_WriteString (    30,    40,  LCD_COLOR_RED,  LCD_COLOR_WHITE,  (uint8_t *)"        " );
															LCD_WriteString (    30,    70,  LCD_COLOR_RED,  LCD_COLOR_WHITE,  (uint8_t *)"        " );
															LCD_WriteString (    30,    100,  LCD_COLOR_RED,  LCD_COLOR_WHITE,  (uint8_t *)"        " );
															flag = 1;
															key_numb = 5;
															posy = 40;
															break;
														default:
															break;
													}
												}
											}else if(posy == 70)
											{
												posy = 40;	
												while(key_numb != 5)
												{		
													LCD_WriteString (   100,    10,  LCD_COLOR_RED,  LCD_COLOR_WHITE,  (uint8_t *)"上斜坡模式" );
													LCD_WriteString (  posx,  posy,  LCD_COLOR_RED,  LCD_COLOR_WHITE,  (uint8_t *)"->" );						
													LCD_WriteString (    30,    40,  LCD_COLOR_RED,  LCD_COLOR_WHITE,  (uint8_t *)"1.频率" );
													LCD_WriteString (    30,    70,  LCD_COLOR_RED,  LCD_COLOR_WHITE,  (uint8_t *)"2.幅度" );
													LCD_WriteString (    30,   100,  LCD_COLOR_RED,  LCD_COLOR_WHITE,  (uint8_t *)"3.相位" );
													key_numb = Key_Scan();															// 按键扫描	
													switch(key_numb)
													{
														case 1:
															LCD_WriteString (  posx,  posy,  LCD_COLOR_RED,  LCD_COLOR_WHITE,  (uint8_t *)"  " );	
															posy += 30;
															if(posy == 130)
															{
																posy = 40;
															}
															LCD_WriteString (  posx,  posy,  LCD_COLOR_RED,  LCD_COLOR_WHITE,  (uint8_t *)"->" );	
															break;
														case 2:
															LCD_WriteString (  posx,  posy,  LCD_COLOR_RED,  LCD_COLOR_WHITE,  (uint8_t *)"  " );	
															posy -= 30;
															if(posy == 10)
															{
																posy = 100;
															}
															LCD_WriteString (  posx,  posy,  LCD_COLOR_RED,  LCD_COLOR_WHITE,  (uint8_t *)"->" );	
															break;
														case 3:
															switch(posy/10)
															{
																case 4:
																	AD9910_RAM_Init();																							// RAM功能初始化
																	AD9910_RAM_Fre_W();																							// 写频率的RAM 写入profie0-4分别2M 4M 6M 8M 10M
																	AD9910_RAM_RAMP_UP_TWO_Fre_R();																	// 回放RAM——————上斜坡模式-----频率       （CFR1寄存器中CFR1[20:17]内部控制）
															
																	break;
																case 7:
																	AD9910_RAM_Init();																							// RAM功能初始化
																	AD9910_RAM_ZB_Fre_Init();                                       // 载波频率初始化
																	AD9910_RAM_ZB_Fre_Set(1000000);                                 // 设置载波频率
																	AD9910_RAM_AMP_W();                                             // 写幅度的RAM  写入profile0 740mv左右  profile1 90mv左右 profile2 180mv左右  profile3 370mv左右 profile4 740mv左右
																	AD9910_RAM_RAMP_UP_TWO_AMP_R();                                 // 回放RAM——————上斜坡模式-----幅度       （CFR1寄存器中CFR1[20:17]内部控制）
																	break;
																case 10:
																	AD9910_RAM_Init();																							// RAM功能初始化
																	AD9910_RAM_ZB_Fre_Init();                                       // 载波频率初始化
																	AD9910_RAM_ZB_Fre_Set(1000000);                                 // 设置载波频率
																	AD9910_RAM_Pha_W();                                             // 写相位的RAM
																	AD9910_RAM_RAMP_UP_TWO_PHA_R();                                 // 回放RAM——————上斜坡模式-----相位       （CFR1寄存器中CFR1[20:17]内部控制）
																	break;
																default:
																	break;
															}
															Set_Profile(0);     //读取profile0的数据  profile的数据在AD9910.c文件中  
															Delay_50ms(10);
															Set_Profile(1);			//读取profile1的数据
															Delay_50ms(10);
															Set_Profile(2);			//读取profile2的数据
															Delay_50ms(10);
															Set_Profile(3);			//读取profile3的数据
															Delay_50ms(10);
															Set_Profile(4);			//读取profile4的数据
															Delay_50ms(10);
															Set_Profile(0);     //读取profile0的数据  profile的数据在AD9910.c文件中  
															break;
														case 4:
															LCD_WriteString (  posx,  posy,  LCD_COLOR_RED,  LCD_COLOR_WHITE,  (uint8_t *)"  " );	
													  	flag = 1;
															key_numb = 5;
															posy = 40;
															break;
														default:
															break;
													}
												}
											}else if(posy == 100)
											{
												posy = 40;	
												while(key_numb != 5)
												{		
													LCD_WriteString (   100,    10,  LCD_COLOR_RED,  LCD_COLOR_WHITE,  (uint8_t *)"双向斜坡模式" );
													LCD_WriteString (  posx,  posy,  LCD_COLOR_RED,  LCD_COLOR_WHITE,  (uint8_t *)"->" );						
													LCD_WriteString (    30,    40,  LCD_COLOR_RED,  LCD_COLOR_WHITE,  (uint8_t *)"1.频率" );
													LCD_WriteString (    30,    70,  LCD_COLOR_RED,  LCD_COLOR_WHITE,  (uint8_t *)"2.幅度" );
													LCD_WriteString (    30,   100,  LCD_COLOR_RED,  LCD_COLOR_WHITE,  (uint8_t *)"3.相位" );
													key_numb = Key_Scan();															// 按键扫描	
													switch(key_numb)
													{
														case 1:
															LCD_WriteString (  posx,  posy,  LCD_COLOR_RED,  LCD_COLOR_WHITE,  (uint8_t *)"  " );	
															posy += 30;
															if(posy == 130)
															{
																posy = 40;
															}
															LCD_WriteString (  posx,  posy,  LCD_COLOR_RED,  LCD_COLOR_WHITE,  (uint8_t *)"->" );	
															break;
														case 2:
															LCD_WriteString (  posx,  posy,  LCD_COLOR_RED,  LCD_COLOR_WHITE,  (uint8_t *)"  " );	
															posy -= 30;
															if(posy == 10)
															{
																posy = 100;
															}
															LCD_WriteString (  posx,  posy,  LCD_COLOR_RED,  LCD_COLOR_WHITE,  (uint8_t *)"->" );	
															break;
														case 3:
															switch(posy/10)
															{
																case 4:
																	AD9910_RAM_Init();																							// RAM功能初始化
																	AD9910_RAM_Fre_W();                                             // 写频率的RAM 写入profie0-4分别2M 4M 6M 8M 10M
																	AD9910_RAM_BID_RAMP_Fre_R();                                    // 回放RAM——————双向斜坡模式-----频率   
																
																	break;
																case 7:
																	AD9910_RAM_Init();																							// RAM功能初始化
																	AD9910_RAM_ZB_Fre_Init();                                       // 载波频率初始化
																	AD9910_RAM_ZB_Fre_Set(1000000);                                 // 设置载波频率
																	AD9910_RAM_AMP_W();                                             // 写幅度的RAM  写入profile0 740mv左右  profile1 90mv左右 profile2 180mv左右  profile3 370mv左右 profile4 740mv左右
																	AD9910_RAM_BID_RAMP_AMP_R();                                    // 回放RAM——————双向斜坡模式-----幅度
																	break;
																case 10:
																	AD9910_RAM_Init();																							// RAM功能初始化
																	AD9910_RAM_ZB_Fre_Init();                                       // 载波频率初始化
																	AD9910_RAM_ZB_Fre_Set(1000000);                                 // 设置载波频率
																	AD9910_RAM_Pha_W();                                             // 写相位的RAM
																	AD9910_RAM_BID_RAMP_PHA_R();                                    // 回放RAM——————双向斜坡模式-----相位
																	break;
																default:
																	break;
															}
//															Set_Profile(0);     //读取profile0的数据  profile的数据在AD9910.c文件中  
//															Delay_50ms(10);
//															Set_Profile(1);			//读取profile1的数据
//															Delay_50ms(10);
//															Set_Profile(2);			//读取profile2的数据
//															Delay_50ms(10);
//															Set_Profile(3);			//读取profile3的数据
//															Delay_50ms(10);
//															Set_Profile(4);			//读取profile4的数据
//															Delay_50ms(10);
//															Set_Profile(0);     //读取profile0的数据  profile的数据在AD9910.c文件中  
															break;
														case 4:
															LCD_WriteString (  posx,  posy,  LCD_COLOR_RED,  LCD_COLOR_WHITE,  (uint8_t *)"  " );	
															flag = 1;
															key_numb = 5;
															posy = 40;
															break;
														default:
															break;
													}
												}
											}else if(posy == 130)
											{
												posy = 40;	
												while(key_numb != 5)
												{		
													LCD_WriteString (   100,    10,  LCD_COLOR_RED,  LCD_COLOR_WHITE,  (uint8_t *)"连续双向斜坡模式" );
													LCD_WriteString (  posx,  posy,  LCD_COLOR_RED,  LCD_COLOR_WHITE,  (uint8_t *)"->" );						
													LCD_WriteString (    30,    40,  LCD_COLOR_RED,  LCD_COLOR_WHITE,  (uint8_t *)"1.频率" );
													LCD_WriteString (    30,    70,  LCD_COLOR_RED,  LCD_COLOR_WHITE,  (uint8_t *)"2.幅度" );
													LCD_WriteString (    30,   100,  LCD_COLOR_RED,  LCD_COLOR_WHITE,  (uint8_t *)"3.相位" );
													key_numb = Key_Scan();															// 按键扫描	
													switch(key_numb)
													{
														case 1:
															LCD_WriteString (  posx,  posy,  LCD_COLOR_RED,  LCD_COLOR_WHITE,  (uint8_t *)"  " );	
															posy += 30;
															if(posy == 130)
															{
																posy = 40;
															}
															LCD_WriteString (  posx,  posy,  LCD_COLOR_RED,  LCD_COLOR_WHITE,  (uint8_t *)"->" );	
															break;
														case 2:
															LCD_WriteString (  posx,  posy,  LCD_COLOR_RED,  LCD_COLOR_WHITE,  (uint8_t *)"  " );	
															posy -= 30;
															if(posy == 10)
															{
																posy = 100;
															}
															LCD_WriteString (  posx,  posy,  LCD_COLOR_RED,  LCD_COLOR_WHITE,  (uint8_t *)"->" );	
															break;
														case 3:
															switch(posy/10)
															{
																case 4:
																	AD9910_RAM_Init();																							// RAM功能初始化
																	AD9910_RAM_Fre_W();                                             // 写频率的RAM  写入profie0 2M 4M 6M 8M 10M
																	AD9910_RAM_CON_BID_RAMP_Fre_R();                                // 回放RAM——————连续双向斜坡模式-----频率
																	break;
																case 7:
																	AD9910_RAM_Init();																							// RAM功能初始化
																	AD9910_RAM_ZB_Fre_Init();                                       // 载波频率初始化
																	AD9910_RAM_ZB_Fre_Set(1000000);                                 // 设置载波频率
																	AD9910_RAM_AMP_W();                                             // 写幅度的RAM  写入profie0 90mv 180mv 370mv 740mv左右
																	AD9910_RAM_CON_BID_RAMP_AMP_R();                                // 回放RAM——————连续双向斜坡模式-----幅度
																	break;
																case 10:
																	AD9910_RAM_Init();																							// RAM功能初始化
																	AD9910_RAM_ZB_Fre_Init();                                       // 载波频率初始化
																	AD9910_RAM_ZB_Fre_Set(1000000);                                 // 设置载波频率
																	AD9910_RAM_Pha_W();                                             // 写相位的RAM
																	AD9910_RAM_CON_BID_RAMP_PHA_R();                                // 回放RAM——————连续双向斜坡模式-----相位
																	break;
																default:
																	break;
															}
															break;
														case 4:
															LCD_WriteString (  posx,  posy,  LCD_COLOR_RED,  LCD_COLOR_WHITE,  (uint8_t *)"  " );
															flag = 1;		
															key_numb = 5;														
															posy = 40;
															break;
														default:
															break;
													}
												}
											}else if(posy == 160)
											{
												posy = 40;	
												while(key_numb != 5)
												{		
													LCD_WriteString (   100,    10,  LCD_COLOR_RED,  LCD_COLOR_WHITE,  (uint8_t *)"连续循环模式"  );
													LCD_WriteString (  posx,  posy,  LCD_COLOR_RED,  LCD_COLOR_WHITE,  (uint8_t *)"->" );						
													LCD_WriteString (    30,    40,  LCD_COLOR_RED,  LCD_COLOR_WHITE,  (uint8_t *)"1.频率" );
													LCD_WriteString (    30,    70,  LCD_COLOR_RED,  LCD_COLOR_WHITE,  (uint8_t *)"2.幅度" );
													LCD_WriteString (    30,   100,  LCD_COLOR_RED,  LCD_COLOR_WHITE,  (uint8_t *)"3.相位" );
													key_numb = Key_Scan();															// 按键扫描	
													switch(key_numb)
													{
														case 1:
															LCD_WriteString (  posx,  posy,  LCD_COLOR_RED,  LCD_COLOR_WHITE,  (uint8_t *)"  " );	
															posy += 30;
															if(posy == 130)
															{
																posy = 40;
															}
															LCD_WriteString (  posx,  posy,  LCD_COLOR_RED,  LCD_COLOR_WHITE,  (uint8_t *)"->" );	
															break;
														case 2:
															LCD_WriteString (  posx,  posy,  LCD_COLOR_RED,  LCD_COLOR_WHITE,  (uint8_t *)"  " );	
															posy -= 30;
															if(posy == 10)
															{
																posy = 100;
															}
															LCD_WriteString (  posx,  posy,  LCD_COLOR_RED,  LCD_COLOR_WHITE,  (uint8_t *)"->" );	
															break;
														case 3:
															switch(posy/10)
															{
																case 4:
																	AD9910_RAM_Init();																							// RAM功能初始化
																	AD9910_RAM_Fre_W();                                             // 写频率的RAM 写入profie0 2M 4M 6M 8M 10M
																	AD9910_RAM_CON_RECIR_Fre_R();                                   // 回放RAM——————连续循环模式-----频率
																	break;
																case 7:
																	AD9910_RAM_Init();																							// RAM功能初始化
																	AD9910_RAM_ZB_Fre_Init();                                       // 载波频率初始化
																	AD9910_RAM_ZB_Fre_Set(1000000);                                 // 设置载波频率
																	AD9910_RAM_AMP_W();                                             // 写幅度的RAM 写入profie0 90mv 180mv 370mv 740mv左右
																	AD9910_RAM_CON_RECIR_AMP_R();                                   // 回放RAM——————连续循环模式-----幅度     （CFR1寄存器中CFR1[20:17]内部控制）
																	break;
																case 10:
																	AD9910_RAM_Init();																							// RAM功能初始化
																	AD9910_RAM_ZB_Fre_Init();                                       // 载波频率初始化
																	AD9910_RAM_ZB_Fre_Set(1000000);                                 // 设置载波频率
																	AD9910_RAM_Pha_W();                                             // 写相位的RAM
																	AD9910_RAM_CON_RECIR_PHA_R();                                   // 回放RAM——————连续循环模式-----相位
																	break;
																default:
																	break;
															}
															break;
														case 4:
															LCD_WriteString (  posx,  posy,  LCD_COLOR_RED,  LCD_COLOR_WHITE,  (uint8_t *)"  " );	
														  flag = 1;
															key_numb = 5;
															posy = 40;
															break;
														default:
															break;
													}
												}
											}
											else if(posy == 190)
											{
												posy = 40;	
												while(key_numb != 5)
												{		
													LCD_WriteString (  posx,  posy,  LCD_COLOR_RED,  LCD_COLOR_WHITE,  (uint8_t *)"->" );						
													LCD_WriteString (    30,    40,  LCD_COLOR_RED,  LCD_COLOR_WHITE,  (uint8_t *)"1.三角波" );
													LCD_WriteString (    30,    70,  LCD_COLOR_RED,  LCD_COLOR_WHITE,  (uint8_t *)"2.方波" );
													key_numb = Key_Scan();															// 按键扫描
													switch(key_numb)
													{
														case 1:
															LCD_WriteString (  posx,  posy,  LCD_COLOR_RED,  LCD_COLOR_WHITE,  (uint8_t *)"  " );	
															posy += 30;
															if(posy == 100)
															{
																posy = 40;
															}
															LCD_WriteString (  posx,  posy,  LCD_COLOR_RED,  LCD_COLOR_WHITE,  (uint8_t *)"->" );	
															break;
														case 2:
															LCD_WriteString (  posx,  posy,  LCD_COLOR_RED,  LCD_COLOR_WHITE,  (uint8_t *)"  " );	
															posy -= 30;
															if(posy == 10)
															{
																posy = 70;
															}
															LCD_WriteString (  posx,  posy,  LCD_COLOR_RED,  LCD_COLOR_WHITE,  (uint8_t *)"->" );	
															break;
														case 3:
															switch(posy/10)
															{
																case 4:
																	AD9910_RAM_Init();																							// RAM功能初始化
																	AD9910_RAM_ZB_Fre_Init();                                       // 载波频率初始化
																	AD9910_RAM_ZB_Fre_Set(0);                                       // 设置载波频率
																	AD9910_WAVE_RAM_AMP_W(2);                                       // 写幅度的RAM
																	AD9910_RAM_CON_RECIR_AMP_R();                                   // 回放RAM——————连续循环模式-----幅度     （CFR1寄存器中CFR1[20:17]内部�
																	break;
																case 7:
																	AD9910_RAM_Init();																							// RAM功能初始化
																	AD9910_RAM_ZB_Fre_Init();                                       // 载波频率初始化
																	AD9910_RAM_ZB_Fre_Set(0);                                       // 设置载波频率
																	AD9910_WAVE_RAM_AMP_W(3);                                       // 写幅度的RAM
																	AD9910_RAM_CON_RECIR_AMP_R();                                   // 回放RAM——————连续循环模式-----幅度     （CFR1寄存器中CFR1[20:17]内部�
																	break;
																default:
																	break;
															}
															break;
														case 4:
															LCD_WriteString (  posx,  posy,  LCD_COLOR_RED,  LCD_COLOR_WHITE,  (uint8_t *)"  " );	
														  flag = 1;
															key_numb = 5;
															posy = 40;
															break;
														default:
															break;
														}
													}
											}
											break;
									case 4:
										LCD_WriteString (  posx,  posy,  LCD_COLOR_RED,  LCD_COLOR_WHITE,  (uint8_t *)"  " );	
										LCD_WriteString (   100,    10,  LCD_COLOR_RED,  LCD_COLOR_WHITE,  (uint8_t *)"           " );
										LCD_WriteString (    30,    40,  LCD_COLOR_RED,  LCD_COLOR_WHITE,  (uint8_t *)"                  " );
										LCD_WriteString (    30,    70,  LCD_COLOR_RED,  LCD_COLOR_WHITE,  (uint8_t *)"                  " );
										LCD_WriteString (    30,   100,  LCD_COLOR_RED,  LCD_COLOR_WHITE,  (uint8_t *)"                  " );
										LCD_WriteString (    30,   130,  LCD_COLOR_RED,  LCD_COLOR_WHITE,  (uint8_t *)"                  " );
										LCD_WriteString (    30,   160,  LCD_COLOR_RED,  LCD_COLOR_WHITE,  (uint8_t *)"                  " );
										LCD_WriteString (    30,   190,  LCD_COLOR_RED,  LCD_COLOR_WHITE,  (uint8_t *)"                  " );
										posy = 40;
										break;
									default:
										break;
									}
								}
								if(key_numb != 4)
								{
//									Set_Profile(0);    //读取RAM profile0的数据
//									Delay_50ms(2);
//									Set_Profile(1);    //读取RAM profile1的数据
//									Delay_50ms(2);
//									Set_Profile(2);    //读取RAM profile2的数据
//									Delay_50ms(2);
//									Set_Profile(3);    //读取RAM profile3的数据
//									Delay_50ms(2);
//									Set_Profile(4);    //读取RAM profile4的数据
//									Delay_50ms(2);
//									Set_Profile(0);    //读取RAM profile0的数据
								}
							}
							break;
						case 10:   //数字斜坡调制模式
							posy = 40;
							while(key_numb != 4)
							{			
									LCD_WriteString (    0,    40,  LCD_COLOR_RED,  LCD_COLOR_WHITE,  (uint8_t *)"                     " );
									LCD_WriteString (    0,    70,  LCD_COLOR_RED,  LCD_COLOR_WHITE,  (uint8_t *)"                     " );
									LCD_WriteString (    0,   100,  LCD_COLOR_RED,  LCD_COLOR_WHITE,  (uint8_t *)"                     " );
									LCD_WriteString (    0,   130,  LCD_COLOR_RED,  LCD_COLOR_WHITE,  (uint8_t *)"                     " );
									LCD_WriteString (    0,   160,  LCD_COLOR_RED,  LCD_COLOR_WHITE,  (uint8_t *)"                     " );
									LCD_WriteString (    0,   190,  LCD_COLOR_RED,  LCD_COLOR_WHITE,  (uint8_t *)"                 " );																					
									while(key_numb != 4)
									{	
										LCD_WriteString (   60,    10,  LCD_COLOR_RED,  LCD_COLOR_WHITE,  (uint8_t *)"数字斜坡调制模式" );
										LCD_WriteString (  posx,  posy,  LCD_COLOR_RED,  LCD_COLOR_WHITE,  (uint8_t *)"->" );						
										LCD_WriteString (    30,    40,  LCD_COLOR_RED,  LCD_COLOR_WHITE,  (uint8_t *)"1.频率" );
										LCD_WriteString (    30,    70,  LCD_COLOR_RED,  LCD_COLOR_WHITE,  (uint8_t *)"2.幅度" );
										LCD_WriteString (    30,   100,  LCD_COLOR_RED,  LCD_COLOR_WHITE,  (uint8_t *)"3.相位" );
										key_numb = Key_Scan();		// 按键扫描
										switch(key_numb)
										{
											case 1:
												LCD_WriteString (  posx,  posy,  LCD_COLOR_RED,  LCD_COLOR_WHITE,  (uint8_t *)"  " );		
												posy += 30;
												if(posy == 130)
												{
													posy = 40;
												}
												LCD_WriteString (  posx,  posy,  LCD_COLOR_RED,  LCD_COLOR_WHITE,  (uint8_t *)"->" );		
												break;
											case 2:
												LCD_WriteString (  posx,  posy,  LCD_COLOR_RED,  LCD_COLOR_WHITE,  (uint8_t *)"  " );		
												posy -= 30;
												if(posy == 10)
												{
													posy = 100;
												}
												LCD_WriteString (  posx,  posy,  LCD_COLOR_RED,  LCD_COLOR_WHITE,  (uint8_t *)"->" );		
												break;
											case 3:
												LCD_WriteString (    30,    40,  LCD_COLOR_RED,  LCD_COLOR_WHITE,  (uint8_t *)"         " );
												LCD_WriteString (    30,    70,  LCD_COLOR_RED,  LCD_COLOR_WHITE,  (uint8_t *)"         " );
												LCD_WriteString (    30,   100,  LCD_COLOR_RED,  LCD_COLOR_WHITE,  (uint8_t *)"         " );
												LCD_WriteString (  posx,  posy,  LCD_COLOR_RED,  LCD_COLOR_WHITE,  (uint8_t *)"  " );
												posx1 = 100;
												posy1 = 40;
												switch(posy/10)
												{
													case 4:			
														LCD_WriteString (    0,    40,  LCD_COLOR_RED,  LCD_COLOR_WHITE,  (uint8_t *)"起始频率:" );
														LCD_WriteString (    0,    70,  LCD_COLOR_RED,  LCD_COLOR_WHITE,  (uint8_t *)"终止频率:" );	
														break;
													case 7:
														LCD_WriteString (    0,    40,  LCD_COLOR_RED,  LCD_COLOR_WHITE,  (uint8_t *)"起始幅度:" );
														LCD_WriteString (    0,    70,  LCD_COLOR_RED,  LCD_COLOR_WHITE,  (uint8_t *)"终止幅度:" );
														break;
													case 10:
														LCD_WriteString (    0,    40,  LCD_COLOR_RED,  LCD_COLOR_WHITE,  (uint8_t *)"起始相位:" );
														LCD_WriteString (    0,    70,  LCD_COLOR_RED,  LCD_COLOR_WHITE,  (uint8_t *)"终止相位:" );
														break;
													default:
														break;
												}
												LCD_WriteString (    0,   100,  LCD_COLOR_RED,  LCD_COLOR_WHITE,  (uint8_t *)"上升步进大小:" );
												LCD_WriteString (    0,   130,  LCD_COLOR_RED,  LCD_COLOR_WHITE,  (uint8_t *)"下降步进大小:" );
												LCD_WriteString (    0,   160,  LCD_COLOR_RED,  LCD_COLOR_WHITE,  (uint8_t *)"上升步进时间:" );
												LCD_WriteString (    0,   190,  LCD_COLOR_RED,  LCD_COLOR_WHITE,  (uint8_t *)"下降步进时间:" );
												LCD_SetFont(&Font16, &Font16_CN);
												LCD_WriteString (     40,   220,  LCD_COLOR_RED,  LCD_COLOR_WHITE,  (uint8_t *)"k1向下  k2向上  k3位权  k4确定" );
												LCD_SetFont(&LCD_DEFAULT_FONT, &LCD_DEFAULT_FONT_CN);
												posx1 += 10; 
												LCD_WriteString (    posx1,    posy1,  LCD_COLOR_RED,  LCD_COLOR_WHITE,  (uint8_t *)"_" );
												Delay_50ms(10);
												LCD_WriteString (    posx1,    posy1,  LCD_COLOR_RED,  LCD_COLOR_WHITE,  (uint8_t *)"  " );
												Delay_50ms(10);
												while(key_numb != 5)
												{
													key_numb = Key_Scan();		// 按键扫描
													switch(key_numb)
													{
														case 1:
															if(num[pos_fre] < 9)
															{
																num[pos_fre]++;
															}else
															{
																num[pos_fre] = 0;
															}
															LCD_WriteString (    posx1,    posy1,  LCD_COLOR_RED,  LCD_COLOR_WHITE,  (uint8_t *)" " );
															LCD_WriteNumChar(    posx1,    posy1,  LCD_COLOR_RED,  LCD_COLOR_WHITE,  (uint8_t )num[pos_fre]);
															break;
														case 2:
															if(num[pos_fre] > 0)
															{
																num[pos_fre]--;
															}else
															{
																num[pos_fre] = 9;
															}
															LCD_WriteString (    posx1,    posy1,  LCD_COLOR_RED,  LCD_COLOR_WHITE,  (uint8_t *)" " );
															LCD_WriteNumChar(    posx1,    posy1,  LCD_COLOR_RED,  LCD_COLOR_WHITE,  (uint8_t )num[pos_fre]);
															break;
														case 3:
															pos_fre++;
															posx1 += 15;
															LCD_WriteString (    posx1,    posy1,  LCD_COLOR_RED,  LCD_COLOR_WHITE,  (uint8_t *)"_" );
															Delay_50ms(10);
															LCD_WriteString (    posx1,    posy1,  LCD_COLOR_RED,  LCD_COLOR_WHITE,  (uint8_t *)"  " );
															Delay_50ms(10);
															break;
														case 4:
															if(posy1 == 40)
															{
																ad9910.start_parm = get_data(pos_fre, num);
																clr_data(pos_fre, num);
																posx1 = 110;
															}else if(posy1 == 70)
															{
																ad9910.stop_parm	= get_data(pos_fre, num);
																clr_data(pos_fre, num);
																posx1 = 154;
															}else if(posy1 == 100){
																ad9910.rising_step = get_data(pos_fre, num);
																clr_data(pos_fre, num);
																posx1 = 154;
															}else if(posy1 == 130)
															{
																ad9910.falling_step = get_data(pos_fre, num);
																clr_data(pos_fre, num);
																posx1 = 154;
															}else if(posy1 == 160)
															{
																ad9910.rising_time = get_data(pos_fre, num);
																clr_data(pos_fre, num);
																posx1 = 154;
															}else if(posy1 == 190)
															{	
																ad9910.falling_time = get_data(pos_fre, num);
																clr_data(pos_fre, num);
																posx1 = 110;	
																LCD_WriteString (    0,    40,  LCD_COLOR_RED,  LCD_COLOR_WHITE,  (uint8_t *)"                     " );
																LCD_WriteString (    0,    70,  LCD_COLOR_RED,  LCD_COLOR_WHITE,  (uint8_t *)"                     " );
																LCD_WriteString (    0,   100,  LCD_COLOR_RED,  LCD_COLOR_WHITE,  (uint8_t *)"                     " );
																LCD_WriteString (    0,   130,  LCD_COLOR_RED,  LCD_COLOR_WHITE,  (uint8_t *)"                     " );
																LCD_WriteString (    0,   160,  LCD_COLOR_RED,  LCD_COLOR_WHITE,  (uint8_t *)"                     " );
																LCD_WriteString (    0,   190,  LCD_COLOR_RED,  LCD_COLOR_WHITE,  (uint8_t *)"                     " );	
																LCD_SetFont(&Font16, &Font16_CN);
																LCD_WriteString (     40,   220,  LCD_COLOR_RED,  LCD_COLOR_WHITE,  (uint8_t *)"k1向下  k2向上  k3确定  k4返回" );
																LCD_SetFont(&LCD_DEFAULT_FONT, &LCD_DEFAULT_FONT_CN);
																key_numb = 5;
															}
															pos_fre = 0;	
															if(posy1 == 190)
															{
															
																if(posy == 40)
																{
																	AD9910_DRG_Fre_Init();																															// 数字斜坡初始化------------频率
																	// 设置（上限频率，下限频率，频率加步进，频率减步进，频率加步进时间，频率减步进时间）
																	AD9910_DRG_Freq_set(ad9910.stop_parm, ad9910.start_parm,   ad9910.rising_step,  ad9910.falling_step, ad9910.rising_time, ad9910.falling_time);		
																	AD9910_DRC_Set;
																}
																else if(posy == 70)
																{
																	AD9910_Singal_Profile_Init();																	// PROFILE初始化
																	AD9910_Singal_Profile_Set(0, 1000000, 0, 0);									// 设置正弦波信息 （ 通道（0~7） ， 频率 ，幅度 ， 相位 ）

																	AD9910_DRG_AMP_Init();
																	// 设置（上限幅度，下限幅度，幅度加步进，幅度减步进，幅度加步进时间，幅度减步进时间）
																	AD9910_DRG_Amp_Set(ad9910.stop_parm, ad9910.start_parm,   ad9910.rising_step,  ad9910.falling_step, ad9910.rising_time, ad9910.falling_time);																	
																}
																else if(posy == 100)
																{			
																	AD9910_Singal_Profile_Init();																    	// PROFILE初始化
																	AD9910_Singal_Profile_Set(0, 1000000, 16383, 0);									// 设置正弦波信息 （ 通道（0~7） ， 频率 ，幅度 ， 相位 ）
																	
																	AD9910_DRG_Pha_Init();
																	// 设置（上限相位，下限相位，相位加步进，相位减步进，相位加步进时间，相位减步进时间）
																	AD9910_DRG_Pha_Set(ad9910.stop_parm, ad9910.start_parm,   ad9910.rising_step,  ad9910.falling_step, ad9910.rising_time, ad9910.falling_time);  																	
																}
															}
															
															posy1 += 30;
															LCD_WriteString (    posx1,    posy1,  LCD_COLOR_RED,  LCD_COLOR_WHITE,  (uint8_t *)"_" );
															Delay_50ms(10);
															LCD_WriteString (    posx1,    posy1,  LCD_COLOR_RED,  LCD_COLOR_WHITE,  (uint8_t *)"  " );
															Delay_50ms(10);
															break;
														default:		
																					
															break;
													}
												}
											
												break;
											case 4:
												break;
											default:
												break;
										}
										if(posy == 40)
										{
										
										}else if(posy == 70)
										{
											AD9910_DRC_Set;	
											Delay_250us(50);
											AD9910_DRC_Clr;
											Delay_250us(50);	
										}else if(posy == 100)
										{
										}										
									}
								}
								AD9910_DRC_Set;	
								break;
						case 13:    //osk模式
							LCD_WriteString (   30,    10,  LCD_COLOR_RED,  LCD_COLOR_WHITE,  (uint8_t *)"OSK模式" );
							AD9910_Osk_Init();																							// OSK功能初始化
							AD9910_Singal_Profile_Set(0,1000000,0 ,0);											// 设置正弦波信息 （ 通道（0~7） ， 频率 ，幅度 ， 相位 ）-----只要设置通道和频率和相位就好，幅度不要设置
							AD9910_Osk_Set();
							while(key_numb != 4)
							{
								key_numb = Key_Scan();
								if(key_numb == 1)						// K1按键按下, 打开OSK输出
								{
									AD9910_OSK_Set;	
								}else if(key_numb == 2)			// K2按键按下, 关闭OSK输出
								{
									AD9910_OSK_Clr;	
								}else if(key_numb == 4)			// K4按键按下, 退出
								{
									LCD_WriteString (   30,    10,  LCD_COLOR_RED,  LCD_COLOR_WHITE,  (uint8_t *)"                         " );
								}  
							}		

							break;
						case 16:     //并行数据端口调制模式
								LCD_WriteString (    0,    40,  LCD_COLOR_RED,  LCD_COLOR_WHITE,  (uint8_t *)"                     " );
								LCD_WriteString (    0,    70,  LCD_COLOR_RED,  LCD_COLOR_WHITE,  (uint8_t *)"                     " );
								LCD_WriteString (    0,   100,  LCD_COLOR_RED,  LCD_COLOR_WHITE,  (uint8_t *)"                     " );
								LCD_WriteString (    0,   130,  LCD_COLOR_RED,  LCD_COLOR_WHITE,  (uint8_t *)"                     " );
								LCD_WriteString (    0,   160,  LCD_COLOR_RED,  LCD_COLOR_WHITE,  (uint8_t *)"                     " );
								LCD_WriteString (    0,   190,  LCD_COLOR_RED,  LCD_COLOR_WHITE,  (uint8_t *)"                 " );		
								LCD_WriteString (   60,    10,  LCD_COLOR_RED,  LCD_COLOR_WHITE,  (uint8_t *)"并行数据端口调制模式" );
								posy = 40;
							
								while(key_numb != 4)
								{		
									LCD_WriteString (   60,    10,  LCD_COLOR_RED,  LCD_COLOR_WHITE,  (uint8_t *)"并行数据端口调制模式" );
									LCD_WriteString (  posx,  posy,  LCD_COLOR_RED,  LCD_COLOR_WHITE,  (uint8_t *)"->" );						
									LCD_WriteString (    30,    40,  LCD_COLOR_RED,  LCD_COLOR_WHITE,  (uint8_t *)"1.频率" );
									LCD_WriteString (    30,    70,  LCD_COLOR_RED,  LCD_COLOR_WHITE,  (uint8_t *)"2.幅度" );
									LCD_WriteString (    30,   100,  LCD_COLOR_RED,  LCD_COLOR_WHITE,  (uint8_t *)"3.相位" );
									key_numb = Key_Scan();		// 按键扫描
									switch(key_numb)
									{
										case 1:
											LCD_WriteString (  posx,  posy,  LCD_COLOR_RED,  LCD_COLOR_WHITE,  (uint8_t *)"  " );
											posy += 30;					
											if(posy == 130)
											{
												posy = 40;
											}	
											LCD_WriteString (  posx,  posy,  LCD_COLOR_RED,  LCD_COLOR_WHITE,  (uint8_t *)"->" );
											break;
										case 2:
											LCD_WriteString (  posx,  posy,  LCD_COLOR_RED,  LCD_COLOR_WHITE,  (uint8_t *)"  " );
											posy -= 30;					
											if(posy == 10)
											{
												posy = 100;
											}	
											LCD_WriteString (  posx,  posy,  LCD_COLOR_RED,  LCD_COLOR_WHITE,  (uint8_t *)"->" );
											break;
										case 3:
											LCD_WriteString (    30,    40,  LCD_COLOR_RED,  LCD_COLOR_WHITE,  (uint8_t *)"         " );
											LCD_WriteString (    30,    70,  LCD_COLOR_RED,  LCD_COLOR_WHITE,  (uint8_t *)"         " );
											LCD_WriteString (    30,   100,  LCD_COLOR_RED,  LCD_COLOR_WHITE,  (uint8_t *)"         " );
											LCD_WriteString (  posx,  posy,  LCD_COLOR_RED,  LCD_COLOR_WHITE,  (uint8_t *)"  " );
											posx1 = 100;
											posy1 = 40;
											switch(posy/10)
											{
												case 4:
													LCD_WriteString (    30,    40,  LCD_COLOR_RED,  LCD_COLOR_WHITE,  (uint8_t *)"起始频率:" );
													LCD_WriteString (    30,    70,  LCD_COLOR_RED,  LCD_COLOR_WHITE,  (uint8_t *)"偏移频率:" );
													LCD_WriteString (    30,   100,  LCD_COLOR_RED,  LCD_COLOR_WHITE,  (uint8_t *)"增益:" );
													LCD_WriteString (    140,    40,  LCD_COLOR_RED,  LCD_COLOR_WHITE,  (uint8_t *)"           " );
													LCD_WriteNumLong(    140,    40,  LCD_COLOR_RED,  LCD_COLOR_WHITE,  fre_data[pos_1]);
													LCD_WriteString (    140,    70,  LCD_COLOR_RED,  LCD_COLOR_WHITE,  (uint8_t *)"           " );
													LCD_WriteNumLong(    140,    70,  LCD_COLOR_RED,  LCD_COLOR_WHITE,  offset_fre[pos_2]);
													LCD_WriteString (    90,    100,  LCD_COLOR_RED,  LCD_COLOR_WHITE,  (uint8_t *)"          " );
													LCD_WriteNumLong(    90,    100,  LCD_COLOR_RED,  LCD_COLOR_WHITE,  gain[pos_3]);
													LCD_WriteString (     0,  posy,  LCD_COLOR_RED,  LCD_COLOR_WHITE,  (uint8_t *)"->" );	
													while(key_numb != 5)
													{			
														key_numb = Key_Scan();		// 按键扫描
														switch(key_numb)
														{
															case 1:
																if(posy == 40)
																{			
																	pos_1 ++;
																	if(pos_1 == (sizeof(fre_data)/sizeof(uint32_t)-1))
																	{
																		pos_1 = 0;
																	}	
																	LCD_WriteString (    140,    40,  LCD_COLOR_RED,  LCD_COLOR_WHITE,  (uint8_t *)"          " );
																	LCD_WriteNumLong(    140,    40,  LCD_COLOR_RED,  LCD_COLOR_WHITE,  fre_data[pos_1]);
																}else if(posy == 70)
																{
																	pos_2 ++;
																	if(pos_2 == 9)
																	{
																		pos_2 = 0;
																	}	
																	LCD_WriteString (    140,    70,  LCD_COLOR_RED,  LCD_COLOR_WHITE,  (uint8_t *)"          " );
																	LCD_WriteNumLong(    140,    70,  LCD_COLOR_RED,  LCD_COLOR_WHITE,  offset_fre[pos_2]);
																}else if(posy == 100)
																{
																	pos_3 ++;
																	if(pos_3 == 16)
																	{
																		pos_3 = 0;
																	}	
																	LCD_WriteString (    90,    100,  LCD_COLOR_RED,  LCD_COLOR_WHITE,  (uint8_t *)"          " );
																	LCD_WriteNumLong(    90,    100,  LCD_COLOR_RED,  LCD_COLOR_WHITE,  gain[pos_3]);
																}
																break;
															case 2:
																if(posy == 40)
																{
																	if(pos_1 == 0)
																	{
																		pos_1 = (sizeof(fre_data)/sizeof(uint32_t)-1);
																	}
																	pos_1 --;
																	LCD_WriteString (    140,    40,  LCD_COLOR_RED,  LCD_COLOR_WHITE,  (uint8_t *)"          " );
																	LCD_WriteNumLong(    140,    40,  LCD_COLOR_RED,  LCD_COLOR_WHITE,  fre_data[pos_1]);
																}else if(posy == 70)
																{
																	if(pos_2 == 0)
																	{
																		pos_2 = 9;
																	}
																	pos_2 --;
																	LCD_WriteString (    140,    70,  LCD_COLOR_RED,  LCD_COLOR_WHITE,  (uint8_t *)"          " );
																	LCD_WriteNumLong(    140,    70,  LCD_COLOR_RED,  LCD_COLOR_WHITE,  offset_fre[pos_2]);				
																}else if(posy == 100)
																{
																	if(pos_3 == 0)
																	{
																		pos_3 = 16;
																	}
																	pos_3 --;
																	LCD_WriteString (    90,    100,  LCD_COLOR_RED,  LCD_COLOR_WHITE,  (uint8_t *)"          " );
																	LCD_WriteNumLong(    90,    100,  LCD_COLOR_RED,  LCD_COLOR_WHITE,  gain[pos_3]);				
																}
																break;
															case 3:
																LCD_WriteString (  0,  posy,  LCD_COLOR_RED,  LCD_COLOR_WHITE,  (uint8_t *)"  " );	
																posy += 30;
																LCD_WriteString (  0,  posy,  LCD_COLOR_RED,  LCD_COLOR_WHITE,  (uint8_t *)"->" );
																if(posy >= 130)
																{
																	LCD_WriteString (  0,  posy,  LCD_COLOR_RED,  LCD_COLOR_WHITE,  (uint8_t *)"  " );
																	Freq_convert(fre_data[pos_1]);				// 基础频率   
																	AD9910_Init_Sin(gain[pos_3]);
																	Par_mod(2, offset_fre[pos_2]);				// 调制频率
																	
																}
																break;
															case 4:
																LCD_WriteString (    0,    40,  LCD_COLOR_RED,  LCD_COLOR_WHITE,  (uint8_t *)"                  " );
																LCD_WriteString (    0,    70,  LCD_COLOR_RED,  LCD_COLOR_WHITE,  (uint8_t *)"             " );
																LCD_WriteString (    0,   100,  LCD_COLOR_RED,  LCD_COLOR_WHITE,  (uint8_t *)"             " );
																posy = 40;
																key_numb = 5;
																break;
															default:
																break;
														}
													}
													break;
												case 7:
													posy = 40;
													LCD_WriteString (    30,    40,  LCD_COLOR_RED,  LCD_COLOR_WHITE,  (uint8_t *)"起始幅度:" );
													LCD_WriteString (    30,    70,  LCD_COLOR_RED,  LCD_COLOR_WHITE,  (uint8_t *)"偏移幅度:" );
													LCD_WriteString (    140,    40,  LCD_COLOR_RED,  LCD_COLOR_WHITE,  (uint8_t *)"          " );
													LCD_WriteNumLong(    140,    40,  LCD_COLOR_RED,  LCD_COLOR_WHITE,  amp_data[pos_4]);
													LCD_WriteString (    140,    70,  LCD_COLOR_RED,  LCD_COLOR_WHITE,  (uint8_t *)"          " );
													LCD_WriteNumLong(    140,    70,  LCD_COLOR_RED,  LCD_COLOR_WHITE,  amp_data[pos_5]);
													LCD_WriteString (     0,  posy,  LCD_COLOR_RED,  LCD_COLOR_WHITE,  (uint8_t *)"->" );	
													while(key_numb != 5)
													{			
														key_numb = Key_Scan();		// 按键扫描
														switch(key_numb)
														{
															case 1:
																if(posy == 40)
																{
																	pos_4 ++;
																	if(pos_4 == 13)
																	{
																		pos_4 = 0;
																	}	
																	LCD_WriteString (    140,    40,  LCD_COLOR_RED,  LCD_COLOR_WHITE,  (uint8_t *)"          " );
																	LCD_WriteNumLong(    140,    40,  LCD_COLOR_RED,  LCD_COLOR_WHITE,  amp_data[pos_4]);
																}else if(posy == 70)
																{
																	pos_5 ++;
																	if(pos_5 == 13)
																	{
																		pos_5 = 0;
																	}	
																	LCD_WriteString (    140,    70,  LCD_COLOR_RED,  LCD_COLOR_WHITE,  (uint8_t *)"          " );
																	LCD_WriteNumLong(    140,    70,  LCD_COLOR_RED,  LCD_COLOR_WHITE,  amp_data[pos_5]);
																}
																break;
															case 2:
																if(posy == 40)
																{
																	if(pos_4 == 0)
																	{
																		pos_4 = 18;
																	}
																	pos_4 --;
																	LCD_WriteString (    140,    40,  LCD_COLOR_RED,  LCD_COLOR_WHITE,  (uint8_t *)"          " );
																	LCD_WriteNumLong(    140,    40,  LCD_COLOR_RED,  LCD_COLOR_WHITE,  amp_data[pos_4]);
																}else if(posy == 70)
																{
																	if(pos_5 == 0)
																	{
																		pos_5 = 18;
																	}
																	pos_5 --;
																	LCD_WriteString (    140,    70,  LCD_COLOR_RED,  LCD_COLOR_WHITE,  (uint8_t *)"          " );
																	LCD_WriteNumLong(    140,    70,  LCD_COLOR_RED,  LCD_COLOR_WHITE,  amp_data[pos_5]);
																}
																break;
															case 3:
																LCD_WriteString (  0,  posy,  LCD_COLOR_RED,  LCD_COLOR_WHITE,  (uint8_t *)"  " );	
																posy += 30;
																LCD_WriteString (  0,  posy,  LCD_COLOR_RED,  LCD_COLOR_WHITE,  (uint8_t *)"->" );
																if(posy >= 100)
																{
																	LCD_WriteString (  0,  posy,  LCD_COLOR_RED,  LCD_COLOR_WHITE,  (uint8_t *)"  " );
																	AD9910_Singal_Profile_Init();																		// 单音PROFILE初始化
																	AD9910_Singal_Profile_Set(0, 1000000, amp_data[pos_4], 0);									// 设置正弦波信息 （ 通道（0~7） ， 频率 ，幅度 ， 相位 ）
																	Par_mod(0, amp_data[pos_5]);				// 调制频率	
																}
																break;
															case 4:
																LCD_WriteString (    0,    40,  LCD_COLOR_RED,  LCD_COLOR_WHITE,  (uint8_t *)"             " );
																LCD_WriteString (    0,    70,  LCD_COLOR_RED,  LCD_COLOR_WHITE,  (uint8_t *)"             " );
																LCD_WriteString (    0,   100,  LCD_COLOR_RED,  LCD_COLOR_WHITE,  (uint8_t *)"             " );					
																key_numb = 5;
																posy = 40;
																break;
															default:
																break;
														}
													}
													break;
												case 10:
													posy = 40;	
													LCD_WriteString (    30,    40,  LCD_COLOR_RED,  LCD_COLOR_WHITE,  (uint8_t *)"起始相位:" );
													LCD_WriteString (    30,    70,  LCD_COLOR_RED,  LCD_COLOR_WHITE,  (uint8_t *)"偏移相位:" );
													LCD_WriteString (    140,    40,  LCD_COLOR_RED,  LCD_COLOR_WHITE,  (uint8_t *)"          " );
													LCD_WriteNumLong(    140,    40,  LCD_COLOR_RED,  LCD_COLOR_WHITE,  pha_data[pos_6]);
													LCD_WriteString (    140,    70,  LCD_COLOR_RED,  LCD_COLOR_WHITE,  (uint8_t *)"          " );
													LCD_WriteNumLong(    140,    70,  LCD_COLOR_RED,  LCD_COLOR_WHITE,  pha_data[pos_7]);												
													LCD_WriteString (     0,  posy,  LCD_COLOR_RED,  LCD_COLOR_WHITE,  (uint8_t *)"->" );	
													while(key_numb != 5)
													{			
														key_numb = Key_Scan();		// 按键扫描
														switch(key_numb)
														{
															case 1:
																if(posy == 40)
																{
																	pos_6 ++;
																	if(pos_6 == 6)
																	{
																		pos_6 = 0;
																	}	
																	LCD_WriteString (    140,    40,  LCD_COLOR_RED,  LCD_COLOR_WHITE,  (uint8_t *)"          " );
																	LCD_WriteNumLong(    140,    40,  LCD_COLOR_RED,  LCD_COLOR_WHITE,  pha_data[pos_6]);
																}else if(posy == 70)
																{
																	pos_7 ++;
																	if(pos_7 == 6)
																	{
																		pos_7 = 0;
																	}	
																	LCD_WriteString (    140,    70,  LCD_COLOR_RED,  LCD_COLOR_WHITE,  (uint8_t *)"          " );
																	LCD_WriteNumLong(    140,    70,  LCD_COLOR_RED,  LCD_COLOR_WHITE,  pha_data[pos_7]);
																}
																break;
															case 2:
																if(posy == 40)
																{
																	if(pos_6 == 0)
																	{
																		pos_6 = 6;
																	}
																	pos_6 --;
																	LCD_WriteString (    140,    40,  LCD_COLOR_RED,  LCD_COLOR_WHITE,  (uint8_t *)"          " );
																	LCD_WriteNumLong(    140,    40,  LCD_COLOR_RED,  LCD_COLOR_WHITE,  pha_data[pos_6]);
																}else if(posy == 70)
																{
																	if(pos_7 == 0)
																	{
																		pos_7 = 6;
																	}
																	pos_7 --;
																	LCD_WriteString (    140,    70,  LCD_COLOR_RED,  LCD_COLOR_WHITE,  (uint8_t *)"          " );
																	LCD_WriteNumLong(    140,    70,  LCD_COLOR_RED,  LCD_COLOR_WHITE,  pha_data[pos_7]);
																}
																break;
															case 3:
																LCD_WriteString (  0,  posy,  LCD_COLOR_RED,  LCD_COLOR_WHITE,  (uint8_t *)"  " );	
																posy += 30;
																LCD_WriteString (  0,  posy,  LCD_COLOR_RED,  LCD_COLOR_WHITE,  (uint8_t *)"->" );
																if(posy >= 100)
																{
																	LCD_WriteString (  0,  posy,  LCD_COLOR_RED,  LCD_COLOR_WHITE,  (uint8_t *)"  " );
																	AD9910_Singal_Profile_Init();																		// 单音PROFILE初始化
																	AD9910_Singal_Profile_Set(0, 1000000, 10230, pha_data[pos_6]);									// 设置正弦波信息 （ 通道（0~7） ， 频率 ，幅度 ， 相位 ）
																	Par_mod(1, amp_data[pos_7]);				// 调制频率	
																}
																break;
															case 4:
																LCD_WriteString (    0,    40,  LCD_COLOR_RED,  LCD_COLOR_WHITE,  (uint8_t *)"             " );
																LCD_WriteString (    0,    70,  LCD_COLOR_RED,  LCD_COLOR_WHITE,  (uint8_t *)"             " );
																LCD_WriteString (    0,   100,  LCD_COLOR_RED,  LCD_COLOR_WHITE,  (uint8_t *)"             " );
															  key_numb = 5;
																posy = 40;
																break;
															default:
																break;
														}
													}
													break;
												default:
													break;
											}
	
											break;
										case 4:
											break;
										default:
											break;
									}
								}
							break;
						default:
							break;
						}
					}
				break;
			case 4:
		
				break;
			default:
				
				break;
			}
		
	}
}
	

//-----------------------------------------------------------------
// End Of File
//-----------------------------------------------------------------
