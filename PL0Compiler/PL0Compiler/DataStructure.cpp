#include <iostream>
#include <cstring>
#include <string>
#include <cstdio>
#include <cstdlib>
#include "DataStructure.h"
#include <assert.h>
#include <cstdarg>

int Word::GetVal()
{
	assert(this->type == UNSIGNED);
	return atoi(this->word);
}

Node::Node(int type, int cnt, ...)
{
	va_list ap;
	va_start(ap, cnt);
	IsWord = 0;
	this->type = type;
	for (int i = 0; i < cnt; i++)
	{
		childs.push_back(va_arg(ap, Node*));
		if (0 == i)
			pos = childs[0]->pos;
	}
}

int Node::GetVal()
{
	if (IsWord)
		return word->GetVal();
	if (childs.size() == 1)
		return childs[0]->word->GetVal();
	else
	{
		if (childs[0]->word->type == MINUS)
			return 0 - childs[1]->word->GetVal();
		else
			return childs[1]->word->GetVal();
	}
}

bool Node::Merge(Node *nd)
{
	if (nd == NULL)
		return 0;
	for (std::vector<Node*>::iterator it = nd->childs.begin(); it != nd->childs.end(); it++)
		AddChild(*it);
	return 1;
}

Record::Record()
{
	name = NULL;
	RecType = variable;
	VarType.IsArr = 0, VarType.type = INT;
	startAddr = 0;
	parameter.clear(), paratype.clear();
	isGlobal = Imme = 0;
}

char *Record::GetDest(char *buff)
{
	if (isGlobal)
	{
		sprintf(buff, "[__base+%d]", startAddr);
		return buff;
	}
	else
	{
		sprintf(buff, "[EBP+%d]", -startAddr);
		return buff;
	}
}