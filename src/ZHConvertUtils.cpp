#include "ZHConvertUtils.h"

/*std::wstring readUnicodeFile(const char* filename)
{
	std::ifstream wif(filename);
	std::stringstream wss;
	wif.seekg(2);
	wss << wif.rdbuf();
	std::string  const &str = wss.str();
	std::wstring wstr;
	wstr.resize(str.size()/sizeof(wchar_t));
	std::memcpy(&wstr[0],str.c_str(),str.size()); // copy data into wstring
	wif.close();
	return wstr;
}*/

wchar_t* readUnicodeFile(const char* filename, size_t &size)
{
	std::ifstream wif(filename, ios::binary);
	// get pointer to associated buffer object
	std::filebuf* pbuf = wif.rdbuf();
	// get file size using buffer's members
	size = (size_t) pbuf->pubseekoff(0,wif.end,wif.in);
	pbuf->pubseekpos(2,wif.in); //Skip BOM
	// allocate memory to contain file data
	char* buffer=new char[size];
	// get file data
	size= size-2;
	pbuf->sgetn(buffer,size);
	size=size/2;
  wif.close();
	return (wchar_t*)buffer;
}

void WriteUnicodetoFile(const char* myFile, wchar_t* ws, size_t len)
{
	ofstream outFile(myFile, ios::out | ios::binary);
	int bom=0xfeff;
	outFile.write((char *)&bom, 2);
	outFile.write((char *)ws, len*2);
	outFile.close();
}

NodeList* ReadConvertTable(const char* myFile, size_t &len)
{
	wchar_t* p = readUnicodeFile(myFile,len);
	wchar_t* q=p;
	Node* data;
	NodeList* nList= new NodeList;
	//	int oldMode = _setmode(_fileno(stdout), _O_U16TEXT);
	int n;
	while (*p)
	{
		if (*p==13)//ÅªÀÉÅª¨ì¸õ¦æ¦r¤¸
		{
			*p=0;
			p++;
			q=p+1;
		}
		else if (*p==',')//Åª¦r¦êÅª¨ì³r¸¹
		{
			data=new Node;
			*p=0;
			n=(int)(p-q);
			p++;
			data->simp= q;
			data->trad= p;
			data->Len=n;
			nList->push_back(data);
			p=p+n-1;
		}
		p++;
	}
	return nList;
}