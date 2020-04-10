#include <reg52.h>

/*可通过数字旋转编码改时间*/
sbit HC595_RCK = P1^0;	// 输出寄存器锁存时钟线
sbit HC595_SCK = P1^1;	//数据输入时钟线
sbit HC595_DATA = P1^4;	//数据线

sbit HC595_SCK_LED = P0^5;		//数据输入时钟线
sbit HC595_RCK_LED = P0^6;		//输出存储器锁存时钟线
sbit HC595_DATA_LED = P0^7;		//输入数据线

sbit encoder_portA  = P2^0;	//编码器1号
sbit encoder_portB  = P2^1;	//3号
sbit encoder_portsd = P2^2;	//5号

sbit bee = P3^4;	//蜂鸣器

typedef unsigned char uchar;
typedef unsigned int uchar16;

code unsigned char duan_led[11] = {0x3f,0x06,0x5b ,0x4f ,0x66 ,0x6d ,0x7d,0x07,0x7f,0x6f,0x40}; 	//段选LED灯,第11位是分隔符
code unsigned char wei_led[8]  = {0x80,0x40,0x20,0x10,0x08,0x04,0x02,0x01};					//位选共阴极


uchar k;		//轮询数码管
uchar16  counter = 0; //计数
				
char Led_eight[8] = {0x00,0x00,0x0A,0x00,0x00,0x0A,0x00,0x00};	//当前的时分秒
//		    时       -      分      -       秒


/****************************************************************************/
//大致思路：
/*
	根据原理图得知 数码管的段选和位选 由此写出duan_led，wei_led 数组
	使用 定时器1 每毫秒的进行轮询Led_eight数组，每秒对当前计数值进行分位，几
	至此判断是否有进制，进制则上级加1，以此类推。该时钟使用12小时制的，不可超过12，分钟不可等于60
	最后更改时钟，是旋转编码器来更改Led_eight数组，当Led_eight数组值被修改时，再由中断处理函数
	显示数据，便可以修改时钟。
	随后加入函数判断下数组中的值是否为闹钟设定时钟，便完成闹钟功能
	Led_eight 数数组的下标所代表的数字去调用函数light（传参duan_led，wei_led）
	Led_eight 数组来显示当前的时分秒，0-1表示 时，2 表示符号'-' ，3-4表示分，5表示符号-，6-7表示秒

*/




/*
	功能：延时函数
	参数：i  毫秒ms
	返回：无
*/
void delay_1ms(uchar16 i)
{
	uchar j;
	for(;i>0;i--)
		for(j=0;j<110;j++);
	
}

/*
	功能：蜂鸣器响应50ms
	参数：无
	返回：无
*/
void beeSound()
{
	bee = 0;
	delay_1ms(50);
	bee = 1;
}

uchar flag_aram = 0;		//闹钟标记

/*
	功能：设定闹钟
	参数：无
	返回：无
*/
void alarm_clock()
{
	if(Led_eight[0] == 0 && Led_eight[1] == 8 && Led_eight[3]== 3 && Led_eight[4] == 0 )
	{
		flag_aram = 1;	//当闹钟响起时，将flag置 1
		bee = 0;
	}else
	{
		if(flag_aram)		//当flag为1时，表示闹钟响起过一次了，那么当时间已经不是预设值时，就需要将闹钟关闭
		{
			flag_aram = 0;
			bee = 1;
		}			
			
	}
	
}


/*
	功能：设定闹钟
	参数：无
	返回：无
*/
void send_data()
{
	unsigned char i;
	HC595_RCK_LED = 0;	//输出存储器锁存时钟线
	for(i=0;i<16;i++)
	{
		HC595_SCK_LED = 0;
		HC595_DATA_LED = 0;			
		HC595_SCK_LED = 1;	
	}
	
	HC595_RCK_LED = 1;	
	
}

/*
	功能：初始化定时器1
	参数：无
	返回值：无
*/
void timer_init()
{
	TMOD &=0x0F;
	TMOD |=0x10;
	TH1 = 0xFC;	 
	TL1 = 0x66;
	TR1 = 1;	//启动定时器
}


/*
	功能：初始化中断
	参数：无
	返回值：无

*/
void inter_init()
{
	ET1 = 1;
	EA = 1;
}



/*
	功能：点亮单个数码管
	参数：u5 u5移位寄存器(74HC595)
	      u10 u10寄存器(74HC595)
	返回：无
*/
void light(unsigned char  u5,unsigned char u10)
{
	unsigned char j,k=0x01;
	HC595_RCK = 0;
	k=0x01;
	for(j=0;j<8;j++)	//这个8表示段选的8
	{
		HC595_SCK = 0;
		if(0 != (u5&0x80))
		{
			HC595_DATA = 1;		//为1亮
		}else
		{
			HC595_DATA = 0;
		}
		u5<<=1;
		HC595_SCK = 1;
	}
		
	
	for(j=0;j<8;j++)	//这个8表示位选的8
	{
		HC595_SCK = 0;
		if(0 != (k & u10))
		{
			HC595_DATA = 0;
		}else
		{
			HC595_DATA = 1;
		}

		k<<=1;
		HC595_SCK = 1;
		
	}	
	HC595_RCK = 1;
}


