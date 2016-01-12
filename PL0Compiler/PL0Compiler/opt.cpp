#include "opt.h"
#include "DataStructure.h"
#include "Err.h"
#include "Function.h"
#include "Asm.h"
#include <set>
#include <map>

Block *blocks[5010];
int BlockCnt = 0;

Block *S, *T;

Block *AddBlock()
{
	return (blocks[BlockCnt++] = new Block());
}

struct DAGNode
{
	std::set<Record *> ident;
	void remove(Record *rec)
	{
		std::set<Record *>::iterator it = ident.find(rec);
		if (it == ident.end())
			return;
		ident.erase(it);
	}
	void add(Record *rec)
	{
		ident.insert(rec);
	}
};

static int NodeCnt = 0;

DAGNode *vertex[5010];

inline DAGNode *NewNode()
{
	return (vertex[NodeCnt++] = new DAGNode());
}

void DAGOpt(Block *block)
{
	std::map<Record*, DAGNode *> record2node;
	std::map<DAGNode*, std::map<DAGNode *, std::map<int, DAGNode*> > > DAG2,DAGArrRead;
	std::map<DAGNode*, std::map<int, std::map<int, DAGNode*>>> DAGImme;
	std::map<DAGNode *, std::map<int, DAGNode*>> DAG1, DAGAddrRead;
	NodeCnt = 0;
	Quaternion *dest = new Quaternion[510];
	Quaternion *src = block->begin, *ptr = dest;
	for (src = block->begin; src <= block->end; src++)
	{
		if (InSegment(src->command, 2, Q_ADD, Q_MUL))
		{
			DAGNode *r1 = record2node[src->params[0].record];
			DAGNode *r2 = record2node[src->params[1].record];
			if (r1 == 0)
			{
				r1 = NewNode();
				r1->add(src->params[0].record);
				record2node[src->params[0].record] = r1;
			}
			if (r2 == 0)
			{
				r2 = NewNode();
				r2->add(src->params[1].record);
				record2node[src->params[1].record] = r2;
			}
			DAGNode *t = DAG2[r1][r2][src->command];
			if (t == 0)
				t = DAG2[r2][r1][src->command];
			if (t == 0)
			{
				*dest = *src;
				dest++;
				t = NewNode();
				DAG2[r1][r2][src->command] = t;
			}
			else
			{
				if (t->ident.empty() == true)
				{
					*dest = *src, dest++;
				}
				else
				{
					dest->command = Q_ASSIGN;
					dest->params[0] = src->params[2];
					dest->params[1].record = *(t->ident.begin());
					dest++;
				}
			}
			if (record2node[src->params[2].record])
				record2node[src->params[2].record]->remove(src->params[2].record);
			record2node[src->params[2].record] = t;
			t->add(src->params[2].record);
		}
		else if (InSegment(src->command, 1, Q_READAddr))
		{
			//DAGNode *r1 = record2node[src->params[0].record];
			DAGNode *r1 = record2node[src->params[1].record];
			if (r1 == 0)
			{
				r1 = NewNode();
				r1->add(src->params[1].record);
				record2node[src->params[1].record] = r1;
			}
			DAGNode *t = DAGAddrRead[r1][src->command];
			if (t == 0)
			{
				*dest = *src; dest++;
				t = NewNode();
				DAGAddrRead[r1][src->command] = t;
			}
			else
			{
				//当前节点是不是有附着的变量，如果有，直接赋值
				if (t->ident.empty() == true)
				{
					*dest = *src, dest++;
				}
				else
				{
					dest->command = Q_ASSIGN;
					dest->params[0] = src->params[0];
					dest->params[1].record = *(t->ident.begin());
					dest++;
				}
			}
			if (record2node[src->params[0].record])
				record2node[src->params[0].record]->remove(src->params[0].record);
			record2node[src->params[0].record] = t;
			t->add(src->params[0].record);
		}
		else if (InSegment(src->command,2,Q_SUB,Q_DIV))
		{
			DAGNode *r1 = record2node[src->params[0].record];
			DAGNode *r2 = record2node[src->params[1].record];
			if (r1 == 0)
			{
				r1 = NewNode();
				r1->add(src->params[0].record);
				record2node[src->params[0].record] = r1;
			}
			if (r2 == 0)
			{
				r2 = NewNode();
				r2->add(src->params[1].record);
				record2node[src->params[1].record] = r2;
			}
			DAGNode *t = DAG2[r1][r2][src->command];
			if (t == 0)
			{
				*dest = *src;
				dest++;
				t = NewNode();
				DAG2[r1][r2][src->command] = t;
			}
			else
			{
				if (t->ident.empty() == true)
				{
					*dest = *src, dest++;
				}
				else
				{
					dest->command = Q_ASSIGN;
					dest->params[0] = src->params[2];
					dest->params[1].record = *(t->ident.begin());
					dest++;
				}
			}
			if (record2node[src->params[2].record])
				record2node[src->params[2].record]->remove(src->params[2].record);
			record2node[src->params[2].record] = t;
			t->add(src->params[2].record);
		}
		else if (InSegment(src->command,2,Q_ADD_C,Q_SUB_C))
		{
			DAGNode *r = record2node[src->params[0].record];
			if (r == 0)
			{
				r = NewNode();
				r->add(src->params[0].record);
				record2node[src->params[0].record] = r;
			}
			DAGNode *t = DAGImme[r][src->params[1].contentInt][src->command];
			if (t == 0)
			{
				*dest = *src; dest++;
				t = NewNode();
				DAGImme[r][src->params[1].contentInt][src->command] = t;
			}
			else
			{
				if (t->ident.empty() == true)
				{
					*dest = *src, dest++;
				}
				else
				{
					dest->command = Q_ASSIGN;
					dest->params[0] = src->params[2];
					dest->params[1].record = *(t->ident.begin());
					dest++;
				}
			}
			if (record2node[src->params[2].record])
				record2node[src->params[2].record]->remove(src->params[2].record);
			record2node[src->params[2].record] = t;
			t->add(src->params[2].record);
		}
		else if (src->command == Q_ARRRead)
		{
			DAGNode *r1 = record2node[src->params[0].record];
			DAGNode *r2 = record2node[src->params[1].record];
			if (r1 == 0)
			{
				r1 = NewNode();
				r1->add(src->params[0].record);
				record2node[src->params[0].record] = r1;
			}
			if (r2 == 0)
			{
				r2 = NewNode();
				r2->add(src->params[1].record);
				record2node[src->params[1].record] = r2;
			}
			DAGNode *t = DAGArrRead[r1][r2][src->command];
			if (t == 0)
			{
				*dest = *src; dest++;
				t = NewNode();
				DAGArrRead[r1][r2][src->command] = t;
			}
			else
			{
				if (t->ident.empty() == true)
				{
					*dest = *src, dest++;
				}
				else
				{
					dest->command = Q_ASSIGN;
					dest->params[0] = src->params[2];
					dest->params[1].record = *(t->ident.begin());
					dest++;
				}
			}
			if (record2node[src->params[2].record])
				record2node[src->params[2].record]->remove(src->params[2].record);
			record2node[src->params[2].record] = t;
			t->add(src->params[2].record);
		}
		else if (src->command == Q_ARRWrite)
		{
			DAGNode *r1 = record2node[src->params[0].record];
			*dest = *src, dest++;
			if (r1 != 0)
				DAGArrRead[r1].clear();
			DAGAddrRead.clear();
		}
		else if (src->command == Q_WRITEAddr || src->command == Q_CALL || src->command == Q_CALL_NULL)
		{
			*dest = *src; dest++;
			record2node.clear();
			NodeCnt = 0;
			DAG1.clear();
			DAG2.clear();
			DAGAddrRead.clear();
			DAGArrRead.clear();
			DAGImme.clear();
		}
		else if (src->command == Q_ASSIGN)
		{
			*dest = *src, dest++;
			DAGNode *r1 = record2node[src->params[1].record];
			if (r1 == 0)
			{
				r1 = NewNode();
				r1->add(src->params[1].record);
				record2node[src->params[1].record] = r1;
			}
			if (record2node[src->params[0].record])
				record2node[src->params[0].record]->remove(src->params[0].record);
			record2node[src->params[0].record] = r1;
			r1->add(src->params[0].record);
		}
		else if (src->command == Q_INPUT)
		{
			*dest = *src, dest++;
			DAGNode *r1 = NewNode();
			if (record2node[src->params[0].record])
				record2node[src->params[0].record]->remove(src->params[0].record);
			record2node[src->params[0].record] = r1;
			r1->add(src->params[0].record);
		}
		else if (src->command == Q_OPP)
		{
			DAGNode *r1 = record2node[src->params[1].record];
			if (r1 == 0)
			{
				r1 = NewNode();
				r1->add(src->params[1].record);
				record2node[src->params[1].record] = r1;
			}
			DAGNode *t = DAG1[r1][src->command];
			if (t == 0)
			{
				*dest = *src; dest++;
				t = NewNode();
				DAG1[r1][src->command] = t;
			}
			else
			{
				//当前节点是不是有附着的变量，如果有，直接赋值
				if (t->ident.empty() == true)
				{
					*dest = *src, dest++;
				}
				else
				{
					dest->command = Q_ASSIGN;
					dest->params[0] = src->params[0];
					dest->params[1].record = *(t->ident.begin());
					dest++;
				}
			}
			if (record2node[src->params[0].record])
				record2node[src->params[0].record]->remove(src->params[0].record);
			record2node[src->params[0].record] = t;
			t->add(src->params[0].record);
		}
		else
		{
			*dest = *src, dest++;
		}
	}
	block->begin = ptr;
	block->end = dest - 1;
}

