/*==============================================================================
 * 文件：OLED.c（OLED显示屏驱动模块）
 * 功能：驱动0.96寸OLED屏（SSD1306主控芯片，I2C接口），实现显示
 *       字符、字符串、数字（十进制/十六进制/二进制/带符号）等功能。
 * 接线：SCL→PB8（时钟线），SDA→PB9（数据线），都是"软件模拟I2C"。
 * I2C原理：I2C是两线制通信协议：
 *   1. 开始信号：SCL高电平时，SDA产生一个高→低的跳变；
 *   2. 停止信号：SCL高电平时，SDA产生一个低→高的跳变；
 *   3. 传输数据：SCL低电平期间SDA才能变化，SCL高电平期间SDA必须保持稳定，
 *      CPU每发完8位数据后，从机要拉低SDA表示"应答"（本驱动不处理应答）。
 * 通信流程（写一个字节）：
 *   起始位 → 发送从机地址0x78（OLED的I2C地址，含写位）→
 *   发送控制字节（0x00=命令 / 0x40=数据）→ 发送内容 → 停止位
 * 屏幕结构：128×64像素，按"页"组织：8页（每页8行像素），每页128列。
 * 知识点：软件模拟I2C、OLED写命令/写数据、字模取模、光标设置
 *============================================================================*/
#include "stm32f10x.h"		// STM32头文件
#include "OLED_Font.h"		// 字模数据头文件（8×16点阵的ASCII字模存放在这里）

/*引脚配置*/
/* 宏定义：把"写SCL/SDA引脚电平"简化成一个函数式宏。
   GPIO_WriteBit(GPIOB, GPIO_Pin_8, (BitAction)(x))：
   把x作为BitAction类型写到PB8引脚。
   这样 OLED_W_SCL(1) 就是"PB8输出高电平"，OLED_W_SCL(0) 就是"PB8输出低电平" */
#define OLED_W_SCL(x)		GPIO_WriteBit(GPIOB, GPIO_Pin_8, (BitAction)(x))
#define OLED_W_SDA(x)		GPIO_WriteBit(GPIOB, GPIO_Pin_9, (BitAction)(x))

/*引脚初始化*/
/* I2C引脚初始化：把PB8、PB9配置为"开漏输出"(Out_OD)。
   为什么用开漏输出？因为I2C总线上"低电平由引脚拉低、高电平由上拉电阻拉高"，
   开漏输出的引脚本身只能输出低电平，高电平靠外部上拉电阻实现，
   这样多个设备可以"线与"连接在同一根总线上，符合I2C规范。 */
void OLED_I2C_Init(void)
{
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);	// 开启GPIOB时钟（SCL/SDA在PB8/PB9）
	
	GPIO_InitTypeDef GPIO_InitStructure;	// 定义GPIO配置结构体
 	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_OD;	// 开漏输出模式（I2C必须用开漏）
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;	// 速度50MHz
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_8;	// 先配置PB8（SCL）
 	GPIO_Init(GPIOB, &GPIO_InitStructure);
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_9;	// 再配置PB9（SDA）
 	GPIO_Init(GPIOB, &GPIO_InitStructure);
	
	OLED_W_SCL(1);	// 初始化后把SCL置高（I2C空闲状态：SCL和SDA都应该为高）
	OLED_W_SDA(1);	// 把SDA置高
	//GPIO_SetBits(GPIOB,GPIO_Pin_8|GPIO_Pin_9);
}

/**
  * @brief  I2C开始
  * @param  无
  * @retval 无
  */
void OLED_I2C_Start(void)
{
	/* I2C起始信号：在SCL保持高电平期间，让SDA产生一个"高→低"的跳变。
	   时序顺序很重要，一步一步来：
	   1. SDA先拉高（保证起始前SDA是高电平）；
	   2. SCL拉高（SCL高电平期间SDA的变化才被识别为起始/停止）；
	   3. SDA拉低（高→低跳变 = 起始信号！）；
	   4. SCL拉低（准备开始传输数据，因为数据要在SCL低电平期间变化） */
	OLED_W_SDA(1);
	OLED_W_SCL(1);
	OLED_W_SDA(0);
	OLED_W_SCL(0);
}

/**
  * @brief  I2C停止
  * @param  无
  * @retval 无
  */
void OLED_I2C_Stop(void)
{
	/* I2C停止信号：在SCL保持高电平期间，让SDA产生一个"低→高"的跳变。
	   和起始信号刚好相反：
	   1. SDA先拉低；
	   2. SCL拉高；
	   3. SDA拉高（低→高跳变 = 停止信号！） */
	OLED_W_SDA(0);
	OLED_W_SCL(1);
	OLED_W_SDA(1);
}

/**
  * @brief  I2C发送一个字节
  * @param  Byte 要发送的一个字节
  * @retval 无
  */
