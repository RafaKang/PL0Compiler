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
		printf("�ʷ��������� ");
		break;
	case GRAMMARERR:
		printf("�﷨�������� ");
		break;
	case SEMANTICERR:
		printf("����������� ");
		break;
	default:
		break;
	}
	printf("%s @ %d�� %d��\n", ErrMsg, x, y);
}

int GetErrorCnt()
{
	return ErrCnt;
}