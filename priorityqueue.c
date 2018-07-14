#include <stdlib.h>
#include <stdio.h>
#include <assert.h>

struct pq
{
	int* data;
	int size;
	int capa;
};

void pq_init(struct pq* p, int capa)
{
	p->data = malloc(sizeof(int) * (capa + 1));
	p->capa = capa;
	p->size = 0;
}

void pq_insert(struct pq* p, int e)
{
	if (p->size == p->capa) {
		p->data = realloc(p->data, (2 * p->capa + 1) * sizeof(int));
		p->capa *= 2;
	}

	int pos = ++p->size;
	for ( ; pos != 1; pos /= 2) {
		int parent = pos / 2;
		if (e < p->data[parent]) {
			p->data[pos] = p->data[parent];
		} else {
			p->data[pos] = e;
			return;
		}
	}
	assert(pos == 1);
	p->data[pos] = e;
}

void down_filter(int data[], int size, int pos)
{
	int v = data[pos];
	int child;
	for ( ; 2 * pos <= size; pos = child) {
		int lchild = 2 * pos;
		int rchild = lchild + 1;
		int min = v;
		if (min <= data[lchild]) {
			if (rchild <= size && min <= data[rchild])
				break;
			if (rchild > size)
				break;
		}
		
		if (min > data[lchild]) {
			min = data[lchild];
			child = lchild;
		}
		if (rchild <= size && min > data[rchild]) {
			min = data[rchild];
			child = rchild;
		}
		data[pos] = min;
	}

	data[pos] = v;
}

int pq_popmin(struct pq* p)
{
	int ret = p->data[1];
	p->data[1] = p->data[p->size--];
	down_filter(p->data, p->size, 1);
	return ret;
}

int pq_min(struct pq* p)
{
	return p->data[1];
}

int main()
{
	struct pq qqq;
	pq_init(&qqq, 100);
	for (int i = 101; i >= 0; --i) {
		pq_insert(&qqq, i);
	}

	printf("min:%d\n", pq_min(&qqq));

	for (int i = 101; i >= 0; --i) {
		int v = pq_popmin(&qqq);
		printf("%d ", v);
	}
}
