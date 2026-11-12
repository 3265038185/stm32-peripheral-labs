/*============================================================================
  文件：OLED.c（0.96 寸 OLED 显示屏驱动 - I2C 方式）
  ----------------------------------------------------------------------------
  硬件连接（软件模拟 I2C，不需要单片机硬件 I2C 外设）：
    SCL（时钟线）-> PB8
    SDA（数据线）-> PB9
  ----------------------------------------------------------------------------
  用到外设/知识点：
    1. GPIO 开漏输出（GPIO_Mode_Out_OD）：
       - 开漏输出的引脚只能"拉低"或"释放"（由外部上拉电阻拉高），
         这是 I2C 总线规范要求的——多个设备可以安全地"线与"；
       - 本模块靠 OLED_W_SCL / OLED_W_SDA 宏直接对引脚写高低电平，
         所以是"软件模拟 I2C"。
    2. I2C 协议（基础时序）：
       - 起始信号：SCL 高电平时，SDA 由高变低；
       - 停止信号：SCL 高电平时，SDA 由低变高；
       - 数据位：SCL 高电平期间 SDA 保持稳定（高=1，低=0），
         SCL 低电平时 SDA 才允许变化；每 8 个数据位后跟 1 个应答位。
    3. OLED 屏幕内部是一块 SSD1306 驱动芯片：
       - 地址 0x78（8 位写法，含读写位后实际 7 位地址是 0x3C）；
       - 分辨率 128x64，分成 8 页（Page），每页 8 行像素；
       - 控制字 0x00 表示后面跟"命令"，0x40 表示后面跟"数据"。
    4. 字模数组 OLED_F8x16（在 OLED_Font.h 里）：16x8 点阵 ASCII 字库，
       每个字符用 16 个字节表示，前 8 字节是上半部分，后 8 字节是下半部分。
  ----------------------------------------------------------------------------
  函数清单：
    OLED_I2C_Init       —— 初始化 SCL/SDA 引脚（开漏输出）
    OLED_I2C_Start/Stop —— I2C 起始/停止时序
    OLED_I2C_SendByte   —— 发送一个字节（MSB 先行）
    OLED_WriteCommand   —— 写命令
    OLED_WriteData      —— 写显示数据
    OLED_SetCursor      —— 设置光标位置（页 + 列）
    OLED_Clear          —— 清屏（全部写 0）
    OLED_ShowChar/ShowString/ShowNum/ShowSignedNum/ShowHexNum/ShowBinNum
                        —— 各种显示函数
    OLED_Pow            —— 自写的整数次方函数（显示数字用）
    OLED_Init           —— 按 SSD1306 手册发送初始化命令序列
  ============================================================================*/
#include "stm32f10x.h"
#include "OLED_Font.h"

/*引脚配置*/
/* 定义 SCL/SDA 引脚写电平的宏：
   GPIO_WriteBit(GPIOB, Pin, 电平) 是标准库函数，把指定引脚设为
   高电平（Bit_SET=1）或低电平（Bit_RESET=0）；
   (BitAction)(x) 把传入的 0/1 强转成标准库要求的枚举类型。 */
#define OLED_W_SCL(x)		GPIO_WriteBit(GPIOB, GPIO_Pin_8, (BitAction)(x))
#define OLED_W_SDA(x)		GPIO_WriteBit(GPIOB, GPIO_Pin_9, (BitAction)(x))

/*引脚初始化*/
/*
 * 函数名：OLED_I2C_Init（I2C 引脚初始化）
 * 参数：无
 * 返回：无
 * 功能：把 PB8（SCL）、PB9（SDA）配置成"开漏输出"。
 * 原理：I2C 总线上所有设备都是开漏接法，靠外部上拉电阻把总线
 *       拉高；某设备想发低电平时就拉低总线，双方不会"打架"，
 *       这叫"线与"。开漏输出在 STM32 里就是 GPIO_Mode_Out_OD。
 *       初始化后把两条线都置高电平，表示总线空闲。
 */
void OLED_I2C_Init(void)
{
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);	// 开启 GPIOB 时钟
	/* 注意：这里没有开 AFIO，因为软件模拟 I2C 只用普通 GPIO 功能，
	   不需要引脚复用/重映射，所以 AFIO 时钟可以不开。 */
	
	GPIO_InitTypeDef GPIO_InitStructure;
 	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_OD;	// 开漏输出（I2C 总线标准接法）
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;	// 输出速度 50MHz
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_8;			// SCL = PB8
 	GPIO_Init(GPIOB, &GPIO_InitStructure);
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_9;			// SDA = PB9
 	GPIO_Init(GPIOB, &GPIO_InitStructure);
	
	OLED_W_SCL(1);	// 初始状态：时钟线置高（总线空闲）
	OLED_W_SDA(1);	// 初始状态：数据线置高（总线空闲）
	//GPIO_SetBits(GPIOB,GPIO_Pin_8|GPIO_Pin_9);
}

/**
  * @brief  I2C开始
  * @param  无
  * @retval 无
  */
