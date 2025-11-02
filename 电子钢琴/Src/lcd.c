#include "lcd.h"
#include "stdlib.h"
#include "font.h" 
#include "usart.h"	 
#include "delay_us.h"	 
#include "stdio.h"

// LCD�Ļ�����ɫ�ͱ���ɫ
uint16_t POINT_COLOR = BLACK; // ������ɫ
uint16_t BACK_COLOR = WHITE;	// ����ɫ

// ����LCD��Ҫ����
// Ĭ��Ϊ����
_lcd_dev lcddev;

// д�Ĵ�������
// regval:�Ĵ���ֵ
void LCD_WR_REG(uint16_t regval)
{
	volatile int i = 0;
	regval = regval;			 // ʹ��-O2�Ż���ʱ��,����������ʱ
	LCD->LCD_REG = regval; // д��Ҫд�ļĴ������
										 // for (i = 0; i < 30; i++)
										 // 	;
}
// дLCD����
// data:Ҫд���ֵ
void LCD_WR_DATA(uint16_t data)
{
	volatile int i = 0;
	data = data; // ʹ��-O2�Ż���ʱ��,����������ʱ
	LCD->LCD_RAM = data;
	// for (i = 0; i < 30; i++)
	// 	;
}

// ��LCD����
// ����ֵ:������ֵ
uint16_t LCD_RD_DATA(void)
{
	uint16_t ram; // ��ֹ���Ż�
	ram = LCD->LCD_RAM;
	return ram;
}
// д�Ĵ���
// LCD_Reg:�Ĵ�����ַ
// LCD_RegValue:Ҫд�������
void LCD_WriteReg(uint16_t LCD_Reg, uint16_t LCD_RegValue)
{
	LCD->LCD_REG = LCD_Reg;			 // д��Ҫд�ļĴ������
	LCD->LCD_RAM = LCD_RegValue; // д������
}
// ���Ĵ���
// LCD_Reg:�Ĵ�����ַ
// ����ֵ:����������
uint16_t LCD_ReadReg(uint16_t LCD_Reg)
{
	LCD_WR_REG(LCD_Reg); // д��Ҫ���ļĴ������
	delay_us(20);
	return LCD_RD_DATA(); // ���ض�����ֵ
}
// ��ʼдGRAM
void LCD_WriteRAM_Prepare(void)
{
	LCD->LCD_REG = lcddev.wramcmd;
}
// LCDдGRAM
// RGB_Code:��ɫֵ
void LCD_WriteRAM(uint16_t RGB_Code)
{
	LCD->LCD_RAM = RGB_Code; // дʮ��λGRAM
}
// ��ILI93xx����������ΪGBR��ʽ��������д���ʱ��ΪRGB��ʽ��
// ͨ���ú���ת��
// c:GBR��ʽ����ɫֵ
// ����ֵ��RGB��ʽ����ɫֵ
uint16_t LCD_BGR2RGB(uint16_t c)
{
	uint16_t r, g, b, rgb;
	b = (c >> 0) & 0x1f;
	g = (c >> 5) & 0x3f;
	r = (c >> 11) & 0x1f;
	rgb = (b << 11) + (g << 5) + (r << 0);
	return (rgb);
}
// ��mdk -O1ʱ���Ż�ʱ��Ҫ����
// ��ʱi
void opt_delay(uint8_t i)
{
	while (i--)
		;
}
// ��ȡ��ĳ�����ɫֵ
// x,y:����
// ����ֵ:�˵����ɫ
uint16_t LCD_ReadPoint(uint16_t x, uint16_t y)
{
	uint16_t r = 0, g = 0, b = 0;
	if (x >= lcddev.width || y >= lcddev.height)
		return 0; // �����˷�Χ,ֱ�ӷ���
	LCD_SetCursor(x, y);
	if (lcddev.id == 0X5510)
		LCD_WR_REG(0X2E00); // 5510 ���Ͷ�GRAMָ��
	r = LCD_RD_DATA();		// dummy Read
	opt_delay(2);
	r = LCD_RD_DATA();			 // ʵ��������ɫ
	if (lcddev.id == 0X5510) // NT35510Ҫ��2�ζ���
	{
		opt_delay(2);
		b = LCD_RD_DATA();
		g = r & 0XFF; // ����5510,��һ�ζ�ȡ����RG��ֵ,R��ǰ,G�ں�,��ռ8λ
		g <<= 8;
	}
	if (lcddev.id == 0X5510)
		return (((r >> 11) << 11) | ((g >> 10) << 5) | (b >> 11)); // NT35510��Ҫ��ʽת��һ��

	return 0;
}
// LCD������ʾ
void LCD_DisplayOn(void)
{
	if (lcddev.id == 0X5510 || lcddev.id == 0X8009)
		LCD_WR_REG(0X2900); // ������ʾ
}
// LCD�ر���ʾ
void LCD_DisplayOff(void)
{
	if (lcddev.id == 0X5510 || lcddev.id == 0X8009)
		LCD_WR_REG(0X2800); // �ر���ʾ
}
// ���ù��λ��
// Xpos:������
// Ypos:������
void LCD_SetCursor(uint16_t Xpos, uint16_t Ypos)
{
	if (lcddev.id == 0x8009)
	{
		/**
			�ڲ鿴8009A��оƬ�ֲ�֮�󣬵ó�
			0x2A00: x����ʼλ�õĸ�8λ
			0x2A01: x����ʼλ�õĵ�8λ
			0x2A02: x������λ�õĸ�8λ
			0x2A03: x������λ�õĵ�8λ
			*/
		LCD_WR_REG(0x2A00);
		LCD_WR_DATA(Xpos >> 8);
		LCD_WR_REG(0x2A01);
		LCD_WR_DATA(Xpos & 0xff);
		LCD_WR_REG(0x2A02);
		LCD_WR_DATA((lcddev.width - 1) >> 8);
		LCD_WR_REG(0x2A03);
		LCD_WR_DATA((lcddev.width - 1) & 0xFF);

		LCD_WR_REG(0x2C00);
		LCD_WR_REG(0x2C00);

		/**
			�ڲ鿴8009A��оƬ�ֲ�֮�󣬵ó�
			0x2B00: y����ʼλ�õĸ�8λ
			0x2B01: y����ʼλ�õĵ�8λ
			0x2B02: y������λ�õĸ�8λ
			0x2B03: y������λ�õĵ�8λ
			*/
		LCD_WR_REG(0x2B00);
		LCD_WR_DATA(Ypos >> 8);
		LCD_WR_REG(0x2B01);
		LCD_WR_DATA(Ypos & 0xff);
		LCD_WR_REG(0x2B02);
		LCD_WR_DATA(lcddev.height >> 8);
		LCD_WR_REG(0x2B03);
		LCD_WR_DATA(lcddev.height & 0xFF);

		LCD_WR_REG(0x2C00);
		LCD_WR_REG(0x2C00);
	}
	if (lcddev.id == 0x5510)
	{
		LCD_WR_REG(lcddev.setxcmd);
		LCD_WR_DATA(Xpos >> 8);
		LCD_WR_REG(lcddev.setxcmd + 1);
		LCD_WR_DATA(Xpos & 0XFF);
		LCD_WR_REG(lcddev.setycmd);
		LCD_WR_DATA(Ypos >> 8);
		LCD_WR_REG(lcddev.setycmd + 1);
		LCD_WR_DATA(Ypos & 0XFF);
	}
}
// ����LCD���Զ�ɨ�跽��
// ע��:�����������ܻ��ܵ��˺������õ�Ӱ��(������9341/6804����������),
// ����,һ������ΪL2R_U2D����,�������Ϊ����ɨ�跽ʽ,���ܵ�����ʾ������.
// dir:0~7,����8������(���嶨���lcd.h)
// 9320/9325/9328/4531/4535/1505/b505/5408/9341/5310/5510/1963��IC�Ѿ�ʵ�ʲ���
void LCD_Scan_Dir(uint8_t dir)
{
	uint16_t regval = 0;
	uint16_t dirreg = 0;
	uint16_t temp;

	if (lcddev.id == 0X5510 || lcddev.id == 0X8009)
	{
		switch (dir)
		{
		case L2R_U2D:
			regval |= (0 << 7) | (0 << 6) | (0 << 5);
			break;
		case L2R_D2U:
			regval |= (1 << 7) | (0 << 6) | (0 << 5);
			break;
		case R2L_U2D:
			regval |= (0 << 7) | (1 << 6) | (0 << 5);
			break;
		case R2L_D2U:
			regval |= (1 << 7) | (1 << 6) | (0 << 5);
			break;
		case U2D_L2R:
			regval |= (0 << 7) | (0 << 6) | (1 << 5);
			break;
		case U2D_R2L:
			regval |= (0 << 7) | (1 << 6) | (1 << 5);
			break;
		case D2U_L2R:
			regval |= (1 << 7) | (0 << 6) | (1 << 5);
			break;
		case D2U_R2L:
			regval |= (1 << 7) | (1 << 6) | (1 << 5);
			break;
		default:
			break;
		}
		if (lcddev.id == 0X5510 || lcddev.id == 0X8009)
			dirreg = 0X3600;

		LCD_WriteReg(dirreg, regval);

		if (regval & 0X20)
		{
			if (lcddev.width < lcddev.height) // ����X,Y
			{
				temp = lcddev.width;
				lcddev.width = lcddev.height;
				lcddev.height = temp;
			}
		}
		else
		{
			if (lcddev.width > lcddev.height) // ����X,Y
			{
				temp = lcddev.width;
				lcddev.width = lcddev.height;
				lcddev.height = temp;
			}
		}

		if (lcddev.id == 0X5510 || lcddev.id == 0X8009)
		{
			LCD_WR_REG(lcddev.setxcmd);
			LCD_WR_DATA(0);
			LCD_WR_REG(lcddev.setxcmd + 1);
			LCD_WR_DATA(0);
			LCD_WR_REG(lcddev.setxcmd + 2);
			LCD_WR_DATA((lcddev.width - 1) >> 8);
			LCD_WR_REG(lcddev.setxcmd + 3);
			LCD_WR_DATA((lcddev.width - 1) & 0XFF);
			LCD_WR_REG(lcddev.setycmd);
			LCD_WR_DATA(0);
			LCD_WR_REG(lcddev.setycmd + 1);
			LCD_WR_DATA(0);
			LCD_WR_REG(lcddev.setycmd + 2);
			LCD_WR_DATA((lcddev.height - 1) >> 8);
			LCD_WR_REG(lcddev.setycmd + 3);
			LCD_WR_DATA((lcddev.height - 1) & 0XFF);
		}
		else
		{
			LCD_WR_REG(lcddev.setxcmd);
			LCD_WR_DATA(0);
			LCD_WR_DATA(0);
			LCD_WR_DATA((lcddev.width - 1) >> 8);
			LCD_WR_DATA((lcddev.width - 1) & 0XFF);
			LCD_WR_REG(lcddev.setycmd);
			LCD_WR_DATA(0);
			LCD_WR_DATA(0);
			LCD_WR_DATA((lcddev.height - 1) >> 8);
			LCD_WR_DATA((lcddev.height - 1) & 0XFF);
		}
	}
}
// ����
// x,y:����
// POINT_COLOR:�˵����ɫ
void LCD_DrawPoint(uint16_t x, uint16_t y)
{
	LCD_SetCursor(x, y);		// ���ù��λ��
	LCD_WriteRAM_Prepare(); // ��ʼд��GRAM
	LCD->LCD_RAM = POINT_COLOR;
}
// ���ٻ���
// x,y:����
// color:��ɫ
void LCD_Fast_DrawPoint(uint16_t x, uint16_t y, uint16_t color)
{
	if (lcddev.id == 0X9341 || lcddev.id == 0X5310)
	{
		LCD_WR_REG(lcddev.setxcmd);
		LCD_WR_DATA(x >> 8);
		LCD_WR_DATA(x & 0XFF);
		LCD_WR_REG(lcddev.setycmd);
		LCD_WR_DATA(y >> 8);
		LCD_WR_DATA(y & 0XFF);
	}
	else if (lcddev.id == 0X5510)
	{
		LCD_WR_REG(lcddev.setxcmd);
		LCD_WR_DATA(x >> 8);
		LCD_WR_REG(lcddev.setxcmd + 1);
		LCD_WR_DATA(x & 0XFF);
		LCD_WR_REG(lcddev.setycmd);
		LCD_WR_DATA(y >> 8);
		LCD_WR_REG(lcddev.setycmd + 1);
		LCD_WR_DATA(y & 0XFF);
	}
	else if (lcddev.id == 0X1963)
	{
		if (lcddev.dir == 0)
			x = lcddev.width - 1 - x;
		LCD_WR_REG(lcddev.setxcmd);
		LCD_WR_DATA(x >> 8);
		LCD_WR_DATA(x & 0XFF);
		LCD_WR_DATA(x >> 8);
		LCD_WR_DATA(x & 0XFF);
		LCD_WR_REG(lcddev.setycmd);
		LCD_WR_DATA(y >> 8);
		LCD_WR_DATA(y & 0XFF);
		LCD_WR_DATA(y >> 8);
		LCD_WR_DATA(y & 0XFF);
	}
	else if (lcddev.id == 0X6804)
	{
		if (lcddev.dir == 1)
			x = lcddev.width - 1 - x; // ����ʱ����
		LCD_WR_REG(lcddev.setxcmd);
		LCD_WR_DATA(x >> 8);
		LCD_WR_DATA(x & 0XFF);
		LCD_WR_REG(lcddev.setycmd);
		LCD_WR_DATA(y >> 8);
		LCD_WR_DATA(y & 0XFF);
	}
	else if (lcddev.id == 0x8009)
	{
		LCD_SetCursor(x, y); // ��������
	}
	else
	{
		if (lcddev.dir == 1)
			x = lcddev.width - 1 - x; // ������ʵ���ǵ�תx,y����
		LCD_WriteReg(lcddev.setxcmd, x);
		LCD_WriteReg(lcddev.setycmd, y);
	}

	LCD_WriteRAM_Prepare(); // ��ʼд��GRAM
	LCD->LCD_RAM = color;
}
// SSD1963 ��������
// pwm:����ȼ�,0~100.Խ��Խ��.
void LCD_SSD_BackLightSet(uint8_t pwm)
{
	LCD_WR_REG(0xBE);				 // ����PWM���
	LCD_WR_DATA(0x05);			 // 1����PWMƵ��
	LCD_WR_DATA(pwm * 2.55); // 2����PWMռ�ձ�
	LCD_WR_DATA(0x01);			 // 3����C
	LCD_WR_DATA(0xFF);			 // 4����D
	LCD_WR_DATA(0x00);			 // 5����E
	LCD_WR_DATA(0x00);			 // 6����F
}

