/*
Copyright (c) 2020  MIPT
Module Name:
    Stack.h
Abstract:
    ��������� ����� ����� ��� 64-������
    ���������� � ��� ������
Author:
    JulesIMF
Last Edit:
    09.10.2020 2:16
Edit Notes:
    1) ������� �������� ���������
    2) ����� stackCapacity
*/


#pragma once
#ifndef JULESIMF_STACK

#define JULESIMF_STACK
#include <stdlib.h>

typedef enum
{
    STACK_OK,
    STACK_NULL,
    STACK_OVERFLOW,
    STACK_UNDERFLOW,
    STACK_INVALID,
    STACK_DUPLICATION_ERROR,
    STACK_RESIZE_ERROR,
    STACK_RESCUE_ERROR
} StackStatus;


typedef struct Stack__ Stack;

Stack*          stackNew        (size_t capacity);
void            stackDelete     (Stack* stack);
StackStatus     stackIsValid    (Stack* stack);
StackStatus     stackPush       (Stack* stack, unsigned long long value);
StackStatus     stackPop        (Stack* stack);
StackStatus     stackSize       (Stack* stack, size_t* value);
StackStatus     stackCapacity   (Stack* stack, size_t* value);
StackStatus     stackTop        (Stack* stack, unsigned long long* value);
StackStatus     stackClear      (Stack* stack);
StackStatus     stackResize     (Stack* stack, size_t capacity);
Stack*          stackCopy       (Stack* stack);
StackStatus     stackRescue     (Stack* stack);


#endif // !JULESIMF_STACK
