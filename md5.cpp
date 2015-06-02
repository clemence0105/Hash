#define _CRT_SECURE_NO_WARNINGS
#include "funcs.h"
#include <iostream>
#include <string.h>
#include <iomanip>
#include <sstream>

using namespace std;

char* md5(byte* m, size_t msgLen);
void md5rounds(dword* chunk, dword* buf);

// функции для 4-х раундов
#define F(x, y, z) ((x & y) | (~x & z))
#define G(x, y, z) ((x & z) | (~z & y))
#define H(x, y, z) (x ^ y ^ z)
#define I(x, y, z) (y ^ (~z | x))

/*[F abcd k s i] : a = b + ((a + F(b,c,d) + X[k] + T[i]) <<< s). */
#define STEP(F, a, b, c, d, xk, s, t)	a = b + ROL((a + F(b, c, d) + xk + (dword)t), s)

int main(int argc, char* argv[])
{
	if (argc != 2)
	{
		cout << "md5 <file_name>" << endl;
		return 1;
	}
    int fileSize = 0;
	byte* buf = readFile(argv[1], fileSize);
	cout << md5(buf, fileSize) << endl;
	return 0;
}

char* md5(byte* m, size_t msgLen)
{
	// Шаг 1. Выравнивание потока
	int zeroCount = 56 - (msgLen + 1) % 64; // необходимое число нулевых байт для выравнивания
	if (zeroCount < 0)
		zeroCount += 64;
	
	size_t newLen = msgLen + zeroCount; // новая длина после добавления нулей
	byte* msg = new byte[newLen + 1 + 8](); // +1 под байт 0х80, +8 под запись длины сообщения
	memset(msg, 0, newLen);
	memcpy(msg, m, msgLen);
	msg[msgLen] = 0x80;
	newLen++;


	// Шаг 2. Добавление длины сообщения
	int L = msgLen * 8; // размер входного сообщения в битах
	for (int i = 0; i < 4; i++, newLen++)
		msg[newLen] = L >> i * 8;
	newLen += 4;


	// Шаг 3. Инициализация буфера
	dword buf[4];
	buf[0] = 0x67452301;
	buf[1] = 0xEFCDAB89;
	buf[2] = 0x98BADCFE;
	buf[3] = 0x10325476;

	// объединяем байты в блоки по 32 бита
	dword* blocks = (dword*)msg;
	size_t blocksLen = newLen / 4;


	// Шаг 4. Вычисление в цикле
	// объединяем блоки по 16 элементов и прогоняем через 4 раунда каждый
	for (size_t i = 0; i < blocksLen; i += 16)
	{
		dword* chunk = blocks + i;
		md5rounds(chunk, buf); // преобразовываем буфер
	}
	delete[] msg;


	// Шаг 5. Результат вычислений
	ostringstream outStream;
	outStream.fill('0');
	for (int i = 0; i < 4; i++)
		outStream << hex << setw(8) << bigToLittle(buf[i]);

	char* resString = new char[33];
	strcpy(resString, (char*)outStream.str().c_str());
	return resString;
}

void md5rounds(dword* chunk, dword* buf)
{
	dword A = buf[0];
	dword B = buf[1];
	dword C = buf[2];
	dword D = buf[3];

	dword AA = A;
	dword BB = B;
	dword CC = C;
	dword DD = D;

	
	unsigned char r1k[] = { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15 };
	dword r1t[] = {
		0xD76AA478, 0xE8C7B756, 0x242070DB, 0xC1BDCEEE,
		0xF57C0FAF, 0x4787C62A, 0xA8304613, 0xFD469501,
		0x698098D8, 0x8B44F7AF, 0xFFFF5BB1, 0x895CD7BE,
		0x6B901122, 0xFD987193, 0xA679438E, 0x49B40821 };
	for (int i = 0; i < 16; i += 4)
	{
		STEP(F, A, B, C, D, chunk[r1k[i + 0]], 7, r1t[i + 0]);
		STEP(F, D, A, B, C, chunk[r1k[i + 1]], 12, r1t[i + 1]);
		STEP(F, C, D, A, B, chunk[r1k[i + 2]], 17, r1t[i + 2]);
		STEP(F, B, C, D, A, chunk[r1k[i + 3]], 22, r1t[i + 3]);
	}

	
	unsigned char r2k[] = { 1, 6, 11, 0, 5, 10, 15, 4, 9, 14, 3, 8, 13, 2, 7, 12 };
	dword r2t[] = { 
		0xF61E2562, 0xC040B340, 0x265E5A51, 0xE9B6C7AA, 
		0xD62F105D, 0x02441453, 0xD8A1E681, 0xE7D3FBC8,
		0x21E1CDE6, 0xC33707D6, 0xF4D50D87, 0x455A14ED,
		0xA9E3E905, 0xFCEFA3F8, 0x676F02D9, 0x8D2A4C8A };
	for (int i = 0; i < 16; i += 4)
	{
		STEP(G, A, B, C, D, chunk[r2k[i + 0]], 5, r2t[i + 0]);
		STEP(G, D, A, B, C, chunk[r2k[i + 1]], 9, r2t[i + 1]);
		STEP(G, C, D, A, B, chunk[r2k[i + 2]], 14, r2t[i + 2]);
		STEP(G, B, C, D, A, chunk[r2k[i + 3]], 20, r2t[i + 3]);
	}

	
	unsigned char r3k[] = { 5, 8, 11, 14, 1, 4, 7, 10, 13, 0, 3, 6, 9, 12, 15, 2 };
	dword r3t[] = {
		0xFFFA3942, 0x8771F681, 0x6D9D6122, 0xFDE5380C,
		0xA4BEEA44, 0x4BDECFA9, 0xF6BB4B60, 0xBEBFBC70,
		0x289B7EC6, 0xEAA127FA, 0xD4EF3085, 0x04881D05,
		0xD9D4D039, 0xE6DB99E5, 0x1FA27CF8, 0xC4AC5665 };
	for (int i = 0; i < 16; i += 4)
	{
		STEP(H, A, B, C, D, chunk[r3k[i + 0]], 4, r3t[i + 0]);
		STEP(H, D, A, B, C, chunk[r3k[i + 1]], 11, r3t[i + 1]);
		STEP(H, C, D, A, B, chunk[r3k[i + 2]], 16, r3t[i + 2]);
		STEP(H, B, C, D, A, chunk[r3k[i + 3]], 23, r3t[i + 3]);
	}


	unsigned char r4k[] = { 0, 7, 14, 5, 12, 3, 10, 1, 8, 15, 6, 13, 4, 11, 2, 9 };
	dword r4t[] = { 
		0xF4292244, 0x432AFF97, 0xAB9423A7, 0xFC93A039,
		0x655B59C3, 0x8F0CCC92, 0xFFEFF47D, 0x85845DD1,
		0x6FA87E4F, 0xFE2CE6E0, 0xA3014314, 0x4E0811A1,
		0xF7537E82, 0xBD3AF235, 0x2AD7D2BB, 0xEB86D391 };
	for (int i = 0; i < 16; i += 4)
	{
		STEP(I, A, B, C, D, chunk[r4k[i + 0]], 6, r4t[i + 0]);
		STEP(I, D, A, B, C, chunk[r4k[i + 1]], 10, r4t[i + 1]);
		STEP(I, C, D, A, B, chunk[r4k[i + 2]], 15, r4t[i + 2]);
		STEP(I, B, C, D, A, chunk[r4k[i + 3]], 21, r4t[i + 3]);
	}

	buf[0] = AA + A;
	buf[1] = BB + B;
	buf[2] = CC + C;
	buf[3] = DD + D;
}