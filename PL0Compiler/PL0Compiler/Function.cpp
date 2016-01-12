#include "Function.h"
#include <cassert>
#include <cstdio>
#include <cstdarg>
#include <cstring>
#include "DataStructure.h"

Record *Table::records[2010];
int Table::RecordsCnt = 0;
int Table::layers[30];
int Table::CurLayer = 0;

int TmpCnt = 0;

Record *AddIDTable()
{
	(Table::records[Table::RecordsCnt++] = new Record());
	Table::records[Table::RecordsCnt - 1]->Layer = Table::CurLayer;
	return Table::records[Table::RecordsCnt - 1];
}

Record *AddTmpIDTable()
{
	Record *rec = (Table::records[Table::RecordsCnt++] = new Record());
	rec->RecType = (Record::variable);
	rec->VarType.IsArr = false;
	rec->VarType.type = INT;
	rec->Layer = Table::CurLayer;
	char *dest = new char[20];
	sprintf(dest, "_Tmp_%d", TmpCnt++);
	rec->name = dest;
	return rec;
}

int GetVarSize()
{
	int res = 0;
	for (int i = Table::RecordsCnt - 1; i >= Table::layers[Table::CurLayer]; i--)
	{
		if (Table::records[i]->isGlobal == 0)
			res += Table::records[i]->GetSize();
	}
	return res;
}

void IncLayer()
{
	Table::layers[++Table::CurLayer] = Table::RecordsCnt;
}

void DecLayer()
{
	Table::RecordsCnt = Table::layers[Table::CurLayer--];
}

void InitTable()
{
	Table::CurLayer = Table::RecordsCnt = 0;
}

Record *GetTableData(char *name)
{
	int x = Table::RecordsCnt;
	while (x --> 0)
	{
		if (strcmp(Table::records[x]->name, name) == 0)
		{
			return Table::records[x];
		}
	}
	return NULL;
}

Record *InTableData(char *name)
{
	int x = Table::RecordsCnt;
	while (x-- > Table::layers[Table::CurLayer])
	{
		if (strcmp(name, Table::records[x]->name) == 0)
			return Table::records[x];
	}
	return NULL;
}