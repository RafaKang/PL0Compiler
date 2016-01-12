#include "Function.h"
#include "DataStructure.h"
#include <iostream>
#include <cstdio>
#include <cstring>
#include <string>
#include "word.h"
#include "Err.h"

static char *SubString(char *st, char *ed)
{
	char *str = new char[ed - st + 2];
	for (int i = 0; i < ed - st + 1; i++)
		str[i] = st[i];
	str[ed - st + 1] = 0;
	return str;
}

static int SearchTerminalWords(char *str)
{
	for (int i = 0; i < TerminalWordCnt; i++)
	if (strcmp(str, TerminalWord[i]) == 0)
		return int(i + 1);
	return SPILIT;
}

static char SPCHAR[4] = { ' ', '\t', '\r', '\n' };
static int SPCNT = 4;
static char DOP[] = { '<', '>', '=', ':' };
static int DOPCNT = 4;
static char SOP[] = { ',', ';', '(', ')', '[', ']', '+', '-', '*', '/','.' };
static int SOPCNT = 11;
int CurCol = 0, CurLine = 1;

char *GetWord(char *str, Word *wd)
{
	char ch;
	wd->line = CurLine;
	wd->col = CurCol;
	while (str[0] != 0 && InSegment(ch=*str,SPCHAR,SPCNT))
	{
		if (ch == '\n' || ch == '\r')
		{
			wd->col = CurCol = 0;
			wd->line = (++CurLine);
		}
		else
		{
			wd->col = (++CurCol);
		}
		str++;
	}
	ch = *str;
	if (str[0] == 0)
		return NULL;
	if (ch == '"')//should be STRING
	{
		int i = 0;
		for (i = 1; str[i] != 0; i++)
		{
			if (str[i] == '"')
			{
				wd->type = STRING;
				wd->word = SubString(str + 1, str + i - 1);
				wd->col = (CurCol += i + 1);
				return str + i + 1;
			}
			else
			{
				if (str[i] == '\n')
				{
					wd->col = CurCol = 0;
					wd->line = (++CurLine);
					i++;
					break;
				}
			}
		}
		PrintErr(WORDERR, "字符串常量定义错误", CurLine, CurCol);
		return str + i;
	}
	else if (ch == '\'')//should be CHARACTER
	{
		if ((str[1] >= 'a' && str[1] <= 'z') || (str[1] >= 'A' && str[1] <= 'Z') || (str[1] >= '0' && str[1] <= '9'))
		{
			if (str[2] == '\'')
			{
				wd->type = CHARACTOR;
				wd->word = SubString(str + 1, str + 1);
				wd->col = (CurCol += 3);
				return str + 3;
			}
		}
		PrintErr(WORDERR, "字符常量定义错误", CurLine, CurCol);
		/*int i;
		for (i = 0; str[i] != 0 && str[i] != ';'; i++)
		{
			if (str[i] == '\n')
			{
				wd->line = (++CurLine);
				wd->col = CurCol = 0;
			}
			else
			{
				wd->col = (++CurCol);
			}
		}*/
		return str + 2;
	}
	else if ((ch >= 'a' && ch <= 'z') || (ch >= 'A' && ch <= 'Z'))//ID or TerminalWord
	{
		for (int i = 1;; i++)
		{
			if (!((str[i] >= 'a' && str[i] <= 'z') || (str[i] >= 'A' && str[i] <= 'Z') || (str[i] >= '0' && str[i] <= '9')))
			{
				wd->word = SubString(str, str + i - 1);
				wd->type = SearchTerminalWords(wd->word);
				wd->type = (wd->type == SPILIT) ? ID : wd->type;
				wd->col = (CurCol += i);
				return str + i;
			}
		}
	}
	else if (ch >= '0' && ch <= '9')
	{
		for (int i = 1;; i++)
		{
			if (!(str[i] >= '0' && str[i] <= '9'))
			{
				wd->type = UNSIGNED;
				wd->word = SubString(str, str + i - 1);
				wd->col = (CurCol += i);
				return str + i;
			}
		}
	}
	else
	{
		if (InSegment(ch, SOP, SOPCNT))
		{
			wd->word = SubString(str, str);
			wd->type = SearchTerminalWords(wd->word);
			wd->col = (++CurCol);
			return str + 1;
		}
		else if (InSegment(ch,DOP,DOPCNT))
		{
			if (str[0] == '<' && str[1] == '>')
			{
				wd->word = SubString(str, str + 1);
				wd->type = SearchTerminalWords(wd->word);
				wd->col = (CurCol += 2);
				return str + 2;
			}
			if (str[1] == '=')
			{
				wd->word = SubString(str, str + 1);
				wd->type = SearchTerminalWords(wd->word);
				wd->col = (CurCol += 2);
				return str + 2;
			}
			else
			{
				wd->word = SubString(str, str);
				wd->type = SearchTerminalWords(wd->word);
				wd->col = (++CurCol);
				return str + 1;
			}
		}
		PrintErr(WORDERR, "无法识别的符号", CurLine, CurCol);
		return str + 1;
		/*int i;
		for (i = 2; str[i] != 0 && str[i] != ';'; i++)
		{
			if (str[i] == '\n')
			{
				wd->line = (++CurLine);
				wd->col = CurCol = 0;
				//break;
			}
			else
			{
				wd->col = (++CurCol);
			}
		}
		return str + i;*/
	}
	//return NULL;
}

