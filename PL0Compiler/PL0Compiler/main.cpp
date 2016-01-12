#include <iostream>
#include <cstdio>
#include <string>
#include <cstring>
#include "DataStructure.h"
#include "word.h"
#include "grammar.h"
#include "semantic.h"
#include "Err.h"
#include "opt.h"
#include "Asm.h"

char file[110];
FILE *fp = NULL;
char buffer[1000010];
int cnt;

extern Block *blocks[5010];
extern int BlockCnt;
Quaternion qs[5010];
Quaternion nqs[5010];

extern Block *S, *T;

int main()
{
	do
	{
		printf("Input File Name :\n");
		scanf("%s", file);
		fp = fopen(file, "r");
	} while (fp == NULL);
	Word *words = new Word[100010];
	char ch;
	while ((ch = fgetc(fp)) != EOF)
		buffer[cnt++] = ch;
	fclose(fp);
	buffer[cnt] = 0;
	int i = 0;
	char *p = buffer;
	while (p = GetWord(p, words + i))
		i++;
	words[i].type = ENDOFFILE;
	//Output(words, i);
	Node *res = GetNodes(Program, words);

	Quaternion *qsptr = qs;
	if (GetErrorCnt() != 0)
	{
		goto END;
	}
	GetQuaternion(res, qs);
	if (GetErrorCnt() != 0)
	{
		goto END;
	}
	Output(qs);

	Quaternion *t = qs;
	while ((t = MakeBlock(t)) != NULL);
	S = blocks[0];
	T = AddBlock();
	for (int i = 0; i < BlockCnt - 1; i++)
		DAGOpt(blocks[i]);
	for (int i = 0; i < BlockCnt - 1; i++)
		Output(blocks[i]);
	ASMinitial();
	for (int i = 0; i < BlockCnt - 1; i++)
		DataFlowInit(i);
	DataFlowCalc();
	
	int k = 0;
	DataFlowProgASM(k);

	//for (int i = 0; i < BlockCnt - 1; i++)
	//	GetASM(blocks[i]);
	ASMfinalize();
END:
	printf("输入任意键以继续...\n");
	char chr = getchar();
	getchar();
	return 0;
}