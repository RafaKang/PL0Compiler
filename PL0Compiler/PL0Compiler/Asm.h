#pragma warning(disable:4996)
#pragma once

#ifndef ASM
#define ASM

struct Quaternion;
struct Record;
struct Block;

bool GetASM(Block *);
void GetASM(Quaternion *q);
void ASMinitial();
void ASMfinalize();

void ClearReg();
void LoadReg(Record *rec);
void SaveReg(Record *rec);

Record **GetRegInfo();

#endif