std::map<char*, Block*> LabelBlock;

void Graph::GivingRegister()
{
	bool InStack[1010];
	memset(InStack, 0, sizeof InStack);
	int stack[1010], top = 0;
	int curd[1010];
	for (int i = 0; i < cnt; i++)
		curd[i] = d[i];
	while (1)
	{
		while (1)
		{
			bool Continue = 0;
			for (int i = 0; i < cnt; i++)
			{
				if (InStack[i] == 1)
					continue;
				if (curd[i] < 3)
				{
					Continue = 1;
					InStack[i] = 1;
					for (int tmp = v[i]; tmp; tmp = next[tmp])
						curd[e[tmp]]--;
					stack[++top] = i;
					break;
				}
			}
			if (Continue == 0)
				break;
		}
		for (int i = 0; i < cnt; i++)
		{
			if (InStack[i] == 0)
			{
				InStack[i] = 1;
				for (int tmp = v[i]; tmp; tmp = next[tmp])
					curd[e[tmp]]--;
				recs[i]->reg = Record::Reg::NOREG;
				stack[++top] = i;
				break;
			}
		}
		bool ShouldContinue = 0;
		for (int i = 0; i < cnt; i++)
		if (curd[i] >= 3)
			ShouldContinue = 1;
		if (ShouldContinue == 0) break;
	}
	for (top; top > 0; top--)
	{
		int i = stack[top];
		if (recs[i]->reg == Record::NOREG)
		{
			InStack[i] = 0;
			continue;
		}
		recs[i]->reg = Record::NOREG;
		bool flag[3] = { 0 };
		for (int tmp = v[i]; tmp; tmp = next[tmp])
		{
			int j = e[tmp];
			if (!InStack[j])
			if (recs[j]->reg != Record::NOREG)
				flag[recs[j]->reg] = 1;
		}
		for (int j = 0; j < 3; j++)
		if (flag[j] == 0)
		{
			recs[i]->reg = (Record::Reg)j; break;
		}
		InStack[i] = 0;
	}
}

