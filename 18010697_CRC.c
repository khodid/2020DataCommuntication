/*
-2020 데이터통신 과제4 : CRC 구현하기
-작성자 : 전자정보통신공학과 18010697 김해리


-구상
--전처리: main 함수 기본형 / 확장형 나누기
--자료형: uint8_t 배열 (각 element엔 0, 1만 표현, 비트단위 연산은 배열 연산으로 대체)
--함수:
---input / output  HEX(AA:BB:CC:DD와 같은) 형식 입출력 담당.
---CRC를 붙인 전송 문자열 생성하는 함수
---noise 생성 함수
---(확장형)랜덤 Data 생성 함수

-제시된 CRC 코드 [전역 static 변수로 선언]
--CRC_8  : 1_0000_0111
--CRC_16 : 1_0001_0000_0010_0001
--CRC_32 : 1_0000_0100_1100_0001_0001_1101_1011_0111
*/
#define _CRT_SECURE_NO_WARNINGS

#include<stdint.h>	// uint8_t 형
#include<stdio.h>	// 기본 입출력 함수
#include<stdlib.h>	// 동적할당, 난수 생성
#include<time.h>	// 난수 생성 시드

/* 기본형과 확장형을 한 소스 코드에 작성하기 위한 전처리*/
#define EXTEND
#undef EXTEND		// (확장형) 이용시 이 line 주석처리할 것.

/* 데이터를 다루기 위한 자료형 */
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
	
	srand(time(NULL));	// 난수 시드 설정

	// [기본형 1. 숫자 입력받기] & [확장형 1. 숫자 생성하기] 
#ifdef EXTEND		// [확장형]

	printf("[SYSTEM] 전송할 Data의 크기(Byte 수)를 입력하세요 : ");
	scanf("%d", &size_of_data);
	printf("\n");
	createRandomData(size_of_data, &Data); // 랜덤으로 데이터 생성
	printf("[SYSTEM] Data가 랜덤으로 생성되었습니다.\n");
#endif
#ifndef EXTEND		// [기본형]
	printf("[SYSTEM] 전송할 Data를 입력하세요(Hex) : ");
	scanf("%s", (Data.hex)); printf("\n");
	getBinaryfromHex(&Data);
#endif
	
	printf("[SYSTEM] 전송할 Data는 \n"); // 출력부
	printf("         length : %d\n", Data.length);
	printf("         (HEX) %s\n", Data.hex);
	printf("         (BINARY) "); printBinary(&Data); printf("\n\n");

	// [기본형 2. 나머지(Remainder)와 데이터 전송열(Message)]

#ifdef EXTEND		// [확장형 - CRC 선택하기]
	printf("[SYSTEM] 적용할 CRC-x의 종류를 입력하세요. (8, 16, 32 중 입력): ");
	scanf("%d", &CRC_type);
#endif
	// 데이터 전송열 선언 & 초기화하기
	initBD(&Message, Data.length + CRC_type);
	for (int i = 0; i < Data.length; i++)		Message.data[i] = Data.data[i];
	for (int i = Data.length; i < Message.length; i++) Message.data[i] = 0;

	// 데이터 전송열을 CRC code로 나누기
	getRemainder(&Message, CRC_type); // Message를 shift & XOR
	for (int i = 0; i < Data.length; i++)	Message.data[i] = Data.data[i]; // 나누기 작업 하느라 훼손된 Data 부분 복구

	printf("[SYSTEM] 송신 메시지:  "); printBinary(&Message); printf("\n\n");

	printf("********* 전송 이후 ********* \n\n");

	// [기본형 3. & 확장형 3.  Noise 만들기]
	num_of_errorbit = makeNoiseOnData(&Message);
	printf("[SYSTEM] 수신 메시지:  "); printBinary(&Message); printf("\n\n");
	
	// [기본형 4. CRC 기능 검사하기]
	getRemainder(&Message, CRC_type);
	printf("[SYSTEM] CRC로 나눈 나머지:  "); printBinary(&Message); printf("\n\n");
	
	if(!checkError(&Message, CRC_type))
	{
		printf("[SYSTEM] 전송 중 오류가 없는 것으로 판단됩니다.\n");
		if (num_of_errorbit == 0) 
		{
			printf("[SYSTEM] CRC-x check를 성공적으로 수행했습니다.\n");
		}
		else 
		{
			printf("[SYSTEM] CRC-x check에 실패했습니다.\n");
			// wrong_cnt += 1;
		}
	}
	else 
	{
		printf("[SYSTEM] 오류를 감지했습니다.\n");
		if (num_of_errorbit != 0)
		{
			printf("[SYSTEM] CRC-x check를 성공적으로 수행했습니다.\n");
		}
		else
		{
			printf("[SYSTEM] CRC-x check에 실패했습니다.\n");
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

// Hex 데이터를 표현한 문자열을 Binary 형식으로 바꾸어 저장한다.
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
		// 0~f를 0000~1111까지 매칭해서 B->data에 4bit씩 채워 넣는 부분.
		// 그냥 scanf에서 char로 받지 말고 Hex 데이터로서 읽도록 구현했으면 나았겠다는 아쉬움이 있음.
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
	// Message의 끝 부분을 나머지로 고침
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
	else printf("<ERROR> getRemainder에 잘못된 CRC_type 입력됨\n");
	
	// Remainder 부분만 출력
	printf("[SYSTEM] Data를 CRC Code로 나눈 나머지는 ");
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

	printf("오류 비트 수를 입력하세요. ");
	printf("(범위 = 0 ~ %2d)\n", Message->length);

#ifndef EXTEND // [기본형] console input
	printf("(99 입력시 burst error) : ");
	scanf("%d", &num_of_error);
	printf("\n");
	if(num_of_error == 99)
	{
		printf("burst가 시작되는 지점과 오류 비트 수를 입력하세요 :");
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
		printf("%d 개의 오류 위치를 각각 입력하세요: ", num_of_error);
		for(int i=0; i < num_of_error; i++)
		{
			// 큰 수를 넣었을 시에 ERROR 메세지 띄우는 안전장치를 해야 하나..?
			int position;
			scanf("%d", &position);
			Message->data[position] = ! Message->data[position];
		}
		return num_of_error;
	}
#endif
#ifdef EXTEND		// [확장형 - 랜덤 생성]
	scanf("%d", &num_of_error); printf("\n");
	
	printf("flip이 일어난 자리: ");
	for(int i=0; i<num_of_error; i++)
	{
		int position = rand() % (Message->length);
		Message->data[position] = ! Message->data[position];
		if (i % 5 == 0) printf("\n");
		printf("<%3d> %4d \t", i+1, position);
		// 문제점 - 중복해서 같은 자리가 나올 가능성 있음. 이걸 검출을 시켜야 할지, 중복 없이 해야 하는 건지 아닌지 고민 됨...
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