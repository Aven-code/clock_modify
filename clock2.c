#include <reg52.h>

/*��ͨ��������ת�����ʱ��*/
sbit HC595_RCK = P1^0;	// ����Ĵ�������ʱ����
sbit HC595_SCK = P1^1;	//��������ʱ����
sbit HC595_DATA = P1^4;	//������

sbit HC595_SCK_LED = P0^5;		//��������ʱ����
sbit HC595_RCK_LED = P0^6;		//����洢������ʱ����
sbit HC595_DATA_LED = P0^7;		//����������

sbit encoder_portA  = P2^0;	//������1��
sbit encoder_portB  = P2^1;	//3��
sbit encoder_portsd = P2^2;	//5��

sbit bee = P3^4;	//������

typedef unsigned char uchar;
typedef unsigned int uchar16;

code unsigned char duan_led[11] = {0x3f,0x06,0x5b ,0x4f ,0x66 ,0x6d ,0x7d,0x07,0x7f,0x6f,0x40}; 	//��ѡLED��,��11λ�Ƿָ���
code unsigned char wei_led[8]  = {0x80,0x40,0x20,0x10,0x08,0x04,0x02,0x01};					//λѡ������


uchar k;		//��ѯ�����
uchar16  counter = 0; //����
				
char Led_eight[8] = {0x00,0x00,0x0A,0x00,0x00,0x0A,0x00,0x00};	//��ǰ��ʱ����
//		    ʱ       -      ��      -       ��


/****************************************************************************/
//����˼·��
/*
	����ԭ��ͼ��֪ ����ܵĶ�ѡ��λѡ �ɴ�д��duan_led��wei_led ����
	ʹ�� ��ʱ��1 ÿ����Ľ�����ѯLed_eight���飬ÿ��Ե�ǰ����ֵ���з�λ����
	�����ж��Ƿ��н��ƣ��������ϼ���1���Դ����ơ���ʱ��ʹ��12Сʱ�Ƶģ����ɳ���12�����Ӳ��ɵ���60
	������ʱ�ӣ�����ת������������Led_eight���飬��Led_eight����ֵ���޸�ʱ�������жϴ�����
	��ʾ���ݣ�������޸�ʱ�ӡ�
	�����뺯���ж��������е�ֵ�Ƿ�Ϊ�����趨ʱ�ӣ���������ӹ���
	Led_eight ��������±������������ȥ���ú���light������duan_led��wei_led��
	Led_eight ��������ʾ��ǰ��ʱ���룬0-1��ʾ ʱ��2 ��ʾ����'-' ��3-4��ʾ�֣�5��ʾ����-��6-7��ʾ��

*/




/*
	���ܣ���ʱ����
	������i  ����ms
	���أ���
*/
void delay_1ms(uchar16 i)
{
	uchar j;
	for(;i>0;i--)
		for(j=0;j<110;j++);
	
}

/*
	���ܣ���������Ӧ50ms
	��������
	���أ���
*/
void beeSound()
{
	bee = 0;
	delay_1ms(50);
	bee = 1;
}

uchar flag_aram = 0;		//���ӱ��

/*
	���ܣ��趨����
	��������
	���أ���
*/
void alarm_clock()
{
	if(Led_eight[0] == 0 && Led_eight[1] == 8 && Led_eight[3]== 3 && Led_eight[4] == 0 )
	{
		flag_aram = 1;	//����������ʱ����flag�� 1
		bee = 0;
	}else
	{
		if(flag_aram)		//��flagΪ1ʱ����ʾ���������һ���ˣ���ô��ʱ���Ѿ�����Ԥ��ֵʱ������Ҫ�����ӹر�
		{
			flag_aram = 0;
			bee = 1;
		}			
			
	}
	
}


/*
	���ܣ��趨����
	��������
	���أ���
*/
void send_data()
{
	unsigned char i;
	HC595_RCK_LED = 0;	//����洢������ʱ����
	for(i=0;i<16;i++)
	{
		HC595_SCK_LED = 0;
		HC595_DATA_LED = 0;			
		HC595_SCK_LED = 1;	
	}
	
	HC595_RCK_LED = 1;	
	
}

