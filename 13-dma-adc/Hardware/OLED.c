/*=============================================================================
 * 文件：OLED.c（OLED 显示屏驱动文件，江协科技风格，软件 I2C 模拟时序）
 * 功能：驱动 0.96 寸 SSD1306 控制的 OLED 屏幕（I2C 接口），
 *       实现显示字符、字符串、数字（十进制/十六进制/二进制）和画点画波形。
 *
 * 用到的外设/知识点：
 *   1. 软件 I2C：不用芯片自带的 I2C 外设，而是用两个 GPIO 引脚
 *      （PB8=SCL 时钟线，PB9=SDA 数据线）按 I2C 协议"手动"翻转电平，
 *      发送时序图上的 0/1，好处是任意引脚都能用，代码直观易懂
 *   2. I2C 协议要点：起始条件、停止条件、数据按位发送（高位在前）、
 *      从机地址应答；本屏幕的 7 位地址是 0x78（含写位后的字节）
 *   3. 屏幕结构：128 列 x 64 行像素，分为 8 页（Page0~Page7），
 *      每页 8 行像素、128 列；往某页某列写 1 个字节，就点亮该列对应的 8 个点
 *   4. 写命令（0x00 控制字节）与写数据（0x40 控制字节）区分
 *   5. 字符显示原理：用字模数组 OLED_F8x16（8x16 像素 ASCII 点阵），
 *      每个字符拆成 16 个字节，上半 8 字节 + 下半 8 字节分两页写入
 *
 * 注：OLED_Font.h 为字模数据文件，不属于本文件，不要修改
 *=============================================================================*/
#include "stm32f10x.h"					//Device header（包含 STM32 标准库函数声明）
#include "OLED_Font.h"					//字模数据：OLED_F8x16 等点阵数组（只读数据，勿改）

/*引脚配置*/
/* 宏定义：把"给引脚写电平"的操作简化成 OLED_W_SCL(1)/OLED_W_SDA(0) 这样的写法
 * GPIO_WriteBit 是标准库函数，作用是把某个引脚的电平写为 1（高）或 0（低）
 * (BitAction)(x) 是强制类型转换：把传入的 1/0 转成标准库要求的枚举类型 */
#define OLED_W_SCL(x)		GPIO_WriteBit(GPIOB, GPIO_Pin_8, (BitAction)(x))	//SCL 时钟线：PB8
#define OLED_W_SDA(x)		GPIO_WriteBit(GPIOB, GPIO_Pin_9, (BitAction)(x))	//SDA 数据线：PB9

/*引脚初始化*/
/* 说明：把 PB8、PB9 配置为开漏输出（Out_OD），初始化时都置高电平。
 * 开漏输出 = 内部管子只能把引脚拉低，拉高靠外部上拉电阻，
 * 这是 I2C 总线的标准接法（线与逻辑），必须用开漏模式 */
void OLED_I2C_Init(void)
{
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);	//开启GPIOB的时钟（PB8/PB9 属于 GPIOB）
	
	GPIO_InitTypeDef GPIO_InitStructure;
 	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_OD;	//模式：开漏输出（I2C 标准接法）
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;	//速度：50MHz
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_8;			//先配置 PB8（SCL）
 	GPIO_Init(GPIOB, &GPIO_InitStructure);
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_9;			//再配置 PB9（SDA）
 	GPIO_Init(GPIOB, &GPIO_InitStructure);
	
	OLED_W_SCL(1);	//SCL 初始化为高电平（I2C 空闲时总线应为高）
	OLED_W_SDA(1);	//SDA 初始化为高电平（I2C 空闲时总线应为高）
}

/**
  * @brief  I2C开始
  * @param  无
  * @retval 无
  */
/* I2C 起始条件：SCL 保持高电平期间，SDA 从高变低。
 * 从机（OLED 屏幕）看到这个信号就知道"主机要开始通信了" */
