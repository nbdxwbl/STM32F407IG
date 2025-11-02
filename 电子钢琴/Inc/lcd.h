/*
 * @Author: your name
 * @Date: 2021-07-08 10:04:19
 * @LastEditTime: 2021-07-15 14:58:44
 * @LastEditors: Please set LastEditors
 * @Description: LCDÏÔÊ¾ÆÁ³õÊ¼»¯
 * @FilePath: \MDK-ARMd:\Project\stm32-Project\STM32F407IGT6_FreeRTOS_LCD_MoreTest\Drivers\BSP\BSP_LCD.h
 */
#ifndef __BSP_LCD_H
#define __BSP_LCD_H

#include "main.h"

//LCDÖØÒª²ÎÊı¼¯
typedef struct
{
    uint16_t width;   //LCD ¿í¶È
    uint16_t height;  //LCD ¸ß¶È
    uint16_t id;      //LCD ID
    uint8_t dir;      //ºáÆÁ»¹ÊÇÊúÆÁ¿ØÖÆ£º0£¬ÊúÆÁ£»1£¬ºáÆÁ¡£
    uint16_t wramcmd; //¿ªÊ¼Ğ´gramÖ¸Áî
    uint16_t setxcmd; //ÉèÖÃx×ø±êÖ¸Áî
    uint16_t setycmd; //ÉèÖÃy×ø±êÖ¸Áî
} _lcd_dev;

//LCD²ÎÊı
extern _lcd_dev lcddev; //¹ÜÀíLCDÖØÒª²ÎÊı

//////////////////////////////////////////////////////////////////////////////////
//LCDµØÖ·½á¹¹Ìå
typedef struct
{
    uint16_t LCD_REG;
    uint16_t LCD_RAM;
} LCD_TypeDef;
//Ê¹ÓÃNOR/SRAMµÄ Bank1.sector4,µØÖ·Î»HADDR[27,26]=11 A6×÷ÎªÊı¾İÃüÁîÇø·ÖÏß
//×¢ÒâÉèÖÃÊ±STM32ÄÚ²¿»áÓÒÒÆÒ»Î»¶ÔÆä! 111 1110=0X7E
#define LCD_BASE ((uint32_t)(0x6C000000))
#define LCD ((LCD_TypeDef *)LCD_BASE)
//////////////////////////////////////////////////////////////////////////////////

//É¨Ãè·    l;½Ïò¶¨Òå
#define L2R_U2D 0 //´Ó×óµ½ÓÒ,´ÓÉÏµ½ÏÂ
#define L2R_D2U 1 //´Ó×óµ½ÓÒ,´ÓÏÂµ½ÉÏ
#define R2L_U2D 2 //´ÓÓÒµ½×ó,´ÓÉÏµ½ÏÂ
#define R2L_D2U 3 //´ÓÓÒµ½×ó,´ÓÏÂµ½ÉÏ

#define U2D_L2R 4 //´ÓÉÏµ½ÏÂ,´Ó×óµ½ÓÒ
#define U2D_R2L 5 //´ÓÉÏµ½ÏÂ,´ÓÓÒµ½×ó
#define D2U_L2R 6 //´ÓÏÂµ½ÉÏ,´Ó×óµ½ÓÒ
#define D2U_R2L 7 //´ÓÏÂµ½ÉÏ,´ÓÓÒµ½×ó

#define DFT_SCAN_DIR D2U_L2R //Ä¬ÈÏµÄÉ¨Ãè·½Ïò

//»­±ÊÑÕÉ«
#define WHITE 0xFFFF
#define BLACK 0x0000
#define BLUE 0x001F
#define BRED 0XF81F
#define GRED 0XFFE0
#define GBLUE 0X07FF
#define RED 0xF800
#define MAGENTA 0xF81F
#define GREEN 0x07E0
#define CYAN 0x7FFF
#define YELLOW 0xFFE0
#define BROWN 0XBC40 //×ØÉ«
#define BRRED 0XFC07 //×ØºìÉ«
#define GRAY 0X8430  //»ÒÉ«
//GUIÑÕÉ«

#define DARKBLUE 0X01CF  //ÉîÀ¶É«
#define LIGHTBLUE 0X7D7C //Ç³À¶É«
#define GRAYBLUE 0X5458  //»ÒÀ¶É«
//ÒÔÉÏÈıÉ«ÎªPANELµÄÑÕÉ«

#define LIGHTGREEN 0X841F //Ç³ÂÌÉ«
//#define LIGHTGRAY        0XEF5B //Ç³»ÒÉ«(PANNEL)
#define LGRAY 0XC618 //Ç³»ÒÉ«(PANNEL),´°Ìå±³¾°É«

#define LGRAYBLUE 0XA651 //Ç³»ÒÀ¶É«(ÖĞ¼ä²ãÑÕÉ«)
#define LBBLUE 0X2B12    //Ç³×ØÀ¶É«(Ñ¡ÔñÌõÄ¿µÄ·´É«)

