#include <fstream>
using namespace std;

typedef unsigned char byte;
typedef unsigned int dword;

dword ROL(dword x, int s);
dword bigToLittle(dword bigEndian);

dword ROL(dword x, int s)
{
	return (x << s & 0xFFFFFFFF) | (x >> (32 - s) & 0xFFFFFFFF);
}

dword bigToLittle(dword bigEndian)
{
	dword littleEndian = 0;
	for (int i = 0; i < 32; i += 8)
	{
		littleEndian |= ((bigEndian >> i) & 0xFF) << (24 - i);
	}

	return littleEndian;
}

byte* readFile(const char* fileName, int& size)
{
    ifstream inStream(fileName);
	if (inStream.fail())
		return NULL;

	inStream.seekg(0, ios::end);
	size = inStream.tellg();
	inStream.seekg(0, ios::beg);

	byte* fileContent = new byte[size];
	if (size)
		inStream.read((char*)fileContent, size);
    inStream.close();

	return fileContent;
}