void OLED_I2C_Start(void)
{
	OLED_W_SDA(1);	//先把 SDA 拉高
	OLED_W_SCL(1);	//再把 SCL 拉高
	OLED_W_SDA(0);	//SCL 为高时 SDA 拉低 = 产生起始条件
	OLED_W_SCL(0);	//拉低 SCL，准备发送数据（数据在 SCL 低电平期间才能变化）
}

/**
  * @brief  I2C停止
  * @param  无
  * @retval 无
  */
/* I2C 停止条件：SCL 保持高电平期间，SDA 从低变高。
 * 从机看到这个信号就知道"通信结束了" */
void OLED_I2C_Stop(void)
{
	OLED_W_SDA(0);	//先把 SDA 拉低
	OLED_W_SCL(1);	//再把 SCL 拉高
	OLED_W_SDA(1);	//SCL 为高时 SDA 拉高 = 产生停止条件
}

/**
  * @brief  I2C发送一个字节
  * @param  Byte 要发送的一个字节
  * @retval 无
  */
/* 原理：一个字节 8 位，从最高位（bit7）开始逐位发送：
   * 先把 SDA 设为当前位（数据必须在 SCL 低电平时变化），
   * 再拉高 SCL 让从机"采样"这一位，最后拉低 SCL 准备下一位。
 * 循环 8 次就把 8 个 bit 全部送出去了。
 * 注意：正常 I2C 每字节后要等从机发"应答位"，这里偷懒不处理，
 * 额外给一个时钟跳过去，OLED 屏幕也认这种写法 */
void OLED_I2C_SendByte(uint8_t Byte)
{
	uint8_t i;
	for (i = 0; i < 8; i++)		//循环 8 次，依次发送 Byte 的 bit7~bit0
	{
		/* 0x80 >> i：把 1 逐位右移，取 Byte 的对应位：
		 * i=0 取 bit7，i=1 取 bit6……i=7 取 bit0。
		 * Byte & (0x80>>i) 的结果只有 0 或 0x80，非 0 即视为 1，写入 SDA */
		OLED_W_SDA(Byte & (0x80 >> i));		//SDA 写入当前位（SCL 为低，可以安全变化）
		OLED_W_SCL(1);						//SCL 拉高：从机在此时采样读走这一位
		OLED_W_SCL(0);						//SCL 拉低：结束这一位，准备下一位
	}
	OLED_W_SCL(1);	//额外的一个时钟，不处理应答信号
	OLED_W_SCL(0);	//应答位跳过后，整个字节发送完毕
}

/**
  * @brief  OLED写命令
  * @param  Command 要写入的命令
  * @retval 无
  */
/* 原理：I2C 发送一帧 = 起始 + 从机地址 + 控制字节 + 数据 + 停止。
 * 0x78 = OLED 的 I2C 从机地址（7 位地址 0x3C 左移 1 位 + 写标志位 0 = 0x78）
 * 0x00 = 控制字节，告诉屏幕"接下来发的是命令" */
void OLED_WriteCommand(uint8_t Command)
{
	OLED_I2C_Start();						//起始条件
	OLED_I2C_SendByte(0x78);		//从机地址（0x78 = OLED 的地址）
	OLED_I2C_SendByte(0x00);		//写命令：控制字节 0x00 表示后续是命令
	OLED_I2C_SendByte(Command); 	//发送命令本身（如 0xAF 开显示）
	OLED_I2C_Stop();						//停止条件
}

/**
  * @brief  OLED写数据
  * @param  Data 要写入的数据
  * @retval 无
  */
/* 与写命令几乎一样，区别在控制字节：0x40 表示"接下来发的是显示数据"，
 * 数据会写到屏幕显存中对应的当前位置，屏幕自动刷新出来 */
void OLED_WriteData(uint8_t Data)
{
	OLED_I2C_Start();
	OLED_I2C_SendByte(0x78);		//从机地址
	OLED_I2C_SendByte(0x40);		//写数据：控制字节 0x40 表示后续是数据
	OLED_I2C_SendByte(Data);		//发送数据本身
	OLED_I2C_Stop();
}

