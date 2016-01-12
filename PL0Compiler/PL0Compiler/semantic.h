#pragma warning(disable:4996)
#pragma once

#ifndef SEMANTIC_H
#define SEMANTIC_H

struct Quaternion;
struct Node;
struct Record;

Quaternion *GetQuaternion(Node *, Quaternion *);
Quaternion *AddQuaternion();
Quaternion *MakeJdgmt(Node *);
void MakeComplex(Node *);
void MakeFunction(Node *);
void MakeProcedure(Node *);
void MakeMain(Node *);
void MakeStmt(Node *);
void MakeConsts(Node *, bool);
void MakeVaribles(Node *, bool);
void MakeBlockProgram(Node *, bool);
Record *MakeExp(Node *);
Record *MakeFactor(Node *);
Record *MakeTerm(Node *);

void Output(Quaternion *);

#endif