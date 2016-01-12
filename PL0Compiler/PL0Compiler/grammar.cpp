#include "DataStructure.h"
#include "Err.h"
#include "Function.h"
#include "grammar.h"

static Node *Match(int type, Word *&words)
{
	if (type <= ENDOFFILE)
	{
		if (type == words->type)
		{
			return new Node(words++);
		}
		else
		{
			return NULL;
		}
	}
	else
	{
		return GetNodes(type, words);
	}
}

static Node *Match(int type, Word *&ptr, int cnt, ...)
{
	Word* tmp = ptr;
	Node* node = new Node(type, 0);
	va_list ap;
	va_start(ap, cnt);
	for (int i = 0; i < cnt; i++)
	{
		Node *res = Match(va_arg(ap, int), ptr);
		if (res == NULL)
		{
			ptr = tmp;
			return NULL;
		}
		else
		{
			node->AddChild(res);
		}
	}
	va_end(ap);
	return node;
}

static void find(Word *&words, int cnt, ...)
{
	va_list ap;
	va_start(ap, cnt);
	std::vector<int> v;
	for (int i = 0; i < cnt; i++)
		v.push_back(va_arg(ap, int));
	while (words->type != ENDOFFILE)
	{
		for (int i = 0; i < cnt; i++)
		{
			if (words->type != v[i])
				continue;
			return;
		}
		words++;
	}
}

static Node *GetNumber(Word *&words)
{
	if (InSegment(words->type, 2, PLUS, MINUS))
	{
		if ((words + 1)->type == UNSIGNED)
		{
			words += 2;
			//printf("����������\n");
			return new Node(Number, 2, new Node(words - 2), new Node(words - 1));
		}
		else
		{
			PrintErr(GRAMMARERR, "�����������", words->line, words->col);
			return NULL;
		}
	}
	else if (words->type == UNSIGNED)
	{
		words++;
		//printf("�������޷�������\n");
		return new Node(Number, 1, new Node(words - 1));
	}
	else
	{
		PrintErr(GRAMMARERR, "�����������", words->line, words->col);
		return NULL;
	}
}

static Node *GetOperator(Word *&words)
{
	if (InSegment(words->type, 6, LT, LET, GT, GET, ET, NET))
	{
		words += 1;
		//printf("���������������\n");
		return new Node(Op, 1, new Node(words - 1));
	}
	else
	{
		PrintErr(GRAMMARERR, "�߼����������", words->line, words->col);
		return NULL;
	}
}

static Node *GetConstDeclare(Word *&words)
{
	Node *res = new Node(ConstDeclare, 0);
	rescan:
	if (words->type != CONST)
	{
		if (InSegment(words->type, 4,VARIABLE, PROCEDURE, FUNCTION, BEGIN))
			return res;
		PrintErr(GRAMMARERR, "����˵������", words->line, words->col);
		find(words, 5, CONST, VARIABLE, PROCEDURE, FUNCTION, BEGIN);
		if (words->type == ENDOFFILE) return NULL;
		goto rescan;
	}
	res->AddChild(new Node(words++));
	Node *t = GetConst(words);
	if (t != NULL)
		res->AddChild(t);
	//res->AddChild(t);
	while (words->type == COMMA)
	{
		res->AddChild(new Node(words++));
		Node *t = GetConst(words);
		if (t != NULL)
			res->AddChild(t);
	}
	if (words->type == SEMICOLON)
	{
		res->AddChild(new Node(words++));
	}
	else
	{
		PrintErr(GRAMMARERR, "���δ��ȷ����", words->line, words->col);
	}
	//printf("�����˳���˵��\n");
	return res;
}