/**
  * @brief  OLED设置光标位置
  * @param  Y 以左上角为原点，向下方向的坐标，范围：0~7（第几页）
  * @param  X 以左上角为原点，向右方向的坐标，范围：0~127（第几列）
  * @retval 无
  */
/* 原理：SSD1306 通过三条命令设置写入位置（页地址 + 列地址）：
 * 0xB0|Y   ：页地址命令，Y=0~7，对应 8 页
 * 0x10|X高4位：列地址的高 4 位（屏幕列地址 0~127 需要 7 位二进制，
 *             拆成高 4 位 + 低 4 位两次发送）
 * 0x00|X低4位：列地址的低 4 位
 * 设置好位置后，再写数据就会从该位置开始自动填充 */
void OLED_SetCursor(uint8_t Y, uint8_t X)
{
	OLED_WriteCommand(0xB0 | Y);					//设置Y位置（页地址：0xB0 + 页号）
	OLED_WriteCommand(0x10 | ((X & 0xF0) >> 4));	//设置X位置高4位（X 的高 4 位右移到低 4 位再拼接）
	OLED_WriteCommand(0x00 | (X & 0x0F));			//设置X位置低4位（X 的低 4 位）
}

/**
  * @brief  OLED清屏
  * @param  无
  * @retval 无
  */
/* 原理：把 8 页 x 128 列 = 1024 个字节全部写成 0x00（全灭），屏幕就清空了。
 * 外层循环选页（0~7），内层循环选列（0~127），每列写一个 0x00 */
void OLED_Clear(void)
{  
	uint8_t i, j;
	for (j = 0; j < 8; j++)		//外层：遍历 8 页
	{
		OLED_SetCursor(j, 0);	//先把光标移到第 j 页的第 0 列
		for(i = 0; i < 128; i++)	//内层：遍历一页中的 128 列
		{
			OLED_WriteData(0x00);	//写 0x00：该列 8 个像素全部熄灭
		}
	}
}

/* 设置起始点坐标（画点专用，和 OLED_SetCursor 功能类似但参数顺序相反）：
 * x = 页地址（0~7），y = 列地址（0~127）
 * 0xB0+x         ：页地址命令（注意 x 在这里当页号用）
 * (y&0x0f)|0x00  ：列地址低 4 位（LOW）
 * ((y&0xf0)>>4)|0x10：列地址高 4 位（HIGHT） */
void OLED_SetPos(unsigned char x, unsigned char y) //设置起始点坐标
{ 
	OLED_WriteCommand(0xb0+x);
	OLED_WriteCommand((y&0x0f)|0x00);//LOW
	OLED_WriteCommand(((y&0xf0)>>4)|0x10);//HIGHT
}
/* 画点函数：在屏幕 (x, y) 处点亮一个像素
 * 参数说明：x = 横坐标（0~127，对应屏幕从左到右的列），
 *           y = 纵坐标（0~63，对应屏幕从上到下的行）
 * 原理（初学者重点）：
 *   1. 屏幕 64 行像素被分成 8 页，每页 8 行（Page0 在最上面，Page7 在最下面）
 *   2. 写一个字节到某页某列，该字节的 8 个 bit 就对应这一页的 8 行
 *      （字节的 bit7 对应页内最上面一行，bit0 对应最下面一行）
 *   3. 那么要画点 (x,y)：
 *      - 它在第几页？   y/8 算出页号（y=0~7 在第 0 页……y=56~63 在第 7 页）
 *        但屏幕页号是 0 在上、7 在下，而 y 越大越靠下，所以取反：
 *        page = 7 - y/8
 *      - 它在页内第几行？ y%8（页内 0~7 行），同样需要上下翻转：move = 7 - y%8
 *      - 字节数据 = 0x01 左移 move 位，让唯一的 1 落在目标行
 *   4. 注意：OLED_SetPos 的第一个参数在这里传的是"页号"，第二个传的是"列"，
 *      所以是 OLED_SetPos(page, x) */
