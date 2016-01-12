#pragma warning(disable:4996)

#include "Err.h"
#include <cstdio>

static int ErrCnt = 0;

void PrintErr(ErrType type, char *ErrMsg,int x,int y)
{
	ErrCnt++;
	switch (type)
	{
	case WORDERR:
		printf("词法分析错误： ");
		break;
	case GRAMMARERR:
		printf("语法分析错误： ");
		break;
	case SEMANTICERR:
		printf("语义分析错误： ");
		break;
	default:
		break;
	}
	printf("%s @ %d行 %d列\n", ErrMsg, x, y);
}

int GetErrorCnt()
{
	return ErrCnt;
}