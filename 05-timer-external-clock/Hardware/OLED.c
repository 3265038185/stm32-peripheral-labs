/*===============================================================================
 * 文件名称：OLED.c（OLED 显示屏驱动文件）
 * 所属工程：3-定时器（本文件与 4-1OLED 子工程中的 OLED.c 内容相同）
 * 模块功能：驱动 0.96 寸 OLED 屏幕（SSD1306 驱动芯片，I2C 接口）：
 *           1. 用 GPIO 模拟 I2C 时序与屏幕通信（软件 I2C，不占硬件外设）
 *           2. 提供显示字符、字符串、数字（十进制/十六进制/二进制/带符号）的函数
 * 用到的外设/知识点：
 *   1. GPIO 开漏输出（GPIO_Mode_Out_OD）：I2C 总线是"线与"结构，
 *      开漏 + 外部上拉电阻才能实现正确的电平，这是 I2C 的硬件要求！
 *   2. 软件模拟 I2C 时序：起始、停止、发送字节（本模块只写不发读，
 *      所以省略了应答位检测）
 *   3. SSD1306 驱动芯片：OLED 的"大脑"，通过发送命令/数据来控制显示
 *   4. 点阵字库：OLED_F8x16 里存了 8x16 像素的 ASCII 字模，
 *      8 像素一列、16 像素分上下两半，所以一个字符要分"上半/下半"两次写
 * 引脚接线（本工程）：
 *   PB8  = SCL（时钟线），PB9 = SDA（数据线）
 * 整体结构：
 *   OLED_I2C_* 底层时序函数 → OLED_WriteCommand/WriteData 中间层 →
 *   OLED_SetCursor/ShowChar/ShowNum 等上层显示函数
 *===============================================================================
 */
#include "stm32f10x.h"
#include "OLED_Font.h"

/*引脚配置*/
/* 宏定义：把"给 PB8 写电平"包装成 OLED_W_SCL(x)，"给 PB9 写电平"包装成
   OLED_W_SDA(x)，后面写代码时语义清晰，像操作变量一样操作引脚。
   GPIO_WriteBit 是标准库函数：第三个参数是 BitAction 类型（0 或 1），
   (BitAction)(x) 是强制类型转换，保证传进来的 0/1 符合函数要求。 */
#define OLED_W_SCL(x)		GPIO_WriteBit(GPIOB, GPIO_Pin_8, (BitAction)(x))
#define OLED_W_SDA(x)		GPIO_WriteBit(GPIOB, GPIO_Pin_9, (BitAction)(x))

/*引脚初始化*/
/*===============================================================================
 * 函数名称：OLED_I2C_Init（I2C 引脚初始化）
 * 功能：    把 PB8（SCL）、PB9（SDA）配置为开漏输出，并初始化为高电平
 * 参数：    无
 * 返回值：  无
 * 实现原理：
 *   1. RCC_APB2PeriphClockCmd(GPIOB, ENABLE)：打开 GPIOB 时钟
 *   2. GPIO_Mode = GPIO_Mode_Out_OD（开漏输出）：
 *      —— I2C 协议要求总线空闲时为高电平，任何设备都可以把总线拉低
 *         （这就是"线与"）。开漏输出只能输出低电平或"释放"（高阻），
 *         高电平要靠外部上拉电阻提供，所以必须用开漏！
 *   3. 初始化后先让 SCL、SDA 都为 1（高电平）：总线空闲状态，为通信做准备
 *===============================================================================
 */
void OLED_I2C_Init(void)
{
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);	// 打开 GPIOB 时钟（GPIO 在 APB2 总线上）
	
	GPIO_InitTypeDef GPIO_InitStructure;
 	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_OD;	// 开漏输出（I2C 必须开漏！高电平靠外部上拉）
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;	// 输出速度 50MHz（I2C 时钟够用）
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_8;	// 先配置 PB8（SCL 时钟线）
 	GPIO_Init(GPIOB, &GPIO_InitStructure);
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_9;	// 再配置 PB9（SDA 数据线）
 	GPIO_Init(GPIOB, &GPIO_InitStructure);
	
	OLED_W_SCL(1);	// SCL 置高：总线空闲状态
	OLED_W_SDA(1);	// SDA 置高：总线空闲状态
	//GPIO_SetBits(GPIOB,GPIO_Pin_8|GPIO_Pin_9);
}

/**
  * @brief  I2C开始
  * @param  无
  * @retval 无
  */
