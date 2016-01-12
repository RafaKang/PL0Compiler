#include "Asm.h"
#include "semantic.h"
#include "Function.h"
#include "DataStructure.h"
#include "word.h"
#include "Err.h"
#include "opt.h"

static int str_cnt = 0;
static int offset = 4;

FILE *fdata, *fins;
Record *reginfo[5] = { 0 };
char dest[210];

void ASMinitial()
{
	sprintf(dest, "%s.d.tmp", "output");
	fdata = fopen(dest, "w");
	sprintf(dest, "%s.i.tmp", "output");
	fins = fopen(dest, "w");
}

static int total_alloc = 0;
Record *alloccr[210];
int alloccv[210];
int alloccc = 0;
int CurLayer = 0;
char *FindIndex(Record *rec)
{
	char *buff = new char[20];
	if (rec->reg != Record::NOREG)
	{
		if (rec->reg == Record::ECX)
		{
			if (reginfo[0] != rec)
			{
				if (reginfo[0] != NULL)
				{
					//	SaveReg(reginfo[0]);
					reginfo[0]->reg = Record::NOREG;
					fprintf(fins, "MOV %s, ECX\n", FindIndex(reginfo[0]));
					reginfo[0]->reg = Record::ECX;
				}
				//LoadReg(rec);
				rec->reg = Record::NOREG;
				fprintf(fins, "MOV ECX, %s\n", FindIndex(rec));
				reginfo[0] = rec;
				rec->reg = Record::ECX;
			}
			return sprintf(buff, "ECX"), buff;
		}
		if (rec->reg == Record::ESI)
		{
			if (reginfo[1] != rec)
			{
				if (reginfo[1] != NULL)
				{
					//	SaveReg(reginfo[0]);
					reginfo[1]->reg = Record::NOREG;
					fprintf(fins, "MOV %s, ESI\n", FindIndex(reginfo[1]));
					reginfo[1]->reg = Record::ESI;
				}
				rec->reg = Record::NOREG;
				fprintf(fins, "MOV ESI, %s\n", FindIndex(rec));
				reginfo[1] = rec;
				rec->reg = Record::ESI;
			}
			return sprintf(buff, "ESI"), buff;
		}
		if (rec->reg == Record::EDI)
		{
			if (reginfo[2] != rec)
			{
				if (reginfo[2] != NULL)
				{
					//	SaveReg(reginfo[0]);
					reginfo[2]->reg = Record::NOREG;
					fprintf(fins, "MOV %s, EDI\n", FindIndex(reginfo[2]));
					reginfo[2]->reg = Record::EDI;
				}
				rec->reg = Record::NOREG;
				fprintf(fins, "MOV EDI, %s\n", FindIndex(rec));
				reginfo[2] = rec;
				rec->reg = Record::EDI;
			}
			return sprintf(buff, "EDI"), buff;
		}
	}
	if (rec->isGlobal == 1)
	{
		sprintf(buff, "[__base+%d]", rec->startAddr);
		return buff;
	}
	if (rec->Layer == CurLayer)
	{
		sprintf(buff, "[EBP+%d]", -rec->startAddr);
		return buff;
	}
	fprintf(fins, "MOV EBX, [EBP+8]\n");
	for (int i = 1; i <= CurLayer - 1 - rec->Layer; i++)
	{
		fprintf(fins, "MOV EBX, [EBX+8]\n");
	}
	sprintf(buff, "[EBX+%d]", -rec->startAddr);
	return buff;
}