Quaternion *MakeBlock(Quaternion *q)
{
	Block *block = AddBlock();
	if (q->command == Q_LABEL)
		LabelBlock[q->params[0].contentStr] = block;
	if (q->command == Q_ASSIGNFUNC || q->command == Q_ASSIGNPROC)
		LabelBlock[q->params[0].record->name] = block;
	block->begin = q++;

	while (q->command != Q_TERMINATE)
	{
		if (InSegment(q->command, 2, Q_ASSIGNFUNC, Q_ASSIGNPROC))//Q_LABEL))
		{
			//LabelBlock[q->params[0].record->name] = block;
			block->end = (--q);
			return ++q;
		}
		if (q->command == Q_LABEL)
		{
			//LabelBlock[q->params[0].contentStr] = block;
			block->end = (--q);
			return ++q;
		}
		if (InSegment(q->command, 9, Q_LET, Q_LT, Q_GET, Q_GT, Q_NET, Q_ET, Q_JUMP, Q_RETURN, Q_RETURN_NULL))
		{
			block->end = q;
			if (q->command == Q_RETURN || q->command == Q_RETURN_NULL)
				block->BlockProgEnd = 1;
			return ++q;
		}
		q++;
	}
	block->BlockProgEnd = 1;
	block->end = q;
	return NULL;
}

