#pragma once

#ifndef GRAMMAR_H
#define GRAMMAR_H

struct Node;
struct Word;

Node *GetNodes(int, Word*&);
static Node *Match(int, Word *&);
static Node *Match(int, Word *&, int, ...);

static void find(Word *&, int, ...);

static Node *GetNumber(Word *&);
static Node *GetOperator(Word *&);
static Node *GetConstDeclare(Word *&);
static Node *GetConst(Word *&);
static Node *GetVarDeclare(Word *&);
static Node *GetVar(Word *&);
static Node *GetParameter(Word *&);
static Node *GetParameterList(Word *&);
static Node *GetFactor(Word *&);
static Node *GetExp(Word *&);
static Node *GetJdgmt(Word *&);
static Node *GetStmt(Word *&);
static Node *GetComplex(Word *&);
static Node *GetFunctionDeclare(Word *&);
static Node *GetProcedureDeclare(Word *&);
static Node *GetBlockProgram(Word *&);
static Node *GetProgram(Word *&);
static Node *GetTerm(Word *&);
static Node *GetProcedure(Word *&);
static Node *GetFunction(Word *&);
static Node *GetFunctionHead(Word *&);

#endif