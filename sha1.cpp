#define _CRT_SECURE_NO_WARNINGS
#include "funcs.h"
#include <iostream>
#include <string.h>
#include <iomanip>
#include <sstream>

using namespace std;

string sha1(byte* m, size_t msgLen);

int main(int argc, char* argv[])
{
	if (argc != 2)
	{
		cout << "sha1 <file_name>" << endl;
		return 1;
	}
    int fileSize = 0;

	byte* buf = readFile(argv[1], fileSize);
	cout << sha1(buf, fileSize) << endl;
	delete[] buf;
	return 0;
}

string sha1(byte* m, size_t msgLen)
{
	// предварительная обработка
	int zeroCount = 56 - (msgLen + 1) % 64;
	if (zeroCount < 0)
		zeroCount += 64;
	
	size_t newLen = msgLen + zeroCount; 
	byte* msg = new byte[newLen + 1 + 8]();
	memset(msg, 0, newLen);
	memcpy(msg, m, msgLen);
	msg[msgLen] = 0x80;
	newLen++;


	// Добавляем длину исходного сообщения(до предварительной обработки) как целое 64 - битное
	// Big - endian число, в битах
	int L = msgLen * 8;
	newLen += 4;
	for (int i = 0; i < 4; i++, newLen++)
		msg[newLen] = L >> (3-i) * 8;


	// объединяем байты в блоки по 32 бита (bigEndian)
	dword* blocks = new dword[newLen / 4];
	size_t blocksLen = newLen / 4;
	for (size_t i = 0; i < newLen; i += 4)
	{
		blocks[i / 4] = 0;
		for (int j = 0; j < 4; j++)
		{
			blocks[i / 4] |= msg[i + j] << (3 - j) * 8;
		}
	}
	delete[] msg;


	dword h[5];
	h[0] = 0x67452301;
	h[1] = 0xEFCDAB89;
	h[2] = 0x98BADCFE;
	h[3] = 0x10325476;
	h[4] = 0xC3D2E1F0;


	// В процессе сообщение разбивается последовательно по 16 блоков
	for (size_t i = 0; i < blocksLen; i += 16)
	{
		dword w[80];
		memcpy(w, blocks + i, 16 * sizeof(dword));

		for (int i = 16; i < 80; i++)
			w[i] = ROL(w[i - 3] ^ w[i - 8] ^ w[i - 14] ^ w[i - 16], 1);

		//  Инициализация хеш-значений этой части:
		dword a = h[0];
		dword b = h[1];
		dword c = h[2];
		dword d = h[3];
		dword e = h[4];

		// Основной цикл:
		for (int i = 0; i < 80; i++)
		{
			dword f;
			dword k;
			if (i >= 0 && i <= 19)
			{
				f = (b & c) | ((~b) & d);
				k = 0x5A827999;
			}
			if (i >= 20 && i <= 39)
			{
				f = b ^ c ^ d;
				k = 0x6ED9EBA1;
			}
			if (i >= 40 && i <= 59)
			{
				f = (b & c) | (b & d) | (c & d);
				k = 0x8F1BBCDC;
			}
			if (i >= 60 && i <= 79)
			{
				f = b ^ c ^ d;
				k = 0xCA62C1D6;
			}

			dword temp = ROL(a, 5);
			temp = ROL(a, 5) + f + e + k + w[i];
			e = d;
			d = c;
			c = ROL(b, 30);
			b = a;
			a = temp;
		}
		h[0] = h[0] + a;
		h[1] = h[1] + b;
		h[2] = h[2] + c;
		h[3] = h[3] + d;
		h[4] = h[4] + e;
	}
	delete[] blocks;
	
	ostringstream outStream;
	outStream.fill('0');
	for (int i = 0; i < 5; i++)
		outStream << hex << setw(8) << h[i];

	char resString[41];
	strcpy(resString, outStream.str().c_str());
	return resString;
}