/* C Program for Bad Character Heuristic of Boyer Moore String Matching Algorithm */
#include "BoyerMooreU.h"
#define NO_OF_CHARS 65536 //for wide char

int max(int a, int b)
{
	return (a > b) ? a : b;
}

void hbadChar(wchar_t *str, int size, int badchar[NO_OF_CHARS])
{
	int i;
	// Initialize all occurrences as -1 
	for (i = 0; i < NO_OF_CHARS; i++)
		badchar[i] = -1;
	// Fill the actual value of last character
	for (i = 0; i < size; i++)
		badchar[(int)str[i]] = i;
}

void replace(wchar_t *txt, int txtl, wchar_t *pat, int patl, wchar_t *rep)
{
	//int patl = wcslen(pat);
	//  int txtl = wcslen(txt);
	int j,k,s = 0;   // s is shift of the pattern with
	int i=txtl - patl;
	//int patla=0, // all replaced char count
	int	lastp= patl- 1;
	int badchar[NO_OF_CHARS];
	hbadChar(pat, patl, badchar);
	while (s <= i)
	{
		j = lastp;
		while (j >= 0 && pat[j] == txt[s + j])
			j--;
		if (j < 0)
		{
			//printf("\n pattern occurs at shift = %d", s);
			for (k=0; k<patl;k++)
				txt[s+k]=rep[k];
//			patla=patla+patl;
			s += (s + patl < txtl) ? patl - badchar[txt[s + patl]] : 1;
		}
		else
			s += max(1, j - badchar[txt[s + j]]);
	}
}

