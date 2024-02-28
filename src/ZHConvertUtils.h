
#ifndef ZHConvertUtilsH
#define ZHConvertUtilsH
#include <iostream>
#include <fstream>
#include <vector>

using namespace std;

struct Node{
	wchar_t* simp;
	wchar_t* trad;
	int Len;
};
typedef Node *PNode;
typedef vector<PNode> NodeList;

void WriteUnicodetoFile(const char* myFile, wchar_t* ws, size_t len);
wchar_t* readUnicodeFile(const char* filename, size_t &size);
NodeList* ReadConvertTable(const char* myFile, size_t &len);
#endif