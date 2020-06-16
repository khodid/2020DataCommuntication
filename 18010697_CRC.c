/*
-2020 ��������� ����4 : CRC �����ϱ�
-�ۼ��� : ����������Ű��а� 18010697 ���ظ�


-����
--��ó��: main �Լ� �⺻�� / Ȯ���� ������
--�ڷ���: uint8_t �迭 (�� element�� 0, 1�� ǥ��, ��Ʈ���� ������ �迭 �������� ��ü)
--�Լ�:
---input / output  HEX(AA:BB:CC:DD�� ����) ���� ����� ���.
---CRC�� ���� ���� ���ڿ� �����ϴ� �Լ�
---noise ���� �Լ�
---(Ȯ����)���� Data ���� �Լ�

-���õ� CRC �ڵ� [���� static ������ ����]
--CRC_8  : 1_0000_0111
--CRC_16 : 1_0001_0000_0010_0001
--CRC_32 : 1_0000_0100_1100_0001_0001_1101_1011_0111
*/
#define _CRT_SECURE_NO_WARNINGS

#include<stdint.h>	// uint8_t ��
#include<stdio.h>	// �⺻ ����� �Լ�
#include<stdlib.h>	// �����Ҵ�, ���� ����
#include<time.h>	// ���� ���� �õ�

/* �⺻���� Ȯ������ �� �ҽ� �ڵ忡 �ۼ��ϱ� ���� ��ó��*/
#define EXTEND
#undef EXTEND		// (Ȯ����) �̿�� �� line �ּ�ó���� ��.

/* �����͸� �ٷ�� ���� �ڷ��� */
typedef struct BinaryData
{
	int length;					// # of bits
	uint8_t* data;				// data(uint8_t) array header
	char hex[15];				// HEX expression of data. maximum(string_length)=length(AA:BB:CC:DD:EE)= 15
} BD;

/* CRC Code */
const uint8_t CRC_8[9] = { 1, 0,0,0,0, 0,1,1,1 };
const uint8_t CRC_16[17] = { 1, 0,0,0,1, 0,0,0,0, 0,0,1,0, 0,0,0,1 };
const uint8_t CRC_32[33] = { 1, 0,0,0,0, 0,1,0,0, 1,1,0,0, 0,0,0,1, 0,0,0,1, 1,1,0,1, 1,0,1,1, 0,1,1,1 };

/* Functions */
void initBD(BD*, int length);
void getBinaryfromHex(BD*);						// includes memory allocation
void printBinary(BD*);				
void getRemainder(BD* Message,int CRC_type);		
int  makeNoiseOnData(BD*);						// includes console input
int  checkError(BD* Message, int CRC_type);

void createRandomData(int size, BD* );			// size: # of Bytes

