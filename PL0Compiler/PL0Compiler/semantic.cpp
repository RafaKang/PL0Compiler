#include "semantic.h"
#include "DataStructure.h"
#include "Err.h"
#include "Function.h"
#include <cstdarg>
#include <cassert>
#include <vector>
#include <cstdio>
#include <algorithm>
#include <stack>

static Quaternion *base, *ptr;

int Quaternion::LabelCnt = 0;

Quaternion *AddQuaternion()
{
	return ptr++;
}

void CreateJmp(Quaternion *&jmp, Quaternion *label)
{
	jmp = AddQuaternion();
	jmp->command = Q_JUMP;
	jmp->params[0].label = label;
}

void CreateLabel(Quaternion *&label)
{
	label = AddQuaternion();
	label->command = Q_LABEL;
	label->params[0].contentStr = Quaternion::CreateLabel();
}

void Create(Quaternion *&q, int type)
{
	q = AddQuaternion();
	q->command = type;
}

Quaternion *GetQuaternion(Node *prog, Quaternion *dest)
{
	base = ptr = dest;
	if (prog->type != Program) return NULL;
	InitTable();
	MakeBlockProgram(prog->childs[0], true);
	ptr->command = Q_TERMINATE;
	return base;
}

void MakeBlockProgram(Node *blockprog, bool global = false)
{
	MakeConsts(blockprog->childs[0], global);
	MakeVaribles(blockprog->childs[1], global);
	if (global)
	{
		Quaternion *GlobalEnd = AddQuaternion();
		GlobalEnd->command = Q_ENDGLOBAL;
	}
	Quaternion *q;
	CreateJmp(q, NULL);
	int i = 2;
	for (i = 2; blockprog->childs[i]->type != Complex; i++)
	{
		if (blockprog->childs[i]->type == ProcedureDeclare)
			for (int j = 0; j < (int)blockprog->childs[i]->childs.size(); j += 2)
				MakeProcedure(blockprog->childs[i]->childs[j]);
		else
			for (int j = 0; j < (int)blockprog->childs[i]->childs.size(); j += 2)
				MakeFunction(blockprog->childs[i]->childs[j]);
	}
	Quaternion *label;
	CreateLabel(label);
	q->params[0].label = label;
	if (global)
		Create(q, Q_MAIN);
	MakeComplex(blockprog->childs[i]);
	if (global)
		q->params[0].contentInt = GetVarSize();
}

void MakeConsts(Node *consts, bool global = false)
{
	for (int i = 1; i < (int) consts->childs.size() - 1; i+=2)
	{
		Node *node = consts->childs[i];
		if (InTableData(node->childs[0]->word->word) != NULL)
		{
			PrintErr(SEMANTICERR, "重复定义常量名", node->childs[0]->pos.line, node->childs[0]->pos.col);
			continue;
		}
		Record *rec = AddIDTable();
		rec->name = node->childs[0]->word->word;
		rec->RecType = Record::constance;
		rec->isGlobal = global;
		if (node->childs[2]->type == Number)
			rec->VarType.type = INT;
		else
			rec->VarType.type = CHAR;
		rec->VarType.IsArr = false;
		Quaternion *q = AddQuaternion();
		if (rec->VarType.type == INT)
			q->params[1].contentInt = node->childs[2]->GetVal();
		else if (rec->VarType.type == CHAR)
			q->params[1].contentChar = node->childs[2]->word->word[0];
		q->command = global ? Q_GLOBAL_C : Q_ALLOC_C;
		q->params[0].record = rec;
	}
}

void MakeVaribles(Node *var, bool global = false)
{
	for (int i = 1; i < (int) var->childs.size() - 1; i+=2)
	{
		Node *node = var->childs[i];
		std::vector<Record*> v;
		v.clear();
		int Size = node->childs.size();
		int type = node->childs[Size - 1]->word->type;// == INT)
		int k;
		for (int j = 0; j < Size; j++)
		{
			if (InTableData(node->childs[j]->word->word) != NULL)
			{
				PrintErr(SEMANTICERR, "重复定义变量名", node->childs[j]->word->line, node->childs[j]->word->col);
			}
			else
			{
				Record *rec = AddIDTable();
				rec->name = node->childs[j]->word->word;
				rec->RecType = Record::variable;
				rec->isGlobal = global;
				rec->VarType.type = type;
				v.push_back(rec);
			}
			if (node->childs[++j]->word->type == COMMA)
				continue;
			else if (node->childs[j++]->word->type == COLON)
			{
				k = j;
				break;
			}
		}
		bool IsArr = false;
		int lenth = 0;
		if (node->childs[k]->word->type == ARRAY)
		{
			IsArr = true;
			lenth = node->childs[k + 2]->GetVal();
		}
		for (int j = 0; j < (int) v.size(); j++)
		{
			v[j]->VarType.IsArr = IsArr;
			v[j]->VarType.len = lenth;
			Quaternion *q = AddQuaternion();
			q->command = global ? Q_GLOBAL : Q_ALLOC;
			q->params[0].record = v[j];
		}
	}
}