#define AddDef(q) if (!block->use.Contains(q)) block->def.add(q);
#define AddUse(q) if (!block->def.Contains(q)) block->use.add(q);

inline bool check(char *name)
{
	return 1;
	if (strlen(name) <= 5)
		return 1;
	if (name[0] == '_' && name[1] == 'T' && name[2] == 'm' && name[3] == 'p' && name[4] == '_')
		return 0;
	return 1;
}

void DataFlowInit(int i)
{
	Block *block = blocks[i];
	block->use.data.clear();
	block->def.data.clear();
	block->in.data.clear();
	block->out.data.clear();
	for (Quaternion *q = block->begin; q <= block->end; q++)
	{
		if (InSegment(q->command, 6, Q_ET, Q_LT, Q_LET, Q_GET, Q_GT, Q_NET))
		{
			//if (q->params[0].record->name[0] != '_')
			if (check(q->params[0].record->name))
				AddUse(q->params[0].record);
			//if (q->params[1].record->name[0] != '_')
			if (check(q->params[1].record->name))
				AddUse(q->params[1].record);
		}
		if (q->command == Q_ALLOC_C || q->command == Q_GLOBAL_C)
		{
			//if (q->params[0].record->name[0] != '_')
			if (check(q->params[0].record->name))
				AddDef(q->params[0].record);
		}
		else if (InSegment(q->command, 5, Q_ADD, Q_SUB, Q_MUL, Q_DIV))
		{
			//if (q->params[0].record->name[0] != '_')
			if (check(q->params[0].record->name))
			AddUse(q->params[0].record);
			//if (q->params[1].record->name[0] != '_')
			if (check(q->params[1].record->name))
			AddUse(q->params[1].record);
			//if (q->params[2].record->name[0] != '_')
			if (check(q->params[2].record->name))
			AddDef(q->params[2].record);
		}
		else if (InSegment(q->command, 3, Q_ADD_C, Q_SUB_C))
		{
			//if (q->params[0].record->name[0] != '_')
			if (check(q->params[0].record->name))
			AddUse(q->params[0].record);
			//if (q->params[2].record->name[0] != '_')
			if (check(q->params[2].record->name))
			AddDef(q->params[2].record);
		}
		else if (q->command == Q_OPP)
		{
			//if (q->params[1].record->name[0] != '_')
			if (check(q->params[1].record->name))
			AddUse(q->params[1].record);
			//if (q->params[0].record->name[0] != '_')
			if (check(q->params[0].record->name))
			AddDef(q->params[0].record);
		}
		else if (InSegment(q->command, 4, Q_ASSIGN, Q_ADDR, Q_READAddr, Q_WRITEAddr))
		{
			//if (q->params[1].record->name[0] != '_')
			if (check(q->params[1].record->name))
			AddUse(q->params[1].record);
			//if (q->params[0].record->name[0] != '_')
			if (check(q->params[0].record->name))
			AddDef(q->params[0].record);
		}
		else if (q->command == Q_ARRRead)
		{
			//if (q->params[1].record->name[0] != '_')
			if (check(q->params[1].record->name))
			AddUse(q->params[1].record);
			//if (q->params[2].record->name[0] != '_')
			if (check(q->params[2].record->name))
			AddDef(q->params[2].record);
		}
		else if (q->command == Q_ARRWrite)
		{
			//if (q->params[1].record->name[0] != '_')
			if (check(q->params[1].record->name))
			AddUse(q->params[1].record);
			//if (q->params[2].record->name[0] != '_')
			if (check(q->params[2].record->name))
			AddUse(q->params[2].record);
		}
		else if (q->command == Q_PUSH)
		{
			//if (q->params[0].record->name[0] != '_')
			if (check(q->params[0].record->name))
			AddUse(q->params[0].record);
		}
		else if (q->command == Q_CALL)
		{
			//if (q->params[1].record->name[0] != '_')
			if (check(q->params[1].record->name))
			AddDef(q->params[1].record);
		}
		else if (q->command == Q_INPUT)
		{
			//if (q->params[0].record->name[0] != '_')
			if (check(q->params[0].record->name))
			AddDef(q->params[0].record);
		}
		else if (q->command == Q_PRINTCHAR || q->command == Q_PRINTINT)
		{
			//if (q->params[0].record->name[0] != '_')
			if (check(q->params[0].record->name))
			AddUse(q->params[0].record);
		}
		else if (q->command == Q_ASSIGNFUNC || q->command == Q_ASSIGNPROC)
		{
			for (int i = 0; i < q->params[0].record->parameter.size(); i++)
			{
				if (check(q->params[0].record->name))
					//if (q->params[0].record->name[0] != '_')
				AddDef(q->params[0].record->parameter[i]);
			}
		}
		else if (q->command == Q_RETURN)
		{
			if (check(q->params[0].record->name))
				//if (q->params[0].record->name[0] != '_' && q->para)
			AddUse(q->params[0].record);
		}
	}
	if (InSegment(block->end->command, 6, Q_ET, Q_LT, Q_LET, Q_GET, Q_GT, Q_NET))
	{
		Block *to = LabelBlock[block->end->params[2].label->params[0].contentStr];
		block->postfix.add(to);
		to->prefix.add(block);
		block->postfix.add(blocks[i+1]);
	}
	else if (block->end->command == Q_JUMP)
	{
		Block *to = LabelBlock[block->end->params[0].label->params[0].contentStr];
		block->postfix.add(to);
		to->prefix.add(block);
	}
	else if (block->end->command == Q_RETURN || block->end->command == Q_RETURN_NULL)
	{
		block->postfix.add(T);
		T->prefix.add(block);
	}
	else
	{
		block->postfix.add(blocks[i+1]);
		(blocks[i+1])->prefix.add(block);
	}
}