void LCD_Init(void); //³õÊ¼»¯
void LCD_Read_ID(void);
uint16_t LCD_RD_DATA(void);
void LCD_DisplayOn(void);                                                                               //¿ªÏÔÊ¾
void LCD_DisplayOff(void);                                                                              //¹ØÏÔÊ¾
void LCD_Clear(uint16_t Color);                                                                         //ÇåÆÁ
void LCD_SetCursor(uint16_t Xpos, uint16_t Ypos);                                                       //ÉèÖÃ¹â±ê
void LCD_DrawPoint(uint16_t x, uint16_t y);                                                             //»­µã
void LCD_Fast_DrawPoint(uint16_t x, uint16_t y, uint16_t color);                                        //¿ìËÙ»­µã
uint16_t LCD_ReadPoint(uint16_t x, uint16_t y);                                                         //¶Áµã
void LCD_Draw_Circle(uint16_t x0, uint16_t y0, uint8_t r);                                              //»­Ô²
void LCD_Draw_Circle_Fill(uint16_t x0, uint16_t y0, uint8_t r);                                     //»­Ô²²¢Ìî³ä
void LCD_DrawLine(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2);                                  //»­Ïß
void LCD_DrawRectangle(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2);                             //»­¾ØĞÎ
void LCD_Fill(uint16_t sx, uint16_t sy, uint16_t ex, uint16_t ey, uint16_t color);                      //Ìî³äµ¥É«
void LCD_Color_Fill(uint16_t sx, uint16_t sy, uint16_t ex, uint16_t ey, uint16_t *color);               //Ìî³äÖ¸¶¨ÑÕÉ«
void LCD_ShowChar(uint16_t x, uint16_t y, uint8_t num, uint8_t size, uint8_t mode);                     //ÏÔÊ¾Ò»¸ö×Ö·û
void LCD_ShowNum(uint16_t x, uint16_t y, uint32_t num, uint8_t len, uint8_t size);                      //ÏÔÊ¾Ò»¸öÊı×Ö
void LCD_ShowxNum(uint16_t x, uint16_t y, uint32_t num, uint8_t len, uint8_t size, uint8_t mode);       //ÏÔÊ¾ Êı×Ö
void LCD_ShowString(uint16_t x, uint16_t y, uint16_t width, uint16_t height, uint8_t size, uint8_t *p); //ÏÔÊ¾Ò»¸ö×Ö·û´®,12/16×ÖÌå

void LCD_WriteReg(uint16_t LCD_Reg, uint16_t LCD_RegValue);
uint16_t LCD_ReadReg(uint16_t LCD_Reg);
void LCD_WriteRAM_Prepare(void);
void LCD_WriteRAM(uint16_t RGB_Code);
void LCD_SSD_BackLightSet(uint8_t pwm);                                         //SSD1963 ±³¹â¿ØÖÆ
void LCD_Scan_Dir(uint8_t dir);                                                 //ÉèÖÃÆÁÉ¨Ãè·½Ïò
void LCD_Display_Dir(uint8_t dir);                                              //ÉèÖÃÆÁÄ»ÏÔÊ¾·½Ïò
void LCD_Set_Window(uint16_t sx, uint16_t sy, uint16_t width, uint16_t height); //ÉèÖÃ´°¿Ú

void LCD_DrawPoint_pic(uint16_t x, uint16_t y, uint16_t color);
void LCD_DrawPicture(uint16_t StartX, uint16_t StartY, uint16_t Xend, uint16_t Yend, uint8_t *pic);

void LCD_Write_Cmd(uint8_t cmd);
void LCD_Write_Data(uint8_t data);

//LCD·Ö±æÂÊÉèÖÃ
#define SSD_HOR_RESOLUTION 800 //LCDË®Æ½·Ö±æÂÊ
#define SSD_VER_RESOLUTION 480 //LCD´¹Ö±·Ö±æÂÊ
//LCDÇı¶¯²ÎÊıÉèÖÃ
#define SSD_HOR_PULSE_WIDTH 1   //Ë®Æ½Âö¿í
#define SSD_HOR_BACK_PORCH 46   //Ë®Æ½Ç°ÀÈ
#define SSD_HOR_FRONT_PORCH 210 //Ë®Æ½ºóÀÈ

#define SSD_VER_PULSE_WIDTH 1  //´¹Ö±Âö¿í
#define SSD_VER_BACK_PORCH 23  //´¹Ö±Ç°ÀÈ
#define SSD_VER_FRONT_PORCH 22 //´¹Ö±Ç°ÀÈ
//ÈçÏÂ¼¸¸ö²ÎÊı£¬×Ô¶¯¼ÆËã
#define SSD_HT (SSD_HOR_RESOLUTION + SSD_HOR_BACK_PORCH + SSD_HOR_FRONT_PORCH)
#define SSD_HPS (SSD_HOR_BACK_PORCH)
#define SSD_VT (SSD_VER_RESOLUTION + SSD_VER_BACK_PORCH + SSD_VER_FRONT_PORCH)
#define SSD_VPS (SSD_VER_BACK_PORCH)

#endif