void MakeProcedure(Node *proc)
{
	Record *procsym = NULL;
	Quaternion *procname;
	Node *Head = proc->childs[0];
	if (InTableData(Head->childs[1]->word->word))
	{
		PrintErr(SEMANTICERR, "过程名重复定义", Head->childs[1]->pos.line, Head->childs[1]->pos.col);
		return;
	}
	procsym = AddIDTable();
	procsym->name = Head->childs[1]->word->word;
	procsym->RecType = Record::proc;
	procsym->VarType.type = 0;
	procsym->VarType.IsArr = false;
	IncLayer();
	procname = AddQuaternion();
	procname->command = Q_ASSIGNPROC;
	procname->params[0].record = procsym;
	if (Head->childs[2]->type == ParameterList)
	{
		Node *PList = Head->childs[2];
		for (int i = 1; i < (int)PList->childs.size() - 1; i+=2)
		{
			Node *P = PList->childs[i];
			int j = 0;
			if (P->childs[0]->word->type == VARIABLE)
				j = 1;
			int Size = P->childs.size();
			int type = INT;
			if (P->childs[Size - 1]->word->type == CHAR)
				type = CHAR;
			for (j; j < Size - 1; j++)
			{
				if (InTableData(P->childs[j]->word->word) != NULL)
				{
					PrintErr(SEMANTICERR, "参数重复定义", P->childs[j]->pos.line, P->childs[j]->pos.col);
				}
				else
				{
					Record *rec = AddIDTable();
					rec->RecType = (j%2) ? Record::reference : Record::variable;
					rec->name = P->childs[j]->word->word;
					rec->VarType.type = type;
					rec->VarType.IsArr = false;
					if (procsym != NULL)
					{
						procsym->parameter.push_back(rec);
						procsym->paratype.push_back(rec->VarType.type);
					}
				}
				if (P->childs[++j]->word->type == COMMA)
					continue;
				else if (P->childs[j]->word->type == COLON)
				{
					j++; break;
				}
			}
			i++;
			if (PList->childs[i]->word->type == RPAREN)
				break;
		}
	}
	MakeBlockProgram(proc->childs[1]);
	Quaternion *ret = AddQuaternion();
	ret->command = Q_RETURN_NULL;
	procsym->LocalDataSize = GetVarSize();
	DecLayer();
}

void MakeFunction(Node *func)
{
	assert(func->type == Function);
	Record *funcsym = NULL;
	Quaternion *funcname;
	Node *Head = func->childs[0];
	if (InTableData(Head->childs[1]->word->word))
	{
		PrintErr(SEMANTICERR, "函数名重复定义", Head->childs[1]->pos.line, Head->childs[1]->pos.col);
		return;
	}
	funcsym = AddIDTable();
	funcsym->name = Head->childs[1]->word->word;
	funcsym->RecType = Record::func;
	funcsym->VarType.type = Head->childs[Head->childs.size() - 2]->word->type;
	funcsym->VarType.IsArr = 0;
	IncLayer();
	funcname = AddQuaternion();
	funcname->command = Q_ASSIGNFUNC;
	funcname->params[0].record = funcsym;
	Record *funcid = AddIDTable();
	char *str = new char[50];
	sprintf(str, "_%s_", funcsym->name);
	funcid->name = str;
	funcid->RecType = Record::variable;
	funcid->VarType = funcsym->VarType;
	Quaternion *q;
	Create(q, Q_ALLOC);
	q->params[0].record = funcid;
	if (Head->childs[2]->type == ParameterList)
	{
		Node *PList = Head->childs[2];
		for (int i = 1; i < (int)PList->childs.size() - 1; i += 2)
		{
			Node *P = PList->childs[i];
			int j = 0;
			if (P->childs[0]->word->type == VARIABLE)
				j = 1;
			int Size = P->childs.size();
			int type = INT;
			if (P->childs[Size - 1]->word->type == CHAR)
				type = CHAR;
			for (j; j < Size - 1; j++)
			{
				if (InTableData(P->childs[j]->word->word) != NULL)
				{
					PrintErr(SEMANTICERR, "参数名重复定义", P->childs[j]->pos.line, P->childs[j]->pos.col);
				}
				else
				{
					Record *rec = AddIDTable();
					rec->RecType = (j % 2) ? Record::reference : Record::variable;
					rec->name = P->childs[j]->word->word;
					rec->VarType.type = type;
					rec->VarType.IsArr = false;
					if (funcsym != NULL)
					{
						funcsym->parameter.push_back(rec);
						funcsym->paratype.push_back(rec->VarType.type);
					}
				}
				if (P->childs[++j]->word->type == COMMA)
					continue;
				else if (P->childs[j]->word->type == COLON)
				{
					j++; break;
				}
			}
			i++;
			if (PList->childs[i]->word->type == RPAREN)
				break;
		}
	}
	MakeBlockProgram(func->childs[1]);
	Quaternion *ret = AddQuaternion();
	ret->command = Q_RETURN;
	ret->params[0].record = funcid;
	funcsym->LocalDataSize = GetVarSize();
	DecLayer();
}