/* I2C 起始条件（START）时序：
   SCL 保持高电平时，SDA 从高变低 —— 这一瞬间代表"总线开始通信"。
   为什么必须这样？因为 I2C 规定：只有在 SCL 为低电平时 SDA 才能变化，
   而 SDA 在 SCL 高电平时的跳变被定义为"起始/停止信号"，这是协议规定的！ */
void OLED_I2C_Start(void)
{
	OLED_W_SDA(1);	// 1. SDA 先拉高
	OLED_W_SCL(1);	// 2. SCL 拉高（此时总线空闲，SCL 高电平期间 SDA 才允许变化）
	OLED_W_SDA(0);	// 3. SCL 高电平时把 SDA 拉低 → 产生起始信号！
	OLED_W_SCL(0);	// 4. 再拉低 SCL，为后续发送数据做准备
}

/**
  * @brief  I2C停止
  * @param  无
  * @retval 无
  */
/* I2C 停止条件（STOP）时序：
   与起始信号相反：SCL 高电平时，SDA 从低变高 → 代表"通信结束"。 */
void OLED_I2C_Stop(void)
{
	OLED_W_SDA(0);	// 1. SDA 先拉低
	OLED_W_SCL(1);	// 2. SCL 拉高（SCL 高电平期间 SDA 变化 = 停止信号）
	OLED_W_SDA(1);	// 3. SCL 高电平时把 SDA 拉高 → 产生停止信号！
}

/**
  * @brief  I2C发送一个字节
  * @param  Byte 要发送的一个字节
  * @retval 无
  */
/* I2C 发送字节的原理（高位在前，MSB 先行）：
   1. 要发送的字节有 8 位，一位一位发：
      - (0x80 >> i) 依次取出第 7 位、第 6 位……第 0 位
      - Byte & (0x80>>i) 取出当前位：结果是 0 或 0x80，非 0 即"1"，
        OLED_W_SDA 收到非 0 值就会写出高电平（GPIO_WriteBit 里 x!=0 写 1）
      - SCL 拉高：此时数据线上的电平就是"这一位的值"，从机在 SCL 高电平时读取
      - SCL 拉低：准备发送下一位（SDA 只能在 SCL 低电平时变化！）
   2. 8 位发完后，本来从机要拉低 SDA 表示"应答（ACK）"，
      本模块只写不读，所以直接补一个额外的时钟周期跳过应答（不检测），
      这就是代码最后 OLED_W_SCL(1) 再 (0) 的作用。 */
void OLED_I2C_SendByte(uint8_t Byte)
{
	uint8_t i;
	for (i = 0; i < 8; i++)	// 循环 8 次，发送 8 位数据
	{
		OLED_W_SDA(Byte & (0x80 >> i));	// 依次取出最高位→最低位，放到 SDA 线上
		OLED_W_SCL(1);	// SCL 拉高：从机在这时"采样"读走这一位
		OLED_W_SCL(0);	// SCL 拉低：准备发送下一位
	}
	OLED_W_SCL(1);	//额外的一个时钟，不处理应答信号
	OLED_W_SCL(0);
}

/**
  * @brief  OLED写命令
  * @param  Command 要写入的命令
  * @retval 无
  */
/* I2C 写命令的"数据帧"结构：
   起始 → 从机地址(0x78) → 控制字节(0x00 表示后面跟的是命令) → 命令字节 → 停止
   注意：0x78 是 OLED 的 I2C 从机地址（0x3C<<1 左移一位，
   即 7 位地址 0x3C 加上读写位拼成的 8 位）；0x00 告诉 SSD1306"接下来是命令" */
void OLED_WriteCommand(uint8_t Command)
{
	OLED_I2C_Start();	// 起始信号
	OLED_I2C_SendByte(0x78);		//从机地址
	OLED_I2C_SendByte(0x00);		//写命令
	OLED_I2C_SendByte(Command); 	// 发送真正的命令字节（如 0xAE 关显示）
	OLED_I2C_Stop();	// 停止信号
}

/**
  * @brief  OLED写数据
  * @param  Data 要写入的数据
  * @retval 无
  */
/* 与写命令几乎一样，唯一区别：控制字节是 0x40（表示后面跟的是"数据"）。
   数据 = 要显示内容的点阵（字模），写入后屏幕对应的像素点亮起。 */
void OLED_WriteData(uint8_t Data)
{
	OLED_I2C_Start();	// 起始信号
	OLED_I2C_SendByte(0x78);		//从机地址
	OLED_I2C_SendByte(0x40);		//写数据
	OLED_I2C_SendByte(Data);	// 发送真正的数据字节（字模点阵）
	OLED_I2C_Stop();	// 停止信号
}