void OLED_I2C_SendByte(uint8_t Byte)
{
	uint8_t i;
	/* 高位先出，逐位发送8个数据位。
	   0x80 >> i：i=0时是0x80（最高位），i=7时是0x01（最低位），
	   用位与(Byte & ...)把Byte的对应位取出来，写SDA。
	   每一位的时序：SDA先就位（SCL低电平期间变化）→ SCL拉高
	   （SCL高电平时从机读取该位）→ SCL拉低（准备下一位） */
	for (i = 0; i < 8; i++)
	{
		OLED_W_SDA(Byte & (0x80 >> i));
		OLED_W_SCL(1);
		OLED_W_SCL(0);
	}
	OLED_W_SCL(1);	//额外的一个时钟，不处理应答信号
	OLED_W_SCL(0);	// 8位数据发完后，第9个时钟是"应答位"，
					// 正常应该由从机拉低SDA表示收到了。
					// 本驱动简单起见不读应答，直接给个时钟跳过
}

/**
  * @brief  OLED写命令
  * @param  Command 要写入的命令
  * @retval 无
  */
void OLED_WriteCommand(uint8_t Command)
{
	/* 写命令：告诉OLED"接下来我要发的是控制命令"
	   数据帧格式：起始位 + 地址0x78 + 控制字节0x00(命令) + 命令内容 + 停止位 */
	OLED_I2C_Start();		// 起始信号
	OLED_I2C_SendByte(0x78);		//从机地址
									// OLED的I2C从机地址是0x78（7位地址0x3C左移一位+写位0）
	OLED_I2C_SendByte(0x00);		//写命令
									// 控制字节：0x00表示"后面跟的是命令"
	OLED_I2C_SendByte(Command); 	// 发送真正的命令内容
	OLED_I2C_Stop();		// 停止信号
}

/**
  * @brief  OLED写数据
  * @param  Data 要写入的数据
  * @retval 无
  */
void OLED_WriteData(uint8_t Data)
{
	/* 写数据：和写命令几乎一样，只是控制字节改成0x40(数据) */
	OLED_I2C_Start();		// 起始信号
	OLED_I2C_SendByte(0x78);		//从机地址（同上）
	OLED_I2C_SendByte(0x40);		//写数据
									// 控制字节：0x40表示"后面跟的是显示数据"
	OLED_I2C_SendByte(Data);		// 发送数据内容
	OLED_I2C_Stop();		// 停止信号
}

/**
  * @brief  OLED设置光标位置
  * @param  Y 以左上角为原点，向下方向的坐标，范围：0~7
  * @param  X 以左上角为原点，向右方向的坐标，范围：0~127
  * @retval 无
  */
void OLED_SetCursor(uint8_t Y, uint8_t X)
{
	/* 设置光标：告诉OLED接下来数据写到哪个位置。
	   OLED的寻址方式：按"页(行)"和"列"定位。
	   命令格式：
	   ① 0xB0|Y：选择第Y页（0~7页，一页=8行像素）；
	   ② 0x10|(X>>4)：列地址的高4位；
	   ③ 0x00|(X&0x0F)：列地址的低4位。
	   列地址X范围0~127，分两半拼起来：高4位+低4位 */
	OLED_WriteCommand(0xB0 | Y);					//设置Y位置
	OLED_WriteCommand(0x10 | ((X & 0xF0) >> 4));	//设置X位置高4位
	OLED_WriteCommand(0x00 | (X & 0x0F));			//设置X位置低4位
}

/**
  * @brief  OLED清屏
  * @param  无
  * @retval 无
  */
void OLED_Clear(void)
{  
	uint8_t i, j;
	for (j = 0; j < 8; j++)
	{
		OLED_SetCursor(j, 0);
		for(i = 0; i < 128; i++)
		{
			OLED_WriteData(0x00);
		}
	}
}

/**
  * @brief  OLED显示一个字符
  * @param  Line 行位置，范围：1~4
  * @param  Column 列位置，范围：1~16
  * @param  Char 要显示的一个字符，范围：ASCII可见字符
  * @retval 无
  */
void OLED_ShowChar(uint8_t Line, uint8_t Column, char Char)
{      	
	uint8_t i;
	OLED_SetCursor((Line - 1) * 2, (Column - 1) * 8);		//设置光标位置在上半部分
	for (i = 0; i < 8; i++)
	{
		OLED_WriteData(OLED_F8x16[Char - ' '][i]);			//显示上半部分内容
	}
	OLED_SetCursor((Line - 1) * 2 + 1, (Column - 1) * 8);	//设置光标位置在下半部分
	for (i = 0; i < 8; i++)
	{
		OLED_WriteData(OLED_F8x16[Char - ' '][i + 8]);		//显示下半部分内容
	}
}

/**
  * @brief  OLED显示字符串
  * @param  Line 起始行位置，范围：1~4
  * @param  Column 起始列位置，范围：1~16
  * @param  String 要显示的字符串，范围：ASCII可见字符
  * @retval 无
  */
void OLED_ShowString(uint8_t Line, uint8_t Column, char *String)
{
	uint8_t i;
	for (i = 0; String[i] != '\0'; i++)
	{
		OLED_ShowChar(Line, Column + i, String[i]);
	}
}

/**
  * @brief  OLED次方函数
  * @retval 返回值等于X的Y次方
  */
uint32_t OLED_Pow(uint32_t X, uint32_t Y)
{
	uint32_t Result = 1;
	while (Y--)
	{
		Result *= X;
	}
	return Result;
}