// ����LCD��ʾ����
// dir:0,������1,����
void LCD_Display_Dir(uint8_t dir)
{
	if (dir == 0) // ����
	{
		lcddev.dir = 0x00;
		lcddev.width = 480;
		lcddev.height = 800;
		lcddev.setxcmd = 0X2A00;
		lcddev.setycmd = 0X2B00;
		lcddev.wramcmd = 0X2C00;
	}
	else // ����
	{
		lcddev.dir = 0x01;
		lcddev.width = 800;			 // LCD ����
		lcddev.height = 480;		 // LCD �߶�
		lcddev.setxcmd = 0X2A00; // ����x����ָ��2A
		lcddev.setycmd = 0X2B00; // ����y����ָ��2B
		lcddev.wramcmd = 0X2C00; // ��ʼдgramָ��
	}
	LCD_Scan_Dir(DFT_SCAN_DIR); // Ĭ��ɨ�跽��
}
// ���ô���,���Զ����û������굽�������Ͻ�(sx,sy).
// sx,sy:������ʼ����(���Ͻ�)
// width,height:���ڿ��Ⱥ͸߶�,�������0!!
// �����С:width*height.
void LCD_Set_Window(uint16_t sx, uint16_t sy, uint16_t width, uint16_t height)
{
	uint16_t twidth, theight;
	twidth = sx + width - 1;
	theight = sy + height - 1;

	LCD_WR_REG(lcddev.setxcmd);
	LCD_WR_DATA(sx >> 8);
	LCD_WR_REG(lcddev.setxcmd + 1);
	LCD_WR_DATA(sx & 0XFF);
	LCD_WR_REG(lcddev.setxcmd + 2);
	LCD_WR_DATA(twidth >> 8);
	LCD_WR_REG(lcddev.setxcmd + 3);
	LCD_WR_DATA(twidth & 0XFF);
	LCD_WR_REG(lcddev.setycmd);
	LCD_WR_DATA(sy >> 8);
	LCD_WR_REG(lcddev.setycmd + 1);
	LCD_WR_DATA(sy & 0XFF);
	LCD_WR_REG(lcddev.setycmd + 2);
	LCD_WR_DATA(theight >> 8);
	LCD_WR_REG(lcddev.setycmd + 3);
	LCD_WR_DATA(theight & 0XFF);
}