void MakeComplex(Node *complex)
{
	assert(complex->type == Complex);
	for (int i = 1; i < (int)complex->childs.size() - 1; i+=2)
		MakeStmt(complex->childs[i]);
}

Quaternion *MakeJdgmt(Node *jdgmt)
{
	assert(jdgmt->type == Jdgmt);
	Quaternion *cur = NULL;
	Record *t1 = MakeExp(jdgmt->childs[0]);
	Record *t2 = MakeExp(jdgmt->childs[2]);
	cur = AddQuaternion();
	switch (jdgmt->childs[1]->childs[0]->word->type)
	{
	case LT:
		cur->command = Q_LT; break;
	case LET:
		cur->command = Q_LET; break;
	case ET:
		cur->command = Q_ET; break;
	case NET:
		cur->command = Q_NET; break;
	case GT:
		cur->command = Q_GT; break;
	case GET:
		cur->command = Q_GET; break;
	default:
		break;
	}
	cur->params[0].record = t1, cur->params[1].record = t2;
	return cur;
}

Record *MakeExpAddr(Node *exp)
{
	assert(exp->type == Exp);
	Node *term, *factor;
	Quaternion *cur;
	Record *rec = AddTmpIDTable();
	Create(cur, Q_ALLOC);
	cur->params[0].record = rec;
	if (exp->childs.size() != 1)
	{
		PrintErr(SEMANTICERR, "该表达式不能计算地址！", exp->childs[0]->word->line, exp->childs[0]->word->col);
		return rec;
	}
	term = exp->childs[0];
	factor = term->childs[0];
	Record *id;
	if (factor->childs[0]->IsWord && factor->childs[0]->word->type == ID)
	{
		id = GetTableData(factor->childs[0]->word->word);
		if (id == NULL)
		{
			PrintErr(SEMANTICERR, "未定义的标识符", factor->childs[0]->word->line, factor->childs[0]->word->col);
			return rec;
		}
		if (id->RecType == Record::reference)
		{
			Quaternion *q = AddQuaternion();
			q->command = Q_ASSIGN;
			q->params[0].record = rec, q->params[1].record = id;
			return rec;
		}
		if (id->RecType != Record::variable)
		{
			PrintErr(SEMANTICERR, "该表达式不能计算地址！", exp->childs[0]->word->line, exp->childs[0]->word->col);
			return rec;
		}
		cur = AddQuaternion();
		cur->command = Q_ADDR;
		cur->params[0].record = rec;
		cur->params[1].record = id;
		if (id->VarType.IsArr)
		{
			Record *t = MakeExp(factor->childs[2]);
			if (t->Imme && (t->ImmeData >= id->VarType.len || t->ImmeData < 0))
			{
				PrintErr(SEMANTICERR, "数组下标越界", factor->childs[2]->pos.line, factor->childs[2]->pos.col);
			}
			if (t->Imme)
			{
				t->ImmeData *= 4;
				Create(cur, Q_ADD_C);
				cur->params[0].record = rec;
				cur->params[1].contentInt = t->ImmeData;
				cur->params[2].record = rec;
			}
			else
			{
				Record *t1 = AddTmpIDTable();
				Create(cur, Q_ALLOC); cur->params[0].record = t1;
				Create(cur, Q_ASSIGN); cur->params[0].record = t1, cur->params[1].record = t;
				for (int i = 1; i <= 2; i++)
				{
					Create(cur, Q_ADD);
					cur->params[0].record = t1;
					cur->params[1].record = t1;
					cur->params[2].record = t1;
				}
				cur = AddQuaternion();
				cur->command = Q_ADD;
				cur->params[0].record = rec;
				cur->params[1].record = t1;
				cur->params[2].record = rec;
			}
		}
	}
	return rec;
}

