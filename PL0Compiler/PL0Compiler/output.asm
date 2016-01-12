.486
.model flat,stdcall
option casemap: none

include \masm32\include\masm32.inc
include \masm32\include\kernel32.inc
include \masm32\macros\macros.asm
include \masm32\include\msvcrt.inc

includelib \masm32\lib\msvcrt.lib
includelib \masm32\lib\masm32.lib
includelib \masm32\lib\kernel32.lib

scanf proto C:dword,:vararg

.data
_intin db "%d",0
_charin db "%c",0
_int db "%d",13,10,0
_char db "%c",13,10,0
_str db "%s",13,10,0
__base dword 0
_m_ byte 4 dup(0)
_x_ byte 4 dup(0)
_y_ byte 4 dup(0)
_a_ byte 4 dup(0)
_b_ byte 4 dup(0)
_f_ byte 4 dup(0)
_g_ byte 4 dup(0)
_c_ byte 4 dup(0)
_z_ byte 4 dup(0)
_i_ byte 4 dup(0)
__STR0 db "GCD is:",0
__STR1 db "LCM is:",0
.code
start:
main proc
PUSH EBP
MOV EBP, ESP
MOV [__base+4], 2
JMP ___label_2
@__mymod:
XOR ECX, ECX
XOR ESI, ESI
XOR EDI, EDI
PUSH EBP
MOV EBP, ESP
SUB ESP, 4
SUB ESP, 40
JMP ___label_0
___label_0:
MOV ECX, [EBP+16]
MOV EAX, ECX
MOV EAX, [EAX]
MOV EDI, [EBP+-8]
MOV EDI, EAX
MOV EAX, EDI
MOV [EBP+-8], EDI
MOV EDI, [EBP+-12]
MOV EDI, EAX
MOV [EBP+-12], EDI
MOV EDI, [EBP+-8]
MOV EAX, EDI
MOV [EBP+-8], EDI
MOV EDI, [EBP+-16]
MOV EDI, EAX
MOV EAX, EDI
MOV [EBP+-16], EDI
MOV EDI, [EBP+-20]
MOV EDI, EAX
MOV [EBP+16], ECX
MOV ECX, [EBP+12]
MOV EAX, ECX
MOV EAX, [EAX]
MOV [EBP+-20], EDI
MOV EDI, [EBP+-24]
MOV EDI, EAX
MOV [EBP+-24], EDI
MOV EDI, [EBP+-20]
MOV EAX, EDI
CDQ
MOV [EBP+-20], EDI
MOV EDI, [EBP+-24]
IDIV DWORD PTR EDI
MOV [EBP+-24], EDI
MOV EDI, [EBP+-20]
MOV EDI, EAX
MOV [EBP+-20], EDI
MOV EDI, [EBP+-24]
MOV EAX, EDI
MOV [EBP+-24], EDI
MOV EDI, [EBP+-28]
MOV EDI, EAX
MOV [EBP+-28], EDI
MOV EDI, [EBP+-20]
MOV EAX, EDI
MOV [EBP+-20], EDI
MOV EDI, [EBP+-28]
MUL DWORD PTR EDI
MOV [EBP+-28], EDI
MOV EDI, [EBP+-20]
MOV EDI, EAX
MOV [EBP+-20], EDI
MOV EDI, [EBP+-12]
MOV EAX, EDI
MOV [EBP+-12], EDI
MOV EDI, [EBP+-20]
SUB EAX, EDI
MOV [EBP+-20], EDI
MOV EDI, [EBP+-12]
MOV EDI, EAX
MOV [EBP+12], ECX
MOV [EBP+-12], EDI
MOV [EBP+12], ECX
MOV ECX, [EBP+16]
MOV EAX, ECX
MOV EBX, EDI
MOV [EAX], EBX
MOV ECX, [EBP+16]
MOV EDI, [EBP+-12]
MOV EAX, ECX
MOV EAX, [EAX]
MOV [EBP+-12], EDI
MOV EDI, [EBP+-32]
MOV EDI, EAX
MOV EAX, EDI
MOV [EBP+-32], EDI
MOV EDI, [EBP+-4]
MOV EDI, EAX
MOV EAX, EDI
MOV [EBP+16], ECX
MOV [EBP+-4], EDI
MOV ESP, EBP
POP EBP
RET
MOV EBX, [EBP+8]
MOV [EBX+16], ECX
MOV EBX, [EBP+8]
MOV [EBX+-4], EDI
@__swap:
XOR ECX, ECX
XOR ESI, ESI
XOR EDI, EDI
PUSH EBP
MOV EBP, ESP
SUB ESP, 4
SUB ESP, 4
JMP ___label_1
___label_1:
MOV ECX, [__base+8]
MOV EAX, ECX
MOV EDI, [EBP+-4]
MOV EDI, EAX
MOV [__base+8], ECX
MOV ECX, [__base+12]
MOV EAX, ECX
MOV [__base+12], ECX
MOV ECX, [__base+8]
MOV ECX, EAX
MOV EAX, EDI
MOV [__base+8], ECX
MOV ECX, [__base+12]
MOV ECX, EAX
MOV [__base+12], ECX
MOV [EBP+-4], EDI
MOV ESP, EBP
POP EBP
RET
MOV [__base+12], ECX
MOV EBX, [EBP+8]
MOV [EBX+-4], EDI
___label_2:
SUB ESP, 72
MOV DWORD PTR [EBP+-4], 2
MOV EAX, [EBP+-4]
MOV [__base+40], EAX
MOV DWORD PTR [EBP+-8], 1
JMP ___label_4
___label_3:
MOV EAX, [__base+40]
SUB EAX, 1
MOV [__base+40], EAX
___label_4:
MOV EAX, [__base+40]
CMP EAX, [EBP+-8]
JL ___label_10
MOV ECX, [EBP+-12]
MOV DWORD PTR ECX, 2
MOV EAX, [__base+40]
CMP EAX, ECX
MOV [EBP+-12], ECX
JNE ___label_5
MOV ECX, [EBP+-16]
MOV DWORD PTR ECX, 45
MOV EAX, ECX
MOV [__base+16], EAX
MOV [EBP+-16], ECX
MOV ECX, [EBP+-20]
MOV DWORD PTR ECX, 27
MOV EAX, ECX
MOV [__base+20], EAX
MOV [EBP+-20], ECX
JMP ___label_6
___label_5:
MOV ECX, [EBP+-24]
MOV DWORD PTR ECX, 0
MOV EAX, ECX
MOV [__base+16], EAX
MOV [EBP+-24], ECX
MOV ECX, [EBP+-28]
MOV DWORD PTR ECX, 5
MOV EAX, ECX
MOV [__base+20], EAX
MOV [EBP+-28], ECX
___label_6:
MOV EAX, [__base+16]
MOV EDI, [__base+8]
MOV EDI, EAX
MOV EAX, [__base+20]
MOV [__base+8], EDI
MOV EDI, [__base+12]
MOV EDI, EAX
LEA EAX, [__base+8]
MOV ECX, [EBP+-32]
MOV ECX, EAX
LEA EAX, [__base+12]
MOV [EBP+-32], ECX
MOV ECX, [EBP+-36]
MOV ECX, EAX
MOV [EBP+-36], ECX
MOV ECX, [EBP+-32]
PUSH ECX
MOV [EBP+-32], ECX
MOV ECX, [EBP+-36]
PUSH ECX
PUSH EBP
MOV [EBP+-36], ECX
MOV [__base+12], EDI
CALL @__mymod
MOV ECX, [EBP+-36]
MOV EDI, [__base+12]
MOV [EBP+-36], ECX
MOV ECX, [EBP+-40]
MOV ECX, EAX
MOV EAX, ECX
MOV [EBP+-40], ECX
MOV ECX, [__base+36]
MOV ECX, EAX
MOV [__base+36], ECX
MOV ECX, [EBP+-44]
MOV DWORD PTR ECX, 0
MOV [__base+12], EDI
MOV EDI, [__base+8]
MOV EAX, EDI
CMP EAX, ECX
MOV [EBP+-44], ECX
MOV [__base+8], EDI
JE ___label_9
___label_7:
PUSH EBP
CALL @__swap
LEA EAX, [__base+8]
MOV ECX, [EBP+-52]
MOV ECX, EAX
LEA EAX, [__base+12]
MOV [EBP+-52], ECX
MOV ECX, [EBP+-56]
MOV ECX, EAX
MOV [EBP+-56], ECX
MOV ECX, [EBP+-52]
PUSH ECX
MOV [EBP+-52], ECX
MOV ECX, [EBP+-56]
PUSH ECX
PUSH EBP
MOV [EBP+-56], ECX
CALL @__mymod
MOV ECX, [EBP+-56]
MOV [EBP+-56], ECX
MOV ECX, [EBP+-60]
MOV ECX, EAX
MOV EAX, ECX
MOV [EBP+-60], ECX
MOV ECX, [__base+36]
MOV ECX, EAX
MOV [__base+36], ECX
MOV ECX, [EBP+-64]
MOV DWORD PTR ECX, 0
MOV EDI, [__base+8]
MOV EAX, EDI
CMP EAX, ECX
MOV [EBP+-64], ECX
MOV [__base+8], EDI
JE ___label_8
JMP ___label_7
___label_8:
___label_9:
MOV EDI, [__base+12]
MOV EAX, EDI
MOV ECX, [__base+24]
MOV ECX, EAX
MOV EAX, [__base+16]
MOV [__base+24], ECX
MOV ECX, [EBP+-68]
MOV ECX, EAX
MOV EAX, ECX
MUL DWORD PTR [__base+20]
MOV ECX, EAX
MOV EAX, ECX
CDQ
MOV [EBP+-68], ECX
MOV ECX, [__base+24]
IDIV DWORD PTR ECX
MOV [__base+24], ECX
MOV ECX, [EBP+-68]
MOV ECX, EAX
MOV EAX, ECX
MOV [EBP+-68], ECX
MOV ECX, [__base+28]
MOV ECX, EAX
PUSH offset __STR0
PUSH offset _str
MOV [__base+28], ECX
MOV [__base+12], EDI
CALL crt_printf
MOV ECX, [__base+28]
MOV EDI, [__base+12]
MOV [__base+28], ECX
MOV ECX, [__base+24]
PUSH DWORD PTR ECX
PUSH offset _int
MOV [__base+24], ECX
MOV [__base+12], EDI
CALL crt_printf
MOV ECX, [__base+24]
MOV EDI, [__base+12]
PUSH offset __STR1
PUSH offset _str
MOV [__base+24], ECX
MOV [__base+12], EDI
CALL crt_printf
MOV ECX, [__base+24]
MOV EDI, [__base+12]
MOV [__base+24], ECX
MOV ECX, [__base+28]
PUSH DWORD PTR ECX
PUSH offset _int
MOV [__base+28], ECX
MOV [__base+12], EDI
CALL crt_printf
MOV ECX, [__base+28]
MOV EDI, [__base+12]
MOV [__base+28], ECX
MOV [__base+12], EDI
JMP ___label_3
___label_10:
LEA EAX, [__base+8]
PUSH EAX
PUSH offset _intin
CALL scanf
PUSH DWORD PTR 0
CALL ExitProcess
main endp
end start