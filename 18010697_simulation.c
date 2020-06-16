/*
-2020 ��������� ����4 : CRC �����ϱ� - �ùķ��̼�
-�ۼ��� : ����������Ű��а� 18010697 ���ظ�

-CRC �߰�Ȯ�� ������ ���� �ùķ��̼� ������ �ڵ带 ���� ¥����.
--printf�� scanf�� ��� �ִ� �κ� ���� �����ϰ�, ���� ���� �ɷ� �����ϴ� �κ� ������. ���� �˰����� �������� �ʾ���.
--�����ϴ� ���� �������� ũ�⳪ CRC ������ �����ϰ� �ʹٸ� < parameter > �� ��ũ�λ���� �ǵ帱 ��.

-���õ� CRC �ڵ� [���� static ������ ����]
--CRC_8  : 1_0000_0111
--CRC_16 : 1_0001_0000_0010_0001
--CRC_32 : 1_0000_0100_1100_0001_0001_1101_1011_0111
*/
#define _CRT_SECURE_NO_WARNINGS

/* < parameter > */
#define SIZE_OF_DATA 4
#define CRC_TYPE 8


#include<stdint.h>	// uint8_t ��
#include<stdio.h>	// �⺻ ����� �Լ�
#include<stdlib.h>	// �����Ҵ�, ���� ����
#include<time.h>	// ���� ���� �õ�

/* �����͸� �ٷ�� ���� �ڷ��� */
typedef struct BinaryData
{
	int length;					// # of bits
	uint8_t* data;				// data(uint8_t) array header
	char hex[SIZE_OF_DATA*3];				// HEX expression of data. maximum(string_length)=length(AA:BB:CC:DD:EE)= 15
} BD;

/* CRC Code */
const uint8_t CRC_8[9] = { 1, 0,0,0,0, 0,1,1,1 };
const uint8_t CRC_16[17] = { 1, 0,0,0,1, 0,0,0,0, 0,0,1,0, 0,0,0,1 };
const uint8_t CRC_32[33] = { 1, 0,0,0,0, 0,1,0,0, 1,1,0,0, 0,0,0,1, 0,0,0,1, 1,1,0,1, 1,0,1,1, 0,1,1,1 };

/* Functions */
void initBD(BD*, int length);
void getBinaryfromHex(BD*);						// includes memory allocation
void printBinary(BD*);
void getRemainder(BD* Message, int CRC_type);
void makeNoiseOnData(BD*, int);						// includes console input
int  checkError(BD* Message, int CRC_type);

void createRandomData(int size, BD*);			// size: # of Bytes

/* Main */
int main() {
	BD Data, Message;
	int size_of_data = SIZE_OF_DATA, CRC_type = CRC_TYPE;		// CRC ������ �����͸� �ٲٰ� ���� ��.
	int num_of_error;
	FILE* fp = fopen("crc_simulate.txt", "w");
	// num of error, accuracy ������ ����� ����.

	srand(time(NULL));	// ���� �õ� ����

	for (int i = 0; i < 1000; i++) {
		int wrong_cnt = 0;
		fprintf(fp, "%d,", i);
		num_of_error = i;
		// [�⺻�� 1. ���� �Է¹ޱ�] & [Ȯ���� 1. ���� �����ϱ�] 
		for (int j = 0; j < 1000; j++) {

			//size_of_data = rand() % 4 + 1;
			createRandomData(size_of_data, &Data); // �������� ������ ����

			// [�⺻�� 2. ������(Remainder)�� ������ ���ۿ�(Message)]

			// ������ ���ۿ� ���� & �ʱ�ȭ�ϱ�
			initBD(&Message, Data.length + CRC_type);
			for (int i = 0; i < Data.length; i++)				Message.data[i] = Data.data[i];
			for (int i = Data.length; i < Message.length; i++)	Message.data[i] = 0;

			// ������ ���ۿ��� CRC code�� ������
			getRemainder(&Message, CRC_type); // Message�� shift & XOR
			for (int i = 0; i < Data.length; i++)	Message.data[i] = Data.data[i]; // ������ �۾� �ϴ��� �Ѽյ� Data �κ� ����

			// [�⺻�� 3. & Ȯ���� 3.  Noise �����]
			makeNoiseOnData(&Message, num_of_error);
			// [�⺻�� 4. CRC ��� �˻��ϱ�]
			getRemainder(&Message, CRC_type);

			if (!checkError(&Message, CRC_type))
			{
				if (num_of_error != 0)
				{
					wrong_cnt += 1;
				}
			}
			else
			{
				if (num_of_error == 0)
				{
					wrong_cnt += 1;
				}
			}

			// memory deallocation
			free(Data.data);
			free(Message.data);
		}
		printf("error_bit = %d \twrong_cnt = %d\n", num_of_error, wrong_cnt);
		fprintf(fp, "%d\n", 1000 - wrong_cnt);
	}
	fclose(fp);
	return 0;
}


void initBD(BD* B, int length)
{
	B->length = length;
	B->data = (uint8_t*)malloc(sizeof(uint8_t) * (B->length));
}

