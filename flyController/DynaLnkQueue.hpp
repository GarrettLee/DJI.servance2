/***
*DynaLnkQueue.h - ��̬��ʽ���еĶ���
*
****/
#ifndef DYNALINKQUEUE_H
#define DYNALINKQUEUE_H

//#pragma once
#include <stdlib.h>
#include <malloc.h>
#include <memory.h>
#include <assert.h>

#include "ElemType.h"

template <typename Type>		//ģ���಻֧�ְ�������ʵ�ַ��ڲ�ͬ���ļ�
class Queue
{
public:
	/*------------------------------------------------------------
	// ��ʽ���нṹ�Ķ���
	------------------------------------------------------------*/

	typedef struct Node
	{
		Type data;              // Ԫ������
		struct Node *next;          // ��ʽ�����н��Ԫ�ص�ָ��
	} QNode, *QueuePtr;

	typedef struct
	{
		QueuePtr front;             // ����ͷָ��
		QueuePtr rear;              // ����βָ��
	} LinkQueue;

	LinkQueue q;
	LinkQueue *queue = &q;

	Queue(void);
	/*------------------------------------------------------------
	// ��ʽ���еĻ�������
	------------------------------------------------------------*/

	bool InitQueue();
	void DestroyQueue();
	bool QueueEmpty();
	int  QueueLength();
	bool GetHead(Type *e);
	void QueueTraverse(void(*fp)(Type));
	void ClearQueue();
	bool EnQueue(Type e);
	bool DeQueue(Type *e);
private:
	// ��
}; 

/*-----------------------------------------------------------
�ṹ��
����ͼ��ʾ��������front��rear�ֱ�ָ����еĿ�ͷ�ͽ�β�������ر���ǣ�frontָ���data���������ݣ������������е�һԱ����rear��ָ���data
�������е�һԱ�������һ�����ݡ�

    			 �����ã�data��
   ��front(QNode)��				   ������1(data)
Q����	         �����ݵ�Ԫ1(QNode)��                ������2��data��
   ��                              �����ݵ�Ԫ2(QNode)��                            ������n+2��data��
   ��                                                �����м��n�����ݵ�Ԫ(QNode)����                         ������n+3��data��
   ����������������������������������������������������������������������������������rear/���ݵ�Ԫn+3��QNode����
																											  ��NULL��QNode��
----------------------------------------------------------------------------------------------------------------------------*/

template <typename Type>
Queue<Type>::Queue(void){

	InitQueue();
}


/*------------------------------------------------------------
����Ŀ�ģ�   ��ʼ������
��ʼ������   ��
���������   ����һ���յĶ���
����������
LinkQueue *Q    ����ʼ���Ķ���
����ֵ��
bool            �����Ƿ�ɹ�
------------------------------------------------------------*/
template <typename Type>
bool Queue<Type>::InitQueue()
{
	queue->front = queue->rear = (QueuePtr)malloc(sizeof(QNode));
	if (queue->front == NULL)
		return false;
	queue->front->next = NULL;
	return true;
}

/*------------------------------------------------------------
����Ŀ�ģ�   ���ٶ���
��ʼ������   ����Q�Ѵ���
���������   ���ٶ���Q
����������
LinkQueue *Q    �����ٵĶ���
����ֵ��
��
------------------------------------------------------------*/
template <typename Type>
void Queue<Type>::DestroyQueue()
{
	assert(queue != NULL);
	while (queue->front)
	{
		queue->rear = queue->front->next;
		free(queue->front);
		queue->front = queue->rear;
	}
}

