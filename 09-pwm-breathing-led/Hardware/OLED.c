/* ============================================================================
 * 文件说明：OLED.c —— 0.96 寸 OLED 显示屏驱动（I2C 接口，江协科技版）
 * ----------------------------------------------------------------------------
 * 【硬件连接】
 *   OLED 屏是 4 引脚 I2C 版：
 *     SCL（时钟线）→ PB8     SDA（数据线）→ PB9
 *   另外两个引脚接 3.3V 和 GND（接线时注意）
 *
 * 【OLED 屏工作原理】
 *   OLED 是 SSD1306 驱动的 128×64 点阵屏：128 列 × 64 行像素。
 *   把屏幕横向分成 8 页（Page0~7），每页 8 行（8 像素 = 1 字节）。
 *   所以显存 = 8 页 × 128 列 = 1024 字节。写屏时：
 *   先"设置光标"（告诉屏幕要从哪一页哪一列开始写），再"写数据"。
 *   本工程用 16×8 的字模（OLED_Font.h 里的 OLED_F8x16），
 *   每个字符占 16 字节：上半页 8 字节 + 下半页 8 字节，正好 2 页。
 *   所以一行能显示 128/8 = 16 个字符，共 64/16 = 4 行。
 *
 * 【I2C 通信知识点】
 *   I2C 是两根线的串行总线：SCL 时钟 + SDA 数据，开漏输出（需要上拉电阻）。
 *   时序要点：起始信号(SDA 在 SCL 高电平时拉低) → 发地址/数据（高位在前）
 *   → 停止信号(SDA 在 SCL 高电平时拉高)。
 *   本工程是"软件模拟 I2C"：用 GPIO 的高低电平按时序手动翻转，简单直观。
 * ========================================================================== */

#include "stm32f10x.h"
#include "OLED_Font.h"

/*引脚配置*/
/* 下面两个宏把"写 SCL / 写 SDA"封装成一句话。
 * GPIO_WriteBit(GPIOB, GPIO_Pin_8, x)：向 PB8 输出电平 x（标准库函数）
 * (BitAction)(x) 是强制类型转换：GPIO_WriteBit 的第三个参数要求 BitAction 枚举类型 */
#define OLED_W_SCL(x)		GPIO_WriteBit(GPIOB, GPIO_Pin_8, (BitAction)(x))   // 写时钟线：PB8
#define OLED_W_SDA(x)		GPIO_WriteBit(GPIOB, GPIO_Pin_9, (BitAction)(x))   // 写数据线：PB9

/*引脚初始化*/
void OLED_I2C_Init(void)
{
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);
	// 开启 GPIOB 时钟（PB8/PB9 挂在 APB2 总线上）
	
	GPIO_InitTypeDef GPIO_InitStructure;
 	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_OD;
	// 配置为"开漏输出"模式（OD = Open-Drain）！
	// 为什么 I2C 必须用开漏输出？
	//   1. 开漏模式下 IO 只能拉低或"释放"（高阻态），靠外部上拉电阻拉高，
	//      这样多个设备都能安全地控制 SDA 线，不会互相打架（线与逻辑）；
	//   2. 电平转换也方便。所以 I2C 协议规定 SDA/SCL 必须开漏输出。
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;   // 翻转速度 50MHz（I2C 速率完全够用）
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_8;           // 先初始化 SCL（PB8）
 	GPIO_Init(GPIOB, &GPIO_InitStructure);
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_9;           // 再初始化 SDA（PB9）
 	GPIO_Init(GPIOB, &GPIO_InitStructure);
	
	OLED_W_SCL(1);      // 初始状态：SCL 拉高（I2C 空闲时两根线都应为高电平）
	OLED_W_SDA(1);      // 初始状态：SDA 拉高
	//GPIO_SetBits(GPIOB,GPIO_Pin_8|GPIO_Pin_9);
}

/**
  * @brief  I2C开始
  * @param  无
  * @retval 无
  */
/* I2C 起始信号时序：
 *   1. SDA 先拉高；2. SCL 拉高；3. 在 SCL 为高期间 SDA 拉低 → 起始信号；
 *   4. SCL 拉低（为后面传输数据做准备）。
 * 关键点：SDA 的电平变化必须发生在 SCL 为"低"的时候，
 *         SCL 为高时 SDA 跳变 = 起始/停止信号，不能被当作数据！ */
void OLED_I2C_Start(void)
{
	OLED_W_SDA(1);
	OLED_W_SCL(1);
	OLED_W_SDA(0);      // SCL 高电平期间 SDA 1→0：产生起始信号
	OLED_W_SCL(0);
}

/**
  * @brief  I2C停止
  * @param  无
  * @retval 无
  */
/* I2C 停止信号时序：SCL 高电平期间，SDA 0→1 = 停止信号 */
void OLED_I2C_Stop(void)
{
	OLED_W_SDA(0);
	OLED_W_SCL(1);
	OLED_W_SDA(1);      // SCL 高电平期间 SDA 0→1：产生停止信号
}

