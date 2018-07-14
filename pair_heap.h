#ifndef PAIR_HEAP_H
#define PAIR_HEAP_H

#include <functional>

template<typename T, typename Comp = std::less<T>, typename Minus = std::minus<T>>
class pair_heap
{
public:

	struct tree_node 
	{
		T element;
		tree_node* prev;
		tree_node* next_sibling;
		tree_node* left_child;
	};

	pair_heap()
		: heap_root( nullptr )
	{
		// empty body
	}

	void insert( const T& new_item, tree_node** loc )
	{
		tree_node* new_node = new tree_node;
		new_node->element = new_item;
		new_node->left_child = nullptr;
		new_node->next_sibling = nullptr;
		new_node->prev = nullptr;

		if( loc != nullptr ) {
			*loc = new_node;
		}

		if( heap_root == nullptr ) {
			heap_root = new_node;
		} else {
			heap_root = compare_and_link( heap_root, new_node );
		}
	}

	T delete_min()
	{
		T min_ele = heap_root->element;
		tree_node* new_root;
		if( heap_root->left_child != nullptr ) {
			new_root = combine_siblings( heap_root->left_child );
		} else {
			new_root = nullptr;
		}

		delete heap_root;
		heap_root = new_root;

		return min_ele;
	}

	void decrease_key( tree_node* pos, const T& delta )
	{
		minus( pos->element, delta ); // decrease key
		// break links:
		if( pos->next_sibling != nullptr ) {
			pos->next_sibling->prev = pos->prev;
		}
		
		if( pos->prev->left_child == pos ) {
			pos->prev->left_child = pos->next_sibling;
		}

		if( pos->prev->next_sibling == pos ) {
			pos->prev->next_sibling = pos->next_sibling;
		}

		pos->next_sibling = nullptr;
		heap_root = compare_and_link( heap_root, pos );
	}

	void clear()
	{
		clear_helper( heap_root );
		heap_root = nullptr;
	}

	bool empty()
	{
		return heap_root == nullptr;
	}

	~pair_heap()
	{
		clear();
	}
private:
	tree_node* heap_root;
	Comp comp;
	Minus minus;

	tree_node* compare_and_link( tree_node* first, tree_node* second )
	{
		if( second == nullptr ) {
			return first;
		} else if( comp( first->element, second->element ) ) {
					// first->element < second->element
			second->prev = first;
			first->next_sibling = second->next_sibling;
			if( first->next_sibling != nullptr ) {
				first->next_sibling->prev = first;
			}
			second->next_sibling = first->left_child;
			if( second->next_sibling != nullptr ) {
				second->next_sibling->prev = second;
			}
			first->left_child = second;

			return first;
		} else {
			second->prev = first->prev;
			first->prev = second;
			first->next_sibling = second->left_child;
			if( first->next_sibling != nullptr ) {
				first->next_sibling->prev = first;
			}
			second->left_child = first;

			return second;
		}
	}

	tree_node* combine_siblings( tree_node* first_sibling )
	{
		if( first_sibling->next_sibling == nullptr ) {
			return first_sibling;
		}

		const int maxsz = 400000;
		using tree_ptr = tree_node*;
		tree_ptr* tree_arr = new tree_ptr[maxsz];

		int num, i, j;
		for( num = 0; first_sibling != nullptr; ++num ) {
			tree_arr[num] = first_sibling;
			first_sibling->prev->next_sibling = nullptr;
			// heap_root->next_sibling will be set as nullptr, but it doesn't matter
			first_sibling = first_sibling->next_sibling;
		}
		tree_arr[num] = nullptr;

		for( i = 0; i + 1 < num; i += 2 ) {
			tree_arr[i] = compare_and_link( tree_arr[i], tree_arr[i + 1] );
		}

		j = i - 2;
		if( j == num - 3 ) {
			tree_arr[j] = compare_and_link( tree_arr[j], tree_arr[j + 2] );
		}

		for( ; j >= 2; j -= 2 ) {
			tree_arr[j - 2] = compare_and_link( tree_arr[j - 2], tree_arr[j] );
		}

		return tree_arr[0];
	}

	void clear_helper( tree_node* root )
	{
		if( root != nullptr ) {
			tree_node* index = root->left_child;
			tree_node* tmp;
			while( index ) {
				tmp = index->next_sibling;
				clear_helper( index );
				index = tmp;
			}
			delete root;
		}
	}
};

#endif