Record *MakeFactor(Node *factor)
{
	assert(factor->type == Factor);
	int i;
	Quaternion *cur = NULL;
	Record *rec = NULL, *id = NULL;
	if (factor->childs[0]->word->type == UNSIGNED)
	{
		rec = AddTmpIDTable();
		Create(cur, Q_ALLOC_C);
		cur->params[0].record = rec;
		cur->params[1].contentInt = factor->childs[0]->GetVal();
		rec->Imme = 1;
		rec->ImmeData = factor->childs[0]->GetVal();
		return rec;
	}
	if (factor->childs[0]->word->type == ID)
	{
		if ((id = GetTableData(factor->childs[0]->word->word)) == NULL)
		{
			PrintErr(SEMANTICERR, "未定义的标识符", factor->childs[0]->pos.line, factor->childs[0]->pos.col);
			return NULL;
		}
		if (factor->childs.size() > 1)
		{
			if (factor->childs[1]->word->type == LBRACHET)
			{
				if (id->VarType.IsArr == false)
				{
					PrintErr(SEMANTICERR, "对非数组变量进行引用", factor->childs[1]->pos.line, factor->childs[1]->pos.col);
				}
				Record *index = MakeExp(factor->childs[2]);
				if (index->Imme && (index->ImmeData >= id->VarType.len || index->ImmeData < 0))
				{
					PrintErr(SEMANTICERR, "数组下标越界", factor->childs[2]->pos.line, factor->childs[2]->pos.col);
				}
				Record *rec = AddTmpIDTable();
				rec->VarType.type = id->VarType.type;
				Create(cur, Q_ALLOC);
				cur->params[0].record = rec;
				Create(cur, Q_ARRRead);
				cur->params[0].record = id;
				cur->params[1].record = index;
				cur->params[2].record = rec;
				return rec;
			}
			else if (factor->childs[1]->word->type == LPAREN)
			{
				if (id->RecType != Record::func)
				{
					PrintErr(SEMANTICERR, "错误的函数调用", factor->childs[0]->word->line, factor->childs[0]->word->col);
					return NULL;
				}
				int x = std::max(0, (int)(((int)factor->childs.size() - 2) / 2));
				if (x != id->parameter.size())
				{
					PrintErr(SEMANTICERR, "参数数量不正确", factor->childs[0]->word->line, factor->childs[0]->word->col);
					return NULL;
				}
				Record *pushes[1010];
				int cnt = 0;
				Record *t1;
				for (int i = 0; i < x; i++)
				{
					int p = 2 * (i + 1);
					if (id->parameter[i]->RecType == Record::reference)
						t1 = MakeExpAddr(factor->childs[p]);
					else
						t1 = MakeExp(factor->childs[p]);
					pushes[cnt++] = t1;
				}
				Record *tmp = AddTmpIDTable();
				Create(cur, Q_ALLOC);
				cur->params[0].record = tmp;
				for (int i = 0; i < cnt; i++)
				{
					Create(cur, Q_PUSH);
					cur->params[0].record = pushes[i];
				}
				Create(cur, Q_CALL);
				cur->params[0].contentStr = id->name;
				cur->params[2].record = id;
				cur->params[1].record = tmp;
				return tmp;
			}
		}
		else
		{
			if (id->RecType == Record::proc)
			{
				PrintErr(SEMANTICERR, "过程不能在表达式中", factor->childs[0]->pos.line, factor->childs[0]->pos.col);
			}
			else if (id->RecType == Record::func)
			{
				if (id->RecType != Record::func)
				{
					PrintErr(SEMANTICERR, "错误的函数调用", factor->childs[0]->word->line, factor->childs[0]->word->col);
					return NULL;
				}
				int x = std::max(0, (int)(((int)factor->childs.size() - 2) / 2));
				if (x != id->parameter.size())
				{
					PrintErr(SEMANTICERR, "参数数量不正确", factor->childs[0]->word->line, factor->childs[0]->word->col);
					return NULL;
				}
				Record *pushes[1010];
				int cnt = 0;
				Record *t1;
				for (int i = 0; i < x; i++)
				{
					int p = 2 * (i + 1);
					if (id->parameter[i]->RecType == Record::reference)
						t1 = MakeExpAddr(factor->childs[p]);
					else
						t1 = MakeExp(factor->childs[p]);
					pushes[cnt++] = t1;
				}
				Record *tmp = AddTmpIDTable();
				Create(cur, Q_ALLOC);
				cur->params[0].record = tmp;
				for (int i = 0; i < cnt; i++)
				{
					Create(cur, Q_PUSH);
					cur->params[0].record = pushes[i];
				}
				Create(cur, Q_CALL);
				cur->params[0].contentStr = id->name;
				cur->params[2].record = id;
				cur->params[1].record = tmp;
				return tmp;
			}
			else if (id->VarType.IsArr)
			{
				PrintErr(SEMANTICERR, "不能对数组整体引用", factor->childs[0]->pos.line, factor->childs[0]->pos.col);
			}
			if (id->RecType != Record::reference)
				return id;
			else
			{
				Record *t = AddTmpIDTable();
				Create(cur, Q_ALLOC);
				cur->params[0].record = t;
				Create(cur, Q_READAddr);
				cur->params[0].record = t, cur->params[1].record = id;
				return t;
			}
		}
	}
	else if (factor->childs[0]->word->type == LPAREN)
	{
		return MakeExp(factor->childs[1]);
	}
	return NULL;
}

