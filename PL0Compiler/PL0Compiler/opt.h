#pragma once
#pragma warning(disable:4996)

#ifndef OPT_H
#define OPT_H

#include <set>
#include <map>
#include <vector>

using std::set;

struct Quaternion;

template<typename T>
struct Set
{
	std::set<T> data;
	bool Contains(T v)
	{
		return data.find(v) != data.end();
	}
	void add(T v)
	{
		data.insert(v);
	}
};

struct Record;

struct Block
{
	Quaternion *begin;
	Quaternion *end;
	bool BlockProgEnd = 0;
	Set<Block*> prefix, postfix;
	Set<Record*> in, out, use, def;
};

struct Graph
{
	std::map<Record*, int> Record2Node;
	std::vector<Record*> recs;
	int cnt;
	bool matrix[501][501];
	int e[20100], v[501], next[20100], edge;
	int d[501];
	int Node(Record* rec)
	{
		if (Record2Node.find(rec) == Record2Node.end())
			return recs.push_back(rec), Record2Node[rec] = cnt++;
		else
			return Record2Node[rec];
	}
	void Connect(int x, int y)
	{
		if (matrix[x][y])
			return;
		matrix[x][y] = matrix[y][x] = true;
		d[x]++, d[y]++;
		next[++edge] = v[x], v[x] = edge, e[edge] = y;
		next[++edge] = v[y], v[y] = edge, e[edge] = x;
	}
	void GivingRegister();
	Graph()
	{
		memset(e, 0, sizeof e);
		memset(v, 0, sizeof v);
		memset(next, 0, sizeof next);
		memset(d, 0, sizeof d);
		memset(matrix, 0, sizeof matrix);
		cnt = edge = 0;
		Record2Node.clear();
		recs.clear();
	}
	~Graph();
};

Quaternion *MakeBlock(Quaternion *q);
Block *AddBlock();
void Output(Block *block);
void DAGOpt(Block *block);

void DataFlowInit(int);
void DataFlowCalc();
void DataFlowProgASM(int &);

#endif