void Output(Word *words, int cnt)
{
	FILE *fp = fopen("LexerRes.txt", "w");
	for (int i = 0; i < cnt; i++)
	{
		fprintf(fp, "%s ", (words + i)->word);// , (words + i)->type);
		switch (words[i].type)
		{
		case ID:
			fprintf(fp, "ID"); break;
		case CONST:
			fprintf(fp, "CONST"); break;
		case VARIABLE:
			fprintf(fp, "VARIABLE"); break;
		case INT:
			fprintf(fp, "INTEGER"); break;
		case CHAR:
			fprintf(fp, "CHAR"); break;
		case ARRAY:
			fprintf(fp, "ARRAY"); break;
		case OF:
			fprintf(fp, "OF"); break;
		case PLUS:
			fprintf(fp, "PLUS"); break;
		case MINUS:
			fprintf(fp, "MINUS"); break;
		case MULT:
			fprintf(fp, "MULT"); break;
		case DIV:
			fprintf(fp, "DIV"); break;
		case ASSIGN:
			fprintf(fp, "ASSIGN"); break;
		case ET:
			fprintf(fp, "ET"); break;
		case LT:
			fprintf(fp, "LT"); break;
		case LET:
			fprintf(fp, "LET"); break;
		case GT:
			fprintf(fp, "GT"); break;
		case GET:
			fprintf(fp, "GET"); break;
		case NET:
			fprintf(fp, "NET"); break;
		case PROCEDURE:
			fprintf(fp, "PROCEDURE"); break;
		case FUNCTION:
			fprintf(fp, "FUNCTION"); break;
		case IF:
			fprintf(fp, "IF"); break;
		case THEN:
			fprintf(fp, "THEN"); break;
		case ELSE:
			fprintf(fp, "ELSE"); break;
		case DO:
			fprintf(fp, "DO"); break;
		case WHILE:
			fprintf(fp, "WHILE"); break;
		case FOR:
			fprintf(fp, "FOR"); break;
		case BEGIN:
			fprintf(fp, "BEGIN"); break;
		case END:
			fprintf(fp, "END"); break;
		case READ:
			fprintf(fp, "READ"); break;
		case WRITE:
			fprintf(fp, "WRITE"); break;
		case LPAREN:
			fprintf(fp, "LPAREN"); break;
		case RPAREN:
			fprintf(fp, "RPAREN"); break;
		case LBRACHET:
			fprintf(fp, "LBRACHET"); break;
		case RBRACHET:
			fprintf(fp, "RBRACHET"); break;
		case SEMICOLON:
			fprintf(fp, "SEMICOLON"); break;
		case COMMA:
			fprintf(fp, "COMMA"); break;
		case SQUOTE:
			fprintf(fp, "SQUOTE"); break;
		case DQUOTE:
			fprintf(fp, "DQUOTE"); break;
		case COLON:
			fprintf(fp, "COLON"); break;
		case UNSIGNED:
			fprintf(fp, "UNSIGNED"); break;
		case OP:
			fprintf(fp, "OP"); break;
		case CHARACTOR:
			fprintf(fp, "CHARACTOR"); break;
		case STRING:
			fprintf(fp, "STRING"); break;
		case ENDOFPROG:
			fprintf(fp, "ENDOFPROG"); break;
		case TO:
			fprintf(fp, "TO"); break;
		case ENDOFFILE:
			fprintf(fp, "ENDOFFILE"); break;
		case DOWNTO:
			fprintf(fp, "DOWNTO"); break;
		default:
			break;
		}
		fprintf(fp,"\n");
	}
}