static Node *GetConst(Word *&words)
{
	Node *res;
	Node *t;
rescan:
	res = new Node(Const, 0);
	if (words->type == ID)
	{
		res->AddChild(new Node(words++));
		if (words->type == ET)
		{
			res->AddChild(new Node(words++));
			if (words->type == CHARACTOR)
				res->AddChild(new Node(words++));
			else
			{
				t = GetNumber(words);
				if (t == NULL)
					goto error;
				res->AddChild(t);
			}
		}
		else goto error;
	}
	else goto error;
	//printf("�����˳�������\n");
	return res;
error:
	PrintErr(GRAMMARERR, "�����������", words->line, words->col);
	find(words, 3, ID, COMMA, SEMICOLON);
	if (words->type == ENDOFFILE) return NULL;
	if (words->type == ID) goto rescan;//FIRST
	if (words->type == COMMA || words->type == SEMICOLON) return NULL;//FOLLOW
	return NULL;
}

static Node *GetVarDeclare(Word *&words)
{
	Node *res = new Node(VarDeclare, 0);
rescan:
	if (words->type != VARIABLE)
	{
		if (InSegment(words->type, 3, PROCEDURE, FUNCTION, BEGIN))
			return res;
		PrintErr(GRAMMARERR, "����˵������", words->line, words->col);
		find(words, 4, VARIABLE, PROCEDURE, FUNCTION, BEGIN);
		if (words->type == ENDOFFILE) return NULL;
		goto rescan;
	}
	res->AddChild(new Node(words++));
	do
	{
		Node *t = GetVar(words);
		if (t != NULL) res->AddChild(t);
		if (words->type == ENDOFFILE) return NULL;
		if (words->type == SEMICOLON) res->AddChild(new Node(words++));
		else PrintErr(GRAMMARERR, "���δ��ȷ����", words->line, words->col);
	} while (!InSegment(words->type, 3, PROCEDURE, FUNCTION, BEGIN));
	//printf("�����˱���˵��\n");
	return res;
}

static Node *GetVar(Word *&words)
{
	Node *res,*t;
rescan:
	res = new Node(Var, 0);
	do
	{
		if (words->type == ID)
			res->AddChild(new Node(words++));
		else
			goto error;
		if (words->type == COMMA)
			res->AddChild(new Node(words++));
		else if (words->type == COLON)
		{
			res->AddChild(new Node(words++));
			if (InSegment(words->type, 2, INT, CHAR))
				res->AddChild(new Node(words++));
			else
			{
				t = Match(Type, words,6, ARRAY, LBRACHET, UNSIGNED, RBRACHET, OF, INT);
				if (t == NULL)
					t = Match(Type, words, 6, ARRAY, LBRACHET, UNSIGNED, RBRACHET, OF, CHAR);
				if (t == NULL)
					goto error;
				res->Merge(t);
			}
			return res;
		}
		else
			goto error;
	} while (1);
	//printf("�����˳�������\n");
	return res;
error:
	PrintErr(GRAMMARERR, "�����������", words->line, words->col);
	find(words, 2, ID, SEMICOLON);
	if (words->type == ID) goto rescan; //FIRST
	if (words->type == SEMICOLON) return res; //FOLLOW
	if (words->type == ENDOFFILE) return NULL; 
	return NULL;
}

static Node *GetFactor(Word *&words)
{
	//printf("����������\n");
	Node *res = new Node(Factor, 0);
	Word *back = words;
	Node *t;
	switch (words->type)
	{
	case LPAREN:
		res->AddChild(new Node(words++));
		t = GetExp(words);
		if (t == NULL)
			goto error;
		res->AddChild(t);
		if (words->type == RPAREN)
			res->AddChild(new Node(words++));
		else 
		{
			PrintErr(GRAMMARERR, "ȱ��������", words->line, words->col); goto error;
		}
		return res;
	case UNSIGNED:
		res->AddChild(new Node(words++));
		return res;
	case ID:
		res->AddChild(new Node(words++));
		if (words->type == LBRACHET)
		{
			res->AddChild(new Node(words++));
			t = GetExp(words);
			if (t == NULL) goto error;
			res->AddChild(t);
			if (words->type != RBRACHET)
			{
				PrintErr(GRAMMARERR, "ȱ���ҷ�����", words->line, words->col); goto error;
			}
			res->AddChild(new Node(words++));
			return res;
		}
		else if (words->type == LPAREN)
		{
			res->AddChild(new Node(words++));
			while (1)
			{
				t = GetExp(words);
				if (t == NULL) goto error;
				res->AddChild(t);
				if (words->type == COMMA)
					res->AddChild(new Node(words++));
				else if (words->type == RPAREN)
				{
					res->AddChild(new Node(words++));
					return res;
				}
				else 		
				{
					PrintErr(GRAMMARERR, "ȱ��������", words->line, words->col); goto error;
				}
			}
		}
		else return res;
	default:
		break;
	}
error:
	PrintErr(GRAMMARERR, "���Ӷ������", words->line, words->col);
	words = back;
	return NULL;
}

