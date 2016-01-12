#pragma warning(disable:4996)
#pragma once

#ifndef DATASTRUCTURE_H
#define DATASTRUCTURE_H

#include <iostream>
#include <vector>

const char TerminalWord[][20] =
{
	"const", "var", "integer", "char", "array", "of", "+", "-", "*", "/", ":=", "=", "<", "<=", ">", ">=", "<>", "procedure", "function",
	"if", "then", "else", "do", "while", "for", "begin", "end", "read", "write", "(", ")", "[", "]", ";", ",", "'", "\"", ":", ".", "to", "downto"
};

const int TerminalWordCnt = 41;

enum WordType
{
	SPILIT = 0, CONST = 1, VARIABLE = 2, INT = 3, CHAR = 4, ARRAY = 5, OF = 6,
	PLUS = 7, MINUS = 8, MULT = 9, DIV = 10,
	ASSIGN = 11, ET = 12, LT = 13, LET = 14, GT = 15, GET = 16, NET = 17,
	PROCEDURE = 18, FUNCTION = 19,
	IF = 20, THEN = 21, ELSE = 22, DO = 23, WHILE = 24, FOR = 25, BEGIN = 26, END = 27, READ = 28, WRITE = 29,
	LPAREN = 30, RPAREN = 31, LBRACHET = 32, RBRACHET = 33, SEMICOLON = 34, COMMA = 35, SQUOTE = 36, DQUOTE = 37, COLON = 38,
	ID = 44, UNSIGNED = 45, OP = 46, CHARACTOR = 42, STRING = 43, ENDOFPROG = 39,TO = 40, ENDOFFILE = 100,DOWNTO = 41
};

struct Word
{
	char *word;
	int type;
	Word(int type, char *str)
	{
		this->type = type;
		this->word = str;
	}
	Word()
	{
		word = NULL;
	}
	int col, line;
	~Word()
	{
		if (word != NULL)
		{
			delete word;
			word = NULL;
		}
	}
	int GetVal();
};

enum NodeType
{
	Number = 101, Op = 102, Const = 103, ConstDeclare = 104, Factor = 105, Term = 106,
	Exp = 107, Jdgmt = 108, Stmt = 109, VarDeclare = 110, Var = 111, Complex = 112, Procedure = 113,
	ProcedureDeclare = 114, Function = 115, FunctionDeclare = 116, BlockProgram = 117, Program = 118,
	EndProg = 119, Type = 120,ProcedureHead = 121, FunctionHead = 122,ParameterList = 123,Parameter = 124
};

struct Node
{
	bool IsWord;
	int type;
	std::vector<Node*> childs;
	Word *word;
	struct
	{
		int col, line;
	} pos;
	Node(int type, int cnt, ...);
	Node(Word *wd)
	{
		this->word = wd;
		this->IsWord = 1;
		this->pos.line = wd->line;
		this->pos.col = wd->col;
	}
	bool AddChild(Node *nd)
	{
		if (IsWord)
			return 0;
		this->childs.push_back(nd);
		if (this->childs.size() == 1)
		{
			this->pos = nd->pos;
		}
		return 1;
	}
	bool Merge(Node *nd);
	int GetVal();
};

enum Register
{
	NOREG = 0,
	EBX = 1,
	ECX = 2,
	ESI = 3,
	EDI = 4,
	REG_MAX = 5
};

struct Record
{
	char *name;
	enum Reg
	{
		NOREG = 4,
		ECX = 0,
		EDI = 1,
		ESI = 2
	} reg;
	enum
	{
		variable, constance, proc, func,reference
	}RecType;
	struct
	{
		int type;
		bool IsArr;
		int len;
	} VarType;
	int startAddr;
	bool isGlobal;
	char  *GetDest(char *buff);
	int GetSize()
	{
		if (this->RecType == func)
		{
			return 0;
		}
		else
		{
			return VarType.IsArr ? (4 * VarType.len) : 4;
		}
	}
	int Layer;
	//Record *parameters[MAXPARAM];
	std::vector<Record *> parameter;
	//int   paraType[MAXPARAM];
	std::vector<int> paratype;
	//int parameterCnt;
	bool Imme;
	int ImmeData;
	int LocalDataSize;
	Record();
//	Register reg;
};

enum QuaternionCommand
{
	Q_ADD = 200,
	Q_ADD_C = 201,
	Q_SUB = 202,
	Q_SUB_C = 203,
	Q_MUL = 204,
	Q_DIV = 205,
	Q_OPP = 206,
	Q_ET = 207,
	Q_LT = 208,
	Q_LET = 209,
	Q_GET = 210,
	Q_GT = 211,
	Q_NET = 212,
	Q_NETZ = 213,
	Q_ARRRead = 214,
	Q_ARRWrite = 215,
	Q_ASSIGN = 216,
	Q_LABEL = 217,
	Q_ASSIGNFUNC = 218,
	Q_RETURN = 219,
	Q_RETURN_NULL = 220,
	Q_ALLOC = 221,
	Q_ALLOC_C = 222,
	Q_PUSH = 223,
	Q_CALL = 224,
	Q_CALL_NULL = 225,
	Q_JUMP = 226,
	Q_PRINTSTR = 227,
	Q_PRINTINT = 228,
	Q_PRINTCHAR = 229,
	Q_INPUT = 230,
	Q_GLOBAL = 231,
	Q_GLOBAL_C = 232,
	Q_ENDGLOBAL = 233,
	Q_TERMINATE = 234,
	Q_ASSIGNPROC = 235,
	Q_ADDR = 236,
	Q_READAddr = 237,
	Q_WRITEAddr = 238,
	Q_MAIN = 239,
	Q_LOCAL = 240
};

struct Quaternion
{
	int command;
	union
	{
		char *contentStr = NULL;
		int contentInt;
		char contentChar;
		Quaternion *label;
		Record *record;
	} params[3];
	static int LabelCnt;
	static char *CreateLabel()
	{
		char *res = new char[20];
		sprintf(res, "_label_%d", LabelCnt++);
		return res;
	}
};

struct Table
{
	static Record *records[2010];
	static int RecordsCnt;
	static int layers[30];
	static int CurLayer;
};

#endif