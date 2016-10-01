/***
*ElemType.h - ElemTypeµÄ¶¨Òå
*
****/

#ifndef ELEMTYPE_H
#define ELEMTYPE_H

typedef struct Elem{
	int len;
	char * data;
}ElemType;

//int  compare(ElemType x, ElemType y);
//void visit(ElemType e);
ElemType newElemType(int len, char * data);
int freeElemType(ElemType);

#endif /* ELEMTYPE_H */