/*  原理：I2C 起始信号 = "SCL 为高电平时，SDA 由高变低"。
    这个电平跳变就是给从机（OLED）发的"我要开始通信了"的暗号。
    顺序：SDA 先拉高 -> SCL 拉高 -> SDA 拉低（此刻产生下降沿）-> SCL 拉低。  */
void OLED_I2C_Start(void)
{
	OLED_W_SDA(1);
	OLED_W_SCL(1);
	OLED_W_SDA(0);	// 关键动作：SCL 高电平时 SDA 产生下降沿 = 起始信号
	OLED_W_SCL(0);
}

/**
  * @brief  I2C停止
  * @param  无
  * @retval 无
  */
/*  原理：I2C 停止信号 = "SCL 为高电平时，SDA 由低变高"，
    告诉从机"通信结束"。  */
void OLED_I2C_Stop(void)
{
	OLED_W_SDA(0);
	OLED_W_SCL(1);
	OLED_W_SDA(1);	// 关键动作：SCL 高电平时 SDA 产生上升沿 = 停止信号
}

/**
  * @brief  I2C发送一个字节
  * @param  Byte 要发送的一个字节
  * @retval 无
  */
/*  原理：I2C 传输规则是"高位在前（MSB first）"：
    Byte & (0x80 >> i) 依次取出第 7 位、第 6 位……第 0 位，
    即先发最高位，每发一位 SCL 就产生一个时钟脉冲（拉高再拉低），
    从机在 SCL 高电平期间"采样"SDA 电平，把 8 位全部收齐。
    之后补一个时钟脉冲，用来接收从机的"应答位"（ACK），
    这里不读应答信号，所以只是象征性地打一个时钟。  */
void OLED_I2C_SendByte(uint8_t Byte)
{
	uint8_t i;
	for (i = 0; i < 8; i++)		// 一个字节 = 8 个数据位
	{
		OLED_W_SDA(Byte & (0x80 >> i));	// 逐位输出（高位在前）
		OLED_W_SCL(1);					// 拉高时钟：从机采样数据
		OLED_W_SCL(0);					// 拉低时钟：准备下一位
	}
	OLED_W_SCL(1);	//额外的一个时钟，不处理应答信号
	OLED_W_SCL(0);
}

/**
  * @brief  OLED写命令
  * @param  Command 要写入的命令
  * @retval 无
  */
/*  原理：先发起始信号，然后发 3 个字节：
    1. 0x78：从机地址（7 位地址 0x3C 左移 1 位 + 写位 0 = 0x78），
       告诉总线上所有从机"我要找地址 0x3C 的设备"；
    2. 0x00：控制字节，表示"接下来的字节是命令"；
    3. Command：真正的命令内容（如 0xAF 开显示）。
    最后发停止信号。  */
void OLED_WriteCommand(uint8_t Command)
{
	OLED_I2C_Start();
	OLED_I2C_SendByte(0x78);		//从机地址
	OLED_I2C_SendByte(0x00);		//写命令
	OLED_I2C_SendByte(Command); 
	OLED_I2C_Stop();
}

/**
  * @brief  OLED写数据
  * @param  Data 要写入的数据
  * @retval 无
  */
/*  与写命令唯一的区别：控制字节是 0x40（表示"接下来的字节是显示数据"），
    显示数据直接决定屏幕哪些像素点亮。  */
void OLED_WriteData(uint8_t Data)
{
	OLED_I2C_Start();
	OLED_I2C_SendByte(0x78);		//从机地址
	OLED_I2C_SendByte(0x40);		//写数据
	OLED_I2C_SendByte(Data);
	OLED_I2C_Stop();
}

/**
  * @brief  OLED设置光标位置
  * @param  Y 以左上角为原点，向下方向的坐标，范围：0~7
  * @param  X 以左上角为原点，向右方向的坐标，范围：0~127
  * @retval 无
  */
/*  原理（SSD1306 命令）：
    - 0xB0 | Y：页地址命令（0xB0~0xB7 对应第 0~7 页）。
      屏幕 128x64 被分成 8 页，每页是 8 个像素高的一条"横带"，
      所以 Y 只能取 0~7；
    - X 坐标 0~127 需要 7 位二进制表示，但命令只能传 8 位，
      所以拆成两次发：高 4 位（0x10 | ...）和低 4 位（0x00 | ...）。
    光标就位后，后面写的数据会从 (X, Y) 位置开始逐列填充。  */
void OLED_SetCursor(uint8_t Y, uint8_t X)
{
	OLED_WriteCommand(0xB0 | Y);					//设置Y位置
	OLED_WriteCommand(0x10 | ((X & 0xF0) >> 4));	//设置X位置高4位
	OLED_WriteCommand(0x00 | (X & 0x0F));			//设置X位置低4位
}

/**
  * @brief  OLED清屏
  * @param  无
  * @retval 无
  */
/*  原理：把 8 页 x 每页 128 列 = 1024 个"列数据"全部写成 0x00，
    0x00 表示这一列 8 个像素全部熄灭，屏幕就全黑了。  */