/*
	���ܣ���ʼ����ʱ��1
	��������
	����ֵ����
*/
void timer_init()
{
	TMOD &=0x0F;
	TMOD |=0x10;
	TH1 = 0xFC;	 
	TL1 = 0x66;
	TR1 = 1;	//������ʱ��
}


/*
	���ܣ���ʼ���ж�
	��������
	����ֵ����

*/
void inter_init()
{
	ET1 = 1;
	EA = 1;
}



/*
	���ܣ��������������
	������u5 u5��λ�Ĵ���(74HC595)
	      u10 u10�Ĵ���(74HC595)
	���أ���
*/
void light(unsigned char  u5,unsigned char u10)
{
	unsigned char j,k=0x01;
	HC595_RCK = 0;
	k=0x01;
	for(j=0;j<8;j++)	//���8��ʾ��ѡ��8
	{
		HC595_SCK = 0;
		if(0 != (u5&0x80))
		{
			HC595_DATA = 1;		//Ϊ1��
		}else
		{
			HC595_DATA = 0;
		}
		u5<<=1;
		HC595_SCK = 1;
	}
		
	
	for(j=0;j<8;j++)	//���8��ʾλѡ��8
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
	���ܣ���ת�������ж�
	��������
	���أ���
*/
void encoder_data()
{
	//ȷ���Ƿ���ת��
	if((1 == encoder_portA) && (1 == encoder_portB))
	{
		rotary_flag=1;
	}
	//�жϷ���
	if((encoder_portA ^ encoder_portB) && (1==rotary_flag))
	{
		sign++;
		if(sign>2)			//ȥ��
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
	
	
	//����� ����� ,ֻ�Էּ�ʱ��
	if((0 == encoder_portA) &&  (0== encoder_portB) && foreward)
	{
		foreward = 0;
		reverse =0;
		rotary_flag = 0;
		
		if(!encoder_portsd)//Сʱ
		{		
			if(Led_eight[0] == 1)
			{
				if(Led_eight[1] <2)
					Led_eight[1]++;	//ֻ�Ը�λ����
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
			Led_eight[4]++;		//ֻ�Է���++
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
	
	//����� 
	if((0 == encoder_portA) &&  (0==encoder_portB) && reverse)
	{
		foreward = 0;
		reverse =0;
		rotary_flag = 0;
		
		if(!encoder_portsd)					//Сʱ
		{	
			//��λʮλ����Ϊ0 ��ô�Ϳ��Լ�
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
					else	//��ʮλΪ0ʱ���Ͳ��ܼ���
					{
						Led_eight[1] = 0;
					}
				}
			}
			
						
		}
		else								//����
		{	
			//��λʮλ����Ϊ0 ��ô�Ϳ��Լ�
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
					else	//��ʮλΪ0ʱ���Ͳ��ܼ���
					{
						Led_eight[4] = 0;
					}
				}
			}

		}	
		
		/*ֻҪ��ת�ͽ�*/
		beeSound();
	}
	

}



void main()
{
	send_data();//�ر�LED��

	timer_init();
	inter_init();
	while(1)
	{
		encoder_data();
		
	}
}


//�жϴ������
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
		Led_eight[7] = sec%10;			//��
		Led_eight[6] = sec/10%10;		//ʮ��
		if(Led_eight[6] == 6)			//��6��һ
		{
			Led_eight[6] = 0x00;			
			sec=0;					//����(��)
			
			Led_eight[4] += 1;			//����+1
			if(Led_eight[4] == 10)		//��ʮ��һ
			{
				Led_eight[4] = 0x00;		//����(��)
				Led_eight[3] += 1;		//ʮ��
				
				if(Led_eight[3] == 6)	//��6��һ
				{
					Led_eight[3] = 0x00;	//����(ʮ��)

					Led_eight[1] += 1;	//ʱ
					
					if(Led_eight[0]==1 && Led_eight[1]==3)	//��СʱʮλΪ1ʱ�Ҹ�λΪ3��ʾ ������ʮ�����ˣ�Ҫ��СʱΪ����
					{
						Led_eight[0] = 0x00;
						Led_eight[1] = 0x01;
					}

					if(Led_eight[1]==10)	//��ʮ��һ
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