static Node *GetExp(Word *&words)
{
	//printf("�����˱��ʽ\n");
	Node *res = new Node(Exp, 0);
	Node *t;
	Word *back = words;
	if (InSegment(words->type, 2, PLUS, MINUS))
		res->AddChild(new Node(words++));
	while (1)
	{
		t = GetTerm(words);
		if (t == NULL)
			goto error;
		res->AddChild(t);
		if (!InSegment(words->type, 2, PLUS, MINUS))
			return res;
		res->AddChild(new Node(words++));
	}
error:
	PrintErr(GRAMMARERR, "���ʽ�������", words->line, words->col);
	words = back;
	return NULL;
}

static Node *GetJdgmt(Word *&words)
{
	Node *res;
	Node *t;
	Word *back = words;
	res = new Node(Jdgmt, 0);
	t = GetExp(words);
	if (t != NULL)
		res->AddChild(t);
	else goto error;
	t = GetOperator(words);
	if (t == NULL)
		return res;
	res->AddChild(t);
	t = GetExp(words);
	if (t != NULL)
		res->AddChild(t);
	else goto error;
	//printf("����������\n");
	return res;
error:
	find(words, 3, THEN, SEMICOLON, END);
	//if (words->type == ENDOFFILE) return NULL;
	words = back;
	return NULL;
}