/*------------------------------------------------------------
����Ŀ�ģ�   �ж϶����Ƿ�Ϊ��
��ʼ������   ����Q�Ѵ���
���������   ��QΪ�ն��У��򷵻�true�����򷵻�false
����������
LinkQueue Q     ���жϵĶ���
����ֵ��
bool            �Ƿ�Ϊ��
------------------------------------------------------------*/
template <typename Type>
bool Queue<Type>::QueueEmpty()
{
	assert(queue->front != NULL && queue->rear != NULL);
	if (queue->front == queue->rear)
		return true;
	else
		return false;
}
/*------------------------------------------------------------
����Ŀ�ģ�   �õ����еĳ���
��ʼ������   ����Q�Ѵ���
���������   ����Q������Ԫ�صĸ���
����������
LinkQueue Q     ����Q
����ֵ��
int             ����Ԫ�صĸ���
------------------------------------------------------------*/
template <typename Type>
int Queue<Type>::QueueLength()
{
	assert(queue->front != NULL);
	QueuePtr p = queue->front;
	int Length = 0;
	while (p != queue->rear)
	{
		Length++;
		p = p->next;
	}
	return Length;
}
/*------------------------------------------------------------
����Ŀ�ģ�   �õ�������Ԫ��
��ʼ������   ����Q�Ѵ���
���������   ��e���ض�����Ԫ��
����������
LinkQueue Q     ����Q
ElemType *e     ������Ԫ�ص�ֵ
����ֵ��
bool            �����Ƿ�ɹ�
------------------------------------------------------------*/
template <typename Type>
bool Queue<Type>::GetHead(Type *e)
{
	assert(queue->front != NULL);
	if (QueueEmpty())
		return false;
	else
	{
		*e = queue->front->next->data;
		return true;
	}

}
/*------------------------------------------------------------
����Ŀ�ģ�   ��������
��ʼ������   ����Q�Ѵ���
���������   ���ζ�Q��ÿ��Ԫ�ص��ú���fp
����������
LinkQueue Q     ����Q
void (*fp)()    ����ÿ������Ԫ�صĺ���ָ��
����ֵ��
��
------------------------------------------------------------*/
template <typename Type>
void Queue<Type>::QueueTraverse(void(*fp)(Type))
{
	assert(queue->front != NULL);
	QueuePtr p = queue->front->next;
	while (p)
	{
		(*fp)(p->data);
		p = p->next;
	}
}

/*------------------------------------------------------------
����Ŀ�ģ�   ��ն���
��ʼ������   ����Q�Ѵ���
���������   ���������
����������
LinkQueue *Q    ����Q
����ֵ��
��
------------------------------------------------------------*/
template <typename Type>
void Queue<Type>::ClearQueue()
{
	assert(queue->front != NULL);
	QueuePtr p = queue->front->next;
	while (p)
	{
		queue->front->next = p->next;
		free(p);
		p = queue->front->next;
	}
	queue->rear = queue->front;
}

/*------------------------------------------------------------
����Ŀ�ģ�   �ڶ���ĩβ����Ԫ��e
��ʼ������   ����Q�Ѵ���
���������   ����Ԫ��e��Ϊ�����µ�β���
����������
LinkQueue *Q        ����Q
ElemType e      �����������Ԫ��
����ֵ��
bool            �����Ƿ�ɹ�
------------------------------------------------------------*/
template <typename Type>
bool Queue<Type>::EnQueue(Type e)
{
	QueuePtr temp = (QueuePtr)malloc(sizeof(QNode));
	if (!temp)
		return false;
	temp->data = e;
	temp->next = NULL;
	queue->rear->next = temp;
	queue->rear = temp;
	
	return true;
}

/*------------------------------------------------------------
����Ŀ�ģ�   ɾ����ʽ���е�ͷ���
��ʼ������   ����Q�Ѵ���
���������   ɾ����ʽ���е�ͷ���
����������
LinkQueue *Q        ����Q
ElemType *e     �����������Ԫ��
����ֵ��
bool            �����Ƿ�ɹ�
------------------------------------------------------------*/
template <typename Type>
bool Queue<Type>::DeQueue(Type *e)
{
	if (queue->front == queue->rear)
		return false;
	QueuePtr temp = queue->front->next;
	if (e != NULL) *e = temp->data;
	queue->front->next = temp->next;
	if (queue->rear == temp)
		queue->rear = queue->front;
	free(temp);
	
	return true;
}

#endif /* DYNALNKQUEUE_H */