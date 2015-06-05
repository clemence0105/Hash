#define _CRT_SECURE_NO_WARNINGS
#include "funcs.h"
#include <iostream>
#include <string.h>
#include <iomanip>
#include <sstream>

using namespace std;

string gost341194(byte* m, size_t msgLen);

void F(byte* Hin, byte* m);
void cntrlSum(byte* sum, byte* m);
void xor32bytes(byte* a, byte* b);
byte* roundKeys(byte* U, byte* V);
void A(byte* Y);
void P(byte* Y);
void E(byte* D, byte* K);
byte* stepE(byte* A, byte* K);
void fiN(byte* Y, int n);


int main(int argc, char* argv[])
{
	if (argc != 2)
	{
		cout << "gost341194 <file_name>" << endl;
		return 1;
	}
    int fileSize = 0;
	byte* buf = readFile(argv[1], fileSize);
	cout << gost341194(buf, fileSize) << endl;
	delete[] buf;
	return 0;
}

string gost341194(byte* m, size_t msgLen)
{
	byte h[32];
	byte sum[32];
	byte L[32];
	memset(h, 0, 32);
	memset(sum, 0, 32);
	memset(L, 0, 32);

	for (int i = 0; i < (int)msgLen - 31; i += 32)
	{
		byte* chunk = m + i;
		cntrlSum(sum, chunk);
		F(h, chunk);
	}
	L[msgLen / 32] = 1;

	if (msgLen % 32)
	{
		L[0] = (msgLen % 32) * 8;

		byte chunk[32];
		memset(chunk, 0, 32);
		memcpy(chunk, m + (msgLen / 32) * 32, msgLen % 32);
		F(h, chunk);
		cntrlSum(sum, chunk);
	}

	F(h, L);
	F(h, sum);

	ostringstream res;
	res.fill('0');
	for (int i = 0; i < 32; i++)
		res << hex << setw(2) << (int)h[i];

	char resString[65];
	strcpy(resString, res.str().c_str());
	return resString;
}

void F(byte* Hin, byte* m)
{
	byte* keys = roundKeys(Hin, m);

	byte S[32];
	for (int i = 0; i < 4; i++)
	{
		byte tmp[8];
		memcpy(tmp, Hin + i * 8, 8);
		E(tmp, keys + i*32);
		memcpy(S + i*8, tmp, 8);
	}
	delete[] keys;

	fiN(S, 12);
	xor32bytes(S, m);
	fiN(S, 1);
	xor32bytes(Hin, S);
	fiN(Hin, 61);
}

void cntrlSum(byte* sum, byte* m)
{
	int c = 0;
	for (int i = 0; i < 32; i++)
	{
		c += sum[i] + m[i];
		sum[i] = c & 0xFF;
		c >>= 8;
	}
}

void xor32bytes(byte* a, byte* b)
{
	for (size_t i = 0; i < 32; i++)
		a[i] ^= b[i];
}

byte* roundKeys(byte* Uc, byte* Vc)
{
	byte U[32];
	byte V[32];
	memcpy(U, Uc, 32);
	memcpy(V, Vc, 32);

	byte C3[] = {
		0x00, 0xff, 0x00, 0xff, 0x00, 0xff, 0x00, 0xff,
		0xff, 0x00, 0xff, 0x00, 0xff, 0x00, 0xff, 0x00,
		0x00, 0xff, 0xff, 0x00, 0xff, 0x00, 0x00, 0xff,
		0xff, 0x00, 0x00, 0x00, 0xff, 0xff, 0x00, 0xff };

	byte* K = new byte[4 * 32];
	byte W[32];
	memcpy(W, V, 32);
	xor32bytes(W, U);
	P(W);
	memcpy(K, W, 32);
	for (int j = 1; j < 4; j++)
	{
		A(U);
		if (j == 2)
			xor32bytes(U, C3);
		A(V);
		A(V);
		memcpy(W, V, 32);
		xor32bytes(W, U);
		P(W);
		memcpy(K + j * 32, W, 32);
	}

	return K;
}