static Node *GetStmt(Word *&words)
{
	//printf("���������\n");
	Node *res;
	Node *t;
	res = new Node(Stmt, 0);
	switch (words->type)
	{
	case IF:
		res->AddChild(new Node(words++));
		t = GetJdgmt(words);
		if (t != NULL) res->AddChild(t);
		if (words->type == THEN)
			res->AddChild(new Node(words++));
		else 
		{
			PrintErr(GRAMMARERR, "ȱ��then", words->line, words->col); goto error;
		}
		t = GetStmt(words);
		if (t != NULL) res->AddChild(t);
		if (words->type == ELSE)
		{
			res->AddChild(new Node(words++));
			t = GetStmt(words);
			if (t != NULL) res->AddChild(t);
		}
		return res;
		break;
	case FOR:
		res->AddChild(new Node(words++));
		if (words->type == ID)
			res->AddChild(new Node(words++));
		else
		{
			PrintErr(GRAMMARERR, "ȱ�ٱ�ʶ��", words->line, words->col); goto error;
		}
		if (words->type == ASSIGN)
			res->AddChild(new Node(words++));
		else 
		{
			PrintErr(GRAMMARERR, "ȱ�ٸ�ֵ", words->line, words->col); goto error;
		}
		t = GetExp(words);
		if (t != NULL) res->AddChild(t);
		else goto error;
		if (InSegment(words->type, 2, DOWNTO, TO))
			res->AddChild(new Node(words++));
		else 		
		{
			PrintErr(GRAMMARERR, "ȱ�ٲ���", words->line, words->col); goto error;
		}
		t = GetExp(words);
		if (t != NULL) res->AddChild(t);
		else goto error;
		if (words->type == DO)
			res->AddChild(new Node(words++));
		else 
		{
			PrintErr(GRAMMARERR, "ȱ��do", words->line, words->col); goto error;
		}
		t = GetStmt(words);
		if (t != NULL) res->AddChild(t);
		return res;
	case DO:
		res->AddChild(new Node(words++));
		t = GetStmt(words);
		if (t != NULL) res->AddChild(t);
		if (words->type == WHILE)
			res->AddChild(new Node(words++));
		else 
		{
			PrintErr(GRAMMARERR, "ȱ��while", words->line, words->col); goto error;
		}
		t = GetJdgmt(words);
		if (t != NULL) res->AddChild(t);
		return res;
	case BEGIN:
		t = GetComplex(words);
		if (t != NULL) res->AddChild(t);
		return res;
	case READ:
		res->AddChild(new Node(words++));
		if (words->type == LPAREN)
			res->AddChild(new Node(words++));
		else 
		{
			PrintErr(GRAMMARERR, "ȱ��������", words->line, words->col); goto error;
		}
		do
		{
			if (words->type == ID)
				res->AddChild(new Node(words++));
			else 
			{
				PrintErr(GRAMMARERR, "ȱ�ٱ�ʶ��", words->line, words->col); goto error;
			}
			if (words->type == COMMA)
				res->AddChild(new Node(words++));
			else if (words->type == RPAREN)
			{
				res->AddChild(new Node(words++));
				return res;
			}
			else goto error;
		} while (1);
		break;
	case WRITE:
		res = Match(Stmt, words, 6, WRITE, LPAREN, STRING, COMMA, Exp, RPAREN);
		if (res == NULL) res = Match(Stmt, words, 4, WRITE, LPAREN, STRING, RPAREN);
		if (res == NULL) res = Match(Stmt, words, 4, WRITE, LPAREN, Exp, RPAREN);
		if (res == NULL) 		
		{
			PrintErr(GRAMMARERR, "write������", words->line, words->col); goto error;
		}
		return res;
	case END:
		return res;
	case ELSE:
		return res;
	case WHILE:
		return res;
	case SEMICOLON:
		return res;
	case ID:
		res->AddChild(new Node(words++));
		if (words->type == LPAREN)
		{
			res->AddChild(new Node(words++));
			do
			{
				t = GetExp(words);
				if (t != NULL) res->AddChild(t);
				else 		
				{
					PrintErr(GRAMMARERR, "��������", words->line, words->col); goto error;
				}
				if (words->type == COMMA)
					res->AddChild(new Node(words++));
				else if (words->type == RPAREN)
				{
					res->AddChild(new Node(words++));
					return res;
				}
				else if (words->type == ENDOFFILE)
					return NULL;
				else 		
				{
					PrintErr(GRAMMARERR, "��������", words->line, words->col); goto error;
				}
			} while (1);
		}
		else if (words->type == LBRACHET)
		{
			res->AddChild(new Node(words++));
			t = GetExp(words);
			if (t != NULL) res->AddChild(t);
			else 		
			{
				PrintErr(GRAMMARERR, "�����±����", words->line, words->col); goto error;
			}
			if (words->type == RBRACHET) res->AddChild(new Node(words++));
			else 		
			{
				PrintErr(GRAMMARERR, "ȱ���ҷ�����", words->line, words->col); goto error;
			}
			if (words->type == ASSIGN)
				res->AddChild(new Node(words++));
			else 		
			{
				PrintErr(GRAMMARERR, "ȱ�ٸ�ֵ", words->line, words->col); goto error;
			}
			t = GetExp(words);
			if (t != NULL) res->AddChild(t);
			else goto error;
		}
		else if (words->type == ASSIGN)
		{
			res->AddChild(new Node(words++));
			t = GetExp(words);
			if (t != NULL) res->AddChild(t);
			else goto error;
		}
		else if (words->type == END || words->type == SEMICOLON)
			return res;
		else goto error;
		return res;
	default:
		goto error;
		break;
	}
error:
	PrintErr(GRAMMARERR, "������", words->line, words->col);
	find(words, 4, SEMICOLON, END, WHILE, ELSE);
	if (words->type == ENDOFFILE) return NULL;
	return res;
}

