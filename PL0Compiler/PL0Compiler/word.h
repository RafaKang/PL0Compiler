#pragma warning(disable:4996)
#pragma once

#ifndef WORD_H
#define WORD_H

static char *SubString(char *, char *);
static int SearchTerminalWords(char *);
char *GetWord(char *, Word *);
void Output(Word *, int);

#endif