void DataFlowCalc()
{
	bool ShouldContinue = 1;
	for (int i = 0; i < BlockCnt - 1; i++)
	{
		blocks[i]->in.data.clear();
		for (std::set<Record*>::iterator it = blocks[i]->use.data.begin(); it != (blocks[i]->use.data.end()); it++)
		{
			blocks[i]->in.add(*it);
		}
	}
	while (1)
	{
		ShouldContinue = 0;
		for (int i = 0; i < BlockCnt - 1; i++)
		{
			Block *block = blocks[i];
			for (std::set<Block*>::iterator it1 = block->postfix.data.begin(); it1 != block->postfix.data.end(); it1++)
				for (std::set<Record*>::iterator it2 = (*it1)->in.data.begin(); it2 != (*it1)->in.data.end(); it2++)
					if (!block->out.Contains(*it2))
						block->out.add(*it2);
			for (std::set<Record*>::iterator it = block->out.data.begin(); it != block->out.data.end(); it++)
			{
				if (!block->def.Contains(*it))
				{
					ShouldContinue |= !block->in.Contains(*it);
					block->in.add(*it);
				}
			}
		}
		if (ShouldContinue == 0)
			break;
	}
}

void DataFlowProgASM(int &r)
{
	std::vector<Block *> CurBlockProg;
	CurBlockProg.push_back(blocks[r]);
	GetASM(blocks[r]);
	for (int i= r + 1; i; i++)
	{
		while (blocks[i]->begin->command == Q_ASSIGNFUNC || blocks[i]->begin->command == Q_ASSIGNPROC)
			DataFlowProgASM(i);
		CurBlockProg.push_back(blocks[i]);
		if (blocks[i]->BlockProgEnd)
		{
			r = i + 1;
			break;
		}
	}
	Record **reginfo = GetRegInfo();
	for (int i = 0; i < 5; i++)
		reginfo[i] = NULL;
	Graph *graph = new Graph();
	for (std::vector<Block *>::iterator it = CurBlockProg.begin(); it != CurBlockProg.end(); it++)
	{
		Block *block = *it;
		for (std::set<Record*>::iterator it1 = block->in.data.begin(); it1 != block->in.data.end(); it1++)
		{
			for (std::set<Record*>::iterator it2 = block->def.data.begin(); it2 != block->def.data.end(); it2++)
			{
				int x = graph->Node(*it1), y = graph->Node(*it2);
				if (x != y)
					graph->Connect(x, y);
			}
		}
	}
	graph->GivingRegister();
	for (int i = 1; i < (int)CurBlockProg.size(); i++)
	{
		Block *block = CurBlockProg[i];
		GetASM(block);
	}
	delete graph;
}