/* Main */
int main(){
	BD Data, Message;
	int size_of_data = 4, CRC_type = 8;
	int num_of_errorbit;
	
	srand(time(NULL));	// ���� �õ� ����

	// [�⺻�� 1. ���� �Է¹ޱ�] & [Ȯ���� 1. ���� �����ϱ�] 
#ifdef EXTEND		// [Ȯ����]

	printf("[SYSTEM] ������ Data�� ũ��(Byte ��)�� �Է��ϼ��� : ");
	scanf("%d", &size_of_data);
	printf("\n");
	createRandomData(size_of_data, &Data); // �������� ������ ����
	printf("[SYSTEM] Data�� �������� �����Ǿ����ϴ�.\n");
#endif
#ifndef EXTEND		// [�⺻��]
	printf("[SYSTEM] ������ Data�� �Է��ϼ���(Hex) : ");
	scanf("%s", (Data.hex)); printf("\n");
	getBinaryfromHex(&Data);
#endif
	
	printf("[SYSTEM] ������ Data�� \n"); // ��º�
	printf("         length : %d\n", Data.length);
	printf("         (HEX) %s\n", Data.hex);
	printf("         (BINARY) "); printBinary(&Data); printf("\n\n");

	// [�⺻�� 2. ������(Remainder)�� ������ ���ۿ�(Message)]

#ifdef EXTEND		// [Ȯ���� - CRC �����ϱ�]
	printf("[SYSTEM] ������ CRC-x�� ������ �Է��ϼ���. (8, 16, 32 �� �Է�): ");
	scanf("%d", &CRC_type);
#endif
	// ������ ���ۿ� ���� & �ʱ�ȭ�ϱ�
	initBD(&Message, Data.length + CRC_type);
	for (int i = 0; i < Data.length; i++)		Message.data[i] = Data.data[i];
	for (int i = Data.length; i < Message.length; i++) Message.data[i] = 0;

	// ������ ���ۿ��� CRC code�� ������
	getRemainder(&Message, CRC_type); // Message�� shift & XOR
	for (int i = 0; i < Data.length; i++)	Message.data[i] = Data.data[i]; // ������ �۾� �ϴ��� �Ѽյ� Data �κ� ����

	printf("[SYSTEM] �۽� �޽���:  "); printBinary(&Message); printf("\n\n");

	printf("********* ���� ���� ********* \n\n");

	// [�⺻�� 3. & Ȯ���� 3.  Noise �����]
	num_of_errorbit = makeNoiseOnData(&Message);
	printf("[SYSTEM] ���� �޽���:  "); printBinary(&Message); printf("\n\n");
	
	// [�⺻�� 4. CRC ��� �˻��ϱ�]
	getRemainder(&Message, CRC_type);
	printf("[SYSTEM] CRC�� ���� ������:  "); printBinary(&Message); printf("\n\n");
	
	if(!checkError(&Message, CRC_type))
	{
		printf("[SYSTEM] ���� �� ������ ���� ������ �Ǵܵ˴ϴ�.\n");
		if (num_of_errorbit == 0) 
		{
			printf("[SYSTEM] CRC-x check�� ���������� �����߽��ϴ�.\n");
		}
		else 
		{
			printf("[SYSTEM] CRC-x check�� �����߽��ϴ�.\n");
			// wrong_cnt += 1;
		}
	}
	else 
	{
		printf("[SYSTEM] ������ �����߽��ϴ�.\n");
		if (num_of_errorbit != 0)
		{
			printf("[SYSTEM] CRC-x check�� ���������� �����߽��ϴ�.\n");
		}
		else
		{
			printf("[SYSTEM] CRC-x check�� �����߽��ϴ�.\n");
			//wrong_cnt += 1;
		}
	}

	// memory deallocation
	free(Data.data);
	free(Message.data);
	
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
	int i=0, cnt = 0, size;
	int index=0;

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

void printBinary(BD* B) 
{
	int i = 0;
	for (i = 0; i < (B->length); i++)
	{
		if ((i % 8 == 0) && (i != 0)) printf("_");
		printf("%u", B->data[i]);
	}
}

void getRemainder(BD* Message, int CRC_type){
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
	
	// Remainder �κи� ���
	printf("[SYSTEM] Data�� CRC Code�� ���� �������� ");
	for (i = data_length; i < Message->length; i++) printf("%u", Message->data[i]);
	printf("\n");
}

int  checkError(BD* Message, int CRC_type) 
{
	for (int i = (Message->length - 1); i >= (Message->length - CRC_type); i--)
	{
		if (Message->data[i] == 1) return 1;
	}
	return 0;
}

int makeNoiseOnData(BD* Message) 
{
	int num_of_error;
	int position_array[100];

	printf("���� ��Ʈ ���� �Է��ϼ���. ");
	printf("(���� = 0 ~ %2d)\n", Message->length);

#ifndef EXTEND // [�⺻��] console input
	printf("(99 �Է½� burst error) : ");
	scanf("%d", &num_of_error);
	printf("\n");
	if(num_of_error == 99)
	{
		printf("burst�� ���۵Ǵ� ������ ���� ��Ʈ ���� �Է��ϼ��� :");
		int start, error_seq;
		scanf("%d%d", &start, &error_seq);
		for(int i=0; i<error_seq; i++)
		{
			Message->data[(start+i)%(Message->length)] = !Message->data[(start + i) % (Message->length)];
		}
		return error_seq;
	}
	else
	{
		printf("%d ���� ���� ��ġ�� ���� �Է��ϼ���: ", num_of_error);
		for(int i=0; i < num_of_error; i++)
		{
			// ū ���� �־��� �ÿ� ERROR �޼��� ���� ������ġ�� �ؾ� �ϳ�..?
			int position;
			scanf("%d", &position);
			Message->data[position] = ! Message->data[position];
		}
		return num_of_error;
	}
#endif
#ifdef EXTEND		// [Ȯ���� - ���� ����]
	scanf("%d", &num_of_error); printf("\n");
	
	printf("flip�� �Ͼ �ڸ�: ");
	for(int i=0; i<num_of_error; i++)
	{
		int position = rand() % (Message->length);
		Message->data[position] = ! Message->data[position];
		if (i % 5 == 0) printf("\n");
		printf("<%3d> %4d \t", i+1, position);
		// ������ - �ߺ��ؼ� ���� �ڸ��� ���� ���ɼ� ����. �̰� ������ ���Ѿ� ����, �ߺ� ���� �ؾ� �ϴ� ���� �ƴ��� ��� ��...
	}
	printf("\n\n");
	return num_of_error;
#endif


}

void createRandomData(int size, BD* B) 
{
	int hexnum, i=0;
	B->length = size * 8;
	size *= 2;
	while(size>0)
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