static Node *GetComplex(Word *&words)
{
	Node *res;
	Node *t;
rescan:
	res = new Node(Complex, 0);
	if (words->type == BEGIN)
		res->AddChild(new Node(words++));
	else goto error;
	do
	{
		t = GetStmt(words);
		if (t != NULL)
			res->AddChild(t);
		if (words->type == END)
		{
			res->AddChild(new Node(words++));
			//printf("�����˸������\n");
			return res;
		}
		else if (words->type == SEMICOLON)
			res->AddChild(new Node(words++));
		else if (words->type == ENDOFFILE)
			return NULL;
		else goto error;
	} while (1);
error:
	PrintErr(GRAMMARERR, "����������", words->line, words->col);
	find(words, 6, BEGIN, SEMICOLON, ENDOFPROG, WHILE, ELSE, END);
	if (words->type == ENDOFFILE) return NULL;
	if (words->type == BEGIN) goto rescan;
	return res;
}

static Node *GetFunction(Word *&words)
{
	Node *res;
	res = new Node(Function, 0);
	Node *t = GetFunctionHead(words);
	if (t != NULL)
		res->AddChild(t);
	t = GetBlockProgram(words);
	if (t != NULL)
		res->AddChild(t);
	//printf("�����˺�������\n");
	return res;
}

static Node *GetFunctionDeclare(Word *&words)
{
	Node *res;
	res = new Node(FunctionDeclare, 0);
rescan:
	if (words->type != FUNCTION)
	{
		if (words->type == BEGIN)
			return res;
		PrintErr(GRAMMARERR, "�����������", words->line, words->col);
		find(words, 2, FUNCTION, BEGIN);
		if (words->type == ENDOFFILE)
			return NULL;
		goto rescan;
	}
	while (words->type == FUNCTION)
	{
		Node *t = GetFunction(words);
		if (t != NULL)
			res->AddChild(t);
		if (words->type == SEMICOLON)
			res->AddChild(new Node(words++));
		else
		{
			PrintErr(GRAMMARERR, "���δ��ȷ����", words->line, words->col);
			find(words, 3,PROCEDURE, FUNCTION, BEGIN);
			if (words->type == ENDOFFILE)
				return NULL;
			return res;
		}
	}
	//printf("�����˺���˵��\n");
	return res;
}

static Node *GetProcedureHead(Word *&words)
{
	Node *res;
	Node *t;
	res = new Node(ProcedureHead, 0);
	if (words->type == PROCEDURE)
		res->AddChild(new Node(words++));
	else 	
	{
		PrintErr(GRAMMARERR, "���̴���������procedure", words->line, words->col); goto error;
	}
	if (words->type == ID)
		res->AddChild(new Node(words++));
	else 	
	{
		PrintErr(GRAMMARERR, "���̱�ʶ������", words->line, words->col); goto error;
	}
	if (words->type == LPAREN)
	{
		t = GetParameterList(words);
		if (t != NULL)
			res->AddChild(t);
	}
	if (words->type == SEMICOLON)
		res->AddChild(new Node(words++));
	else 
	{
		PrintErr(GRAMMARERR, "ȱ�ٷֺ�", words->line, words->col); goto error;
	}
	//printf("�����˹����ײ�\n");
	return res;
error:
	PrintErr(GRAMMARERR, "�����ײ�����", words->line, words->col);
	find(words, 5, CONST, VARIABLE, PROCEDURE, FUNCTION, BEGIN);//FOLLOW����
	if (words->type == ENDOFFILE)
		return NULL;
	return res;
}