Record *MakeTerm(Node *term)
{
	Quaternion *cur = NULL;
	assert(term->type == Term);
	Record *rec = MakeFactor(term->childs[0]);
	if (term->childs.size() > 1)
	{
		Record *tmp = AddTmpIDTable();
		Create(cur, Q_ALLOC);
		cur->params[0].record = tmp;
		Create(cur, Q_ASSIGN);
		cur->params[0].record = tmp;
		cur->params[1].record = rec;
		rec = tmp;
		rec->Imme = false;
	}
	for (int i = 1; i < (int)term->childs.size(); i += 2)
	{
		Record *tmp = MakeFactor(term->childs[i + 1]);
		if (term->childs[i]->word->type == MULT)
			Create(cur, Q_MUL);
		else
			Create(cur, Q_DIV);
		cur->params[0].record = rec;
		cur->params[1].record = tmp;
		cur->params[2].record = rec;
	}
	return rec;
}

Record *MakeExp(Node *exp)
{
	assert(exp->type == Exp);
	Quaternion *cur = NULL;
	Record *t = NULL, *t2 = NULL;
	int i = 0;
	if (exp->childs[0]->IsWord)
	{
		t = MakeTerm(exp->childs[1]);
		if (exp->childs[0]->word->type == MINUS)
		{
			
			t2 = AddTmpIDTable();
			Create(cur, Q_ALLOC);
			cur->params[0].record = t2;
			Create(cur, Q_OPP);
			cur->params[0].record = t2;
			cur->params[1].record = t;
			if (t->Imme)
				t->ImmeData = -t->ImmeData;
			t = t2;
		}
		i = 1;
	}
	else t = MakeTerm(exp->childs[0]);
	i++;
	if (i <(int)exp->childs.size())
	{
		Record *tmp = AddTmpIDTable();
		Create(cur, Q_ALLOC);
		cur->params[0].record = tmp;
		Create(cur, Q_ASSIGN);
		cur->params[0].record = tmp;
		cur->params[1].record = t;
		t = tmp;
		t->Imme = false;
	}
	for (; i < (int)exp->childs.size(); i += 2)
	{
		t2 = MakeTerm(exp->childs[i + 1]);
		if (t2->Imme == 0)
		{
			if (exp->childs[i]->word->type == PLUS)
				Create(cur, Q_ADD);
			else
				Create(cur, Q_SUB);
			cur->params[0].record = t, cur->params[1].record = t2, cur->params[2].record = t;
		}
		else
		{
			if (exp->childs[i]->word->type == PLUS)
				Create(cur, Q_ADD_C);
			else
				Create(cur, Q_SUB_C);
			cur->params[0].record = t, cur->params[1].contentInt = t2->ImmeData, cur->params[2].record = t;
		}
	}
	return t;
}

