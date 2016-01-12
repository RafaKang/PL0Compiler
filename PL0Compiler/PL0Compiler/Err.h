#pragma once

#ifndef ERR_H
#define ERR_H

enum ErrType
{
	WORDERR = 1,
	GRAMMARERR = 2,
	SEMANTICERR = 3
};

void PrintErr(ErrType, char *, int, int);
int GetErrorCnt();

#endif