void GetASM(Quaternion *q)
{
	if (!InSegment(q->command, Q_ALLOC, Q_ALLOC_C) && total_alloc > 0)
	{
		//fprintf(fins, "SUB ESP, %d\n", total_alloc);
		for (int i = 0; i < alloccc; i++)
		{
			fprintf(fins, "MOV DWORD PTR %s, %d\n", FindIndex(alloccr[i]), alloccv[i]);
		}
		alloccc = 0;
		total_alloc = 0;
	}
	if (q->command == Q_ALLOC)
	{
		q->params[0].record->startAddr = offset;
		offset += q->params[0].record->GetSize();
		total_alloc += q->params[0].record->GetSize();
	}
	if (q->command == Q_ALLOC_C)
	{
		q->params[0].record->startAddr = offset;
		offset += q->params[0].record->GetSize();
		total_alloc += q->params[0].record->GetSize();
		alloccr[alloccc] = q->params[0].record;
		if (q->params[0].record->VarType.type == INT)
		{
			alloccv[alloccc] = q->params[1].contentInt;
		}
		else
		{
			alloccv[alloccc] = q->params[1].contentChar;
		}
		alloccc++;

	}
	if (q->command == Q_ADD)
	{
		fprintf(fins, "MOV EAX, %s\n", FindIndex(q->params[0].record));
		fprintf(fins, "ADD EAX, %s\n", FindIndex(q->params[1].record));
		fprintf(fins, "MOV %s, EAX\n", FindIndex(q->params[2].record));

	}
	if (q->command == Q_ADD_C)
	{
		fprintf(fins, "MOV EAX, %s\n", FindIndex(q->params[0].record));
		fprintf(fins, "ADD EAX, %d\n", q->params[1].contentInt);
		fprintf(fins, "MOV %s, EAX\n", FindIndex(q->params[2].record));

	}
	if (q->command == Q_SUB)
	{
		fprintf(fins, "MOV EAX, %s\n", FindIndex(q->params[0].record));
		fprintf(fins, "SUB EAX, %s\n", FindIndex(q->params[1].record));
		fprintf(fins, "MOV %s, EAX\n", FindIndex(q->params[2].record));

	}
	if (q->command == Q_SUB_C)
	{
		fprintf(fins, "MOV EAX, %s\n", FindIndex(q->params[0].record));
		fprintf(fins, "SUB EAX, %d\n", q->params[1].contentInt);
		fprintf(fins, "MOV %s, EAX\n", FindIndex(q->params[2].record));

	}
	if (q->command == Q_DIV)
	{
		fprintf(fins, "MOV EAX, %s\n", FindIndex(q->params[0].record));
		fprintf(fins, "CDQ\n");
		fprintf(fins, "IDIV DWORD PTR %s\n", FindIndex(q->params[1].record));
		fprintf(fins, "MOV %s, EAX\n", FindIndex(q->params[2].record));

	}
	if (q->command == Q_MUL)
	{
		fprintf(fins, "MOV EAX, %s\n", FindIndex(q->params[0].record));
		fprintf(fins, "MUL DWORD PTR %s\n", FindIndex(q->params[1].record));
		fprintf(fins, "MOV %s, EAX\n", FindIndex(q->params[2].record));

	}
	if (q->command == Q_OPP)
	{
		fprintf(fins, "MOV EAX, %s\n", FindIndex(q->params[1].record));
		fprintf(fins, "NEG EAX\n");
		fprintf(fins, "MOV %s, EAX\n", FindIndex(q->params[0].record));

	}
	if (q->command == Q_ET)
	{
		fprintf(fins, "MOV EAX, %s\n", FindIndex(q->params[0].record));
		fprintf(fins, "CMP EAX, %s\n", FindIndex(q->params[1].record));
		for (int i = 0; i < 3; i++)
		if (reginfo[i] != NULL)
			SaveReg(reginfo[i]), reginfo[i] = NULL;
		fprintf(fins, "JNE __%s\n", q->params[2].label->params[0].contentStr);

	}
	if (q->command == Q_NET)
	{
		fprintf(fins, "MOV EAX, %s\n", FindIndex(q->params[0].record));
		fprintf(fins, "CMP EAX, %s\n", FindIndex(q->params[1].record));
		for (int i = 0; i < 3; i++)
		if (reginfo[i] != NULL)
			SaveReg(reginfo[i]), reginfo[i] = NULL;
		fprintf(fins, "JE __%s\n", q->params[2].label->params[0].contentStr);

	}
	if (q->command == Q_LT)
	{
		fprintf(fins, "MOV EAX, %s\n", FindIndex(q->params[0].record));
		fprintf(fins, "CMP EAX, %s\n", FindIndex(q->params[1].record));
		for (int i = 0; i < 3; i++)
		if (reginfo[i] != NULL)
			SaveReg(reginfo[i]), reginfo[i] = NULL;
		fprintf(fins, "JNL __%s\n", q->params[2].label->params[0].contentStr);

	}
	if (q->command == Q_GT)
	{
		fprintf(fins, "MOV EAX, %s\n", FindIndex(q->params[0].record));
		fprintf(fins, "CMP EAX, %s\n", FindIndex(q->params[1].record));
		for (int i = 0; i < 3; i++)
		if (reginfo[i] != NULL)
			SaveReg(reginfo[i]), reginfo[i] = NULL;
		fprintf(fins, "JNG __%s\n", q->params[2].label->params[0].contentStr);

	}
	if (q->command == Q_LET)
	{
		fprintf(fins, "MOV EAX, %s\n", FindIndex(q->params[0].record));
		fprintf(fins, "CMP EAX, %s\n", FindIndex(q->params[1].record));
		for (int i = 0; i < 3; i++)
		if (reginfo[i] != NULL)
			SaveReg(reginfo[i]), reginfo[i] = NULL;
		fprintf(fins, "JG __%s\n", q->params[2].label->params[0].contentStr);

	}
	if (q->command == Q_GET)
	{
		fprintf(fins, "MOV EAX, %s\n", FindIndex(q->params[0].record));
		fprintf(fins, "CMP EAX, %s\n", FindIndex(q->params[1].record));
		for (int i = 0; i < 3; i++)
		if (reginfo[i] != NULL)
			SaveReg(reginfo[i]),reginfo[i] = NULL;
		fprintf(fins, "JL __%s\n", q->params[2].label->params[0].contentStr);

	}
	if (q->command == Q_MAIN)
	{
		offset = 4;
		fprintf(fins, "SUB ESP, %d\n", q->params[0].contentInt);
	}
	if (q->command == Q_ASSIGN)
	{
		fprintf(fins, "MOV EAX, %s\n", FindIndex(q->params[1].record));
		fprintf(fins, "MOV %s, EAX\n", FindIndex(q->params[0].record));

	}
	if (q->command == Q_ARRRead)
	{
		fprintf(fins, "MOV EAX, %s\n", FindIndex(q->params[1].record));
		fprintf(fins, "SHL EAX, 2\n");
		fprintf(fins, "ADD EAX, %d\n", q->params[0].record->startAddr);
		if (q->params[0].record->isGlobal)
			fprintf(fins, "MOV EAX, [__base+EAX]\n");
		else
		{
			if (q->params[0].record->Layer == CurLayer)
			{
				fprintf(fins, "MOV EAX, [EBP+EAX]\n");// -q->params[0].record->startAddr);
			}
			else
			{
				fprintf(fins, "MOV EBX, [EBP+8]\n");
				for (int i = 1; i <= CurLayer - 1 - q->params[0].record->Layer; i++)
				{
					fprintf(fins, "MOV EBX, [EBX+8]\n");
				}
				fprintf(fins, "MOV EAX, [EBX+EAX]\n");// -q->params[0].record->startAddr);
			}
		}
		fprintf(fins, "MOV %s, EAX\n", FindIndex(q->params[2].record));

	}
	if (q->command == Q_ARRWrite)
	{
		fprintf(fins, "MOV EAX, %s\n", FindIndex(q->params[1].record));
		fprintf(fins, "SHL EAX, 2\n");
		fprintf(fins, "ADD EAX, %d\n", q->params[0].record->startAddr);
		fprintf(fins, "MOV EDX, %s\n", FindIndex(q->params[2].record));
		if (q->params[0].record->isGlobal)
			fprintf(fins, "MOV [__base+EAX], EDX\n");
		else
		{
			if (q->params[0].record->Layer == CurLayer)
				fprintf(fins, "MOV [EBP+EAX], EDX\n");
			else
			{
				fprintf(fins, "MOV EBX, [EBP+8]\n");
				for (int i = 1; i <= CurLayer - 1 - q->params[0].record->Layer; i++)
				{
					fprintf(fins, "MOV EBX, [EBX+8]\n");
				}
				fprintf(fins, "MOV [EBX+EAX], EDX\n");// -q->params[0].record->startAddr);
			}
		}

	}
	if (q->command == Q_GLOBAL)
	{
		fprintf(fdata, "_%s_ byte %d dup(0)\n", q->params[0].record->name, q->params[0].record->GetSize());
		q->params[0].record->startAddr = offset;
		offset += q->params[0].record->GetSize();

	}
	if (q->command == Q_GLOBAL_C)
	{
		fprintf(fdata, "_%s_ byte %d dup(0)\n", q->params[0].record->name, q->params[0].record->GetSize());
		q->params[0].record->startAddr = offset;
		offset += q->params[0].record->GetSize();
		if (q->params[0].record->VarType.type == INT)
			fprintf(fins, "MOV %s, %d\n", FindIndex(q->params[0].record), q->params[1].contentInt);
		else
			fprintf(fins, "MOV %s, %d\n", FindIndex(q->params[0].record), q->params[1].contentChar);

	}
	if (q->command == Q_LABEL)
	{
		fprintf(fins, "__%s:\n", q->params[0].contentStr);

	}
	if (q->command == Q_ASSIGNFUNC)
	{
		CurLayer++;
		fprintf(fins, "@__%s:\n", q->params[0].record->name);
		ClearReg();
		fprintf(fins, "PUSH EBP\n");
		fprintf(fins, "MOV EBP, ESP\n");
		fprintf(fins, "SUB ESP, 4\n");
		fprintf(fins, "SUB ESP, %d\n", q->params[0].record->LocalDataSize);
		offset = -8;
		for (int i = q->params[0].record->parameter.size() - 1; i >= 0; i--)
		{
			offset -= q->params[0].record->parameter[i]->GetSize();
			q->params[0].record->parameter[i]->startAddr = offset;
		}
		offset = 4;

	}
	if (q->command == Q_ASSIGNPROC)
	{
		CurLayer++;
		fprintf(fins, "@__%s:\n", q->params[0].record->name);
		ClearReg();
		fprintf(fins, "PUSH EBP\n");
		fprintf(fins, "MOV EBP, ESP\n");
		fprintf(fins, "SUB ESP, 4\n");
		fprintf(fins, "SUB ESP, %d\n", q->params[0].record->LocalDataSize);
		offset = -8;
		for (int i = q->params[0].record->parameter.size() - 1; i >= 0; i--)
		{
			offset -= q->params[0].record->parameter[i]->GetSize();
			q->params[0].record->parameter[i]->startAddr = offset;
		}
		offset = 4;

	}
	if (q->command == Q_CALL)
	{
		Record *rec = q->params[2].record;
		if (rec->Layer == CurLayer)
		{
			fprintf(fins, "PUSH EBP\n");
		}
		else
		{
			fprintf(fins, "MOV EBX, [EBP+8]\n");
			for (int i = 1; i <= CurLayer - 1 - q->params[0].record->Layer; i++)
			{
				fprintf(fins, "MOV EBX, [EBX+8]\n");
			}
			fprintf(fins, "PUSH EBX\n");
		}
		for (int i = 0; i < 3; i++)
		if (reginfo[i] != NULL)
			SaveReg(reginfo[i]);
		fprintf(fins, "CALL @__%s\n", q->params[0].contentStr);
		for (int i = 0; i < 3; i++)
		if (reginfo[i] != NULL)
			LoadReg(reginfo[i]);
		fprintf(fins, "MOV %s, EAX\n", FindIndex(q->params[1].record));

	}
	if (q->command == Q_CALL_NULL)
	{
		Record *rec = q->params[2].record;
		if (rec->Layer == CurLayer)
		{
			fprintf(fins, "PUSH EBP\n");
		}
		else
		{
			fprintf(fins, "MOV EBX, [EBP+8]\n");
			for (int i = 1; i <= CurLayer - 1 - q->params[0].record->Layer; i++)
			{
				fprintf(fins, "MOV EBX, [EBX+8]\n");
			}
			fprintf(fins, "PUSH EBX\n");
		}
		for (int i = 0; i < 3; i++)
		if (reginfo[i] != NULL)
			SaveReg(reginfo[i]);
		fprintf(fins, "CALL @__%s\n", q->params[0].contentStr);
		for (int i = 0; i < 3; i++)
		if (reginfo[i] != NULL)
			LoadReg(reginfo[i]);

	}
	if (q->command == Q_INPUT)
	{
		if (q->params[0].record->VarType.type == INT)
		{
			Record::Reg back = q->params[0].record->reg;
			q->params[0].record->reg = Record::NOREG;
			//fprintf(fins, "MOV DWORD PTR %s, 0\n", FindIndex(q->params[0].record));
			fprintf(fins, "LEA EAX, %s\n", FindIndex(q->params[0].record));
			q->params[0].record->reg = back;				fprintf(fins, "PUSH EAX\n");
			//fprintf(fins, "PUSH addr DWORD PTR %s\n", FindIndex(q->params[0].record));
			fprintf(fins, "PUSH offset _intin\n");
			for (int i = 0; i < 3; i++)
			if (reginfo[i] != NULL)
				SaveReg(reginfo[i]);
			fprintf(fins, "CALL scanf\n");
			for (int i = 0; i < 3; i++)
			if (reginfo[i] != NULL)
				LoadReg(reginfo[i]);
			//fprintf(fins, "INVOKE scanf, offset _intin, addr DWORD PTR %s\n", FindIndex(q->params[0].record));// ->getDest(buff));
		}
		else
		{
			Record::Reg back = q->params[0].record->reg;
			q->params[0].record->reg = Record::NOREG;
			fprintf(fins, "MOV DWORD PTR %s, 0\n", FindIndex(q->params[0].record));
			fprintf(fins, "LEA EAX, %s\n", FindIndex(q->params[0].record));
			q->params[0].record->reg = back;
			fprintf(fins, "PUSH EAX\n");
			//fprintf(fins, "PUSH addr DWORD PTR %s\n", FindIndex(q->params[0].record));
			fprintf(fins, "PUSH offset _charin\n");
			for (int i = 0; i < 3; i++)
			if (reginfo[i] != NULL)
				SaveReg(reginfo[i]);
			fprintf(fins, "CALL scanf\n");
			for (int i = 0; i < 3; i++)
			if (reginfo[i] != NULL)
				LoadReg(reginfo[i]);
			//fprintf(fins, "INVOKE scanf, offset _charin, addr DWORD PTR %s\n", FindIndex(q->params[0].record));
		}

	}
	if (q->command == Q_JUMP)
	{
		for (int i = 0; i < 3; i++)
		if (reginfo[i] != NULL)
			SaveReg(reginfo[i]),reginfo[i] = NULL;
		fprintf(fins, "JMP __%s\n", q->params[0].label->params[0].contentStr);
	}
	if (q->command == Q_PRINTSTR)
	{
		fprintf(fdata, "__STR%d db \"%s\",0\n", str_cnt, q->params[0].contentStr);
		fprintf(fins, "PUSH offset __STR%d\n", str_cnt++);
		fprintf(fins, "PUSH offset _str\n");
		for (int i = 0; i < 3; i++)
		if (reginfo[i] != NULL)
			SaveReg(reginfo[i]);
		fprintf(fins, "CALL crt_printf\n");
		for (int i = 0; i < 3; i++)
		if (reginfo[i] != NULL)
			LoadReg(reginfo[i]);
		//fprintf(fins, "INVOKE crt_printf, offset _str, offset __STR%d\n", str_cnt++);

	}
	if (q->command == Q_PRINTINT)
	{
		fprintf(fins, "PUSH DWORD PTR %s\n", FindIndex(q->params[0].record));
		fprintf(fins, "PUSH offset _int\n");
		for (int i = 0; i < 3; i++)
		if (reginfo[i] != NULL)
			SaveReg(reginfo[i]);
		fprintf(fins, "CALL crt_printf\n");
		for (int i = 0; i < 3; i++)
		if (reginfo[i] != NULL)
			LoadReg(reginfo[i]);
		//fprintf(fins, "INVOKE crt_printf, offset _int, DWORD PTR %s\n", FindIndex(q->params[0].record));

	}
	if (q->command == Q_PRINTCHAR)
	{
		fprintf(fins, "PUSH DWORD PTR %s\n", FindIndex(q->params[0].record));
		fprintf(fins, "PUSH offset _char\n");
		for (int i = 0; i < 3; i++)
		if (reginfo[i] != NULL)
			SaveReg(reginfo[i]);
		fprintf(fins, "CALL crt_printf\n");
		for (int i = 0; i < 3; i++)
		if (reginfo[i] != NULL)
			LoadReg(reginfo[i]);
		//fprintf(fins, "INVOKE crt_printf, offset _char, DWORD PTR %s\n", FindIndex(q->params[0].record));

	}
	if (q->command == Q_PUSH)
	{
		fprintf(fins, "PUSH %s\n", FindIndex(q->params[0].record));

	}
	if (q->command == Q_RETURN)
	{
		fprintf(fins, "MOV EAX, %s\n", FindIndex(q->params[0].record));
		for (int i = 0; i < 3; i++)
		if (reginfo[i] != NULL)
			SaveReg(reginfo[i]);
		fprintf(fins, "MOV ESP, EBP\n");
		fprintf(fins, "POP EBP\n");
		fprintf(fins, "RET\n");
		CurLayer--;

	}
	if (q->command == Q_RETURN_NULL)
	{
		for (int i = 0; i < 3; i++)
		if (reginfo[i] != NULL)
			SaveReg(reginfo[i]);
		fprintf(fins, "MOV ESP, EBP\n");
		fprintf(fins, "POP EBP\n");
		fprintf(fins, "RET\n");
		CurLayer--;

	}
	if (q->command == Q_ADDR)
	{
		Record::Reg back = q->params[1].record->reg;
		q->params[1].record->reg = Record::NOREG;
		fprintf(fins, "LEA EAX, %s\n", FindIndex(q->params[1].record));
		q->params[1].record->reg = back;
		fprintf(fins, "MOV %s, EAX\n", FindIndex(q->params[0].record));

	}
	if (q->command == Q_READAddr)
	{
		fprintf(fins, "MOV EAX, %s\n", FindIndex(q->params[1].record));
		fprintf(fins, "MOV EAX, [EAX]\n");
		fprintf(fins, "MOV %s, EAX\n", FindIndex(q->params[0].record));

	}
	if (q->command == Q_WRITEAddr)
	{
		for (int i = 0; i < 3; i++)
		if (reginfo[i] != NULL)
			SaveReg(reginfo[i]);
		fprintf(fins, "MOV EAX, %s\n", FindIndex(q->params[0].record));
		fprintf(fins, "MOV EBX, %s\n", FindIndex(q->params[1].record));
		fprintf(fins, "MOV [EAX], EBX\n");
		for (int i = 0; i < 3; i++)
		if (reginfo[i] != NULL)
			LoadReg(reginfo[i]);

	}
	if (q->command == Q_TERMINATE)
	{
		for (int i = 0; i < 3; i++)
		if (reginfo[i] != NULL)
			SaveReg(reginfo[i]);
		fprintf(fins, "PUSH DWORD PTR 0\nCALL ExitProcess\n");
		//fprintf(fins, "invoke ExitProcess,0\n");

	}
}

