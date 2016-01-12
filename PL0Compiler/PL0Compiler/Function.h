#pragma warning(disable:4996)
#pragma once

#ifndef FUNCTION_H
#define FUNCTION_H

#include <cstdarg>

struct Record;

template <typename T>
inline bool InSegment(T x, T *Arr, int len)
{
	while (len--)
	{
		if (Arr[len] == x)
			return 1;
	}
	return 0;
}

template <typename T>
inline bool InSegment(T x, int len, ...)
{
	va_list ap;
	va_start(ap, len);
	for (int i = 0; i < len; i++)
	{
		if (x == va_arg(ap, T))
			return 1;
	}
	va_end(ap);
	return 0;
}

int GetVarSize();

Record *AddIDTable();
Record *AddTmpIDTable();

void IncLayer();
void DecLayer();

void InitTable();

Record *GetTableData(char *);
Record *InTableData(char *);

#endif