Graph::~Graph()
{
	Record2Node.clear();
	//delete d, e, v, next, matrix;
}


void Output(Block *block)
{
	static FILE *fp = fopen("Block.txt", "w");
	static int i = 0;
	fprintf(fp, "Block %d\n", ++i);
	for (Quaternion *q = block->begin; q <= block->end; q++)
	{
		switch (q->command)
		{
		case Q_ADD:// = 200,
			fprintf(fp, "ADD %s %s %s", q->params[0].record->name, q->params[1].record->name, q->params[2].record->name);
			break;
		case Q_ADD_C:// = 201,
			fprintf(fp, "ADD %s %d %s", q->params[0].record->name, q->params[1].contentInt, q->params[2].record->name);
			break;
		case Q_SUB:// = 202,
			fprintf(fp, "SUB %s %s %s", q->params[0].record->name, q->params[1].record->name, q->params[2].record->name);
			break;
		case Q_SUB_C:// = 203,
			fprintf(fp, "SUB %s %d %s", q->params[0].record->name, q->params[1].contentInt, q->params[2].record->name);
			break;
		case Q_MUL:// = 204,
			fprintf(fp, "MUL %s %s %s", q->params[0].record->name, q->params[1].record->name, q->params[2].record->name);
			break;
		case Q_DIV:// = 205,
			fprintf(fp, "DIV %s %s %s", q->params[0].record->name, q->params[1].record->name, q->params[2].record->name);
			break;
		case Q_OPP:// = 206,
			fprintf(fp, "OPP %s", q->params[0].record->name);// q->params[1].record->name, q->params[2].record->name);
			break;
		case Q_ET: //= 207,
			fprintf(fp, "JNE %s %s %s", q->params[0].record->name, q->params[1].record->name, q->params[2].label->params[0].contentStr);
			break;
		case Q_LT: //= 208,
			fprintf(fp, "JNL %s %s %s", q->params[0].record->name, q->params[1].record->name, q->params[2].label->params[0].contentStr);
			break;
		case Q_LET:// = 209,
			fprintf(fp, "JNLE %s %s %s", q->params[0].record->name, q->params[1].record->name, q->params[2].label->params[0].contentStr);
			break;
		case Q_GET: //= 210,
			fprintf(fp, "JNGE %s %s %s", q->params[0].record->name, q->params[1].record->name, q->params[2].label->params[0].contentStr);
			break;
		case Q_GT: //= 211,
			fprintf(fp, "JNG %s %s %s", q->params[0].record->name, q->params[1].record->name, q->params[2].label->params[0].contentStr);
			break;
		case Q_NET: //= 212,
			fprintf(fp, "JE %s %s %s", q->params[0].record->name, q->params[1].record->name, q->params[2].label->params[0].contentStr);
			break;
		case Q_ARRRead: //= 214,
			fprintf(fp, "ARRRead %s %s %s", q->params[0].record->name, q->params[1].record->name, q->params[2].record->name);
			break;
		case Q_ARRWrite:// = 215,
			fprintf(fp, "ARRWrite %s %s %s", q->params[0].record->name, q->params[1].record->name, q->params[2].record->name);
			break;
		case Q_ASSIGN: //= 216,
			fprintf(fp, "ASSIGN %s %s", q->params[0].record->name, q->params[1].record->name);// , q->params[2].record->name);
			break;
		case Q_LABEL: //= 217,
			fprintf(fp, "LABEL %s", q->params[0].contentStr);// ->name, q->params[1].record->name, q->params[2].label->params[0].record->name);
			break;
		case Q_ASSIGNFUNC:// = 218,
			fprintf(fp, "ASSIGNFUNC %s", q->params[0].record->name);// , q->params[1].record->name, q->params[2].label->params[0].record->name);
			break;
		case Q_RETURN: //= 219,
			fprintf(fp, "RETURN %s", q->params[0].record->name); break;
		case Q_RETURN_NULL: //= 220,
			fprintf(fp, "RETURN_NULL"); break;
		case Q_ALLOC: //= 221,
			fprintf(fp, "ALLOC %s", q->params[0].record->name);
			break;
		case Q_ALLOC_C: //= 222,
			fprintf(fp, "ALLOC %s %d", q->params[0].record->name, q->params[1].contentInt);
			break;
		case Q_PUSH: //= 223,
			fprintf(fp, "PUSH %s", q->params[0].record->name);
			break;
		case Q_CALL: //= 224,
			fprintf(fp, "CALL %s", q->params[0].contentStr);
			break;
		case Q_CALL_NULL:// = 225,
			fprintf(fp, "CALL_NULL %s", q->params[0].contentStr);
			break;
		case Q_JUMP: //= 226,
			fprintf(fp, "JMP %s", q->params[0].label->params[0].contentStr); break;
		case Q_PRINTSTR:// = 227,
			fprintf(fp, "PRINTSTR %s", q->params[0].contentStr); break;
		case Q_PRINTINT: //= 228,
			fprintf(fp, "PRINTINT %s", q->params[0].record->name); break;
		case Q_PRINTCHAR: //= 229,
			fprintf(fp, "PRINTCHAR %s", q->params[0].record->name); break;
		case Q_INPUT: //= 230,
			fprintf(fp, "INPUT %s", q->params[0].record->name); break;
		case Q_GLOBAL: //= 231,
			fprintf(fp, "GLOBAL %s", q->params[0].record->name); break;
		case Q_GLOBAL_C: //= 232,
			fprintf(fp, "GLOBAL %s", q->params[0].record->name); break;
		case Q_ENDGLOBAL: //= 233,
			fprintf(fp, "ENDGLOBAL"); break;
		case Q_TERMINATE: //= 234,
			fprintf(fp, "TERMINATE"); break;
			break;
		case Q_ASSIGNPROC:// = 235,
			fprintf(fp, "ASSIGNPROC %s", q->params[0].record->name);
			break;
		case Q_ADDR: //= 236
			fprintf(fp, "ADDR %s", q->params[0].record->name); break;
		case Q_READAddr:
			fprintf(fp, "ReadAddr %s, %s", q->params[0].record->name, q->params[1].record->name);
			break;
		case Q_WRITEAddr:
			fprintf(fp, "WriteAddr %s, %s", q->params[0].record->name, q->params[1].record->name);
			break;
		default:
			break;
		}
		fputc('\n', fp);
	}
}