// Hex �����͸� ǥ���� ���ڿ��� Binary �������� �ٲپ� �����Ѵ�.
void getBinaryfromHex(BD* B)
{
	int i = 0, cnt = 0, size;
	int index = 0;

	while (B->hex[index] != '\0')
	{
		if (B->hex[index] == ':') cnt++;
		index++;
	}
	size = cnt + 1;

	initBD(B, size * 8);

	index = 0;
	while (B->hex[i] != '\0')
	{
		// 0~f�� 0000~1111���� ��Ī�ؼ� B->data�� 4bit�� ä�� �ִ� �κ�.
		// �׳� scanf���� char�� ���� ���� Hex �����ͷμ� �е��� ���������� ���Ұڴٴ� �ƽ����� ����.
		switch (B->hex[i])
		{
		case ':':
			break;
		case '0':
			B->data[4 * index] = 0; B->data[4 * index + 1] = 0; B->data[4 * index + 2] = 0; B->data[4 * index + 3] = 0;
			index++;
			break; // 0000
		case '1':
			B->data[4 * index] = 0; B->data[4 * index + 1] = 0; B->data[4 * index + 2] = 0; B->data[4 * index + 3] = 1;
			index++;
			break; // 0001
		case '2':
			B->data[4 * index] = 0; B->data[4 * index + 1] = 0; B->data[4 * index + 2] = 1; B->data[4 * index + 3] = 0;
			index++;
			break; // 0010
		case '3':
			B->data[4 * index] = 0; B->data[4 * index + 1] = 0; B->data[4 * index + 2] = 1; B->data[4 * index + 3] = 1;
			index++;
			break; // 0011
		case '4':
			B->data[4 * index] = 0; B->data[4 * index + 1] = 1; B->data[4 * index + 2] = 0; B->data[4 * index + 3] = 0;
			index++;
			break; // 0100
		case '5':
			B->data[4 * index] = 0; B->data[4 * index + 1] = 1; B->data[4 * index + 2] = 0; B->data[4 * index + 3] = 1;
			index++;
			break; // 0101
		case '6':
			B->data[4 * index] = 0; B->data[4 * index + 1] = 1; B->data[4 * index + 2] = 1; B->data[4 * index + 3] = 0;
			index++;
			break; // 0110
		case '7':
			B->data[4 * index] = 0; B->data[4 * index + 1] = 1; B->data[4 * index + 2] = 1; B->data[4 * index + 3] = 1;
			index++;
			break; // 0111
		case '8':
			B->data[4 * index] = 1; B->data[4 * index + 1] = 0; B->data[4 * index + 2] = 0; B->data[4 * index + 3] = 0;
			index++;
			break; // 1000
		case '9':
			B->data[4 * index] = 1; B->data[4 * index + 1] = 0; B->data[4 * index + 2] = 0; B->data[4 * index + 3] = 1;
			index++;
			break; // 1001
		case 'A':
			B->data[4 * index] = 1; B->data[4 * index + 1] = 0; B->data[4 * index + 2] = 1; B->data[4 * index + 3] = 0;
			index++;
			break; // 1010
		case 'B':
			B->data[4 * index] = 1; B->data[4 * index + 1] = 0; B->data[4 * index + 2] = 1; B->data[4 * index + 3] = 1;
			index++;
			break; // 1011
		case 'C':
			B->data[4 * index] = 1; B->data[4 * index + 1] = 1; B->data[4 * index + 2] = 0; B->data[4 * index + 3] = 0;
			index++;
			break; // 1100
		case 'D':
			B->data[4 * index] = 1; B->data[4 * index + 1] = 1; B->data[4 * index + 2] = 0; B->data[4 * index + 3] = 1;
			index++;
			break; // 1101
		case 'E':
			B->data[4 * index] = 1; B->data[4 * index + 1] = 1; B->data[4 * index + 2] = 1; B->data[4 * index + 3] = 0;
			index++;
			break; // 1110
		case 'F':
			B->data[4 * index] = 1; B->data[4 * index + 1] = 1; B->data[4 * index + 2] = 1; B->data[4 * index + 3] = 1;
			index++;
			break; // 1111
		default:
			break;
		}
		i++;
	}
}

void getRemainder(BD* Message, int CRC_type) {
	// Message�� �� �κ��� �������� ��ħ
	int i;
	int data_length = Message->length - CRC_type;

	if (CRC_type == 8)
	{
		for (i = 0; i < data_length; i++)
		{
			if (Message->data[i] == 1)
			{
				// shift and XOR
				for (int j = 0; j <= 8; j++)
				{
					Message->data[i + j] = Message->data[i + j] ^ CRC_8[j];
				}
			}
		}
	}
	else if (CRC_type == 16)
	{
		for (i = 0; i < data_length; i++)
		{
			if (Message->data[i] == 1)
			{
				// shift and XOR
				for (int j = 0; j <= 16; j++)
				{
					Message->data[i + j] = Message->data[i + j] ^ CRC_16[j];
				}
			}
		}
	}
	else if (CRC_type == 32)
	{
		for (i = 0; i < (Message->length - CRC_type); i++)
		{
			if (Message->data[i] == 1)
			{
				// shift and XOR
				for (int j = 0; j <= 32; j++)
				{
					Message->data[i + j] = Message->data[i + j] ^ CRC_32[j];
				}
			}
		}
	}
	else printf("<ERROR> getRemainder�� �߸��� CRC_type �Էµ�\n");
}

int  checkError(BD* Message, int CRC_type)
{
	for (int i = (Message->length - 1); i >= (Message->length - CRC_type); i--)
	{
		if (Message->data[i] == 1) return 1;
	}
	return 0;
}

void makeNoiseOnData(BD* Message, int num_of_error)
{
	for (int i = 0; i < num_of_error; i++)
	{
		int position = rand() % (Message->length);
		Message->data[position] = !Message->data[position];
	}
}



void createRandomData(int size, BD* B)
{
	int hexnum, i = 0;
	B->length = size * 8;
	size *= 2;
	while (size > 0)
	{
		if (i % 3 == 2)
		{
			B->hex[i++] = ':'; continue;
		}
		hexnum = rand() % 16;
		if (hexnum < 10) B->hex[i++] = '0' + hexnum;
		else B->hex[i++] = (hexnum - 10) + 'A';
		size--;
	}
	B->hex[i] = '\0';
	getBinaryfromHex(B);
}