/**
  * @brief  I2C发送一个字节
  * @param  Byte 要发送的一个字节
  * @retval 无
  */
/* 发送过程：一个字节 8 位，从最高位(bit7)开始一位一位发。
 * Byte & (0x80 >> i)：i=0 时取 bit7，i=7 时取 bit0，逐个取出每一位。
 * 每位发送时：先把 SDA 放好电平 → SCL 拉高（从机在上升沿采样数据）→ SCL 拉低。
 * 最后多给一个时钟：正常情况下发完 8 位数据后第 9 个时钟是从机回"应答信号"的，
 * 这里不处理应答，只把时钟走完。 */
void OLED_I2C_SendByte(uint8_t Byte)
{
	uint8_t i;
	for (i = 0; i < 8; i++)        // 循环 8 次，发完 8 个 bit
	{
		OLED_W_SDA(Byte & (0x80 >> i));   // 取出 Byte 的第 7-i 位放到 SDA 上
		OLED_W_SCL(1);                    // SCL 拉高：数据被从机采样
		OLED_W_SCL(0);                    // SCL 拉低：准备发下一位
	}
	OLED_W_SCL(1);	//额外的一个时钟，不处理应答信号
	OLED_W_SCL(0);
}

/**
  * @brief  OLED写命令
  * @param  Command 要写入的命令
  * @retval 无
  */
/* I2C 传输格式：起始 → 从机地址(0x78) → 控制字节(0x00 表示后面是命令) → 命令 → 停止。
 * 0x78 是 OLED 屏的 I2C 从机地址（7 位地址 0x3C 左移 1 位 + 读写位 = 0x78）；
 * 0x00 是"控制字节"：告诉屏幕接下来发的是命令（若是 0x40 则接下来是显示数据）。 */
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
/* 与写命令的区别：控制字节是 0x40（表示后面是显示数据） */
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
/* 光标 = 告诉屏幕"接下来写的数据从哪开始放"。三条命令：
 *   1. 0xB0 | Y      ：页地址命令（0xB0~0xB7 对应第 0~7 页），Y 就是页号
 *   2. 0x10 | 高4位   ：列地址高 4 位（屏幕列地址 0~127，8 位，分成高 4 位低 4 位发）
 *   3. 0x00 | 低4位   ：列地址低 4 位
 * 例如 X=32（0x20）：高4位=2，低4位=0 → 发 0x12、0x00 */
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
/* 清屏原理：把 8 页 × 128 列 = 1024 个像素点全部写 0（熄灭）。
 * 外层循环切换页（0~7），内层循环把这一页的 128 列全写 0x00。 */