uchar reverse=0,foreward=0,rotary_flag = 0;
uchar sign=0;


/*
	功能：旋转编码器判断
	参数：无
	返回：无
*/
void encoder_data()
{
	//确定是否有转动
	if((1 == encoder_portA) && (1 == encoder_portB))
	{
		rotary_flag=1;
	}
	//判断方向
	if((encoder_portA ^ encoder_portB) && (1==rotary_flag))
	{
		sign++;
		if(sign>2)			//去抖
		{
			sign = 0;
			if((1== encoder_portA) &&  (0 == encoder_portB))	//foreward
			{
				foreward =1;
			}
			if((0 == encoder_portA) &&  (1 ==encoder_portB))
			{
				reverse = 1;
			}
		}
	}
	
	
	//正向加 反向减 ,只对分加时间
	if((0 == encoder_portA) &&  (0== encoder_portB) && foreward)
	{
		foreward = 0;
		reverse =0;
		rotary_flag = 0;
		
		if(!encoder_portsd)//小时
		{		
			if(Led_eight[0] == 1)
			{
				if(Led_eight[1] <2)
					Led_eight[1]++;	//只对个位自增
				else
					Led_eight[1] = 0;
			}
			else
			{
				Led_eight[1]++;
			}
			
			if(Led_eight[1]==10 && Led_eight[0]!=1)
			{
				Led_eight[0]++;
				Led_eight[1] = 0;
			}
						
		}
		else
		{
			Led_eight[4]++;		//只对分钟++
			if(Led_eight[4] == 10)
			{
				Led_eight[3]+=1;
				Led_eight[4]=0;
				if(Led_eight[3] == 6)
				{
					Led_eight[3] = 0;
				}
				
			}
		}				
		beeSound();
		
	}
	
	//反向减 
	if((0 == encoder_portA) &&  (0==encoder_portB) && reverse)
	{
		foreward = 0;
		reverse =0;
		rotary_flag = 0;
		
		if(!encoder_portsd)					//小时
		{	
			//个位十位都不为0 那么就可以减
			if(Led_eight[0]!=0 || Led_eight[1]!=0)	
			{
				Led_eight[1]--;
				if(Led_eight[1] < 0)
				{
					if(Led_eight[0]!= 0)
					{	
						Led_eight[1] = 9;
						Led_eight[0]--;
					}
					else	//当十位为0时，就不能减了
					{
						Led_eight[1] = 0;
					}
				}
			}
			
						
		}
		else								//分钟
		{	
			//个位十位都不为0 那么就可以减
			if(Led_eight[3]!=0 || Led_eight[4]!=0)
			{
				Led_eight[4]--;
				if(Led_eight[4] < 0)
				{
					if(Led_eight[3]!= 0)
					{	
						Led_eight[4] = 9;
						Led_eight[3]--;
					}
					else	//当十位为0时，就不能减了
					{
						Led_eight[4] = 0;
					}
				}
			}

		}	
		
		/*只要旋转就叫*/
		beeSound();
	}
	

}



void main()
{
	send_data();//关闭LED灯

	timer_init();
	inter_init();
	while(1)
	{
		encoder_data();
		
	}
}


//中断处理程序
void interrputer_3() interrupt 3
{
	static uchar16 sec = 0;
	TH1 = 0xFC;	 
	TL1 = 0x66;

	counter++;

	
	if(1000 == counter)
	{
		sec++;
		counter = 0;
		Led_eight[7] = sec%10;			//秒
		Led_eight[6] = sec/10%10;		//十秒
		if(Led_eight[6] == 6)			//逢6进一
		{
			Led_eight[6] = 0x00;			
			sec=0;					//清零(秒)
			
			Led_eight[4] += 1;			//分钟+1
			if(Led_eight[4] == 10)		//逢十进一
			{
				Led_eight[4] = 0x00;		//清零(分)
				Led_eight[3] += 1;		//十分
				
				if(Led_eight[3] == 6)	//逢6进一
				{
					Led_eight[3] = 0x00;	//清零(十秒)

					Led_eight[1] += 1;	//时
					
					if(Led_eight[0]==1 && Led_eight[1]==3)	//当小时十位为1时且个位为3表示 ，超过十二点了，要将小时为归零
					{
						Led_eight[0] = 0x00;
						Led_eight[1] = 0x01;
					}

					if(Led_eight[1]==10)	//逢十进一
					{
						Led_eight[1] = 0x00;
						Led_eight[0] += 1;
					}
				}
				
			}
				
		}
		
	}
	
	if(k<7)
		k++;
	else
		k=0;
	
	light(duan_led[Led_eight[k]],wei_led[k]);
	alarm_clock();
	//TF1 = 0;
}