void OLED_DrawPoint(uint8_t x,uint8_t y)/*这里x是横坐标，y是纵坐标，在（x,y）处画一个点*/
{
	if(y>63)
		y=63;		//保护：y 最大 63，超出则钳位到 63（防止数组越界、防止写错页）
	
	uint8_t page,move,data;
	
	page=7-y/8;			//数据写在第几页（上下翻转：y 小=页号大=屏幕上方）
	move=7-y%8;			//0x01要移动的位数（页内行号，同样上下翻转）
	data=0x01<<move;	//要写什么数据（唯一一个 1 就代表点亮的那一行）
	
	OLED_SetPos(page,x);	//光标移到该点所在页和列
	OLED_WriteData(data);	//写入数据：该点对应的 bit 为 1（点亮），其余 bit 为 0
	/* 注意：这样直接写会"覆盖"该列原来的数据（其他行被清零），
	 * 本项目中波形每帧都先清屏再画，所以不会出问题 */
}

/**
  * @brief  OLED显示一个字符
  * @param  Line 行位置，范围：1~4（1 行 = 2 页，共 8 页）
  * @param  Column 列位置，范围：1~16（1 列 = 8 个像素宽）
  * @param  Char 要显示的一个字符，范围：ASCII可见字符
  * @retval 无
  */
/* 原理：字符是 8x16 点阵，纵向占 2 页。
 * OLED_F8x16[Char-' '] 是字符对应的字模数组（下标从空格开始排）：
 *   前 8 个字节 = 字符上半部分（8 行），写进上面一页
 *   后 8 个字节 = 字符下半部分（8 行），写进下面一页
 * 每页 8 个字节 = 8 列像素，正好组成一个 8x16 的字符 */
void OLED_ShowChar(uint8_t Line, uint8_t Column, char Char)
{      	
	uint8_t i;
	OLED_SetCursor((Line - 1) * 2, (Column - 1) * 8);		//设置光标位置在上半部分
	/* (Line-1)*2：行号 1 对应第 0 页，行号 2 对应第 2 页……
	 * (Column-1)*8：列号 1 对应第 0 列，列号 2 对应第 8 列…… */
	for (i = 0; i < 8; i++)
	{
		OLED_WriteData(OLED_F8x16[Char - ' '][i]);			//显示上半部分内容（8 个字节）
	}
	OLED_SetCursor((Line - 1) * 2 + 1, (Column - 1) * 8);	//设置光标位置在下半部分（下一页）
	for (i = 0; i < 8; i++)
	{
		OLED_WriteData(OLED_F8x16[Char - ' '][i + 8]);		//显示下半部分内容（后 8 个字节）
	}
}

/**
  * @brief  OLED显示字符串
  * @param  Line 起始行位置，范围：1~4
  * @param  Column 起始列位置，范围：1~16
  * @param  String 要显示的字符串，范围：ASCII可见字符
  * @retval 无
  */
/* 原理：C 语言字符串以 '\0' 结尾，循环逐个取出字符调用 OLED_ShowChar，
 * 每显示一个字符列号 +1，直到遇到结尾符 '\0' 停止 */
void OLED_ShowString(uint8_t Line, uint8_t Column, char *String)
{
	uint8_t i;
	for (i = 0; String[i] != '\0'; i++)	//遍历字符串，直到结尾符 '\0'
	{
		OLED_ShowChar(Line, Column + i, String[i]);	//第 i 个字符显示在"起始列 + i"的位置
	}
}

/**
  * @brief  OLED次方函数
  * @param  X 底数
  * @param  Y 指数
  * @retval 返回值等于X的Y次方
  */
/* 原理：自己实现一个 X^Y：Result 从 1 开始，循环 Y 次每次都乘 X。
 * 标准库有 pow() 但它是浮点运算、慢且引入数学库，这里自己写整数版更快 */