static Node *GetParameter(Word *&words)
{
	Node *res;
rescan:
	res = new Node(Parameter, 0);
	do
	{
		if (words->type == VARIABLE)
			res->AddChild(new Node(words++));
		if (words->type == ID)
			res->AddChild(new Node(words++));
		else
		{
			PrintErr(GRAMMARERR, "��������", words->line, words->col); goto error;
		}
		if (words->type == COMMA)
			res->AddChild(new Node(words++));
		else if (words->type == COLON)
		{
			res->AddChild(new Node(words++));
			if (InSegment(words->type, 2, INT, CHAR))
				res->AddChild(new Node(words++));
			else 
			{
				PrintErr(GRAMMARERR, "�������ʹ���", words->line, words->col); goto error;
			}
			return res;
		}
		else
			goto error;
	} while (1);
	//printf("�����˲�����\n");
	return res;
error:
	PrintErr(GRAMMARERR, "�����ζ������", words->line, words->col);
	find(words, 3, ID, SEMICOLON,RPAREN);
	if (words->type == ENDOFFILE)
		return NULL;
	if (words->type == SEMICOLON || words->type == RPAREN) return res; //FOLLOW
	if (words->type == ID) goto rescan;	//FIRST
	return NULL;
}

static Node *GetFunctionHead(Word *&words)
{
	Node *res;
	Node *t;
	res = new Node(FunctionHead, 0);
	if (words->type == FUNCTION)
		res->AddChild(new Node(words++));
	else 	
	{
		PrintErr(GRAMMARERR, "����ͷ������������function", words->line, words->col); goto error;
	}
	if (words->type == ID)
		res->AddChild(new Node(words++));
	else 	{
		PrintErr(GRAMMARERR, "������ʶ������", words->line, words->col); goto error;
	}
	if (words->type == LPAREN)
	{
		t = GetParameterList(words);
		if (t != NULL)
			res->AddChild(t);
	}
	if (words->type == COLON)
		res->AddChild(new Node(words++));
	else 	
	{
		PrintErr(GRAMMARERR, "ȱ�ٶ���", words->line, words->col); goto error;
	}
	if (InSegment(words->type, 2, CHAR, INT))
		res->AddChild(new Node(words++));
	else
	{
		PrintErr(GRAMMARERR, "�������ʹ���", words->line, words->col); goto error;
	}
	if (words->type == SEMICOLON)
		res->AddChild(new Node(words++));
	else 
	{
		PrintErr(GRAMMARERR, "ȱ�ٷֺ�", words->line, words->col); goto error;
	}
	////printf("�����˺����ײ�\n");
	return res;
error:
	PrintErr(GRAMMARERR, "�����ײ�����", words->line, words->col);
	find(words, 5, CONST, VARIABLE, PROCEDURE, FUNCTION, BEGIN);//FOLLOW����
	if (words->type == ENDOFFILE)
		return NULL;
	return res;
}

static Node *GetParameterList(Word *&words)
{
	Node *res;
	Node *t;
	res = new Node(ParameterList, 0);
	if (words->type == LPAREN)
		res->AddChild(new Node(words++));
	else 	
	{
		PrintErr(GRAMMARERR, "ȱ��������", words->line, words->col); goto error;
	}
	t = GetParameter(words);
	if (t != NULL)
		res->AddChild(t);
	while (words->type == SEMICOLON)
	{
		t = GetParameter(words);
		if (t != NULL)
			res->AddChild(t);
	}
	if (words->type == RPAREN)
		res->AddChild(new Node(words++));
	else
	{
		PrintErr(GRAMMARERR, "ȱ��������", words->line, words->col); goto error;
	}
	////printf("�����˲�����\n");
	return res;
error:
	PrintErr(GRAMMARERR, "�����������", words->line, words->col);
	find(words, 1, SEMICOLON);
	if (words->type == ENDOFFILE)
		return NULL;
	return res;
}

static Node *GetProcedure(Word *&words)
{
	Node *res;
	res = new Node(Procedure, 0);
	Node *t = GetProcedureHead(words);
	if (t != NULL)
		res->AddChild(t);
	t = GetBlockProgram(words);
	if (t != NULL)
		res->AddChild(t);
	//printf("�����˹��̶���\n");
	return res;
}