void MakeStmt(Node *stmt)
{
	assert(stmt->type == Stmt);
	Quaternion *cur = NULL, *jump = NULL, *label = NULL, *delta, *tmp = NULL;
	Record *id = NULL, *dest = NULL, *t1 = NULL, *exp = NULL;
	int cnt, i;
	if (stmt->childs.size() == 0)
		return;
	if (stmt->childs[0]->IsWord)
	{

		switch (stmt->childs[0]->word->type)
		{
		case IF:
			cur = MakeJdgmt(stmt->childs[1]);
			MakeStmt(stmt->childs[3]);
			if (stmt->childs.size() > 4)
			{
				CreateJmp(jump, NULL);
				CreateLabel(label);
				cur->params[2].label = label;
				MakeStmt(stmt->childs[5]);
				CreateLabel(label);
				jump->params[0].label = label;
			}
			else
			{
				CreateLabel(label);
				cur->params[2].label = label;
			}
			break;
		case FOR:
			if ((id = GetTableData(stmt->childs[1]->word->word)) == NULL)
			{
				PrintErr(SEMANTICERR, "没有定义的标识符", stmt->childs[1]->pos.line, stmt->childs[1]->pos.col);
			}
			else if (id->RecType != Record::variable)
			{
				PrintErr(SEMANTICERR, "不能对非变量赋值", stmt->childs[1]->pos.line, stmt->childs[1]->pos.col);
			}
			else if (id->VarType.IsArr)
			{
				PrintErr(SEMANTICERR, "不能对数组整体赋值", stmt->childs[1]->pos.line, stmt->childs[1]->pos.col);
			}
			t1 = MakeExp(stmt->childs[3]);
			cur = AddQuaternion();
			cur->command = Q_ASSIGN;
			cur->params[0].record = id, cur->params[1].record = t1;
			t1 = MakeExp(stmt->childs[5]);
			if (stmt->childs[4]->word->type == DOWNTO)
			{
				CreateJmp(jump, NULL);
				CreateLabel(delta);
				cur = AddQuaternion();
				cur->command = Q_SUB_C, cur->params[2].record = id, cur->params[1].contentInt = 1, cur->params[0].record = id;
				CreateLabel(label);
				jump->params[0].label = label;
				cur = AddQuaternion();
				cur->command = Q_GET, cur->params[0].record = id, cur->params[1].record = t1;
				MakeStmt(stmt->childs[7]);
				CreateJmp(jump, delta);
				CreateLabel(label);
				cur->params[2].label = label;
			}
			else
			{
				CreateJmp(jump, NULL);
				CreateLabel(delta);
				cur = AddQuaternion();
				cur->command = Q_ADD_C, cur->params[2].record = id, cur->params[1].contentInt = 1, cur->params[0].record = id;
				CreateLabel(label);
				jump->params[0].label = label;
				cur = AddQuaternion();
				cur->command = Q_LET, cur->params[0].record = id, cur->params[1].record = t1;
				MakeStmt(stmt->childs[7]);
				CreateJmp(jump, delta);
				CreateLabel(label);
				cur->params[2].label = label;
			}
			break;
		case DO:
			CreateLabel(label);
			MakeStmt(stmt->childs[1]);
			cur = MakeJdgmt(stmt->childs[3]);
			CreateJmp(jump, label);
			CreateLabel(label);
			cur->params[2].label = label;
			break;
		case READ:
			cnt = (stmt->childs.size() - 2) / 2;
			for (int i = 1; i <= cnt; i++)
			{
				int p = 2 * i;
				if ((id = GetTableData(stmt->childs[p]->word->word)) == NULL)
				{
					PrintErr(SEMANTICERR, "未定义的标识符", stmt->childs[p]->word->line, stmt->childs[p]->word->col);
				}

				else if (id->RecType == Record::variable)
				{
					cur = AddQuaternion();
					cur->command = Q_INPUT;
					cur->params[0].record = id;
				}
				else if (id->RecType == Record::reference)
				{					
					t1 = AddTmpIDTable();
					Create(cur, Q_ALLOC);
					cur->params[0].record = t1;
					Create(cur, Q_INPUT);
					cur->params[0].record = t1;
					Create(cur, Q_WRITEAddr);
					cur->params[0].record = id, cur->params[1].record = t1;
				}
				else
				{
					PrintErr(SEMANTICERR, "不能对非变量写入", stmt->childs[p]->word->line, stmt->childs[p]->word->col);
				}
			}
			break;
		case WRITE:
			if (stmt->childs.size() == 6)
			{
				Record *rec = MakeExp(stmt->childs[4]);
				cur = AddQuaternion();
				cur->command = Q_PRINTSTR;
				cur->params[0].contentStr = stmt->childs[2]->word->word;
				if (rec != NULL)
				{
					cur = AddQuaternion();
					cur->params[0].record = rec;
					if (rec->VarType.type == INT)
					{
						cur->command = Q_PRINTINT;
					}
					else
					{
						cur->command = Q_PRINTCHAR;
					}
				}
			}
			else if (stmt->childs[2]->IsWord)
			{
				cur = AddQuaternion();
				cur->command = Q_PRINTSTR;
				cur->params[0].contentStr = stmt->childs[2]->word->word;
			}
			else
			{
				Record *rec = MakeExp(stmt->childs[2]);
				if (rec != NULL)
				{
					cur = AddQuaternion();
					cur->params[0].record = rec;
					if (rec->VarType.type == INT)
					{
						cur->command = Q_PRINTINT;
					}
					else
					{
						cur->command = Q_PRINTCHAR;
					}
				}
			}
			break;
		case BEGIN:
			MakeComplex(stmt->childs[0]);
			break;
		case ID:
			if ((id = GetTableData(stmt->childs[0]->word->word)) == NULL)
			{
				PrintErr(SEMANTICERR, "未定义的标识符", stmt->childs[0]->word->line, stmt->childs[0]->word->col);
				break;
			}
			if (stmt->childs.size() == 1 || stmt->childs[1]->word->type == LPAREN)
			{
				if (id->RecType != Record::proc)
				{
					PrintErr(SEMANTICERR, "错误的过程调用", stmt->childs[0]->word->line, stmt->childs[0]->word->col);
					break;
				}
				int x = std::max(0, (int)((int)(stmt->childs.size() - 2) / 2));
				if (x != id->parameter.size())
				{
					PrintErr(SEMANTICERR, "参数数量不正确", stmt->childs[0]->word->line, stmt->childs[0]->word->col);
					break;
				}
				Record *pushes[1010];
				int cnt = 0;
				for (int i = 0; i < x; i++)
				{
					int p = 2 * (i + 1);
					if (id->parameter[i]->RecType == Record::reference)
						t1 = MakeExpAddr(stmt->childs[p]);
					else
						t1 = MakeExp(stmt->childs[p]);
					if (id->paratype[i] != t1->VarType.type)
					{
						PrintErr(SEMANTICERR, "参数类型不正确", stmt->childs[p]->pos.line, stmt->childs[p]->pos.col);
					}
					pushes[cnt++] = t1;
				}
				Record *tmp = AddTmpIDTable();
				Create(cur, Q_ALLOC);
				cur->params[0].record = tmp;
				for (int i = 0; i < cnt; i++)
				{
					Create(cur, Q_PUSH);
					cur->params[0].record = pushes[i];
				}
				Create(cur, Q_CALL_NULL);
				cur->params[0].contentStr = id->name;
				cur->params[2].record = id;
				break;
			}
			else if (stmt->childs[1]->word->type == ASSIGN)
			{
				if (id->RecType == Record::constance || id->RecType == Record::proc)
				{
					PrintErr(SEMANTICERR, "无法赋值的变量类型", stmt->childs[0]->word->line, stmt->childs[0]->word->col);
				}
				else if (id->RecType == Record::func)
				{
					char *str = new char[20];
					sprintf(str, "_%s_", stmt->childs[0]->word->word);
					if ((id = GetTableData(str)) == NULL)
					{
						PrintErr(SEMANTICERR, "在函数体里才能对函数赋值", stmt->childs[0]->word->line, stmt->childs[0]->word->line);
						break;
					}
				}
				t1 = MakeExp(stmt->childs[2]);
				if (t1 == NULL)
					return;
				if (id->RecType == Record::reference)
				{
					Create(cur, Q_WRITEAddr);
					cur->params[0].record = id, cur->params[1].record = t1;
				}
				else
				{
					cur = AddQuaternion();
					cur->command = Q_ASSIGN;
					cur->params[0].record = id; cur->params[1].record = t1;
				}
			}
			else if (stmt->childs[1]->word->type == LBRACHET)
			{
				Record *index = MakeExp(stmt->childs[2]);
				if (id->VarType.IsArr != true)
				{
					PrintErr(SEMANTICERR, "该变量不是数组", stmt->childs[0]->word->line, stmt->childs[1]->word->col);
				}
				else
				{
					if (index->Imme)
					{
						if (index->ImmeData >= id->VarType.len || index->ImmeData < 0)
						{
							PrintErr(SEMANTICERR, "数组下标越界", stmt->childs[2]->pos.line, stmt->childs[2]->pos.col);
						}
					}
						exp = MakeExp(stmt->childs[5]);
						Record *exp2 = MakeExp(stmt->childs[2]);
						Create(cur, Q_ARRWrite);
						cur->params[0].record = id;
						cur->params[1].record = exp2;
						cur->params[2].record = exp;
				}
			}
			break;
		default:
			break;
		}
	}
	else
		MakeComplex(stmt->childs[0]);
}