void OLED_Clear(void)
{  
	uint8_t i, j;
	for (j = 0; j < 8; j++)          // 逐页清屏（共 8 页）
	{
		OLED_SetCursor(j, 0);        // 光标定位到第 j 页第 0 列
		for(i = 0; i < 128; i++)     // 该页 128 列全部写 0
		{
			OLED_WriteData(0x00);    // 写 0x00：这一列的 8 个像素全灭
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
/* 显示原理：
   - 字模表 OLED_F8x16 是 16×8 的 ASCII 字模：每个字符 16 字节，
     前 8 字节画"上半部分"，后 8 字节画"下半部分"。
   - OLED_F8x16[Char - ' ']：字模表从空格(' ')开始存，所以字符要减去 ' ' 才是下标。
   - 上半部分写到第 (Line-1)*2 页，下半部分写到 (Line-1)*2+1 页，
     每部分 8 字节正好对应 8 列（8 像素宽）。
   - Column 列号要乘 8：因为每个字符占 8 列像素。 */
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
/* 原理：循环取出字符串里的每个字符（遇到 '\0' 结束符停止），
 * 逐个调用 OLED_ShowChar 显示，列号依次 +1。 */
void OLED_ShowString(uint8_t Line, uint8_t Column, char *String)
{
	uint8_t i;
	for (i = 0; String[i] != '\0'; i++)    // 直到字符串结束符 '\0'
	{
		OLED_ShowChar(Line, Column + i, String[i]);   // 第 i 个字符显示在第 Column+i 列
	}
}

/**
  * @brief  OLED次方函数
  * @retval 返回值等于X的Y次方
  */
/* 自己实现的整数次方函数（标准库没有整数版的 pow）。
 * 原理：Result 从 1 开始，连续乘 Y 次 X。例如 Pow(10,3) = 1×10×10×10 = 1000 */
uint32_t OLED_Pow(uint32_t X, uint32_t Y)
{
	uint32_t Result = 1;
	while (Y--)              // Y 次循环，每循环一次乘一个 X
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
/* 原理（按位拆数字）：
   Number / Pow(10, Length-i-1) % 10  —— 先除后取余：
   例：显示 123，Length=3：
     i=0：123/100%10 = 1    i=1：123/10%10 = 2    i=2：123/1%10 = 3
   最后 + '0' 把数字 0~9 转成 ASCII 字符 '0'~'9' 才能显示。 */
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
/* 原理：先判断正负：
   - 正数：第一位显示 '+'，后面照常拆位；
   - 负数：第一位显示 '-'，取绝对值后拆位。
   注意：拆位显示从 Column+1 列开始（第一列被符号占了）。 */
void OLED_ShowSignedNum(uint8_t Line, uint8_t Column, int32_t Number, uint8_t Length)
{
	uint8_t i;
	uint32_t Number1;
	if (Number >= 0)               // 正数或 0
	{
		OLED_ShowChar(Line, Column, '+');   // 第一位显示加号
		Number1 = Number;                  // 直接用原值拆位
	}
	else                            // 负数
	{
		OLED_ShowChar(Line, Column, '-');   // 第一位显示负号
		Number1 = -Number;                 // 取绝对值再拆位
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
/* 原理：和十进制类似，但拆位用 16 为底。
   每一位的数字是 0~15：0~9 显示成字符 '0'~'9'，10~15 显示成 'A'~'F' */
void OLED_ShowHexNum(uint8_t Line, uint8_t Column, uint32_t Number, uint8_t Length)
{
	uint8_t i, SingleNumber;
	for (i = 0; i < Length; i++)							
	{
		SingleNumber = Number / OLED_Pow(16, Length - i - 1) % 16;   // 取出第 i 位的值（0~15）
		if (SingleNumber < 10)                        // 0~9 直接转数字字符
		{
			OLED_ShowChar(Line, Column + i, SingleNumber + '0');
		}
		else                                          // 10~15 转成 A~F
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
/* 原理：以 2 为底拆位，每位只能是 0 或 1 */
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
/* 初始化流程：
   1. 上电延时：让 OLED 内部电源稳定（刚上电时芯片还没准备好，立即发命令可能失败）；
   2. 初始化 I2C 引脚（PB8/PB9 开漏输出）；
   3. 通过 I2C 向屏幕写一串"初始化命令序列"：配置显示时钟、复用率、偏移、扫描方向、
      对比度、充电泵等参数（这些都是 SSD1306 数据手册规定的标准初始化步骤）；
   4. 打开显示（0xAF）并清屏。 */
void OLED_Init(void)
{
	uint32_t i, j;
	
	for (i = 0; i < 1000; i++)			//上电延时
	{
		for (j = 0; j < 1000; j++);     // 两层空循环浪费约几十毫秒
	}
	
	OLED_I2C_Init();			//端口初始化
	
	OLED_WriteCommand(0xAE);	//关闭显示（先关屏再配置，避免配置过程中画面乱跳）
	
	OLED_WriteCommand(0xD5);	//设置显示时钟分频比/振荡器频率
	OLED_WriteCommand(0x80);
	
	OLED_WriteCommand(0xA8);	//设置多路复用率（本屏 64 行，所以参数 0x3F = 63）
	OLED_WriteCommand(0x3F);
	
	OLED_WriteCommand(0xD3);	//设置显示偏移（0x00 表示不偏移）
	OLED_WriteCommand(0x00);
	
	OLED_WriteCommand(0x40);	//设置显示开始行（从第 0 行开始）
	
	OLED_WriteCommand(0xA1);	//设置左右方向，0xA1正常 0xA0左右反置
	
	OLED_WriteCommand(0xC8);	//设置上下方向，0xC8正常 0xC0上下反置

	OLED_WriteCommand(0xDA);	//设置COM引脚硬件配置（0x12 是 128×64 屏的标准值）
	OLED_WriteCommand(0x12);
	
	OLED_WriteCommand(0x81);	//设置对比度控制（0xCF 越大屏幕越亮，可调）
	OLED_WriteCommand(0xCF);

	OLED_WriteCommand(0xD9);	//设置预充电周期
	OLED_WriteCommand(0xF1);

	OLED_WriteCommand(0xDB);	//设置VCOMH取消选择级别
	OLED_WriteCommand(0x30);

	OLED_WriteCommand(0xA4);	//设置整个显示打开/关闭（0xA4：使用显存内容显示）

	OLED_WriteCommand(0xA6);	//设置正常/倒转显示（0xA6 正常显示，0xA7 黑白颠倒）

	OLED_WriteCommand(0x8D);	//设置充电泵（OLED 内部需要升压电路，必须打开）
	OLED_WriteCommand(0x14);    // 0x14 = 打开充电泵。这一步忘配置屏幕会一直黑屏！

	OLED_WriteCommand(0xAF);	//开启显示（最后一步打开屏幕，前面配置正式生效）
		
	OLED_Clear();				//OLED清屏（清掉上电时的随机花屏内容）
}