static Node *GetProcedureDeclare(Word *&words)
{
	Node *res = new Node(ProcedureDeclare, 0);
	rescan:
	if (words->type != PROCEDURE)
	{
		if (InSegment(words->type, 2, FUNCTION, BEGIN))
			return res;
		PrintErr(GRAMMARERR, "���̶������", words->line, words->col);
		find(words, 3, PROCEDURE, FUNCTION, BEGIN);
		if (words->type == ENDOFFILE)
			return NULL;
		goto rescan;
	}
	while (words->type == PROCEDURE)
	{
		Node *t = GetProcedure(words);
		if (t != NULL)
			res->AddChild(t);
		if (words->type == SEMICOLON)
			res->AddChild(new Node(words++));
		else
		{
			PrintErr(GRAMMARERR, "���δ��ȷ����", words->line, words->col);
			find(words, 3, PROCEDURE, FUNCTION, BEGIN);
			if (words->type == ENDOFFILE)
				return NULL;
			continue;
		}
	}
	//printf("�����˹���˵��\n");
	return res;
}

static Node *GetBlockProgram(Word *&words)
{
	Node *res = new Node(BlockProgram, 0);
	Node *t;
	t = GetConstDeclare(words);
	if (t != NULL)
		res->AddChild(t);
	t = GetVarDeclare(words);
	if (t != NULL)
		res->AddChild(t);
	while (words->type != BEGIN)
	{
		if (words->type == PROCEDURE)
		{
			t = GetProcedureDeclare(words);
			if (t != NULL)
				res->AddChild(t);
		}
		if (words->type == FUNCTION)
		{
			t = GetFunctionDeclare(words);
			if (t != NULL)
				res->AddChild(t);
		}
	}
	t = GetComplex(words);
	if (t != NULL)
		res->AddChild(t);
	//printf("�����˷ֳ���\n");
	return res;
}

static Node *GetProgram(Word *&words)
{
	Node *res = new Node(Program, 0);
	Node *t = GetBlockProgram(words);
	res->AddChild(t);
	if (words->type != ENDOFPROG)
		goto error;
	else
		res->AddChild(new Node(words++));
	if (words->type != ENDOFFILE) 
	{
		PrintErr(GRAMMARERR, "����δ��ȷ����\n", words->line, words->col); 
		goto error;
	}
	return res;

error:
	PrintErr(GRAMMARERR, "�޷�ʶ��ı�ʶ��", words->line, words->col);
	return NULL;
}

static Node *GetTerm(Word *&words)
{
	Node *res = new Node(Term, 0);
	Word *back = words;
	while (1)
	{
		Node *t = GetFactor(words);
		if (t == NULL)
			goto error;
		res->AddChild(t);
		if (!InSegment(words->type, 2, MULT, DIV))
			return res;
		res->AddChild(new Node(words++));
	}
error:
	PrintErr(GRAMMARERR, "������", words->line, words->col);
	words = back;
	return NULL;
}

Node *GetNodes(int type, Word*&words)
{
	switch (type)
	{
	case Number:
		return GetNumber(words);
	case Op:
		return GetOperator(words);
	case ConstDeclare:
		return GetConstDeclare(words);
	case Const:
		return GetConst(words);
	case VarDeclare:
		return GetVarDeclare(words);
	case Var:
		return GetVar(words);
	case Factor:
		return GetFactor(words);
	case Exp:
		return GetExp(words);
	case Jdgmt:
		return GetJdgmt(words);
	case Stmt:
		return GetStmt(words);
	case Complex:
		return GetComplex(words);
	case Function:
		return GetFunction(words);
	case FunctionDeclare:
		return GetFunctionDeclare(words);
	case ProcedureHead:
		return GetProcedureHead(words);
	case Parameter:
		return GetParameter(words);
	case FunctionHead:
		return GetFunctionHead(words);
	case ParameterList:
		return GetParameterList(words);
	case Procedure:
		return GetProcedure(words);
	case ProcedureDeclare:
		return GetProcedureDeclare(words);
	case BlockProgram:
		return GetBlockProgram(words);
	case Program:
		return GetProgram(words);
	case Term:
		return GetTerm(words);
	default:
		return NULL;
		break;
	}
}