void ClearReg()
{
	fprintf(fins, "XOR ECX, ECX\n");
	fprintf(fins, "XOR ESI, ESI\n");
	fprintf(fins, "XOR EDI, EDI\n");
}

void ASMfinalize()
{
	fclose(fdata);
	fclose(fins);
	sprintf(dest, "%s.d.tmp", "output");
	fdata = fopen(dest, "r");
	sprintf(dest, "%s.i.tmp", "output");
	fins = fopen(dest, "r");
	FILE *fheader0 = fopen("fheader0", "r");
	FILE *fheader1 = fopen("fheader1", "r");
	FILE *ffooter = fopen("ffooter", "r");

	sprintf(dest, "output.asm");
	FILE *res = fopen(dest, "w");
	int t;
	while ((t = fgetc(fheader0)) != EOF)
		fputc(t, res);
	while ((t = fgetc(fdata)) != EOF)
		fputc(t, res);
	while ((t = fgetc(fheader1)) != EOF)
		fputc(t, res);
	while ((t = fgetc(fins)) != EOF)
		fputc(t, res);
	while ((t = fgetc(ffooter)) != EOF)
		fputc(t, res);
	fclose(fheader0), fclose(fheader1), fclose(ffooter), fclose(res);
	fclose(fins), fclose(fdata);
}

bool GetASM(Block *block)
{
	for (Quaternion *q = block->begin; q <= block->end; q++)
	{
		GetASM(q);
	}
	//GetASM(block->end);
	for (int i = 0; i < 3; i++)
	if (reginfo[i] != NULL)
		SaveReg(reginfo[i]), reginfo[i] = NULL;
	return 0;
}

void LoadReg(Record *rec)
{
	Record::Reg back = rec->reg;
	char *buff = FindIndex(rec);
	rec->reg = Record::NOREG;
	char *buff2 = FindIndex(rec);
	rec->reg = back;
	fprintf(fins, "MOV %s, %s\n", buff, buff2);
}
void SaveReg(Record *rec)
{
	Record::Reg back = rec->reg;
	char *buff = FindIndex(rec);
	rec->reg = Record::NOREG;
	char *buff2 = FindIndex(rec);
	rec->reg = back;
	fprintf(fins, "MOV %s, %s\n", buff2, buff);
}

Record **GetRegInfo()
{
	return reginfo;
}