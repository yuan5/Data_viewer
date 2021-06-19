#ifndef ZENG_DATA_RECEIVE_H
#define ZENG_DATA_RECEIVE_H

#include <QMainWindow>
#include <stdio.h>
#include <windows.h>

typedef struct DATA_PROTOCOL_{
	uint head_length;
	uint data_length;
	float data_scale;
	uint8_t head[10];
	uint8_t raw_data[100];
	double data[10];
	int data_cnt;
	bool is_check;
	uint8_t Check_sum;
	uint8_t single_data_size;
	uint8_t data_start;
	uint check_locate;
}DATA_PROTOCOL;

extern DATA_PROTOCOL communication_data;

int Zeng_data_receive_init(void);

int16_t Zeng_input_usart_init(uint8_t com_num, uint32_t baudrate);
int Zeng_data_ready_task(uint8_t port_num, uint32_t baudrate);
int Zeng_data_suspend_task(void);
WINBOOL Zeng_input_usart_close(void);

#endif // ZENG_DATA_RECEIVE_H
