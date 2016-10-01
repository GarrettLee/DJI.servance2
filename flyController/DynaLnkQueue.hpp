/***
*DynaLnkQueue.h - 动态链式队列的定义
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

template <typename Type>		//模板类不支持把声明和实现放在不同的文件
class Queue
{
public:
	/*------------------------------------------------------------
	// 链式队列结构的定义
	------------------------------------------------------------*/

	typedef struct Node
	{
		Type data;              // 元素数据
		struct Node *next;          // 链式队列中结点元素的指针
	} QNode, *QueuePtr;

	typedef struct
	{
		QueuePtr front;             // 队列头指针
		QueuePtr rear;              // 队列尾指针
	} LinkQueue;

	LinkQueue q;
	LinkQueue *queue = &q;

	Queue(void);
	/*------------------------------------------------------------
	// 链式队列的基本操作
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
	// …
}; 

/*-----------------------------------------------------------
结构：
如下图所示，队列中front和rear分别指向队列的开头和结尾，并且特别的是，front指向的data是无用数据，不算做队列中的一员，而rear所指向的data
算作队列的一员且是最后一个数据。

    			 ┌无用（data）
   ┌front(QNode)┤				   ┌数据1(data)
Q─┤	         └数据单元1(QNode)┤                ┌数据2（data）
   │                              └数据单元2(QNode)┤                            ┌数据n+2（data）
   │                                                └…中间的n的数据单元(QNode)…┤                         ┌数据n+3（data）
   └───────────────────────────────────────┴rear/数据单元n+3（QNode）┤
																											  └NULL（QNode）
----------------------------------------------------------------------------------------------------------------------------*/

template <typename Type>
Queue<Type>::Queue(void){

	InitQueue();
}


/*------------------------------------------------------------
操作目的：   初始化队列
初始条件：   无
操作结果：   构造一个空的队列
函数参数：
LinkQueue *Q    待初始化的队列
返回值：
bool            操作是否成功
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
操作目的：   销毁队列
初始条件：   队列Q已存在
操作结果：   销毁队列Q
函数参数：
LinkQueue *Q    待销毁的队列
返回值：
无
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
操作目的：   判断队列是否为空
初始条件：   队列Q已存在
操作结果：   若Q为空队列，则返回true，否则返回false
函数参数：
LinkQueue Q     待判断的队列
返回值：
bool            是否为空
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
操作目的：   得到队列的长度
初始条件：   队列Q已存在
操作结果：   返回Q中数据元素的个数
函数参数：
LinkQueue Q     队列Q
返回值：
int             数据元素的个数
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
操作目的：   得到队列首元素
初始条件：   队列Q已存在
操作结果：   用e返回队列首元素
函数参数：
LinkQueue Q     队列Q
ElemType *e     队列首元素的值
返回值：
bool            操作是否成功
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
操作目的：   遍历队列
初始条件：   队列Q已存在
操作结果：   依次对Q的每个元素调用函数fp
函数参数：
LinkQueue Q     队列Q
void (*fp)()    访问每个数据元素的函数指针
返回值：
无
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
操作目的：   清空队列
初始条件：   队列Q已存在
操作结果：   将队列清空
函数参数：
LinkQueue *Q    队列Q
返回值：
无
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
操作目的：   在队列末尾插入元素e
初始条件：   队列Q已存在
操作结果：   插入元素e作为队列新的尾结点
函数参数：
LinkQueue *Q        队列Q
ElemType e      待插入的数据元素
返回值：
bool            操作是否成功
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
操作目的：   删除链式队列的头结点
初始条件：   队列Q已存在
操作结果：   删除链式队列的头结点
函数参数：
LinkQueue *Q        队列Q
ElemType *e     待插入的数据元素
返回值：
bool            操作是否成功
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