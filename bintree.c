#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

typedef int T;

struct node
{
	T val;
	struct node* left;
	struct node* right;
	struct node* parent;
};

struct bin_tree
{
	struct node* root;
	int cnt;
};

void bt_init(struct bin_tree* t)
{
	t->root = NULL;
	t->cnt = 0;
}

void bt_insert(struct bin_tree* t, T e)
{
	struct node* pn = malloc(sizeof(struct node));
	pn->val = e;
	pn->left = pn->right = pn->parent = NULL;

	struct node* tmp = NULL;
	struct node** pp = &t->root;
	while (*pp) {
		tmp = *pp;
		if ((*pp)->val > e) {
			pp = &(*pp)->left;
		} else if ((*pp)->val < e) {
			pp = &(*pp)->right;
		} else {
			return;
		}		   
	}

	(*pp) = pn;
	pn->parent = tmp;

	t->cnt++;
}

struct node* bt_find(struct bin_tree* t, T e)
{
	struct node* p = t->root;
	while (p) {
		if (e == p->val)
			return p;
		else if (e < p->val)
			p = p->left;
		else
			p = p->right;
	}
	
	return p;
}

int bt_delete(struct bin_tree* t, T e)
{
	struct node* p = bt_find(t, e);
	if (!p) {
		return 0;
	}
	
	if (!p->left && !p->right) {
		if (p->parent) {
			if (p == p->parent->left) {
				p->parent->left = NULL;
			} else {
				p->parent->right = NULL;
			}
		} else {
			t->root = NULL;
		}

	} else if (p->left && p->right) {

		struct node* lm = p->right;
		while (lm->left)
			lm = lm->left;

		if (lm != p->right) {
			lm->parent->left = lm->right;
			if (lm->right)
				lm->right->parent = lm->parent;
			
			lm->left = p->left;
			p->left->parent = lm;
			lm->right = p->right;
			p->right->parent = lm;
			
			lm->parent = p->parent;
			if (p->parent) {
				if (p->parent->left == p)
					p->parent->left = lm;
				else
					p->parent->right = lm;
			} else {
				assert(lm->parent  == NULL);
				t->root = lm;
			}

		} else {
			lm->left = p->left;
			p->left->parent = lm;
			lm->parent = p->parent;
			if (p->parent) {
				if (p->parent->left == p)
					p->parent->left = lm;
				else
					p->parent->right = lm;
			} else {
				t->root = lm;
			}
		}
		
	} else {
		struct node* son = p->left ? p->left : p->right;
		son->parent = p->parent;
		if (p->parent) {
			if (p->parent->left == p) {
				p->parent->left = son;
			} else {
				p->parent->right = son;
			}
		} else {
			t->root = son;
		}
	}

	t->cnt--;
	free(p);
	return 1;
}

struct node* bt_next(struct node* p)
{
	if (p->right) {
		struct node* lm = p->right;
		while (lm->left)
			lm = lm->left;
		return lm;
	} else {		
		while (p->parent) {
			if (p->parent->left == p) {
				return p->parent;
			}
			p = p->parent;
		}
		return NULL;
	}
}

struct node* bt_prev(struct node* p)
{
	if (p->left) {
		struct node* lm = p->left;
		while (lm->right)
			lm = lm->right;
		return lm;
	} else {		
		while (p->parent) {
			if (p->parent->right == p) {
				return p->parent;
			}
			p = p->parent;
		}
		return NULL;
	}
}

struct node* bt_begin(struct bin_tree* t)
{
	struct node* lm = t->root;
	if (lm) {
		while (lm->left)
			lm = lm->left;
		return lm;
	} else
		return NULL;
}

struct node* bt_rbegin(struct bin_tree* t)
{
	struct node* rm = t->root;
	if (rm) {
		while (rm->right) {
			rm = rm->right;
		}
		return rm;
	} else
		return NULL;
}

#define SZ 100

void swap(T* a, T* b)
{
	T tmp = *a;
	*a = *b;
	*b = tmp;
}

int randbetween(int a, int b)
{
	return rand() % (b - a + 1) + a;
}

int main()
{
	struct bin_tree tree;
	bt_init(&tree);
	T arr[SZ];
	for (int i = 0; i < SZ; ++i)
		arr[i] = i;
	for (int i = 0; i < SZ; ++i) {
		int pos = randbetween(i, SZ - 1);
		swap(&arr[i], &arr[pos]);
	}

	for (int i = 0; i < SZ; ++i) {
		bt_insert(&tree, arr[i]);
	}
	printf("size: %d\n", tree.cnt);

	struct node* beg = bt_begin(&tree);
	while (beg != NULL) {
		printf("%d ", beg->val);
		beg = bt_next(beg);
	}
	putchar('\n');


	printf("find1:\n");
	for (int i = 0; i < SZ; ++i) {
		int f = !!bt_find(&tree, arr[i]);
		printf("%d ", f);
	}
	printf("\nfind2:\n");
	for (int i = 100; i < 200; ++i) {
		int f = !!bt_find(&tree, i);
		printf("%d ", f);
	}

	printf("\nbegin delete\n");
	for (int i = 0; i < SZ; ++i) {
		int d = bt_delete(&tree, arr[i]);
		printf("%d ", d);
	}
	printf("\nsize: %d\n", tree.cnt);
	printf("root: %p\n", tree.root);
}