uint32_t OLED_Pow(uint32_t X, uint32_t Y)
{
	uint32_t Result = 1;
	while (Y--)			//循环 Y 次
	{
		Result *= X;	//每次乘一个 X
	}
	return Result;		//返回 X^Y
}

/**
  * @brief  OLED显示数字（十进制，正数）
  * @param  Line 起始行位置，范围：1~4
  * @param  Column 起始列位置，范围：1~16
  * @param  Number 要显示的数字，范围：0~4294967295
  * @param  Length 要显示数字的长度，范围：1~10
  * @retval 无
  */
/* 原理（初学者重点）：按"位"把数字拆开显示。
 * 以 Number=123, Length=5 为例：
 *   第 0 位：123 / 10^4 % 10 = 0 —— 先除以 10000 得到万位，再 %10 取个位
 *   第 1 位：123 / 10^3 % 10 = 0
 *   第 2 位：123 / 10^2 % 10 = 1
 *   第 3 位：123 / 10^1 % 10 = 2
 *   第 4 位：123 / 10^0 % 10 = 3
 * 最后加上字符 '0'（ASCII 码 48），把数字转成对应字符（0->'0', 1->'1'...） */
void OLED_ShowNum(uint8_t Line, uint8_t Column, uint32_t Number, uint8_t Length)
{
	uint8_t i;
	for (i = 0; i < Length; i++)							//按长度循环，一位一位显示
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
/* 原理：先判断正负。
   * 正数：先显示 '+'，再把原数按十进制逐位显示；
 * 负数：先显示 '-'，再取绝对值（-Number）按十进制逐位显示。
 * 注意：负数取绝对值后仍是 uint32_t 无符号类型，可以安全存放 0~2^31 */
void OLED_ShowSignedNum(uint8_t Line, uint8_t Column, int32_t Number, uint8_t Length)
{
	uint8_t i;
	uint32_t Number1;
	if (Number >= 0)		//正数
	{
		OLED_ShowChar(Line, Column, '+');	//先显示正号
		Number1 = Number;					//绝对值就是它本身
	}
	else					//负数
	{
		OLED_ShowChar(Line, Column, '-');	//先显示负号
		Number1 = -Number;					//取绝对值（消除负号便于按位拆分）
	}
	for (i = 0; i < Length; i++)							//符号后面逐位显示数字
	{
		OLED_ShowChar(Line, Column + i + 1, Number1 / OLED_Pow(10, Length - i - 1) % 10 + '0');
		/* 列号要 +1：因为符号占了第 Column 列，数字从第 Column+1 列开始 */
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
/* 原理：和十进制同理，只是底数换成 16：除以 16^(位数-1) 再 %16 取出每一位。
 * 每一位可能是 0~15：0~9 显示字符 '0'~'9'，
 * 10~15 显示字符 'A'~'F'（SingleNumber-10+'A'） */
void OLED_ShowHexNum(uint8_t Line, uint8_t Column, uint32_t Number, uint8_t Length)
{
	uint8_t i, SingleNumber;
	for (i = 0; i < Length; i++)							//按长度循环，一位一位显示
	{
		SingleNumber = Number / OLED_Pow(16, Length - i - 1) % 16;	//取出当前位的十六进制值（0~15）
		if (SingleNumber < 10)			//0~9 用数字字符显示
		{
			OLED_ShowChar(Line, Column + i, SingleNumber + '0');
		}
		else							//10~15 用字母 A~F 显示
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
/* 原理：底数换成 2，每一位只可能是 0 或 1，直接 +'0' 转成字符 */
void OLED_ShowBinNum(uint8_t Line, uint8_t Column, uint32_t Number, uint8_t Length)
{
	uint8_t i;
	for (i = 0; i < Length; i++)							//按长度循环，一位一位显示
	{
		OLED_ShowChar(Line, Column + i, Number / OLED_Pow(2, Length - i - 1) % 2 + '0');
	}
}

/**
  * @brief  OLED初始化
  * @param  无
  * @retval 无
  */
/* 原理：SSD1306 上电后需要发一串"初始化命令序列"（屏幕厂商规定好的寄存器配置）：
 * 关闭显示 -> 配置时钟/多路复用/偏移/扫描方向/对比度/充电泵等 -> 开启显示。
 * 每一步 OLED_WriteCommand 的取值含义：
 *   0xAE/0xAF：关显示 / 开显示
 *   0xD5 0x80：时钟分频比与振荡器频率
 *   0xA8 0x3F：多路复用率（64 行）
 *   0xD3 0x00：显示偏移（0）
 *   0x40      ：显示起始行（第 0 行）
 *   0xA1      ：列扫描方向（正常，从左到右）
 *   0xC8      ：行扫描方向（正常，从上到下）
 *   0xDA 0x12：COM 引脚硬件配置（常见 0.96 寸屏固定值）
 *   0x81 0xCF：对比度（亮度）
 *   0xD9 0xF1：预充电周期
 *   0xDB 0x30：VCOMH 电平
 *   0xA4      ：显示内容来自显存
 *   0xA6      ：正常显示（非反色）
 *   0x8D 0x14：开启充电泵（内部升压给 OLED 供电，必须开，否则屏幕不亮！）
 * 初学不需要背这些值，知道"是屏幕的初始化配置流程"即可 */
void OLED_Init(void)
{
	uint32_t i, j;
	
	for (i = 0; i < 1000; i++)			//上电延时：等屏幕内部电源稳定后再发命令
	{
		for (j = 0; j < 1000; j++);		//空循环耗时间（软件延时，约几毫秒）
	}
	
	OLED_I2C_Init();			//端口初始化（PB8/PB9 开漏输出）
	
	OLED_WriteCommand(0xAE);	//关闭显示（初始化期间屏幕先不亮，避免花屏）
	
	OLED_WriteCommand(0xD5);	//设置显示时钟分频比/振荡器频率
	OLED_WriteCommand(0x80);	//分频值参数
	
	OLED_WriteCommand(0xA8);	//设置多路复用率
	OLED_WriteCommand(0x3F);	//0x3F = 64：屏幕总共 64 行（1/64 duty）
	
	OLED_WriteCommand(0xD3);	//设置显示偏移
	OLED_WriteCommand(0x00);	//偏移 0 行
	
	OLED_WriteCommand(0x40);	//设置显示开始行（第 0 行开始）
	
	OLED_WriteCommand(0xA1);	//设置左右方向，0xA1正常 0xA0左右反置
	
	OLED_WriteCommand(0xC8);	//设置上下方向，0xC8正常 0xC0上下反置

	OLED_WriteCommand(0xDA);	//设置COM引脚硬件配置
	OLED_WriteCommand(0x12);	//常用固定值
	
	OLED_WriteCommand(0x81);	//设置对比度控制
	OLED_WriteCommand(0xCF);	//对比度 0xCF（值越大屏幕越亮）
	
	OLED_WriteCommand(0xD9);	//设置预充电周期
	OLED_WriteCommand(0xF1);	//常用固定值

	OLED_WriteCommand(0xDB);	//设置VCOMH取消选择级别
	OLED_WriteCommand(0x30);	//常用固定值

	OLED_WriteCommand(0xA4);	//设置整个显示打开/关闭（0xA4 = 显示内容跟随显存）

	OLED_WriteCommand(0xA6);	//设置正常/倒转显示（0xA6 = 正常，白点黑底）

	OLED_WriteCommand(0x8D);	//设置充电泵
	OLED_WriteCommand(0x14);	//0x14 = 开启充电泵（升压电路，不开屏幕不亮！）

	OLED_WriteCommand(0xAF);	//开启显示（初始化完成，屏幕开始点亮）
		
	OLED_Clear();				//OLED清屏（初始化后先清一次，保证屏幕干净）
}