/**
  * @brief  OLED设置光标位置
  * @param  Y 以左上角为原点，向下方向的坐标，范围：0~7
  * @param  X 以左上角为原点，向右方向的坐标，范围：0~127
  * @retval 无
  */
/* 光标定位原理：SSD1306 的显存（GDDRAM）是 128x64 像素，被分成 8 页
   （Page0~Page7，每页 8 行像素，正好一页对应 8x128 的点阵区）。
   定位需要三条命令：
   1. 0xB0 | Y：页地址命令，Y 0~7 对应 8 页（即 8 行像素条）
   2. 0x10 | (X高4位)：列地址高 4 位（列地址是 0~127，需要 7 位，分两次发）
   3. 0x00 | (X低4位)：列地址低 4 位
   之后每写一个数据字节，光标自动向右移动一列 */
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
/* 清屏原理：把 8 页 × 每页 128 列 = 1024 个"数据字节"全部写成 0x00
   （0x00 表示这一列的 8 个像素全灭），屏幕自然就全黑了。 */
void OLED_Clear(void)
{  
	uint8_t i, j;
	for (j = 0; j < 8; j++)	// 遍历 8 页（0~7）
	{
		OLED_SetCursor(j, 0);	// 光标定位到第 j 页第 0 列
		for(i = 0; i < 128; i++)	// 该页共有 128 列
		{
			OLED_WriteData(0x00);	// 每列写 0x00：该列 8 个像素全部熄灭
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
/* 显示字符原理：
   1. 屏幕共 4 行、每行 16 个字符；每个字符占 8x16 像素
      （宽 8 列像素、高 16 行像素 = 2 页的高度）
   2. 字模存在 OLED_F8x16[字符][16] 里：每个字符 16 个字节，
      前 8 个字节是字符"上半部分"（第 1 页），后 8 个字节是"下半部分"（第 2 页）
   3. OLED_F8x16[Char - ' ']：字库下标 0 对应空格（ASCII 码 0x20），
      所以用 字符ASCII码 - ' ' 算出它在字库里的位置
   4. 先定位到上半页，连续写 8 个字节（上半点阵）；再定位到下半页，
      再写 8 个字节（下半点阵），一个 8x16 的字符就完整显示出来了 */
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
/* 显示字符串原理：C 语言字符串以 '\0'（空字符）结尾，
   从第一个字符开始逐个调用 OLED_ShowChar 显示，列号依次 +1，
   直到遇到 '\0' 停止。 */
void OLED_ShowString(uint8_t Line, uint8_t Column, char *String)
{
	uint8_t i;
	for (i = 0; String[i] != '\0'; i++)	// 从第 0 个字符开始，遇到字符串结束符 '\0' 停止
	{
		OLED_ShowChar(Line, Column + i, String[i]);	// 逐个显示，列号随 i 递增
	}
}

/**
  * @brief  OLED次方函数
  * @retval 返回值等于X的Y次方
  */
/* 原理：Result 初始为 1，循环乘 Y 次 X。
   例如 OLED_Pow(10, 2) = 10*10 = 100。
   注意：这是用循环实现的"整数次方"，比调用数学库函数省空间 */
uint32_t OLED_Pow(uint32_t X, uint32_t Y)
{
	uint32_t Result = 1;
	while (Y--)	// 循环 Y 次（Y 用完自动为 0，退出循环）
	{
		Result *= X;	// 每次乘一个 X
	}
	return Result;	// 返回 X 的 Y 次方
}

/**
  * @brief  OLED显示数字（十进制，正数）
  * @param  Line 起始行位置，范围：1~4
  * @param  Column 起始列位置，范围：1~16
  * @param  Number 要显示的数字，范围：0~4294967295
  * @param  Length 要显示数字的长度，范围：1~10
  * @retval 无
  */
/* 显示十进制数字原理（逐位取出）：
   以 Number=123、Length=3 为例：
   - 第 0 位：123 / 100 % 10 = 1（先除以 10^2 去掉后两位，再取余 10 得到百位）
   - 第 1 位：123 / 10 % 10 = 2
   - 第 2 位：123 / 1 % 10 = 3
   最后 + '0' 把数字 0~9 变成字符 '0'~'9'（ASCII 码连续，'0'=48） */
void OLED_ShowNum(uint8_t Line, uint8_t Column, uint32_t Number, uint8_t Length)
{
	uint8_t i;
	for (i = 0; i < Length; i++)	// 依次显示每一位						
	{
		// Number / 10^(Length-1-i) 把要显示的位挪到个位，% 10 取出该位，+ '0' 转成字符
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
/* 带符号数显示原理：
   1. 先判断正负：正数在第 1 列显示 '+'，负数显示 '-'（并把 Number 取绝对值）
   2. 从第 2 列开始按十进制逐位显示（逻辑同 OLED_ShowNum）
   注意：先判断符号再取绝对值，避免负数取绝对值时的溢出问题 */
void OLED_ShowSignedNum(uint8_t Line, uint8_t Column, int32_t Number, uint8_t Length)
{
	uint8_t i;
	uint32_t Number1;
	if (Number >= 0)	// 正数或 0
	{
		OLED_ShowChar(Line, Column, '+');	// 前面显示正号
		Number1 = Number;	// 原样保留
	}
	else	// 负数
	{
		OLED_ShowChar(Line, Column, '-');	// 前面显示负号
		Number1 = -Number;	// 取绝对值（转为无符号，防止溢出）
	}
	for (i = 0; i < Length; i++)	// 从符号后面开始逐位显示						
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
/* 十六进制显示原理：和十进制类似，只是把"取位"的底数从 10 换成 16：
   - 某一位数值 0~9 → 显示字符 '0'~'9'（+ '0'）
   - 某一位数值 10~15 → 显示字符 'A'~'F'（- 10 + 'A'） */
void OLED_ShowHexNum(uint8_t Line, uint8_t Column, uint32_t Number, uint8_t Length)
{
	uint8_t i, SingleNumber;
	for (i = 0; i < Length; i++)	// 依次显示每一位						
	{
		SingleNumber = Number / OLED_Pow(16, Length - i - 1) % 16;	// 取出当前位的值（0~15）
		if (SingleNumber < 10)	// 0~9
		{
			OLED_ShowChar(Line, Column + i, SingleNumber + '0');	// 显示数字字符
		}
		else	// 10~15
		{
			OLED_ShowChar(Line, Column + i, SingleNumber - 10 + 'A');	// 显示 A~F 字符
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
/* 二进制显示原理：与十进制同理，底数换成 2，每一位只有 0 或 1 */
void OLED_ShowBinNum(uint8_t Line, uint8_t Column, uint32_t Number, uint8_t Length)
{
	uint8_t i;
	for (i = 0; i < Length; i++)	// 依次显示每一位						
	{
		// 取出当前位（0 或 1），+ '0' 转成字符 '0' 或 '1'
		OLED_ShowChar(Line, Column + i, Number / OLED_Pow(2, Length - i - 1) % 2 + '0');
	}
}

/**
  * @brief  OLED初始化
  * @param  无
  * @retval 无
  */
/* 初始化原理：SSD1306 驱动芯片上电后需要按固定顺序写入一串配置命令
   （这些命令出自 SSD1306 数据手册的"初始化时序"），逐条解释：
   - 0xAE：关闭显示（配置期间先关屏，避免花屏）
   - 0xD5/0x80：设置显示时钟分频比/振荡器频率（默认值即可）
   - 0xA8/0x3F：设置多路复用率 = 64（屏幕共 64 行像素）
   - 0xD3/0x00：设置显示偏移 = 0（显示从第 0 行开始）
   - 0x40：设置显示开始行 = 第 0 行
   - 0xA1：设置左右方向正常（0xA0 为左右镜像）
   - 0xC8：设置上下方向正常（0xC0 为上下镜像）
   - 0xDA/0x12：COM 引脚硬件配置（0.96 寸屏固定值）
   - 0x81/0xCF：设置对比度（亮度），0xCF 是常用值
   - 0xD9/0xF1：设置预充电周期（固定值）
   - 0xDB/0x30：设置 VCOMH 取消选择级别（固定值）
   - 0xA4：整个显示打开（0xA5 为全亮）
   - 0xA6：正常显示（0xA7 为反色显示）
   - 0x8D/0x14：打开内部充电泵！—— 0.96 寸屏没有外部升压电路，
     必须开启芯片内部电荷泵才能点亮
   - 0xAF：开启显示（配置完成，亮屏！）
   最后 OLED_Clear() 清屏，避免显示上电残留的乱码 */
void OLED_Init(void)
{
	uint32_t i, j;
	
	for (i = 0; i < 1000; i++)			//上电延时（等 SSD1306 内部稳定）
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
