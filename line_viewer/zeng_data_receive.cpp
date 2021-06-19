#include "zeng_data_receive.h"

#include <qdebug.h>

using namespace std;


LPCWSTR Com_name[10] = {L"com0",
						L"com1",
						L"com2",
						L"com3",
						L"com4",
						L"com5",
						L"com6",
						L"com7",
						L"com8",
						L"com9"};
HANDLE hComm = NULL;
HANDLE receive_task_handle = NULL;

DATA_PROTOCOL communication_data;


int16_t Zeng_input_usart_init(uint8_t com_num, uint32_t baudrate)
{
	INT16 ret = 0;
	DCB dcb;

	hComm = CreateFile(Com_name[com_num]/*TEXT("com5")*/, \
		GENERIC_READ | GENERIC_WRITE, \
		0, \
		NULL, \
		OPEN_EXISTING, \
		FILE_ATTRIBUTE_NORMAL/*|FILE_FLAG_OVERLAPPED*/, \
		NULL);
	if (INVALID_HANDLE_VALUE == hComm)
	{
		ret = -1;
	} else {
		COMMTIMEOUTS TimeOuts;
		TimeOuts.ReadIntervalTimeout = 0;
		TimeOuts.ReadTotalTimeoutMultiplier = 1;
		TimeOuts.ReadTotalTimeoutConstant = 1;
		TimeOuts.WriteTotalTimeoutMultiplier = 500;
		TimeOuts.WriteTotalTimeoutConstant = 2000;
		SetCommTimeouts(hComm, &TimeOuts);

		GetCommState(hComm, &dcb);
		dcb.BaudRate = baudrate;
		dcb.ByteSize = 8;
		dcb.StopBits = ONESTOPBIT;

		SetCommState(hComm, &dcb);
	}

	return ret;
}

WINBOOL Zeng_input_usart_close(void)
{
	return CloseHandle(hComm);
}

int Zeng_uart_read_data(uint16_t cnt, char *buff)
{
	int ret = 0;
	DWORD read_cnt;

	ReadFile(hComm, buff, cnt, &read_cnt, NULL);
	ret = read_cnt;
	return ret;
}

int Zeng_get_head(char *buf, int (*f)(uint16_t, char*), char *header, uint head_lenth)
{
	int ret = 0;
	uint i = 0;
	uint time_out = 100;
	while(time_out)
	{
		f(1,&buf[i]);
		if(buf[i]==header[i])
		{
			i++;
		}else{
			i = 0;
		}
		if(i>=head_lenth)
		{
			//ret = 0;
			break;
		}
		time_out--;
	}
	if(0 == time_out)
	{
		ret = 1;
	}
	return ret;
}


uint8_t Zeng_cal_CheckSum(uint8_t *start, uint8_t len)
{
	uint8_t sum = 0;
	uint8_t i = 0;
	for(i=0; i<len; i++)
	{
		sum += start[i];
	}
	return sum;
}

int Zeng_receive_data(char *buff, DATA_PROTOCOL *protocol, int (*f)(uint16_t, char*))
{
	int ret = 0;
	uint data_cnt = 0;

	//read head
	ret = Zeng_get_head(buff, f, (char*)protocol->head, protocol->head_length);
	if(ret != 0)
	{
		//get header fail!
		qDebug()<<"get header fail!"<<endl;
		ret = -1;
	}else{
		//get last data
		//qDebug()<<"get header successful!"<<endl;


		data_cnt = f(protocol->data_length-protocol->head_length, &buff[protocol->head_length]);
		if(data_cnt != (protocol->data_length-protocol->head_length))
		{
			ret = 1;
		}
	}

	return ret;
}


DWORD WINAPI Zeng_data_receive_task(LPVOID lpParam)
{
	DATA_PROTOCOL *data = &communication_data;
	int ret = 0;
	int temp = 0;
	uint8_t check_sum = 0;
	lpParam = lpParam;
	while(1)
	{
		//Sleep(5);
		ret = Zeng_receive_data((char*)data->raw_data, data, Zeng_uart_read_data);
		if(ret != 0)
		{
			Sleep(1);
			qDebug()<<"read data fail!!!"<<endl;
		}else{
			check_sum = Zeng_cal_CheckSum((uint8_t*)data->raw_data, data->data_length-2);
			if(check_sum == data->raw_data[data->data_length-1])
			{
				for(int i=0; i<data->data_cnt; i++)
				{
					temp = 0;
					for(int j=data->single_data_size-1; j>=0; j--)
					{
						temp = ((temp<<8)|data->raw_data[data->data_start+i*data->single_data_size+j]);
					}
					data->data[i] = temp*data->data_scale;
				}
			}else{
				qDebug()<<"real:"<<(short)data->raw_data[data->data_length-1]<<", cal:"<<(short)check_sum<<endl;
			}

		}
	}
}

int Zeng_data_suspend_task(void)
{
	int ret = 0;
	if(NULL == receive_task_handle)
	{
		ret = 1;
	}else{
		if(CloseHandle(hComm))
		{
			ret = 0;
			hComm = NULL;

		}else{
			ret = 2;
		}
		SuspendThread(receive_task_handle);
	}
	return ret;
}

int Zeng_data_ready_task(uint8_t port_num, uint32_t baudrate)
{
	int ret = 0;
	if(NULL == receive_task_handle)
	{
		ret = 1;
	}else{
		//Port init
		ret = Zeng_input_usart_init(port_num, baudrate);
		if(0 == ret)
		{
			//qDebug()<<"com" <<(short)port_num << " baudrate:" << baudrate << " Zeng_input_usart_init successful!" << endl;
			ResumeThread(receive_task_handle);
		}else{
			//qDebug()<<"Zeng_input_usart_init fail!" << endl;
		}

	}
	return ret;
}

int Zeng_data_receive_init(void)
{
	int ret = 0;
	receive_task_handle = CreateThread(
									NULL,
									0,
									Zeng_data_receive_task,
									NULL,
									0,
									NULL);
	Zeng_data_suspend_task();

	return ret;
}