void OLED_Clear(void)
{  
	uint8_t i, j;
	for (j = 0; j < 8; j++)		// 遍历 8 页
	{
		OLED_SetCursor(j, 0);	// 光标移到第 j 页、第 0 列
		for(i = 0; i < 128; i++)	// 每页 128 列
		{
			OLED_WriteData(0x00);	// 全写 0（熄灭）
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
/*  原理：字符采用 8x16 点阵（8 列宽 x 16 行高），
    一个字符要占"两个页"（每页 8 行像素）：
    - 上半部分：OLED_F8x16[Char-' '][0~7]  写在第 2*(Line-1) 页；
    - 下半部分：OLED_F8x16[Char-' '][8~15] 写在第 2*(Line-1)+1 页。
    为什么下标是 Char - ' '？
      因为字模数组从空格（ASCII 码 32）开始存，字符 'A'(65) 对应数组
      下标 65-32=33，这样数组下标 0 就对应空格，节省存储。
    屏幕一共 64 行，每 8 行一页共 8 页，能放 4 行 16 点阵字符，
    所以 Line 范围 1~4；每列 8 像素，128/8=16 列，所以 Column 范围 1~16。  */
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
/*  原理：C 语言字符串以 '\0' 结尾，循环条件 String[i] != '\0'
    表示"一直显示到字符串结束"，每个字符依次往右排（Column+i）。  */
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
/*  原理：从 1 开始连乘 Y 次 X，就是 X 的 Y 次方。
    例：OLED_Pow(10, 3) = 1*10*10*10 = 1000。
    它是显示数字的"辅助函数"，下面拆位时要用到。  */
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
/*  原理（逐位拆分，从最高位开始）：
    Number / OLED_Pow(10, Length-i-1) 先整除去掉低位，得到"高 i 位"的数，
    再 % 10 取个位，就得到第 i 位数字；+'0' 把数字 0~9 转成字符 '0'~'9'。
    例：Number=123，Length=3：
      第 0 位：123/100%10 = 1，显示 '1'；
      第 1 位：123/10%10  = 2，显示 '2'；
      第 2 位：123/1%10   = 3，显示 '3'。
    （Length 指定固定位数，位数不够前面自动补 0。）  */
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
/*  原理：正数先显示 '+'，负数先显示 '-' 并把数值取绝对值，
    然后和 OLED_ShowNum 一样逐位拆分。
    注意：数字部分从 Column+1 开始显示，因为第 1 列被符号占了。  */
void OLED_ShowSignedNum(uint8_t Line, uint8_t Column, int32_t Number, uint8_t Length)
{
	uint8_t i;
	uint32_t Number1;
	if (Number >= 0)		// 非负数：显示正号
	{
		OLED_ShowChar(Line, Column, '+');
		Number1 = Number;
	}
	else					// 负数：显示负号，并取绝对值方便拆分
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
/*  原理：和十进制一样逐位拆分，只是基数换成 16。
    第 i 位 = Number / 16^(Length-i-1) % 16；
    结果 0~9 转成 '0'~'9'，结果 10~15 转成 'A'~'F'（十六进制字母）。  */
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
/*  原理：基数换成 2，第 i 位 = Number / 2^(Length-i-1) % 2，结果只有 0/1。  */
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
/*  原理：SSD1306 上电后必须按固定顺序发送一串"初始化命令"，
    屏幕才能正常工作。这些命令的含义（可对照 SSD1306 数据手册）：
    0xAE   关闭显示（初始化期间屏幕先黑着，防止花屏）；
    0xD5/0x80 设置显示时钟分频比（决定扫描频率）；
    0xA8/0x3F 设置多路复用率 = 64 行（正好对应 64 像素高）；
    0xD3/0x00 设置显示偏移量 = 0；
    0x40   设置显示起始行为第 0 行；
    0xA1   设置列扫描方向正常（0xA0 是左右镜像）；
    0xC8   设置行扫描方向正常（0xC0 是上下镜像）；
    0xDA/0x12 设置 COM 引脚硬件配置（每页 8 行像素的来源）；
    0x81/0xCF 设置对比度（亮度）为 0xCF；
    0xD9/0xF1 设置预充电周期；
    0xDB/0x30 设置 VCOMH 电平（影响对比度）；
    0xA4   显示内容跟随 RAM（0xA5 是全屏点亮，测试用）；
    0xA6   正常显示（0xA7 是反色显示）；
    0x8D/0x14 开启内部充电泵（升压电路，OLED 需要 7~15V 驱动，
              没有它屏幕根本不亮）；
    0xAF   打开显示。
    最后清屏，把所有像素熄灭，屏幕进入待命状态。
    注意：这些命令是"标准答案"，照抄即可，不必死记。  */
void OLED_Init(void)
{
	uint32_t i, j;
	
	for (i = 0; i < 1000; i++)			//上电延时
	{
		for (j = 0; j < 1000; j++);
	}
	/*  上面双层空循环 = 软件延时：等 OLED 内部电源稳定后再发命令，
	    防止上电瞬间通信失败。  */
	
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
