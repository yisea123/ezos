#include "usmart.h"
#include "usart.h"
#include "sys.h"	      
//////////////////////////////////////////////////////////////////////////////////	 
//本程序只供学习使用，未经作者许可，不得用于其它任何用途
//ALIENTEK STM32开发板	   
//正点原子@ALIENTEK
//技术论坛:www.openedv.com
//创建日期:2011/6/13
//版本：V2.1
//版权所有，盗版必究。
//Copyright(C) 正点原子 2011-2021
//All rights reserved
//********************************************************************************
//升级说明
//V1.4
//增加了对参数为string类型的函数的支持.适用范围大大提高.
//优化了内存占用,静态内存占用为79个字节@10个参数.动态适应数字及字符串长度
//V2.0 
//1,修改了list指令,打印函数的完整表达式.
//2,增加了id指令,打印每个函数的入口地址.
//3,修改了参数匹配,支持函数参数的调用(输入入口地址).
//4,增加了函数名长度宏定义.	
//V2.1 20110707		 
//1,增加dec,hex两个指令,用于设置参数显示进制,及执行进制转换.
//注:当dec,hex不带参数的时候,即设定显示参数进制.当后跟参数的时候,即执行进制转换.
//如:"dec 0XFF" 则会将0XFF转为255,由串口返回.
//如:"hex 100" 	则会将100转为0X64,由串口返回
//2,新增usmart_get_cmdname函数,用于获取指令名字.
/////////////////////////////////////////////////////////////////////////////////////
//系统命令
u8 *sys_cmd_tab[]=
{
	"?",
	"help",
	"list",
	"id",
	"hex",
	"dec",
};	    
//处理系统指令
//0,成功处理;其他,错误代码;
u8 usmart_sys_cmd_exe(u8 *str)
{
	u8 i;
	u8 sfname[MAX_FNAME_LEN];//存放本地函数名
	u8 pnum;
	u32 res;

	usmart_get_cmdname(str,sfname,&i);//得到指令及指令长度
	str+=i;					 			    
	for(i=0;i<6;i++)//支持6个系统指令
	{
		if(usmart_strcmp(sfname,sys_cmd_tab[i]))break;
	}
	switch(i)
	{					   
		case 0:
		case 1://帮助指令
			printf("\r\n");
#if USMART_USE_HELP
			printf("------------------------USMART V2.1------------------------ \r\n");
			printf("    USMART是由ALIENTEK开发的一个灵巧的串口调试互交组件,通过 \r\n");
			printf("它,你可以通过串口助手调用程序里面的任何函数,并执行.因此,你可\r\n");
			printf("以随意更改函数的输入参数(支持数字(10/16进制)、字符串、函数入\r\n");	  
			printf("口地址等作为参数),单个函数最多支持10个输入参数,并支持函数返 \r\n");
			printf("回值显示.新增参数显示进制设置功能,新增进制转换功能.\r\n");
			printf("技术支持:www.openedv.com\r\n");
			printf("USMART有6个系统命令:\r\n");
			printf("?:    获取帮助信息\r\n");
			printf("help: 获取帮助信息\r\n");
			printf("list: 可用的函数列表\r\n\n");
			printf("id:   可用函数的ID列表\r\n\n");
			printf("hex:  参数16进制显示,后跟空格+数字即执行进制转换\r\n\n");
			printf("dec:  参数10进制显示,后跟空格+数字即执行进制转换\r\n\n");
			printf("请按照程序编写格式输入函数名及参数并以回车键结束.\r\n");    
			printf("--------------------------ALIENTEK------------------------- \r\n");
#else
			printf("指令失效\r\n");
#endif
			break;
		case 2://查询指令
			printf("\r\n");
			printf("-------------------------函数清单--------------------------- \r\n");
			for(i=0;i<usmart_dev.fnum;i++)printf("%s\r\n",usmart_dev.funs[i].name);
			printf("\r\n");
			break;	 
		case 3://查询ID
			printf("\r\n");
			printf("-------------------------函数 ID --------------------------- \r\n");
			for(i=0;i<usmart_dev.fnum;i++)
			{
				usmart_get_fname((u8*)usmart_dev.funs[i].name,sfname,&pnum);//得到本地函数名 
				printf("%s id is:\r\n0X%08X\r\n",sfname,usmart_dev.funs[i].func); //显示ID
			}
			printf("\r\n");
			break;
		case 4:
			printf("\r\n");
			usmart_get_aparm(str,sfname,&i);
			if(i==0)//参数正常
			{
				i=usmart_str2num(sfname,&res);	   	//记录该参数	
				if(i==0)						  	//进制转换功能
				{
					printf("HEX:0X%X\r\n",res);	   	//转为16进制
				}else if(i!=4)return USMART_PARMERR;//参数错误.
				else 				   				//参数显示设定功能
				{
					printf("16进制参数显示!\r\n");
					usmart_dev.sptype=SP_TYPE_HEX;  
				}

			}else return USMART_PARMERR;			//参数错误.
			printf("\r\n"); 
			break;
		case 5:
			printf("\r\n");
			usmart_get_aparm(str,sfname,&i);
			if(i==0)//参数正常
			{
				i=usmart_str2num(sfname,&res);	   	//记录该参数	
				if(i==0)						   	//进制转换功能
				{
					printf("DEC:%lu\r\n",res);	   	//转为10进制
				}else if(i!=4)return USMART_PARMERR;//参数错误.
				else 				   				//参数显示设定功能
				{
					printf("10进制参数显示!\r\n");
					usmart_dev.sptype=SP_TYPE_DEC;  
				}

			}else return USMART_PARMERR;			//参数错误. 
			printf("\r\n"); 
			break;	  
		default://非法指令
			return USMART_FUNCERR;
	}
	return 0;
}
////////////////////////////////////////////////////////////////////////////////////////
//下面这两个函数,非USMART函数,放到这里,仅仅方便移植. 
//定时器2中断服务程序	 
void TIM2_IRQHandler(void)
{ 		    		  			    
	if(TIM2->SR&0X0001)//溢出中断
	{
		usmart_dev.scan();//执行usmart扫描			    				   				     	    	
	}				   
	TIM2->SR&=~(1<<0);//清除中断标志位 	    
}
//使能定时器2,使能中断.
void Timer2_Init(u16 arr,u16 psc)
{
	RCC->APB1ENR|=1<<0;//TIM2时钟使能    
 	TIM2->ARR=arr;  //设定计数器自动重装值  
	TIM2->PSC=psc;  //预分频器7200,得到10Khz的计数时钟
	//这两个东东要同时设置才可以使用中断
	TIM2->DIER|=1<<0;   //允许更新中断				
	TIM2->DIER|=1<<6;   //允许触发中断
		  							    
	TIM2->CR1|=0x01;    //使能定时器2
  	MY_NVIC_Init(3,3,TIM2_IRQChannel,2);//抢占3，子优先级3，组2(组2中优先级最低的)									 
}
////////////////////////////////////////////////////////////////////////////////////////
//初始化串口控制器
void usmart_init(void)
{
	//必须使能串口中断接收
	uart_init(72,9600); 	//串口1初始化 
	Timer2_Init(1000,7199);	//7200分频,时钟为10K ,100ms中断一次
}		
//从str中获取函数名,id,及参数信息
//*str:字符串指针.
//返回值:0,识别成功;其他,错误代码.
u8 usmart_cmd_rec(u8*str) 
{
	u8 sta,i;//状态	 
	u8 rpnum,spnum;
	u8 rfname[MAX_FNAME_LEN];//暂存空间,用于存放接收到的函数名  
	u8 sfname[MAX_FNAME_LEN];//存放本地函数名
	sta=usmart_get_fname(str,rfname,&rpnum);//得到接收到的数据的函数名及参数个数	  
	if(sta)return sta;//错误
	for(i=0;i<usmart_dev.fnum;i++)
	{
		sta=usmart_get_fname((u8*)usmart_dev.funs[i].name,sfname,&spnum);//得到本地函数名及参数个数
		if(sta)return sta;//本地解析有误	  
		if(usmart_strcmp(sfname,rfname))//相等
		{
			if(spnum>rpnum)return USMART_PARMERR;//参数错误(输入参数比源函数参数少)
			usmart_dev.id=i;//记录函数ID.
			break;//跳出.
		}	
	}
	if(i==usmart_dev.fnum)return USMART_NOFUNCFIND;	//未找到匹配的函数
 	sta=usmart_get_fparam(str,&i);					//得到函数参数个数	
	if(sta)return sta;								//返回错误
	usmart_dev.pnum=i;								//参数个数记录
    return USMART_OK;
}
//usamrt执行函数
//该函数用于最终执行从串口收到的有效函数.
//最多支持10个参数的函数,更多的参数支持也很容易实现.不过用的很少.一般5个左右的参数的函数已经很少见了.
//该函数会在串口打印执行情况.以:"函数名(参数1，参数2...参数N)=返回值".的形式打印.
//当所执行的函数没有返回值的时候,所打印的返回值是一个无意义的数据.
void usmart_exe(void)
{
	u8 id,i;
	u32 res;		   
	u32 temp[MAX_PARM];//参数转换,使之支持了字符串 
	u8 sfname[MAX_FNAME_LEN];//存放本地函数名
	u8 pnum;
	id=usmart_dev.id;
	if(id>=usmart_dev.fnum)return;//不执行.
	usmart_get_fname((u8*)usmart_dev.funs[id].name,sfname,&pnum);//得到本地函数名,及参数个数 
	printf("\r\n%s(",sfname);//输出正要执行的函数名
	for(i=0;i<pnum;i++)//输出参数
	{
		if(usmart_dev.parmtype&(1<<i))//参数是字符串
		{
			printf("%c",'"');			 
			printf("%s",usmart_dev.parm+usmart_get_parmpos(i));
			printf("%c",'"');
			temp[i]=(u32)&(usmart_dev.parm[usmart_get_parmpos(i)]);
		}else						  //参数是数字
		{
			temp[i]=*(u32*)(usmart_dev.parm+usmart_get_parmpos(i));
			if(usmart_dev.sptype==SP_TYPE_DEC)printf("%lu",temp[i]);//10进制参数显示
			else printf("0X%X",temp[i]);//16进制参数显示 	   
		}
		if(i!=pnum-1)printf(",");
	}
	printf(")");
	switch(usmart_dev.pnum)
	{
		case 0://无参数(void类型)											  
			res=(*(u32(*)())usmart_dev.funs[id].func)();
			break;
	    case 1://有1个参数
			res=(*(u32(*)())usmart_dev.funs[id].func)(temp[0]);
			break;
	    case 2://有2个参数
			res=(*(u32(*)())usmart_dev.funs[id].func)(temp[0],temp[1]);
			break;
	    case 3://有3个参数
			res=(*(u32(*)())usmart_dev.funs[id].func)(temp[0],temp[1],temp[2]);
			break;
	    case 4://有4个参数
			res=(*(u32(*)())usmart_dev.funs[id].func)(temp[0],temp[1],temp[2],temp[3]);
			break;
	    case 5://有5个参数
			res=(*(u32(*)())usmart_dev.funs[id].func)(temp[0],temp[1],temp[2],temp[3],temp[4]);
			break;
	    case 6://有6个参数
			res=(*(u32(*)())usmart_dev.funs[id].func)(temp[0],temp[1],temp[2],temp[3],temp[4],\
			temp[5]);
			break;
	    case 7://有7个参数
			res=(*(u32(*)())usmart_dev.funs[id].func)(temp[0],temp[1],temp[2],temp[3],temp[4],\
			temp[5],temp[6]);
			break;
	    case 8://有8个参数
			res=(*(u32(*)())usmart_dev.funs[id].func)(temp[0],temp[1],temp[2],temp[3],temp[4],\
			temp[5],temp[6],temp[7]);
			break;
	    case 9://有9个参数
			res=(*(u32(*)())usmart_dev.funs[id].func)(temp[0],temp[1],temp[2],temp[3],temp[4],\
			temp[5],temp[6],temp[7],temp[8]);
			break;
	    case 10://有10个参数
			res=(*(u32(*)())usmart_dev.funs[id].func)(temp[0],temp[1],temp[2],temp[3],temp[4],\
			temp[5],temp[6],temp[7],temp[8],temp[9]);
			break;
	}
	if(usmart_dev.sptype==SP_TYPE_DEC)printf("=%lu;\r\n",res);//输出执行结果(10进制参数显示)
	else printf("=0X%X",res);//输出执行结果(16进制参数显示)	   
	
}
//usmart扫描函数
//通过调用该函数,实现usmart的各个控制.该函数需要每隔一定时间被调用一次
//以及时执行从串口发过来的各个函数.
//本函数可以在中断里面调用,从而实现自动管理.
//如果非ALIENTEK用户,则USART_RX_STA和USART_RX_BUF[]需要用户自己实现
void usmart_scan(void)
{
	u8 sta,len;
	if(USART_RX_STA&0x8000)//串口接收完成？
	{					   
		len=USART_RX_STA&0x3fff;	//得到此次接收到的数据长度
		USART_RX_BUF[len]='\0';	//在末尾加入结束符. 
		sta=usmart_dev.cmd_rec(USART_RX_BUF);//得到函数各个信息
		if(sta==0)usmart_dev.exe();//执行函数
		else 
		{  
			len=usmart_sys_cmd_exe(USART_RX_BUF);
			if(len!=USMART_FUNCERR)sta=len;
			if(sta)
			{
				switch(sta)
				{
					case USMART_FUNCERR:
						printf("函数错误!\r\n");   			
						break;	
					case USMART_PARMERR:
						printf("参数错误!\r\n");   			
						break;				
					case USMART_PARMOVER:
						printf("参数太多!\r\n");   			
						break;		
					case USMART_NOFUNCFIND:
						printf("未找到匹配的函数!\r\n");   			
						break;		
				}
			}
		}
		USART_RX_STA=0;//状态寄存器清空	    
	}
}		 