/**
  * @brief  OLED显示数字（十进制，正数）
  * @param  Line 起始行位置，范围：1~4
  * @param  Column 起始列位置，范围：1~16
  * @param  Number 要显示的数字，范围：0~4294967295
  * @param  Length 要显示数字的长度，范围：1~10
  * @retval 无
  */
void OLED_ShowNum(uint8_t Line, uint8_t Column, uint32_t Number, uint8_t Length)
{
	uint8_t i;
	for (i = 0; i < Length; i++)							
	{
		OLED_ShowChar(Line, Column + i, Number / OLED_Pow(10, Length - i - 1) % 10 + '0');
	}
}

/**
  * @brief  OLED显示数字（十进制，带符号数）
  * @param  Line 起始行位置，范围：1~4
  * @param  Column 起始列位置，范围：1~16
  * @param  Number 要显示的数字，范围：-2147483648~2147483647
  * @param  Length 要显示数字的长度，范围：1~10
  * @retval 无
  */
void OLED_ShowSignedNum(uint8_t Line, uint8_t Column, int32_t Number, uint8_t Length)
{
	uint8_t i;
	uint32_t Number1;
	if (Number >= 0)
	{
		OLED_ShowChar(Line, Column, '+');
		Number1 = Number;
	}
	else
	{
		OLED_ShowChar(Line, Column, '-');
		Number1 = -Number;
	}
	for (i = 0; i < Length; i++)							
	{
		OLED_ShowChar(Line, Column + i + 1, Number1 / OLED_Pow(10, Length - i - 1) % 10 + '0');
	}
}

/**
  * @brief  OLED显示数字（十六进制，正数）
  * @param  Line 起始行位置，范围：1~4
  * @param  Column 起始列位置，范围：1~16
  * @param  Number 要显示的数字，范围：0~0xFFFFFFFF
  * @param  Length 要显示数字的长度，范围：1~8
  * @retval 无
  */
void OLED_ShowHexNum(uint8_t Line, uint8_t Column, uint32_t Number, uint8_t Length)
{
	uint8_t i, SingleNumber;
	for (i = 0; i < Length; i++)							
	{
		SingleNumber = Number / OLED_Pow(16, Length - i - 1) % 16;
		if (SingleNumber < 10)
		{
			OLED_ShowChar(Line, Column + i, SingleNumber + '0');
		}
		else
		{
			OLED_ShowChar(Line, Column + i, SingleNumber - 10 + 'A');
		}
	}
}

/**
  * @brief  OLED显示数字（二进制，正数）
  * @param  Line 起始行位置，范围：1~4
  * @param  Column 起始列位置，范围：1~16
  * @param  Number 要显示的数字，范围：0~1111 1111 1111 1111
  * @param  Length 要显示数字的长度，范围：1~16
  * @retval 无
  */
void OLED_ShowBinNum(uint8_t Line, uint8_t Column, uint32_t Number, uint8_t Length)
{
	uint8_t i;
	for (i = 0; i < Length; i++)							
	{
		OLED_ShowChar(Line, Column + i, Number / OLED_Pow(2, Length - i - 1) % 2 + '0');
	}
}

/**
  * @brief  OLED初始化
  * @param  无
  * @retval 无
  */
void OLED_Init(void)
{
	uint32_t i, j;
	
	for (i = 0; i < 1000; i++)			//上电延时
	{
		for (j = 0; j < 1000; j++);
	}
	
	OLED_I2C_Init();			//端口初始化
	
	OLED_WriteCommand(0xAE);	//关闭显示
	
	OLED_WriteCommand(0xD5);	//设置显示时钟分频比/振荡器频率
	OLED_WriteCommand(0x80);
	
	OLED_WriteCommand(0xA8);	//设置多路复用率
	OLED_WriteCommand(0x3F);
	
	OLED_WriteCommand(0xD3);	//设置显示偏移
	OLED_WriteCommand(0x00);
	
	OLED_WriteCommand(0x40);	//设置显示开始行
	
	OLED_WriteCommand(0xA1);	//设置左右方向，0xA1正常 0xA0左右反置
	
	OLED_WriteCommand(0xC8);	//设置上下方向，0xC8正常 0xC0上下反置

	OLED_WriteCommand(0xDA);	//设置COM引脚硬件配置
	OLED_WriteCommand(0x12);
	
	OLED_WriteCommand(0x81);	//设置对比度控制
	OLED_WriteCommand(0xCF);

	OLED_WriteCommand(0xD9);	//设置预充电周期
	OLED_WriteCommand(0xF1);

	OLED_WriteCommand(0xDB);	//设置VCOMH取消选择级别
	OLED_WriteCommand(0x30);

	OLED_WriteCommand(0xA4);	//设置整个显示打开/关闭

	OLED_WriteCommand(0xA6);	//设置正常/倒转显示

	OLED_WriteCommand(0x8D);	//设置充电泵
	OLED_WriteCommand(0x14);

	OLED_WriteCommand(0xAF);	//开启显示
		
	OLED_Clear();				//OLED清屏
}