// ��ʼ��lcd
// �ó�ʼ���������Գ�ʼ������ILI93XXҺ��,�������������ǻ���ILI9320��!!!
// �������ͺŵ�����оƬ��û�в���!
void LCD_Init(void)
{
	uint16_t id_temp = 0;
	uint16_t a;
	HAL_Delay(50); // delay 50 ms
	LCD_WriteReg(0x0000, 0x0001);
	HAL_Delay(100);									 // delay 50 ms
	printf(" lcddev.id:%x\r\n", lcddev.id);				 // ��ӡLCD ID
	lcddev.id = 0x8009;						 // ����LCD�ͺ�
	//lcddev.id = 0x5510;						 // ����LCD�ͺ�
	printf(" DAT ID:%x\r\n", lcddev.id);				 // ��ӡLCD ID
	if (lcddev.id < 0XFF || lcddev.id == 0XFFFF) // ����ID����ȷ,����lcddev.id==0X9300�жϣ���Ϊ9341��δ����λ������»ᱻ����9300
	{
		

		LCD_WR_REG(0XDA00);
		lcddev.id = LCD_RD_DATA(); // ����0X00
		LCD_WR_REG(0XDB00);
		lcddev.id = LCD_RD_DATA(); // ����0X80
		lcddev.id <<= 8;
		LCD_WR_REG(0XDC00);
		lcddev.id |= LCD_RD_DATA(); // ����0X00
		if (lcddev.id == 0x8000)		// NT35510
			lcddev.id = 0x5510;				// NT3 5510���ص�ID��8000H,Ϊ��������,����ǿ������Ϊ5510
		else
		{
			// ����OTM8009 ID�Ķ�ȡ		  OK20220912
			LCD_WR_REG(0xFF00);
			LCD_WR_DATA(0X0080);
			LCD_WR_REG(0xFF01);
			LCD_WR_DATA(0X0009);
			LCD_WR_REG(0xFF02);
			LCD_WR_DATA(0X0001);

			LCD_WR_REG(0xFF80);
			LCD_WR_DATA(0X0080);
			LCD_WR_REG(0xFF81);
			LCD_WR_DATA(0X0009);

			id_temp = LCD_ReadReg(0xD202);
			printf("==== 0xD202: %x\r\n", id_temp);
			lcddev.id = id_temp << 8;
			id_temp = LCD_ReadReg(0xD203);
			printf("==== 0xD203: %x\r\n", id_temp);
			lcddev.id |= id_temp;
			printf("==== lcddev.id: 0x%x\r\n", lcddev.id);
			if (lcddev.id == 0x80)
				lcddev.id = 0x8009; // OTM8009A���ص�ID��80H,Ϊ��������,����ǿ������Ϊ8009
		}
	}

	{
		// ��������дʱ����ƼĴ�����ʱ��
		FSMC_Bank1E->BWTR[6] &= ~(0XF << 0); // ��ַ����ʱ��(ADDSET)����
		FSMC_Bank1E->BWTR[6] &= ~(0XF << 8); // ���ݱ���ʱ������
		FSMC_Bank1E->BWTR[6] |= 3 << 0;			 // ��ַ����ʱ��(ADDSET)Ϊ3��HCLK =18ns
		FSMC_Bank1E->BWTR[6] |= 2 << 8;			 // ���ݱ���ʱ��(DATAST)Ϊ6ns*3��HCLK=18ns
	}

	printf(" LCD ID:%x\r\n", lcddev.id); // ��ӡLCD ID
	if (lcddev.id == 0x5510)
	{
		LCD_WriteReg(0xF000, 0x55);
		LCD_WriteReg(0xF001, 0xAA);
		LCD_WriteReg(0xF002, 0x52);
		a= LCD_ReadReg(0xF002);	
		printf("0xF002=%04x\n",a);
		LCD_WriteReg(0xF003, 0x08);
		LCD_WriteReg(0xF004, 0x01);
		// AVDD Set AVDD 5.2V
		LCD_WriteReg(0xB000, 0x0D);
		LCD_WriteReg(0xB001, 0x0D);
		LCD_WriteReg(0xB002, 0x0D);
		// AVDD ratio
		LCD_WriteReg(0xB600, 0x34);
		LCD_WriteReg(0xB601, 0x34);
		LCD_WriteReg(0xB602, 0x34);
		// AVEE -5.2V
		LCD_WriteReg(0xB100, 0x0D);
		LCD_WriteReg(0xB101, 0x0D);
		LCD_WriteReg(0xB102, 0x0D);
		// AVEE ratio
		LCD_WriteReg(0xB700, 0x34);
		LCD_WriteReg(0xB701, 0x34);
		LCD_WriteReg(0xB702, 0x34);
		// VCL -2.5V
		LCD_WriteReg(0xB200, 0x00);
		LCD_WriteReg(0xB201, 0x00);
		LCD_WriteReg(0xB202, 0x00);
		// VCL ratio
		LCD_WriteReg(0xB800, 0x24);
		LCD_WriteReg(0xB801, 0x24);
		LCD_WriteReg(0xB802, 0x24);
		// VGH 15V (Free pump)
		LCD_WriteReg(0xBF00, 0x01);
		LCD_WriteReg(0xB300, 0x0F);
		LCD_WriteReg(0xB301, 0x0F);
		LCD_WriteReg(0xB302, 0x0F);
		// VGH ratio
		LCD_WriteReg(0xB900, 0x34);
		LCD_WriteReg(0xB901, 0x34);
		LCD_WriteReg(0xB902, 0x34);
		// VGL_REG -10V
		LCD_WriteReg(0xB500, 0x08);
		LCD_WriteReg(0xB501, 0x08);
		LCD_WriteReg(0xB502, 0x08);
		LCD_WriteReg(0xC200, 0x03);
		// VGLX ratio
		LCD_WriteReg(0xBA00, 0x24);
		LCD_WriteReg(0xBA01, 0x24);
		LCD_WriteReg(0xBA02, 0x24);
		// VGMP/VGSP 4.5V/0V
		LCD_WriteReg(0xBC00, 0x00);
		LCD_WriteReg(0xBC01, 0x78);
		LCD_WriteReg(0xBC02, 0x00);
		// VGMN/VGSN -4.5V/0V
		LCD_WriteReg(0xBD00, 0x00);
		LCD_WriteReg(0xBD01, 0x78);
		LCD_WriteReg(0xBD02, 0x00);
		// VCOM
		LCD_WriteReg(0xBE00, 0x00);
		LCD_WriteReg(0xBE01, 0x64);
		// Gamma Setting
		LCD_WriteReg(0xD100, 0x00);
		LCD_WriteReg(0xD101, 0x33);
		LCD_WriteReg(0xD102, 0x00);
		LCD_WriteReg(0xD103, 0x34);
		LCD_WriteReg(0xD104, 0x00);
		LCD_WriteReg(0xD105, 0x3A);
		LCD_WriteReg(0xD106, 0x00);
		LCD_WriteReg(0xD107, 0x4A);
		LCD_WriteReg(0xD108, 0x00);
		LCD_WriteReg(0xD109, 0x5C);
		LCD_WriteReg(0xD10A, 0x00);
		LCD_WriteReg(0xD10B, 0x81);
		LCD_WriteReg(0xD10C, 0x00);
		LCD_WriteReg(0xD10D, 0xA6);
		LCD_WriteReg(0xD10E, 0x00);
		LCD_WriteReg(0xD10F, 0xE5);
		LCD_WriteReg(0xD110, 0x01);
		LCD_WriteReg(0xD111, 0x13);
		LCD_WriteReg(0xD112, 0x01);
		LCD_WriteReg(0xD113, 0x54);
		LCD_WriteReg(0xD114, 0x01);
		LCD_WriteReg(0xD115, 0x82);
		LCD_WriteReg(0xD116, 0x01);
		LCD_WriteReg(0xD117, 0xCA);
		LCD_WriteReg(0xD118, 0x02);
		LCD_WriteReg(0xD119, 0x00);
		LCD_WriteReg(0xD11A, 0x02);
		LCD_WriteReg(0xD11B, 0x01);
		LCD_WriteReg(0xD11C, 0x02);
		LCD_WriteReg(0xD11D, 0x34);
		LCD_WriteReg(0xD11E, 0x02);
		LCD_WriteReg(0xD11F, 0x67);
		LCD_WriteReg(0xD120, 0x02);
		LCD_WriteReg(0xD121, 0x84);
		LCD_WriteReg(0xD122, 0x02);
		LCD_WriteReg(0xD123, 0xA4);
		LCD_WriteReg(0xD124, 0x02);
		LCD_WriteReg(0xD125, 0xB7);
		LCD_WriteReg(0xD126, 0x02);
		LCD_WriteReg(0xD127, 0xCF);
		LCD_WriteReg(0xD128, 0x02);
		LCD_WriteReg(0xD129, 0xDE);
		LCD_WriteReg(0xD12A, 0x02);
		LCD_WriteReg(0xD12B, 0xF2);
		LCD_WriteReg(0xD12C, 0x02);
		LCD_WriteReg(0xD12D, 0xFE);
		LCD_WriteReg(0xD12E, 0x03);
		LCD_WriteReg(0xD12F, 0x10);
		LCD_WriteReg(0xD130, 0x03);
		LCD_WriteReg(0xD131, 0x33);
		LCD_WriteReg(0xD132, 0x03);
		LCD_WriteReg(0xD133, 0x6D);

		LCD_WriteReg(0xD200, 0x00);
		LCD_WriteReg(0xD201, 0x33);
		LCD_WriteReg(0xD202, 0x00);
		LCD_WriteReg(0xD203, 0x34);
		LCD_WriteReg(0xD204, 0x00);
		LCD_WriteReg(0xD205, 0x3A);
		LCD_WriteReg(0xD206, 0x00);
		LCD_WriteReg(0xD207, 0x4A);
		LCD_WriteReg(0xD208, 0x00);
		LCD_WriteReg(0xD209, 0x5C);
		LCD_WriteReg(0xD20A, 0x00);
		LCD_WriteReg(0xD20B, 0x81);
		LCD_WriteReg(0xD20C, 0x00);
		LCD_WriteReg(0xD20D, 0xA6);
		LCD_WriteReg(0xD20E, 0x00);
		LCD_WriteReg(0xD20F, 0xE5);
		LCD_WriteReg(0xD210, 0x01);
		LCD_WriteReg(0xD211, 0x13);
		LCD_WriteReg(0xD212, 0x01);
		LCD_WriteReg(0xD213, 0x54);
		LCD_WriteReg(0xD214, 0x01);
		LCD_WriteReg(0xD215, 0x82);
		LCD_WriteReg(0xD216, 0x01);
		LCD_WriteReg(0xD217, 0xCA);
		LCD_WriteReg(0xD218, 0x02);
		LCD_WriteReg(0xD219, 0x00);
		LCD_WriteReg(0xD21A, 0x02);
		LCD_WriteReg(0xD21B, 0x01);
		LCD_WriteReg(0xD21C, 0x02);
		LCD_WriteReg(0xD21D, 0x34);
		LCD_WriteReg(0xD21E, 0x02);
		LCD_WriteReg(0xD21F, 0x67);
		LCD_WriteReg(0xD220, 0x02);
		LCD_WriteReg(0xD221, 0x84);
		LCD_WriteReg(0xD222, 0x02);
		LCD_WriteReg(0xD223, 0xA4);
		LCD_WriteReg(0xD224, 0x02);
		LCD_WriteReg(0xD225, 0xB7);
		LCD_WriteReg(0xD226, 0x02);
		LCD_WriteReg(0xD227, 0xCF);
		LCD_WriteReg(0xD228, 0x02);
		LCD_WriteReg(0xD229, 0xDE);
		LCD_WriteReg(0xD22A, 0x02);
		LCD_WriteReg(0xD22B, 0xF2);
		LCD_WriteReg(0xD22C, 0x02);
		LCD_WriteReg(0xD22D, 0xFE);
		LCD_WriteReg(0xD22E, 0x03);
		LCD_WriteReg(0xD22F, 0x10);
		LCD_WriteReg(0xD230, 0x03);
		LCD_WriteReg(0xD231, 0x33);
		LCD_WriteReg(0xD232, 0x03);
		LCD_WriteReg(0xD233, 0x6D);

		LCD_WriteReg(0xD300, 0x00);
		LCD_WriteReg(0xD301, 0x33);
		LCD_WriteReg(0xD302, 0x00);
		LCD_WriteReg(0xD303, 0x34);
		LCD_WriteReg(0xD304, 0x00);
		LCD_WriteReg(0xD305, 0x3A);
		LCD_WriteReg(0xD306, 0x00);
		LCD_WriteReg(0xD307, 0x4A);
		LCD_WriteReg(0xD308, 0x00);
		LCD_WriteReg(0xD309, 0x5C);
		LCD_WriteReg(0xD30A, 0x00);
		LCD_WriteReg(0xD30B, 0x81);
		LCD_WriteReg(0xD30C, 0x00);
		LCD_WriteReg(0xD30D, 0xA6);
		LCD_WriteReg(0xD30E, 0x00);
		LCD_WriteReg(0xD30F, 0xE5);
		LCD_WriteReg(0xD310, 0x01);
		LCD_WriteReg(0xD311, 0x13);
		LCD_WriteReg(0xD312, 0x01);
		LCD_WriteReg(0xD313, 0x54);
		LCD_WriteReg(0xD314, 0x01);
		LCD_WriteReg(0xD315, 0x82);
		LCD_WriteReg(0xD316, 0x01);
		LCD_WriteReg(0xD317, 0xCA);
		LCD_WriteReg(0xD318, 0x02);
		LCD_WriteReg(0xD319, 0x00);
		LCD_WriteReg(0xD31A, 0x02);
		LCD_WriteReg(0xD31B, 0x01);
		LCD_WriteReg(0xD31C, 0x02);
		LCD_WriteReg(0xD31D, 0x34);
		LCD_WriteReg(0xD31E, 0x02);
		LCD_WriteReg(0xD31F, 0x67);
		LCD_WriteReg(0xD320, 0x02);
		LCD_WriteReg(0xD321, 0x84);
		LCD_WriteReg(0xD322, 0x02);
		LCD_WriteReg(0xD323, 0xA4);
		LCD_WriteReg(0xD324, 0x02);
		LCD_WriteReg(0xD325, 0xB7);
		LCD_WriteReg(0xD326, 0x02);
		LCD_WriteReg(0xD327, 0xCF);
		LCD_WriteReg(0xD328, 0x02);
		LCD_WriteReg(0xD329, 0xDE);
		LCD_WriteReg(0xD32A, 0x02);
		LCD_WriteReg(0xD32B, 0xF2);
		LCD_WriteReg(0xD32C, 0x02);
		LCD_WriteReg(0xD32D, 0xFE);
		LCD_WriteReg(0xD32E, 0x03);
		LCD_WriteReg(0xD32F, 0x10);
		LCD_WriteReg(0xD330, 0x03);
		LCD_WriteReg(0xD331, 0x33);
		LCD_WriteReg(0xD332, 0x03);
		LCD_WriteReg(0xD333, 0x6D);

		LCD_WriteReg(0xD400, 0x00);
		LCD_WriteReg(0xD401, 0x33);
		LCD_WriteReg(0xD402, 0x00);
		LCD_WriteReg(0xD403, 0x34);
		LCD_WriteReg(0xD404, 0x00);
		LCD_WriteReg(0xD405, 0x3A);
		LCD_WriteReg(0xD406, 0x00);
		LCD_WriteReg(0xD407, 0x4A);
		LCD_WriteReg(0xD408, 0x00);
		LCD_WriteReg(0xD409, 0x5C);
		LCD_WriteReg(0xD40A, 0x00);
		LCD_WriteReg(0xD40B, 0x81);
		LCD_WriteReg(0xD40C, 0x00);
		LCD_WriteReg(0xD40D, 0xA6);
		LCD_WriteReg(0xD40E, 0x00);
		LCD_WriteReg(0xD40F, 0xE5);
		LCD_WriteReg(0xD410, 0x01);
		LCD_WriteReg(0xD411, 0x13);
		LCD_WriteReg(0xD412, 0x01);
		LCD_WriteReg(0xD413, 0x54);
		LCD_WriteReg(0xD414, 0x01);
		LCD_WriteReg(0xD415, 0x82);
		LCD_WriteReg(0xD416, 0x01);
		LCD_WriteReg(0xD417, 0xCA);
		LCD_WriteReg(0xD418, 0x02);
		LCD_WriteReg(0xD419, 0x00);
		LCD_WriteReg(0xD41A, 0x02);
		LCD_WriteReg(0xD41B, 0x01);
		LCD_WriteReg(0xD41C, 0x02);
		LCD_WriteReg(0xD41D, 0x34);
		LCD_WriteReg(0xD41E, 0x02);
		LCD_WriteReg(0xD41F, 0x67);
		LCD_WriteReg(0xD420, 0x02);
		LCD_WriteReg(0xD421, 0x84);
		LCD_WriteReg(0xD422, 0x02);
		LCD_WriteReg(0xD423, 0xA4);
		LCD_WriteReg(0xD424, 0x02);
		LCD_WriteReg(0xD425, 0xB7);
		LCD_WriteReg(0xD426, 0x02);
		LCD_WriteReg(0xD427, 0xCF);
		LCD_WriteReg(0xD428, 0x02);
		LCD_WriteReg(0xD429, 0xDE);
		LCD_WriteReg(0xD42A, 0x02);
		LCD_WriteReg(0xD42B, 0xF2);
		LCD_WriteReg(0xD42C, 0x02);
		LCD_WriteReg(0xD42D, 0xFE);
		LCD_WriteReg(0xD42E, 0x03);
		LCD_WriteReg(0xD42F, 0x10);
		LCD_WriteReg(0xD430, 0x03);
		LCD_WriteReg(0xD431, 0x33);
		LCD_WriteReg(0xD432, 0x03);
		LCD_WriteReg(0xD433, 0x6D);

		LCD_WriteReg(0xD500, 0x00);
		LCD_WriteReg(0xD501, 0x33);
		LCD_WriteReg(0xD502, 0x00);
		LCD_WriteReg(0xD503, 0x34);
		LCD_WriteReg(0xD504, 0x00);
		LCD_WriteReg(0xD505, 0x3A);
		LCD_WriteReg(0xD506, 0x00);
		LCD_WriteReg(0xD507, 0x4A);
		LCD_WriteReg(0xD508, 0x00);
		LCD_WriteReg(0xD509, 0x5C);
		LCD_WriteReg(0xD50A, 0x00);
		LCD_WriteReg(0xD50B, 0x81);
		LCD_WriteReg(0xD50C, 0x00);
		LCD_WriteReg(0xD50D, 0xA6);
		LCD_WriteReg(0xD50E, 0x00);
		LCD_WriteReg(0xD50F, 0xE5);
		LCD_WriteReg(0xD510, 0x01);
		LCD_WriteReg(0xD511, 0x13);
		LCD_WriteReg(0xD512, 0x01);
		LCD_WriteReg(0xD513, 0x54);
		LCD_WriteReg(0xD514, 0x01);
		LCD_WriteReg(0xD515, 0x82);
		LCD_WriteReg(0xD516, 0x01);
		LCD_WriteReg(0xD517, 0xCA);
		LCD_WriteReg(0xD518, 0x02);
		LCD_WriteReg(0xD519, 0x00);
		LCD_WriteReg(0xD51A, 0x02);
		LCD_WriteReg(0xD51B, 0x01);
		LCD_WriteReg(0xD51C, 0x02);
		LCD_WriteReg(0xD51D, 0x34);
		LCD_WriteReg(0xD51E, 0x02);
		LCD_WriteReg(0xD51F, 0x67);
		LCD_WriteReg(0xD520, 0x02);
		LCD_WriteReg(0xD521, 0x84);
		LCD_WriteReg(0xD522, 0x02);
		LCD_WriteReg(0xD523, 0xA4);
		LCD_WriteReg(0xD524, 0x02);
		LCD_WriteReg(0xD525, 0xB7);
		LCD_WriteReg(0xD526, 0x02);
		LCD_WriteReg(0xD527, 0xCF);
		LCD_WriteReg(0xD528, 0x02);
		LCD_WriteReg(0xD529, 0xDE);
		LCD_WriteReg(0xD52A, 0x02);
		LCD_WriteReg(0xD52B, 0xF2);
		LCD_WriteReg(0xD52C, 0x02);
		LCD_WriteReg(0xD52D, 0xFE);
		LCD_WriteReg(0xD52E, 0x03);
		LCD_WriteReg(0xD52F, 0x10);
		LCD_WriteReg(0xD530, 0x03);
		LCD_WriteReg(0xD531, 0x33);
		LCD_WriteReg(0xD532, 0x03);
		LCD_WriteReg(0xD533, 0x6D);

		LCD_WriteReg(0xD600, 0x00);
		LCD_WriteReg(0xD601, 0x33);
		LCD_WriteReg(0xD602, 0x00);
		LCD_WriteReg(0xD603, 0x34);
		LCD_WriteReg(0xD604, 0x00);
		LCD_WriteReg(0xD605, 0x3A);
		LCD_WriteReg(0xD606, 0x00);
		LCD_WriteReg(0xD607, 0x4A);
		LCD_WriteReg(0xD608, 0x00);
		LCD_WriteReg(0xD609, 0x5C);
		LCD_WriteReg(0xD60A, 0x00);
		LCD_WriteReg(0xD60B, 0x81);
		LCD_WriteReg(0xD60C, 0x00);
		LCD_WriteReg(0xD60D, 0xA6);
		LCD_WriteReg(0xD60E, 0x00);
		LCD_WriteReg(0xD60F, 0xE5);
		LCD_WriteReg(0xD610, 0x01);
		LCD_WriteReg(0xD611, 0x13);
		LCD_WriteReg(0xD612, 0x01);
		LCD_WriteReg(0xD613, 0x54);
		LCD_WriteReg(0xD614, 0x01);
		LCD_WriteReg(0xD615, 0x82);
		LCD_WriteReg(0xD616, 0x01);
		LCD_WriteReg(0xD617, 0xCA);
		LCD_WriteReg(0xD618, 0x02);
		LCD_WriteReg(0xD619, 0x00);
		LCD_WriteReg(0xD61A, 0x02);
		LCD_WriteReg(0xD61B, 0x01);
		LCD_WriteReg(0xD61C, 0x02);
		LCD_WriteReg(0xD61D, 0x34);
		LCD_WriteReg(0xD61E, 0x02);
		LCD_WriteReg(0xD61F, 0x67);
		LCD_WriteReg(0xD620, 0x02);
		LCD_WriteReg(0xD621, 0x84);
		LCD_WriteReg(0xD622, 0x02);
		LCD_WriteReg(0xD623, 0xA4);
		LCD_WriteReg(0xD624, 0x02);
		LCD_WriteReg(0xD625, 0xB7);
		LCD_WriteReg(0xD626, 0x02);
		LCD_WriteReg(0xD627, 0xCF);
		LCD_WriteReg(0xD628, 0x02);
		LCD_WriteReg(0xD629, 0xDE);
		LCD_WriteReg(0xD62A, 0x02);
		LCD_WriteReg(0xD62B, 0xF2);
		LCD_WriteReg(0xD62C, 0x02);
		LCD_WriteReg(0xD62D, 0xFE);
		LCD_WriteReg(0xD62E, 0x03);
		LCD_WriteReg(0xD62F, 0x10);
		LCD_WriteReg(0xD630, 0x03);
		LCD_WriteReg(0xD631, 0x33);
		LCD_WriteReg(0xD632, 0x03);
		LCD_WriteReg(0xD633, 0x6D);
		// LV2 Page 0 enable
		LCD_WriteReg(0xF000, 0x55);
		LCD_WriteReg(0xF001, 0xAA);
		LCD_WriteReg(0xF002, 0x52);
		LCD_WriteReg(0xF003, 0x08);
		LCD_WriteReg(0xF004, 0x00);

		// 480*800
		//	LCD_WriteReg(0xB500, 0x50);///////////////////////////////////////////////////////
		// Display control
		LCD_WriteReg(0xB100, 0xCC);
		LCD_WriteReg(0xB101, 0x00);
		// Source hold time
		LCD_WriteReg(0xB600, 0x05);
		// Gate EQ control
		LCD_WriteReg(0xB700, 0x70);
		LCD_WriteReg(0xB701, 0x70);
		// Source EQ control (Mode 2)
		LCD_WriteReg(0xB800, 0x01);
		LCD_WriteReg(0xB801, 0x03);
		LCD_WriteReg(0xB802, 0x03);
		LCD_WriteReg(0xB803, 0x03);
		// Inversion mode (2-dot)
		LCD_WriteReg(0xBC00, 0x02);
		LCD_WriteReg(0xBC01, 0x00);
		LCD_WriteReg(0xBC02, 0x00);

		// Frame rate	(Nova non-used)
		//		LCD_WriteReg(0xBD00,0x01);//////////////////////////////////////////////////////
		//		LCD_WriteReg(0xBD01,0x84);//////////////////////////////////////////////////////
		//		LCD_WriteReg(0xBD02,0x1C);//////////////////////////////////////////////////////
		//		LCD_WriteReg(0xBD03,0x1C);//////////////////////////////////////////////////////
		//		LCD_WriteReg(0xBD04,0x00);//////////////////////////////////////////////////////

		// Timing control 4H w/ 4-delay
		LCD_WriteReg(0xC900, 0xD0);
		LCD_WriteReg(0xC901, 0x02);
		LCD_WriteReg(0xC902, 0x50);
		LCD_WriteReg(0xC903, 0x50);
		LCD_WriteReg(0xC904, 0x50);

		//	LCD_WriteReg(0x3600,0x00);//////////////////////////////////////////////
		LCD_WriteReg(0x3500, 0x00);

		//		LCD_WriteReg(0xFF00,0xAA);//////////////////////////////////////////////
		//		LCD_WriteReg(0xFF01,0x55);//////////////////////////////////////////////
		//		LCD_WriteReg(0xFF02,0x25);//////////////////////////////////////////////
		//		LCD_WriteReg(0xFF03,0x01);//////////////////////////////////////////////
		//
		//		LCD_WriteReg(0xFC00,0xAA);//////////////////////////////////////////////
		//		LCD_WriteReg(0xFC01,0xAA);//////////////////////////////////////////////
		//		LCD_WriteReg(0xFC02,0xAA);//////////////////////////////////////////////
		//		LCD_WriteReg(0x3A00,0x06);//////////////////////////////////////////////

		LCD_WriteReg(0x3500, 0x00);
		LCD_WriteReg(0x3A00, 0x55); // 16-bit/pixel
		LCD_WR_REG(0x1100);
		delay_us(160);
		LCD_WR_REG(0x2900);
	}
	else if (lcddev.id == 0x8009)
	{
		/// HSD43+OTM8009A  20221116  ok
		LCD_WR_REG(0xff00); //
		LCD_WR_DATA(0x80);
		LCD_WR_REG(0xff01); // enable EXTC
		LCD_WR_DATA(0x09);
		LCD_WR_REG(0xff02); //
		LCD_WR_DATA(0x01);

		LCD_WR_REG(0xff80); // enable Orise mode
		LCD_WR_DATA(0x80);
		LCD_WR_REG(0xff81); //
		LCD_WR_DATA(0x09);

		LCD_WR_REG(0xff03); // enable SPI+I2C cmd2 read
		LCD_WR_DATA(0x01);

		// gamma DC
		LCD_WR_REG(0xc0b4);
		LCD_WR_DATA(0x50); // Panel Driving Mode 0x50  column inversion add 20221116
		LCD_WR_REG(0xC489);
		LCD_WR_DATA(0x08); // reg off	  add
		LCD_WR_REG(0xC0a3);
		LCD_WR_DATA(0x00); // pre-charge //V02   add

		LCD_WR_REG(0xC582); // REG-pump23
		LCD_WR_DATA(0xA3);
		LCD_WR_REG(0xC590); // Pump setting (3x=D6)-->(2x=96)//v02 01/11
		LCD_WR_DATA(0xd6);
		LCD_WR_REG(0xC591); // Pump setting(VGH/VGL)
		LCD_WR_DATA(0x87);
		LCD_WR_REG(0xD800); // GVDD=4.5V
		LCD_WR_DATA(0x73);
		LCD_WR_REG(0xD801); // NGVDD=4.5V
		LCD_WR_DATA(0x71);
		// VCOMDC
		LCD_WR_REG(0xd900); // VCOMDC=
		LCD_WR_DATA(0x58);	// 0x50  20221116

		LCD_WR_REG(0xE1);
		LCD_WR_DATA(0x09);
		LCD_WR_DATA(0x10);
		LCD_WR_DATA(0x14);
		LCD_WR_DATA(0x13);
		LCD_WR_DATA(0x07);
		LCD_WR_DATA(0x24);
		LCD_WR_DATA(0x13);
		LCD_WR_DATA(0x13);
		LCD_WR_DATA(0x01);
		LCD_WR_DATA(0x04);
		LCD_WR_DATA(0x05);
		LCD_WR_DATA(0x06);
		LCD_WR_DATA(0x14);
		LCD_WR_DATA(0x37);
		LCD_WR_DATA(0x34);
		LCD_WR_DATA(0x05);

		LCD_WR_REG(0xE2);
		LCD_WR_DATA(0x09);
		LCD_WR_DATA(0x10);
		LCD_WR_DATA(0x14);
		LCD_WR_DATA(0x13);
		LCD_WR_DATA(0x07);
		LCD_WR_DATA(0x24);
		LCD_WR_DATA(0x13);
		LCD_WR_DATA(0x13);
		LCD_WR_DATA(0x01);
		LCD_WR_DATA(0x04);
		LCD_WR_DATA(0x05);
		LCD_WR_DATA(0x06);
		LCD_WR_DATA(0x14);
		LCD_WR_DATA(0x37);
		LCD_WR_DATA(0x34);
		LCD_WR_DATA(0x05);

		LCD_WR_REG(0xC181); // Frame rate 65Hz//V02
		LCD_WR_DATA(0x66);

		//// RGB I/F setting VSYNC for OTM8018 0x0e

		LCD_WR_REG(0xC1a1); // external Vsync(08)  /Vsync,Hsync(0c) /Vsync,Hsync,DE(0e) //V02(0e)  / all  included clk(0f)
		LCD_WR_DATA(0x08);
		LCD_WR_REG(0xC0a3); // pre-charge //V02
		LCD_WR_DATA(0x1b);
		LCD_WR_REG(0xC481); // source bias //V02
		LCD_WR_DATA(0x83);
		LCD_WR_REG(0xC592); // Pump45
		LCD_WR_DATA(0x01);	//(01)
		LCD_WR_REG(0xC5B1); // DC voltage setting ;[0]GVDD output, default: 0xa8
		LCD_WR_DATA(0xA9);

		//--------------------------------------------------------------------------------
		//		initial setting 2 < tcon_goa_wave >
		//--------------------------------------------------------------------------------
		// CE8x : vst1, vst2, vst3, vst4
		LCD_WR_REG(0xCE80); // ce81[7:0] : vst1_shift[7:0]
		LCD_WR_DATA(0x85);
		LCD_WR_REG(0xCE81); // ce82[7:0] : 0000,	vst1_width[3:0]
		LCD_WR_DATA(0x03);
		LCD_WR_REG(0xCE82); // ce83[7:0] : vst1_tchop[7:0]
		LCD_WR_DATA(0x00);
		LCD_WR_REG(0xCE83); // ce84[7:0] : vst2_shift[7:0]
		LCD_WR_DATA(0x84);
		LCD_WR_REG(0xCE84); // ce85[7:0] : 0000,	vst2_width[3:0]
		LCD_WR_DATA(0x03);
		LCD_WR_REG(0xCE85); // ce86[7:0] : vst2_tchop[7:0]
		LCD_WR_DATA(0x00);
		LCD_WR_REG(0xCE86); // ce87[7:0] : vst3_shift[7:0]
		LCD_WR_DATA(0x83);
		LCD_WR_REG(0xCE87); // ce88[7:0] : 0000,	vst3_width[3:0]
		LCD_WR_DATA(0x03);
		LCD_WR_REG(0xCE88); // ce89[7:0] : vst3_tchop[7:0]
		LCD_WR_DATA(0x00);
		LCD_WR_REG(0xCE89); // ce8a[7:0] : vst4_shift[7:0]
		LCD_WR_DATA(0x82);
		LCD_WR_REG(0xCE8a); // ce8b[7:0] : 0000,	vst4_width[3:0]
		LCD_WR_DATA(0x03);
		LCD_WR_REG(0xCE8b); // ce8c[7:0] : vst4_tchop[7:0]
		LCD_WR_DATA(0x00);

		// CEAx : clka1, clka2
		LCD_WR_REG(0xCEa0); // cea1[7:0] : clka1_width[3:0], clka1_shift[11:8]
		LCD_WR_DATA(0x38);
		LCD_WR_REG(0xCEa1); // cea2[7:0] : clka1_shift[7:0]
		LCD_WR_DATA(0x02);
		LCD_WR_REG(0xCEa2); // cea3[7:0] : clka1_sw_tg, odd_high, flat_head, flat_tail, switch[11:8]
		LCD_WR_DATA(0x03);
		LCD_WR_REG(0xCEa3); // cea4[7:0] : clka1_switch[7:0]
		LCD_WR_DATA(0x21);
		LCD_WR_REG(0xCEa4); // cea5[7:0] : clka1_extend[7:0]
		LCD_WR_DATA(0x00);
		LCD_WR_REG(0xCEa5); // cea6[7:0] : clka1_tchop[7:0]
		LCD_WR_DATA(0x00);
		LCD_WR_REG(0xCEa6); // cea7[7:0] : clka1_tglue[7:0]
		LCD_WR_DATA(0x00);
		LCD_WR_REG(0xCEa7); // cea8[7:0] : clka2_width[3:0], clka2_shift[11:8]
		LCD_WR_DATA(0x38);
		LCD_WR_REG(0xCEa8); // cea9[7:0] : clka2_shift[7:0]
		LCD_WR_DATA(0x01);
		LCD_WR_REG(0xCEa9); // ceaa[7:0] : clka2_sw_tg, odd_high, flat_head, flat_tail, switch[11:8]
		LCD_WR_DATA(0x03);
		LCD_WR_REG(0xCEaa); // ceab[7:0] : clka2_switch[7:0]
		LCD_WR_DATA(0x22);
		LCD_WR_REG(0xCEab); // ceac[7:0] : clka2_extend
		LCD_WR_DATA(0x00);
		LCD_WR_REG(0xCEac); // cead[7:0] : clka2_tchop
		LCD_WR_DATA(0x00);
		LCD_WR_REG(0xCEad); // ceae[7:0] : clka2_tglue
		LCD_WR_DATA(0x00);

		// CEBx : clka3, clka4
		LCD_WR_REG(0xCEb0); // ceb1[7:0] : clka3_width[3:0], clka3_shift[11:8]
		LCD_WR_DATA(0x38);
		LCD_WR_REG(0xCEb1); // ceb2[7:0] : clka3_shift[7:0]
		LCD_WR_DATA(0x00);
		LCD_WR_REG(0xCEb2); // ceb3[7:0] : clka3_sw_tg, odd_high, flat_head, flat tail, switch[11:8]
		LCD_WR_DATA(0x03);
		LCD_WR_REG(0xCEb3); // ceb4[7:0] : clka3_switch[7:0]
		LCD_WR_DATA(0x23);
		LCD_WR_REG(0xCEb4); // ceb5[7:0] : clka3_extend[7:0]
		LCD_WR_DATA(0x00);
		LCD_WR_REG(0xCEb5); // ceb6[7:0] : clka3_tchop[7:0]
		LCD_WR_DATA(0x00);
		LCD_WR_REG(0xCEb6); // ceb7[7:0] : clka3_tglue[7:0]
		LCD_WR_DATA(0x00);
		LCD_WR_REG(0xCEb7); // ceb8[7:0] : clka4_width[3:0], clka2 shift[11:8]
		LCD_WR_DATA(0x30);
		LCD_WR_REG(0xCEb8); // ceb9[7:0] : clka4_shift[7:0]
		LCD_WR_DATA(0x00);
		LCD_WR_REG(0xCEb9); // ceba[7:0] : clka4_sw_tg, odd_high, flat_head, flat_tail, switch[11:8]
		LCD_WR_DATA(0x03);
		LCD_WR_REG(0xCEba); // cebb[7:0] : clka4_switch[7:0]
		LCD_WR_DATA(0x24);
		LCD_WR_REG(0xCEbb); // cebc[7:0] : clka4_extend
		LCD_WR_DATA(0x00);
		LCD_WR_REG(0xCEbc); // cebd[7:0] : clka4_tchop
		LCD_WR_DATA(0x00);
		LCD_WR_REG(0xCEbd); // cebe[7:0] : clka4_tglue
		LCD_WR_DATA(0x00);

		// CECx : clkb1, clkb2
		LCD_WR_REG(0xCEc0); // cec1[7:0] : clkb1_width[3:0], clkb1_shift[11:8]
		LCD_WR_DATA(0x30);
		LCD_WR_REG(0xCEc1); // cec2[7:0] : clkb1_shift[7:0]
		LCD_WR_DATA(0x01);
		LCD_WR_REG(0xCEc2); // cec3[7:0] : clkb1_sw_tg, odd_high, flat_head, flat tail, switch[11:8]
		LCD_WR_DATA(0x03);
		LCD_WR_REG(0xCEc3); // cec4[7:0] : clkb1_switch[7:0]
		LCD_WR_DATA(0x25);
		LCD_WR_REG(0xCEc4); // cec5[7:0] : clkb1_extend[7:0]
		LCD_WR_DATA(0x00);
		LCD_WR_REG(0xCEc5); // cec6[7:0] : clkb1_tchop[7:0]
		LCD_WR_DATA(0x00);
		LCD_WR_REG(0xCEc6); // cec7[7:0] : clkb1_tglue[7:0]
		LCD_WR_DATA(0x00);
		LCD_WR_REG(0xCEc7); // cec8[7:0] : clkb2_width[3:0], clkb2_shift[11:8]
		LCD_WR_DATA(0x30);
		LCD_WR_REG(0xCEc8); // cec9[7:0] : clkb2_shift[7:0]
		LCD_WR_DATA(0x02);
		LCD_WR_REG(0xCEc9); // ceca[7:0] : clkb2_sw_tg, odd_high, flat_head, flat tail, switch[11:8]
		LCD_WR_DATA(0x03);
		LCD_WR_REG(0xCEca); // cecb[7:0] : clkb2_switch[7:0]
		LCD_WR_DATA(0x26);
		LCD_WR_REG(0xCEcb); // cecc[7:0] : clkb2_extend
		LCD_WR_DATA(0x00);
		LCD_WR_REG(0xCEcc); // cecd[7:0] : clkb2_tchop
		LCD_WR_DATA(0x00);
		LCD_WR_REG(0xCEcd); // cece[7:0] : clkb2_tglue
		LCD_WR_DATA(0x00);

		// CEDx : clkb3, clkb4
		LCD_WR_REG(0xCEd0); // ced1[7:0] : clkb3_width[3:0], clkb3_shift[11:8]
		LCD_WR_DATA(0x30);
		LCD_WR_REG(0xCEd1); // ced2[7:0] : clkb3_shift[7:0]
		LCD_WR_DATA(0x03);
		LCD_WR_REG(0xCEd2); // ced3[7:0] : clkb3_sw_tg, odd_high, flat_head, flat tail, switch[11:8]
		LCD_WR_DATA(0x03);
		LCD_WR_REG(0xCEd3); // ced4[7:0] : clkb3_switch[7:0]
		LCD_WR_DATA(0x27);
		LCD_WR_REG(0xCEd4); // ced5[7:0] : clkb3_extend[7:0]
		LCD_WR_DATA(0x00);
		LCD_WR_REG(0xCEd5); // ced6[7:0] : clkb3_tchop[7:0]
		LCD_WR_DATA(0x00);
		LCD_WR_REG(0xCEd6); // ced7[7:0] : clkb3_tglue[7:0]
		LCD_WR_DATA(0x00);
		LCD_WR_REG(0xCEd7); // ced8[7:0] : clkb4_width[3:0], clkb4_shift[11:8]
		LCD_WR_DATA(0x30);
		LCD_WR_REG(0xCEd8); // ced9[7:0] : clkb4_shift[7:0]
		LCD_WR_DATA(0x04);
		LCD_WR_REG(0xCEd9); // ceda[7:0] : clkb4_sw_tg, odd_high, flat_head, flat tail, switch[11:8]
		LCD_WR_DATA(0x03);
		LCD_WR_REG(0xCEda); // cedb[7:0] : clkb4_switch[7:0]
		LCD_WR_DATA(0x28);
		LCD_WR_REG(0xCEdb); // cedc[7:0] : clkb4_extend
		LCD_WR_DATA(0x00);
		LCD_WR_REG(0xCEdc); // cedd[7:0] : clkb4_tchop
		LCD_WR_DATA(0x00);
		LCD_WR_REG(0xCEdd); // cede[7:0] : clkb4_tglue
		LCD_WR_DATA(0x00);

		// CFCx :
		LCD_WR_REG(0xCFc0); // cfc1[7:0] : eclk_normal_width[7:0]
		LCD_WR_DATA(0x00);
		LCD_WR_REG(0xCFc1); // cfc2[7:0] : eclk_partial_width[7:0]
		LCD_WR_DATA(0x00);
		LCD_WR_REG(0xCFc2); // cfc3[7:0] : all_normal_tchop[7:0]
		LCD_WR_DATA(0x00);
		LCD_WR_REG(0xCFc3); // cfc4[7:0] : all_partial_tchop[7:0]
		LCD_WR_DATA(0x00);
		LCD_WR_REG(0xCFc4); // cfc5[7:0] : eclk1_follow[3:0], eclk2_follow[3:0]
		LCD_WR_DATA(0x00);
		LCD_WR_REG(0xCFc5); // cfc6[7:0] : eclk3_follow[3:0], eclk4_follow[3:0]
		LCD_WR_DATA(0x00);
		LCD_WR_REG(0xCFc6); // cfc7[7:0] : 00, vstmask, vendmask, 00, dir1, dir2 (0=VGL, 1=VGH)
		LCD_WR_DATA(0x00);
		LCD_WR_REG(0xCFc7); // cfc8[7:0] : reg_goa_gnd_opt, reg_goa_dpgm_tail_set, reg_goa_f_gating_en, reg_goa_f_odd_gating, toggle_mod1, 2, 3, 4
		LCD_WR_DATA(0x00);
		LCD_WR_REG(0xCFc8); // cfc9[7:0] : duty_block[3:0], DGPM[3:0]
		LCD_WR_DATA(0x00);
		LCD_WR_REG(0xCFc9); // cfca[7:0] : reg_goa_gnd_period[7:0]
		LCD_WR_DATA(0x00);

		// CFDx :
		LCD_WR_REG(0xCFd0); // cfd1[7:0] : 0000000, reg_goa_frame_odd_high
		LCD_WR_DATA(0x00);	// Parameter 1

		//--------------------------------------------------------------------------------
		//		initial setting 3 < Panel setting >
		//--------------------------------------------------------------------------------
		// cbcx
		LCD_WR_REG(0xCBc0); // cbc1[7:0] : enmode H-byte of sig1  (pwrof_0, pwrof_1, norm, pwron_4 )
		LCD_WR_DATA(0x00);
		LCD_WR_REG(0xCBc1); // cbc2[7:0] : enmode H-byte of sig2  (pwrof_0, pwrof_1, norm, pwron_4 )
		LCD_WR_DATA(0x00);
		LCD_WR_REG(0xCBc2); // cbc3[7:0] : enmode H-byte of sig3  (pwrof_0, pwrof_1, norm, pwron_4 )
		LCD_WR_DATA(0x00);
		LCD_WR_REG(0xCBc3); // cbc4[7:0] : enmode H-byte of sig4  (pwrof_0, pwrof_1, norm, pwron_4 )
		LCD_WR_DATA(0x00);
		LCD_WR_REG(0xCBc4); // cbc5[7:0] : enmode H-byte of sig5  (pwrof_0, pwrof_1, norm, pwron_4 )
		LCD_WR_DATA(0x04);
		LCD_WR_REG(0xCBc5); // cbc6[7:0] : enmode H-byte of sig6  (pwrof_0, pwrof_1, norm, pwron_4 )
		LCD_WR_DATA(0x04);
		LCD_WR_REG(0xCBc6); // cbc7[7:0] : enmode H-byte of sig7  (pwrof_0, pwrof_1, norm, pwron_4 )
		LCD_WR_DATA(0x04);
		LCD_WR_REG(0xCBc7); // cbc8[7:0] : enmode H-byte of sig8  (pwrof_0, pwrof_1, norm, pwron_4 )
		LCD_WR_DATA(0x04);
		LCD_WR_REG(0xCBc8); // cbc9[7:0] : enmode H-byte of sig9  (pwrof_0, pwrof_1, norm, pwron_4 )
		LCD_WR_DATA(0x04);
		LCD_WR_REG(0xCBc9); // cbca[7:0] : enmode H-byte of sig10 (pwrof_0, pwrof_1, norm, pwron_4 )
		LCD_WR_DATA(0x04);
		LCD_WR_REG(0xCBca); // cbcb[7:0] : enmode H-byte of sig11 (pwrof_0, pwrof_1, norm, pwron_4 )
		LCD_WR_DATA(0x00);
		LCD_WR_REG(0xCBcb); // cbcc[7:0] : enmode H-byte of sig12 (pwrof_0, pwrof_1, norm, pwron_4 )
		LCD_WR_DATA(0x00);
		LCD_WR_REG(0xCBcc); // cbcd[7:0] : enmode H-byte of sig13 (pwrof_0, pwrof_1, norm, pwron_4 )
		LCD_WR_DATA(0x00);
		LCD_WR_REG(0xCBcd); // cbce[7:0] : enmode H-byte of sig14 (pwrof_0, pwrof_1, norm, pwron_4 )
		LCD_WR_DATA(0x00);
		LCD_WR_REG(0xCBce); // cbcf[7:0] : enmode H-byte of sig15 (pwrof_0, pwrof_1, norm, pwron_4 )
		LCD_WR_DATA(0x00);

		// cbdx
		LCD_WR_REG(0xCBd0); // cbd1[7:0] : enmode H-byte of sig16 (pwrof_0, pwrof_1, norm, pwron_4 )
		LCD_WR_DATA(0x00);
		LCD_WR_REG(0xCBd1); // cbd2[7:0] : enmode H-byte of sig17 (pwrof_0, pwrof_1, norm, pwron_4 )
		LCD_WR_DATA(0x00);
		LCD_WR_REG(0xCBd2); // cbd3[7:0] : enmode H-byte of sig18 (pwrof_0, pwrof_1, norm, pwron_4 )
		LCD_WR_DATA(0x00);
		LCD_WR_REG(0xCBd3); // cbd4[7:0] : enmode H-byte of sig19 (pwrof_0, pwrof_1, norm, pwron_4 )
		LCD_WR_DATA(0x00);
		LCD_WR_REG(0xCBd4); // cbd5[7:0] : enmode H-byte of sig20 (pwrof_0, pwrof_1, norm, pwron_4 )
		LCD_WR_DATA(0x00);
		LCD_WR_REG(0xCBd5); // cbd6[7:0] : enmode H-byte of sig21 (pwrof_0, pwrof_1, norm, pwron_4 )
		LCD_WR_DATA(0x00);
		LCD_WR_REG(0xCBd6); // cbd7[7:0] : enmode H-byte of sig22 (pwrof_0, pwrof_1, norm, pwron_4 )
		LCD_WR_DATA(0x00);
		LCD_WR_REG(0xCBd7); // cbd8[7:0] : enmode H-byte of sig23 (pwrof_0, pwrof_1, norm, pwron_4 )
		LCD_WR_DATA(0x00);
		LCD_WR_REG(0xCBd8); // cbd9[7:0] : enmode H-byte of sig24 (pwrof_0, pwrof_1, norm, pwron_4 )
		LCD_WR_DATA(0x00);
		LCD_WR_REG(0xCBd9); // cbda[7:0] : enmode H-byte of sig25 (pwrof_0, pwrof_1, norm, pwron_4 )
		LCD_WR_DATA(0x04);
		LCD_WR_REG(0xCBda); // cbdb[7:0] : enmode H-byte of sig26 (pwrof_0, pwrof_1, norm, pwron_4 )
		LCD_WR_DATA(0x04);
		LCD_WR_REG(0xCBdb); // cbdc[7:0] : enmode H-byte of sig27 (pwrof_0, pwrof_1, norm, pwron_4 )
		LCD_WR_DATA(0x04);
		LCD_WR_REG(0xCBdc); // cbdd[7:0] : enmode H-byte of sig28 (pwrof_0, pwrof_1, norm, pwron_4 )
		LCD_WR_DATA(0x04);
		LCD_WR_REG(0xCBdd); // cbde[7:0] : enmode H-byte of sig29 (pwrof_0, pwrof_1, norm, pwron_4 )
		LCD_WR_DATA(0x04);
		LCD_WR_REG(0xCBde); // cbdf[7:0] : enmode H-byte of sig30 (pwrof_0, pwrof_1, norm, pwron_4 )
		LCD_WR_DATA(0x04);

		// cbex
		LCD_WR_REG(0xCBe0); // cbe1[7:0] : enmode H-byte of sig31 (pwrof_0, pwrof_1, norm, pwron_4 )
		LCD_WR_DATA(0x00);
		LCD_WR_REG(0xCBe1); // cbe2[7:0] : enmode H-byte of sig32 (pwrof_0, pwrof_1, norm, pwron_4 )
		LCD_WR_DATA(0x00);
		LCD_WR_REG(0xCBe2); // cbe3[7:0] : enmode H-byte of sig33 (pwrof_0, pwrof_1, norm, pwron_4 )
		LCD_WR_DATA(0x00);
		LCD_WR_REG(0xCBe3); // cbe4[7:0] : enmode H-byte of sig34 (pwrof_0, pwrof_1, norm, pwron_4 )
		LCD_WR_DATA(0x00);
		LCD_WR_REG(0xCBe4); // cbe5[7:0] : enmode H-byte of sig35 (pwrof_0, pwrof_1, norm, pwron_4 )
		LCD_WR_DATA(0x00);
		LCD_WR_REG(0xCBe5); // cbe6[7:0] : enmode H-byte of sig36 (pwrof_0, pwrof_1, norm, pwron_4 )
		LCD_WR_DATA(0x00);
		LCD_WR_REG(0xCBe6); // cbe7[7:0] : enmode H-byte of sig37 (pwrof_0, pwrof_1, norm, pwron_4 )
		LCD_WR_DATA(0x00);
		LCD_WR_REG(0xCBe7); // cbe8[7:0] : enmode H-byte of sig38 (pwrof_0, pwrof_1, norm, pwron_4 )
		LCD_WR_DATA(0x00);
		LCD_WR_REG(0xCBe8); // cbe9[7:0] : enmode H-byte of sig39 (pwrof_0, pwrof_1, norm, pwron_4 )
		LCD_WR_DATA(0x00);
		LCD_WR_REG(0xCBe9); // cbea[7:0] : enmode H-byte of sig40 (pwrof_0, pwrof_1, norm, pwron_4 )
		LCD_WR_DATA(0x00);

		// cc8x
		LCD_WR_REG(0xCC80); // cc81[7:0] : reg setting for signal01 selection with u2d mode
		LCD_WR_DATA(0x00);
		LCD_WR_REG(0xCC81); // cc82[7:0] : reg setting for signal02 selection with u2d mode
		LCD_WR_DATA(0x00);
		LCD_WR_REG(0xCC82); // cc83[7:0] : reg setting for signal03 selection with u2d mode
		LCD_WR_DATA(0x00);
		LCD_WR_REG(0xCC83); // cc84[7:0] : reg setting for signal04 selection with u2d mode
		LCD_WR_DATA(0x00);
		LCD_WR_REG(0xCC84); // cc85[7:0] : reg setting for signal05 selection with u2d mode
		LCD_WR_DATA(0x0C);
		LCD_WR_REG(0xCC85); // cc86[7:0] : reg setting for signal06 selection with u2d mode
		LCD_WR_DATA(0x0A);
		LCD_WR_REG(0xCC86); // cc87[7:0] : reg setting for signal07 selection with u2d mode
		LCD_WR_DATA(0x10);
		LCD_WR_REG(0xCC87); // cc88[7:0] : reg setting for signal08 selection with u2d mode
		LCD_WR_DATA(0x0E);
		LCD_WR_REG(0xCC88); // cc89[7:0] : reg setting for signal09 selection with u2d mode
		LCD_WR_DATA(0x03);
		LCD_WR_REG(0xCC89); // cc8a[7:0] : reg setting for signal10 selection with u2d mode
		LCD_WR_DATA(0x04);

		// cc9x
		LCD_WR_REG(0xCC90); // cc91[7:0] : reg setting for signal11 selection with u2d mode
		LCD_WR_DATA(0x00);
		LCD_WR_REG(0xCC91); // cc92[7:0] : reg setting for signal12 selection with u2d mode
		LCD_WR_DATA(0x00);
		LCD_WR_REG(0xCC92); // cc93[7:0] : reg setting for signal13 selection with u2d mode
		LCD_WR_DATA(0x00);
		LCD_WR_REG(0xCC93); // cc94[7:0] : reg setting for signal14 selection with u2d mode
		LCD_WR_DATA(0x00);
		LCD_WR_REG(0xCC94); // cc95[7:0] : reg setting for signal15 selection with u2d mode
		LCD_WR_DATA(0x00);
		LCD_WR_REG(0xCC95); // cc96[7:0] : reg setting for signal16 selection with u2d mode
		LCD_WR_DATA(0x00);
		LCD_WR_REG(0xCC96); // cc97[7:0] : reg setting for signal17 selection with u2d mode
		LCD_WR_DATA(0x00);
		LCD_WR_REG(0xCC97); // cc98[7:0] : reg setting for signal18 selection with u2d mode
		LCD_WR_DATA(0x00);
		LCD_WR_REG(0xCC98); // cc99[7:0] : reg setting for signal19 selection with u2d mode
		LCD_WR_DATA(0x00);
		LCD_WR_REG(0xCC99); // cc9a[7:0] : reg setting for signal20 selection with u2d mode
		LCD_WR_DATA(0x00);
		LCD_WR_REG(0xCC9a); // cc9b[7:0] : reg setting for signal21 selection with u2d mode
		LCD_WR_DATA(0x00);
		LCD_WR_REG(0xCC9b); // cc9c[7:0] : reg setting for signal22 selection with u2d mode
		LCD_WR_DATA(0x00);
		LCD_WR_REG(0xCC9c); // cc9d[7:0] : reg setting for signal23 selection with u2d mode
		LCD_WR_DATA(0x00);
		LCD_WR_REG(0xCC9d); // cc9e[7:0] : reg setting for signal24 selection with u2d mode
		LCD_WR_DATA(0x00);
		LCD_WR_REG(0xCC9e); // cc9f[7:0] : reg setting for signal25 selection with u2d mode
		LCD_WR_DATA(0x0B);

		// ccax
		LCD_WR_REG(0xCCa0); // cca1[7:0] : reg setting for signal26 selection with u2d mode
		LCD_WR_DATA(0x09);
		LCD_WR_REG(0xCCa1); // cca2[7:0] : reg setting for signal27 selection with u2d mode
		LCD_WR_DATA(0x0F);
		LCD_WR_REG(0xCCa2); // cca3[7:0] : reg setting for signal28 selection with u2d mode
		LCD_WR_DATA(0x0D);
		LCD_WR_REG(0xCCa3); // cca4[7:0] : reg setting for signal29 selection with u2d mode
		LCD_WR_DATA(0x01);
		LCD_WR_REG(0xCCa4); // cca5[7:0] : reg setting for signal20 selection with u2d mode
		LCD_WR_DATA(0x02);
		LCD_WR_REG(0xCCa5); // cca6[7:0] : reg setting for signal31 selection with u2d mode
		LCD_WR_DATA(0x00);
		LCD_WR_REG(0xCCa6); // cca7[7:0] : reg setting for signal32 selection with u2d mode
		LCD_WR_DATA(0x00);
		LCD_WR_REG(0xCCa7); // cca8[7:0] : reg setting for signal33 selection with u2d mode
		LCD_WR_DATA(0x00);
		LCD_WR_REG(0xCCa8); // cca9[7:0] : reg setting for signal34 selection with u2d mode
		LCD_WR_DATA(0x00);
		LCD_WR_REG(0xCCa9); // ccaa[7:0] : reg setting for signal35 selection with u2d mode
		LCD_WR_DATA(0x00);
		LCD_WR_REG(0xCCaa); // ccab[7:0] : reg setting for signal36 selection with u2d mode
		LCD_WR_DATA(0x00);
		LCD_WR_REG(0xCCab); // ccac[7:0] : reg setting for signal37 selection with u2d mode
		LCD_WR_DATA(0x00);
		LCD_WR_REG(0xCCac); // ccad[7:0] : reg setting for signal38 selection with u2d mode
		LCD_WR_DATA(0x00);
		LCD_WR_REG(0xCCad); // ccae[7:0] : reg setting for signal39 selection with u2d mode
		LCD_WR_DATA(0x00);
		LCD_WR_REG(0xCCae); // ccaf[7:0] : reg setting for signal40 selection with u2d mode
		LCD_WR_DATA(0x00);

		// ccbx
		LCD_WR_REG(0xCCb0); // ccb1[7:0] : reg setting for signal01 selection with d2u mode
		LCD_WR_DATA(0x00);
		LCD_WR_REG(0xCCb1); // ccb2[7:0] : reg setting for signal02 selection with d2u mode
		LCD_WR_DATA(0x00);
		LCD_WR_REG(0xCCb2); // ccb3[7:0] : reg setting for signal03 selection with d2u mode
		LCD_WR_DATA(0x00);
		LCD_WR_REG(0xCCb3); // ccb4[7:0] : reg setting for signal04 selection with d2u mode
		LCD_WR_DATA(0x00);
		LCD_WR_REG(0xCCb4); // ccb5[7:0] : reg setting for signal05 selection with d2u mode
		LCD_WR_DATA(0x0D);
		LCD_WR_REG(0xCCb5); // ccb6[7:0] : reg setting for signal06 selection with d2u mode
		LCD_WR_DATA(0x0F);
		LCD_WR_REG(0xCCb6); // ccb7[7:0] : reg setting for signal07 selection with d2u mode
		LCD_WR_DATA(0x09);
		LCD_WR_REG(0xCCb7); // ccb8[7:0] : reg setting for signal08 selection with d2u mode
		LCD_WR_DATA(0x0B);
		LCD_WR_REG(0xCCb8); // ccb9[7:0] : reg setting for signal09 selection with d2u mode
		LCD_WR_DATA(0x02);
		LCD_WR_REG(0xCCb9); // ccba[7:0] : reg setting for signal10 selection with d2u mode
		LCD_WR_DATA(0x01);

		// cccx
		LCD_WR_REG(0xCCc0); // ccc1[7:0] : reg setting for signal11 selection with d2u mode
		LCD_WR_DATA(0x00);
		LCD_WR_REG(0xCCc1); // ccc2[7:0] : reg setting for signal12 selection with d2u mode
		LCD_WR_DATA(0x00);
		LCD_WR_REG(0xCCc2); // ccc3[7:0] : reg setting for signal13 selection with d2u mode
		LCD_WR_DATA(0x00);
		LCD_WR_REG(0xCCc3); // ccc4[7:0] : reg setting for signal14 selection with d2u mode
		LCD_WR_DATA(0x00);
		LCD_WR_REG(0xCCc4); // ccc5[7:0] : reg setting for signal15 selection with d2u mode
		LCD_WR_DATA(0x00);
		LCD_WR_REG(0xCCc5); // ccc6[7:0] : reg setting for signal16 selection with d2u mode
		LCD_WR_DATA(0x00);
		LCD_WR_REG(0xCCc6); // ccc7[7:0] : reg setting for signal17 selection with d2u mode
		LCD_WR_DATA(0x00);
		LCD_WR_REG(0xCCc7); // ccc8[7:0] : reg setting for signal18 selection with d2u mode
		LCD_WR_DATA(0x00);
		LCD_WR_REG(0xCCc8); // ccc9[7:0] : reg setting for signal19 selection with d2u mode
		LCD_WR_DATA(0x00);
		LCD_WR_REG(0xCCc9); // ccca[7:0] : reg setting for signal20 selection with d2u mode
		LCD_WR_DATA(0x00);
		LCD_WR_REG(0xCCca); // cccb[7:0] : reg setting for signal21 selection with d2u mode
		LCD_WR_DATA(0x00);
		LCD_WR_REG(0xCCcb); // cccc[7:0] : reg setting for signal22 selection with d2u mode
		LCD_WR_DATA(0x00);
		LCD_WR_REG(0xCCcc); // cccd[7:0] : reg setting for signal23 selection with d2u mode
		LCD_WR_DATA(0x00);
		LCD_WR_REG(0xCCcd); // ccce[7:0] : reg setting for signal24 selection with d2u mode
		LCD_WR_DATA(0x00);
		LCD_WR_REG(0xCCce); // cccf[7:0] : reg setting for signal25 selection with d2u mode
		LCD_WR_DATA(0x0E);

		// ccdx
		LCD_WR_REG(0xCCd0); // ccd1[7:0] : reg setting for signal26 selection with d2u mode
		LCD_WR_DATA(0x10);
		LCD_WR_REG(0xCCd1); // ccd2[7:0] : reg setting for signal27 selection with d2u mode
		LCD_WR_DATA(0x0A);
		LCD_WR_REG(0xCCd2); // ccd3[7:0] : reg setting for signal28 selection with d2u mode
		LCD_WR_DATA(0x0C);
		LCD_WR_REG(0xCCd3); // ccd4[7:0] : reg setting for signal29 selection with d2u mode
		LCD_WR_DATA(0x04);
		LCD_WR_REG(0xCCd4); // ccd5[7:0] : reg setting for signal30 selection with d2u mode
		LCD_WR_DATA(0x03);
		LCD_WR_REG(0xCCd5); // ccd6[7:0] : reg setting for signal31 selection with d2u mode
		LCD_WR_DATA(0x00);
		LCD_WR_REG(0xCCd6); // ccd7[7:0] : reg setting for signal32 selection with d2u mode
		LCD_WR_DATA(0x00);
		LCD_WR_REG(0xCCd7); // ccd8[7:0] : reg setting for signal33 selection with d2u mode
		LCD_WR_DATA(0x00);
		LCD_WR_REG(0xCCd8); // ccd9[7:0] : reg setting for signal34 selection with d2u mode
		LCD_WR_DATA(0x00);
		LCD_WR_REG(0xCCd9); // ccda[7:0] : reg setting for signal35 selection with d2u mode
		LCD_WR_DATA(0x00);
		LCD_WR_REG(0xCCda); // ccdb[7:0] : reg setting for signal36 selection with d2u mode
		LCD_WR_DATA(0x00);
		LCD_WR_REG(0xCCdb); // ccdc[7:0] : reg setting for signal37 selection with d2u mode
		LCD_WR_DATA(0x00);
		LCD_WR_REG(0xCCdc); // ccdd[7:0] : reg setting for signal38 selection with d2u mode
		LCD_WR_DATA(0x00);
		LCD_WR_REG(0xCCdd); // ccde[7:0] : reg setting for signal39 selection with d2u mode
		LCD_WR_DATA(0x00);
		LCD_WR_REG(0xCCde); // ccdf[7:0] : reg setting for signal40 selection with d2u mode
		LCD_WR_DATA(0x00);

		LCD_WR_REG(0x3A00);
		LCD_WR_DATA(0x55);

		LCD_WR_REG(0x1100);

		HAL_Delay(120);
		LCD_WR_REG(0x2900);

		HAL_Delay(50);
	}

	LCD_Display_Dir(1); // ����
	LCD_Clear(BLACK);		// ��ʾ
	HAL_Delay(1000);
}
// ��������
// color:Ҫ���������ɫ
void LCD_Clear(uint16_t color)
{
	uint32_t index = 0;
	uint32_t totalpoint = lcddev.width;
	totalpoint *= lcddev.height; // �õ��ܵ���
	LCD_SetCursor(0, 0);				 // ���ù��λ��
	LCD_WriteRAM_Prepare();		 // ��ʼд��GRAM
	for (index = 0; index < totalpoint; index++)
	{
		LCD->LCD_RAM = color;
	}
}
// ��ָ����������䵥����ɫ
//(sx,sy),(ex,ey):�����ζԽ�����,�����СΪ:(ex-sx+1)*(ey-sy+1)
// color:Ҫ������ɫ
void LCD_Fill(uint16_t sx, uint16_t sy, uint16_t ex, uint16_t ey, uint16_t color)
{
	uint16_t i, j;
	uint16_t xlen = 0;
	xlen = ex - sx + 1;
	for (i = sy; i <= ey; i++)
	{
		LCD_SetCursor(sx, i);		// ���ù��λ��
		LCD_WriteRAM_Prepare(); // ��ʼд��GRAM
		for (j = 0; j < xlen; j++)
			LCD->LCD_RAM = color; // ��ʾ��ɫ
	}
}
// ��ָ�����������ָ����ɫ��
//(sx,sy),(ex,ey):�����ζԽ�����,�����СΪ:(ex-sx+1)*(ey-sy+1)
// color:Ҫ������ɫ
void LCD_Color_Fill(uint16_t sx, uint16_t sy, uint16_t ex, uint16_t ey, uint16_t *color)
{
	uint16_t height, width;
	uint16_t i, j;
	width = ex - sx + 1;	// �õ����Ŀ���
	height = ey - sy + 1; // �߶�
	for (i = 0; i < height; i++)
	{
		LCD_SetCursor(sx, sy + i); // ���ù��λ��
		LCD_WriteRAM_Prepare();		 // ��ʼд��GRAM
		for (j = 0; j < width; j++)
			LCD->LCD_RAM = color[i * width + j]; // д������
	}
}
// ����
// x1,y1:�������
// x2,y2:�յ�����
void LCD_DrawLine(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2)
{
	uint16_t t;
	int xerr = 0, yerr = 0, delta_x, delta_y, distance;
	int incx, incy, uRow, uCol;
	delta_x = x2 - x1; // ������������
	delta_y = y2 - y1;
	uRow = x1;
	uCol = y1;
	if (delta_x > 0)
		incx = 1; // ���õ�������
	else if (delta_x == 0)
		incx = 0; // ��ֱ��
	else
	{
		incx = -1;
		delta_x = -delta_x;
	}
	if (delta_y > 0)
		incy = 1;
	else if (delta_y == 0)
		incy = 0; // ˮƽ��
	else
	{
		incy = -1;
		delta_y = -delta_y;
	}
	if (delta_x > delta_y)
		distance = delta_x; // ѡȡ��������������
	else
		distance = delta_y;
	for (t = 0; t <= distance + 1; t++) // �������
	{
		LCD_DrawPoint(uRow, uCol); // ����
		xerr += delta_x;
		yerr += delta_y;
		if (xerr > distance)
		{
			xerr -= distance;
			uRow += incx;
		}
		if (yerr > distance)
		{
			yerr -= distance;
			uCol += incy;
		}
	}
}
// ������
//(x1,y1),(x2,y2):���εĶԽ�����
void LCD_DrawRectangle(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2)
{
	LCD_DrawLine(x1, y1, x2, y1);
	LCD_DrawLine(x1, y1, x1, y2);
	LCD_DrawLine(x1, y2, x2, y2);
	LCD_DrawLine(x2, y1, x2, y2);
}
// ��ָ��λ�û�һ��ָ����С��Բ
//(x,y):���ĵ�
// r    :�뾶
void LCD_Draw_Circle(uint16_t x0, uint16_t y0, uint8_t r)
{
	int a, b;
	int di;
	a = 0;
	b = r;
	di = 3 - (r << 1); // �ж��¸���λ�õı�־
	while (a <= b)
	{
		LCD_DrawPoint(x0 + a, y0 - b); // 5
		LCD_DrawPoint(x0 + b, y0 - a); // 0
		LCD_DrawPoint(x0 + b, y0 + a); // 4
		LCD_DrawPoint(x0 + a, y0 + b); // 6
		LCD_DrawPoint(x0 - a, y0 + b); // 1
		LCD_DrawPoint(x0 - b, y0 + a);
		LCD_DrawPoint(x0 - a, y0 - b); // 2
		LCD_DrawPoint(x0 - b, y0 - a); // 7
		a++;
		// ʹ��Bresenham�㷨��Բ
		if (di < 0)
			di += 4 * a + 6;
		else
		{
			di += 10 + 4 * (a - b);
			b--;
		}
	}
}
// ��ָ��λ����ʾһ���ַ�
// x,y:��ʼ����
// num:Ҫ��ʾ���ַ�:" "--->"~"
// size:�����С 12/16/24
// mode:���ӷ�ʽ(1)���Ƿǵ��ӷ�ʽ(0)
void LCD_ShowChar(uint16_t x, uint16_t y, uint8_t num, uint8_t size, uint8_t mode)
{
	uint8_t temp, t1, t;
	uint16_t y0 = y;
	uint8_t csize = (size / 8 + ((size % 8) ? 1 : 0)) * (size / 2); // �õ�����һ���ַ���Ӧ������ռ���ֽ���
	num = num - ' ';																												// �õ�ƫ�ƺ��ֵ��ASCII�ֿ��Ǵӿո�ʼȡģ������-' '���Ƕ�Ӧ�ַ����ֿ⣩
	for (t = 0; t < csize; t++)
	{
		if (size == 12)
			temp = asc2_1206[num][t]; // ����1206����
		else if (size == 16)
			temp = asc2_1608[num][t]; // ����1608����
		else if (size == 24)
			temp = asc2_2412[num][t]; // ����2412����
		else
			return; // û�е��ֿ�
		for (t1 = 0; t1 < 8; t1++)
		{
			if (temp & 0x80)
				LCD_Fast_DrawPoint(x, y, POINT_COLOR);
			else if (mode == 0)
				LCD_Fast_DrawPoint(x, y, BACK_COLOR);
			temp <<= 1;
			y++;
			if (y >= lcddev.height)
				return; // ��������
			if ((y - y0) == size)
			{
				y = y0;
				x++;
				if (x >= lcddev.width)
					return; // ��������
				break;
			}
		}
	}
}
// m^n����
// ����ֵ:m^n�η�.
uint32_t LCD_Pow(uint8_t m, uint8_t n)
{
	uint32_t result = 1;
	while (n--)
		result *= m;
	return result;
}
// ��ʾ����,��λΪ0,����ʾ
// x,y :�������
// len :���ֵ�λ��
// size:�����С
// color:��ɫ
// num:��ֵ(0~4294967295);
void LCD_ShowNum(uint16_t x, uint16_t y, uint32_t num, uint8_t len, uint8_t size)
{
	uint8_t t, temp;
	uint8_t enshow = 0;
	for (t = 0; t < len; t++)
	{
		temp = (num / LCD_Pow(10, len - t - 1)) % 10;
		if (enshow == 0 && t < (len - 1))
		{
			if (temp == 0)
			{
				LCD_ShowChar(x + (size / 2) * t, y, ' ', size, 0);
				continue;
			}
			else
				enshow = 1;
		}
		LCD_ShowChar(x + (size / 2) * t, y, temp + '0', size, 0);
	}
}
// ��ʾ����,��λΪ0,������ʾ
// x,y:�������
// num:��ֵ(0~999999999);
// len:����(��Ҫ��ʾ��λ��)
// size:�����С
// mode:
//[7]:0,�����;1,���0.
//[6:1]:����
//[0]:0,�ǵ�����ʾ;1,������ʾ.
void LCD_ShowxNum(uint16_t x, uint16_t y, uint32_t num, uint8_t len, uint8_t size, uint8_t mode)
{
	uint8_t t, temp;
	uint8_t enshow = 0;
	for (t = 0; t < len; t++)
	{
		temp = (num / LCD_Pow(10, len - t - 1)) % 10;
		if (enshow == 0 && t < (len - 1))
		{
			if (temp == 0)
			{
				if (mode & 0X80)
					LCD_ShowChar(x + (size / 2) * t, y, '0', size, mode & 0X01);
				else
					LCD_ShowChar(x + (size / 2) * t, y, ' ', size, mode & 0X01);
				continue;
			}
			else
				enshow = 1;
		}
		LCD_ShowChar(x + (size / 2) * t, y, temp + '0', size, mode & 0X01);
	}
}
// ��ʾ�ַ���
// x,y:�������
// width,height:�����С
// size:�����С
//*p:�ַ�����ʼ��ַ
void LCD_ShowString(uint16_t x, uint16_t y, uint16_t width, uint16_t height, uint8_t size, uint8_t *p)
{
	uint8_t x0 = x;
	width += x;
	height += y;
	while ((*p <= '~') && (*p >= ' ')) // �ж��ǲ��ǷǷ��ַ�!
	{
		if (x >= width)
		{
			x = x0;
			y += size;
		}
		if (y >= height)
			break; // �˳�
		LCD_ShowChar(x, y, *p, size, 0);
		x += size / 2;
		p++;
	}
}
void LCD_DrawPoint_pic(uint16_t x, uint16_t y, uint16_t color)
{
	LCD_SetCursor(x, y);		// ���ù��λ��
	LCD_WriteRAM_Prepare(); // ��ʼд��GRAM
	LCD->LCD_RAM = color;
}

/****************************************************************************
* ��    �ƣ�void LCD_DrawPicture(uint16_t StartX,uint16_t StartY,uint16_t EndX,uint16_t EndY,uint16_t *pic)
* ��    �ܣ���ָ�����귶Χ��ʾһ��ͼƬ
* ��ڲ����� StartX     ����ʼ����
*           StartY     ����ʼ����
*           EndX       �н�������
*           EndY       �н�������
							pic        ͼƬͷָ��
* ���ڲ�������
* ˵    ����ͼƬȡģ��ʽΪˮƽɨ�裬16λ��ɫģʽ
* ���÷�����LCD_DrawPicture(0,0,100,100,(uint16_t*)demo);
****************************************************************************/
void LCD_DrawPicture(uint16_t StartX, uint16_t StartY, uint16_t Xend, uint16_t Yend, uint8_t *pic)
{
	static uint16_t i = 0, j = 0;
	uint16_t *bitmap = (uint16_t *)pic;
	for (j = 0; j < Yend - StartY; j++)
	{
		for (i = 0; i < Xend - StartX; i++)
			LCD_DrawPoint_pic(StartX + i, StartY + j, *bitmap++);
	}
}