void Output(Quaternion *q)
{
	FILE *fp = fopen("Quaternion.txt", "w");
	while (q->command != Q_TERMINATE)
	{
		switch (q->command)
		{
		case Q_ADD:// = 200,
			fprintf(fp,"ADD %s %s %s", q->params[0].record->name, q->params[1].record->name, q->params[2].record->name);
			break;
		case Q_ADD_C:// = 201,
			fprintf(fp,"ADD %s %d %s", q->params[0].record->name, q->params[1].contentInt, q->params[2].record->name);
			break;
		case Q_SUB:// = 202,
			fprintf(fp,"SUB %s %s %s", q->params[0].record->name, q->params[1].record->name, q->params[2].record->name);
			break;
		case Q_SUB_C:// = 203,
			fprintf(fp,"SUB %s %d %s", q->params[0].record->name, q->params[1].contentInt, q->params[2].record->name);
			break;
		case Q_MUL:// = 204,
			fprintf(fp,"MUL %s %s %s", q->params[0].record->name, q->params[1].record->name, q->params[2].record->name);
			break;
		case Q_DIV:// = 205,
			fprintf(fp,"DIV %s %s %s", q->params[0].record->name, q->params[1].record->name, q->params[2].record->name);
			break;
		case Q_OPP:// = 206,
			fprintf(fp,"OPP %s", q->params[0].record->name);// q->params[1].record->name, q->params[2].record->name);
			break;
		case Q_ET: //= 207,
			fprintf(fp,"JNE %s %s %s", q->params[0].record->name, q->params[1].record->name, q->params[2].label->params[0].contentStr);
			break;
		case Q_LT: //= 208,
			fprintf(fp,"JNL %s %s %s", q->params[0].record->name, q->params[1].record->name, q->params[2].label->params[0].contentStr);
			break;
		case Q_LET:// = 209,
			fprintf(fp,"JNLE %s %s %s", q->params[0].record->name, q->params[1].record->name, q->params[2].label->params[0].contentStr);
			break;
		case Q_GET: //= 210,
			fprintf(fp,"JNGE %s %s %s", q->params[0].record->name, q->params[1].record->name, q->params[2].label->params[0].contentStr);
			break;
		case Q_GT: //= 211,
			fprintf(fp,"JNG %s %s %s", q->params[0].record->name, q->params[1].record->name, q->params[2].label->params[0].contentStr);
			break;
		case Q_NET: //= 212,
			fprintf(fp,"JE %s %s %s", q->params[0].record->name, q->params[1].record->name, q->params[2].label->params[0].contentStr);
			break;
		case Q_ARRRead: //= 214,
			fprintf(fp,"ARRRead %s %s %s", q->params[0].record->name, q->params[1].record->name, q->params[2].record->name);
			break;
		case Q_ARRWrite:// = 215,
			fprintf(fp,"ARRWrite %s %s %s", q->params[0].record->name, q->params[1].record->name, q->params[2].record->name);
			break;
		case Q_ASSIGN: //= 216,
			fprintf(fp, "ASSIGN %s %s", q->params[0].record->name, q->params[1].record->name);// , q->params[2].record->name);
			break;
		case Q_LABEL: //= 217,
			fprintf(fp,"LABEL %s", q->params[0].contentStr);// ->name, q->params[1].record->name, q->params[2].label->params[0].record->name);
			break;
		case Q_ASSIGNFUNC:// = 218,
			fprintf(fp,"ASSIGNFUNC %s", q->params[0].record->name);// , q->params[1].record->name, q->params[2].label->params[0].record->name);
			break;
		case Q_RETURN: //= 219,
			fprintf(fp,"RETURN %s", q->params[0].record->name); break;
		case Q_RETURN_NULL: //= 220,
			fprintf(fp,"RETURN_NULL"); break;
		case Q_ALLOC: //= 221,
			fprintf(fp,"ALLOC %s", q->params[0].record->name);
			break;
		case Q_ALLOC_C: //= 222,
			fprintf(fp,"ALLOC %s %d", q->params[0].record->name,q->params[1].contentInt);
			break;
		case Q_PUSH: //= 223,
			fprintf(fp,"PUSH %s", q->params[0].record->name);
			break;
		case Q_CALL: //= 224,
			fprintf(fp,"CALL %s", q->params[0].contentStr);
			break;
		case Q_CALL_NULL:// = 225,
			fprintf(fp,"CALL_NULL %s", q->params[0].contentStr);
			break;
		case Q_JUMP: //= 226,
			fprintf(fp,"JMP %s", q->params[0].label->params[0].contentStr); break;
		case Q_PRINTSTR:// = 227,
			fprintf(fp,"PRINTSTR %s", q->params[0].contentStr); break;
		case Q_PRINTINT: //= 228,
			fprintf(fp,"PRINTINT %s", q->params[0].record->name); break;
		case Q_PRINTCHAR: //= 229,
			fprintf(fp,"PRINTCHAR %s", q->params[0].record->name); break;
		case Q_INPUT: //= 230,
			fprintf(fp,"INPUT %s", q->params[0].record->name); break;
		case Q_GLOBAL: //= 231,
			fprintf(fp,"GLOBAL %s", q->params[0].record->name); break;
		case Q_GLOBAL_C: //= 232,
			fprintf(fp,"GLOBAL %s", q->params[0].record->name); break;
		case Q_ENDGLOBAL: //= 233,
			fprintf(fp,"ENDGLOBAL"); break;
		case Q_TERMINATE: //= 234,
		case Q_ASSIGNPROC:// = 235,
			fprintf(fp,"ASSIGNPROC %s", q->params[0].record->name);
			break;
		case Q_ADDR: //= 236
			fprintf(fp,"ADDR %s", q->params[0].record->name); break;
		case Q_READAddr:
			fprintf(fp, "ReadAddr %s, %s", q->params[0].record->name, q->params[1].record->name);
			break;
		case Q_WRITEAddr:
			fprintf(fp, "WriteAddr %s, %s", q->params[0].record->name, q->params[1].record->name);
			break;
		default:
			break;
		}
		fputc('\n',fp);
		q++;
	}
	fclose(fp);
}