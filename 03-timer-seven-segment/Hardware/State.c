#include "stm32f10x.h"                  // Device header
 switch(Key_Buf[i].KeyStatus.KEY_STATUS)
        {
            //状态0：没有按键按下
            case KEY_NULL:
				if(Key_Buf[i].KeyStatus.KEY_FLAG == HIGH_LEVER)//有按键按下
              	{
                    Key_Buf[i].KeyStatus.KEY_STATUS = KEY_SURE;//转入状态1
					Key_Buf[i].KeyStatus.KEY_EVENT = KEY_NULL;//空事件
                }
                else
                {
                    Key_Buf[i].KeyStatus.KEY_EVENT = KEY_NULL;//空事件
                }
                break;
            //状态1：按键按下确认
            case KEY_SURE:
                if(Key_Buf[i].KeyStatus.KEY_FLAG == HIGH_LEVER)//确认和上次相同
                {
                    Key_Buf[i].KeyStatus.KEY_STATUS = KEY_PRESS;//转入状态2
					Key_Buf[i].KeyStatus.KEY_EVENT = KEY_PRESS;//按下事件
                    Key_Buf[i].KeyStatus.KEY_TIMECOUNT = 0;//计数器清零
                }
                else
                {
                    Key_Buf[i].KeyStatus.KEY_STATUS = KEY_NULL;//转入状态0
                    Key_Buf[i].KeyStatus.KEY_EVENT = KEY_NULL;//空事件
                }
                break;
            //状态2：按键按下
            case KEY_PRESS:
                if(Key_Buf[i].KeyStatus.KEY_FLAG != HIGH_LEVER)//按键释放，端口高电平
                {
                    Key_Buf[i].KeyStatus.KEY_STATUS = KEY_NULL;//转入状态0
                    Key_Buf[i].KeyStatus.KEY_EVENT = KEY_RAISE;//松开事件
                }
                else if((Key_Buf[i].KeyStatus.KEY_FLAG == HIGH_LEVER)
					&& (++Key_Buf[i].KeyStatus.KEY_TIMECOUNT >= KEY_LONG_DOWN_DELAY)) 
                    //超过KEY_LONG_DOWN_DELAY没有释放
                {
                    Key_Buf[i].KeyStatus.KEY_STATUS = KEY_LONG;//转入状态3
                    Key_Buf[i].KeyStatus.KEY_EVENT = KEY_LONG;//长按事件
					Key_Buf[i].KeyStatus.KEY_TIMECOUNT = 0;//计数器清零
                }
                else
                {
                    Key_Buf[i].KeyStatus.KEY_EVENT = KEY_NULL;//空事件
                }
                break;
            //状态3：按键连续按下
            case KEY_LONG:
                if(Key_Buf[i].KeyStatus.KEY_FLAG != HIGH_LEVER)//按键释放，端口高电平
                {
                    Key_Buf[i].KeyStatus.KEY_STATUS = KEY_NULL;//转入状态0
                    Key_Buf[i].KeyStatus.KEY_EVENT = KEY_RAISE;//松开事件
                }
                else if((Key_Buf[i].KeyStatus.KEY_FLAG == HIGH_LEVER) 
                && (++Key_Buf[i].KeyStatus.KEY_TIMECOUNT >= KEY_LONG_DOWN_DELAY)) 
                    //超过KEY_LONG_DOWN_DELAY没有释放
                {
                    Key_Buf[i].KeyStatus.KEY_EVENT = KEY_LONG;//长按事件
                    Key_Buf[i].KeyStatus.KEY_TIMECOUNT = 0;//计数器清零
                }
                else
                {
                    Key_Buf[i].KeyStatus.KEY_EVENT = KEY_NULL;//空事件
                }
                break;
			default:
				break;
————————————————

                            版权声明：本文为博主原创文章，遵循 CC 4.0 BY-SA 版权协议，转载请附上原文出处链接和本声明。
                        
原文链接：https://blog.csdn.net/qq_45893260/article/details/119787933