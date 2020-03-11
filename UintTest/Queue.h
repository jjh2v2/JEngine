#pragma once
#include "pch.h"

//���� ť - ����ũ�� ����, ���� ����
#include <stdio.h>

#define QUEUE_SIZE  10
#define NEXT(index)   ((index+1)%QUEUE_SIZE)  //���� ť���� �ε����� �����ϴ� ��ũ�� �Լ�

typedef struct Queue //Queue ����ü ����
{
	int buf[QUEUE_SIZE];//�����
	int front; //���� �ε���(���� �������� ������ �����Ͱ� �ִ� �ε���)
	int rear;//������ �ε���
}Queue;

void InitQueue(Queue *queue);//ť �ʱ�ȭ
int IsFull(Queue *queue); //ť�� �� á���� Ȯ��
int IsEmpty(Queue *queue); //ť�� ������� Ȯ��
void Enqueue(Queue *queue, int data); //ť�� ����
int Dequeue(Queue *queue); //ť���� ����

//int main(void)
//{
//	int i;
//	Queue queue;
//
//	InitQueue(&queue);//ť �ʱ�ȭ
//	for (i = 1; i <= 5; i++)//1~5���� ť�� ����
//	{
//		Enqueue(&queue, i);
//	}
//	while (!IsEmpty(&queue))//ť�� ������� �ʴٸ� �ݺ�
//	{
//		printf("%d ", Dequeue(&queue));//ť���� ������ �� ���
//	}
//	printf("\n");
//	return 0;
//}

void InitQueue(Queue *queue)
{
	queue->front = queue->rear = 0; //front�� rear�� 0���� ����
}
int IsFull(Queue *queue)
{
	//���� ť���� �� á���� ������� üũ�� �� �ְ� rear ���� ������ �� ���¸� �����մϴ�.
	return NEXT(queue->rear) == queue->front;//���� rear�� front�� ������ �� �� ����
}
int IsEmpty(Queue *queue)
{
	return queue->front == queue->rear;    //front�� rear�� ������ �� ����
}
void Enqueue(Queue *queue, int data)
{
	if (IsFull(queue))//ť�� �� á�� ��
	{
		printf("ť�� �� á��\n");
		return;
	}
	queue->buf[queue->rear] = data;//rear �ε����� ������ ����
	queue->rear = NEXT(queue->rear); //rear�� ���� ��ġ�� ����
}
int Dequeue(Queue *queue)
{
	int re = 0;
	if (IsEmpty(queue))//ť�� ����� ��
	{
		printf("ť�� �����\n");
		return re;
	}
	re = queue->buf[queue->front];//front �ε����� ������ ���� re�� ����
	queue->front = NEXT(queue->front);//front�� ���� ��ġ�� ����
	return re;
}