void A(byte* Y)
{
	byte res[32];
	for (int i = 0; i < 24; i++)
		res[i] = Y[i + 8];

	for (int i = 0; i < 8; i++)
		res[i + 24] = Y[i] ^ Y[i + 8];

	memcpy(Y, res, 32);
}

void P(byte* Y)
{
	byte res[32];
	for (int i = 0; i <= 3; i++)
	for (int k = 1; k <= 8; k++)
		res[i + 1 + 4 * (k - 1) - 1] = Y[8 * i + k - 1];

	memcpy(Y, res, 32);
}

void E(byte* D, byte* K)
{
	byte A[4];
	byte B[4];
	memcpy(A, D, 4);
	memcpy(B, D + 4, 4);

	for (int j = 0; j < 3; j++)
	{
		for (int i = 0; i < 8; i++)
		{
			byte* tmp = stepE(A, K + i*4);
			for (int i = 0; i < 4; i++)
				tmp[i] ^= B[i];
			memcpy(B, A, 4);
			memcpy(A, tmp, 4);
			delete[] tmp;
		}
	}

	for (int i = 7; i >= 0; i--)
	{
		byte* tmp = stepE(A, K + i * 4);
		for (int i = 0; i < 4; i++)
			tmp[i] ^= B[i];
		memcpy(B, A, 4);
		memcpy(A, tmp, 4);
		delete[] tmp;
	}

	memcpy(D, B, 4);
	memcpy(D + 4, A, 4);
}

byte* stepE(byte* A, byte* K)
{
	unsigned char S[8][16] = {
		{ 4, 10, 9, 2, 13, 8, 0, 14, 6, 11, 1, 12, 7, 15, 5, 3 },
		{ 14, 11, 4, 12, 6, 13, 15, 10, 2, 3, 8, 1, 0, 7, 5, 9 },
		{ 5, 8, 1, 13, 10, 3, 4, 2, 14, 15, 12, 7, 6, 0, 9, 11 },
		{ 7, 13, 10, 1, 0, 8, 9, 15, 14, 4, 6, 12, 11, 2, 5, 3 },
		{ 6, 12, 7, 1, 5, 15, 13, 8, 4, 10, 9, 14, 0, 3, 11, 2 },
		{ 4, 11, 10, 0, 7, 2, 1, 13, 3, 6, 8, 5, 9, 12, 15, 14 },
		{ 13, 11, 4, 1, 3, 15, 5, 9, 0, 10, 14, 7, 6, 8, 2, 12 },
		{ 1, 15, 13, 0, 5, 7, 10, 4, 9, 2, 3, 14, 6, 11, 8, 12 },
	};

	byte* res = new byte[4];
	int c = 0;
	for (int i = 0; i < 4; i++)
	{
		c += A[i] + K[i];
		res[i] = c & 0xFF;
		c >>= 8;
	}

	for (int i = 0; i < 8; i++)
	{
		int x = res[i >> 1] & ((i & 1) ? 0xF0 : 0x0F);

		res[i >> 1] ^= x;
		x >>= (i & 1) ? 4 : 0;
		x = S[i][x];
		res[i >> 1] |= x << ((i & 1) ? 4 : 0);
	}

	int tmp = res[3];
	res[3] = res[2];
	res[2] = res[1];
	res[1] = res[0];
	res[0] = tmp;
	tmp = res[0] >> 5;

	for (int i = 1; i < 4; i++)
	{
		int nTmp = res[i] >> 5;
		res[i] = (res[i] << 3) | tmp;
		tmp = nTmp;
	}

	res[0] = (res[0] << 3) | tmp;
	return res;
}

void fiN(byte* Y, int n)
{
	for (int i = 0; i < n; i++)
	{
		unsigned char tmp[] = { 0, 0 };

		char indexes[] = { 1, 2, 3, 4, 13, 16 };
		for (int j = 0; j < sizeof(indexes); j++)
		{
			tmp[0] ^= Y[2 * (indexes[j] - 1)];
			tmp[1] ^= Y[2 * (indexes[j] - 1) + 1];
		}

		for (int i = 0; i < 30; i++)
			Y[i] = Y[i + 2];

		Y[30] = tmp[0];
		Y[31] = tmp[1];
	}
}