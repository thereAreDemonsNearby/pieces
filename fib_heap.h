#ifndef FIB_HEAP_H__
#define FIB_HEAP_H__

#include <vector>
#include <cmath>
#include <cassert>

template <typename T, typename Comp>
class fib_heap;

template<typename T, typename C>
void swap( fib_heap<T, C>& lhs, fib_heap<T, C>& rhs );

template<typename T, typename C>
fib_heap<T, C> merge( fib_heap<T, C>&, fib_heap<T, C>& );

template<typename T, typename Comp = std::less<T>>
class fib_heap
{
	struct node
	{
		T val;
		bool mark = false;
		size_t degree = 0;
		node* child = nullptr;
		node* prev = nullptr;
		node* next = nullptr;
		node* parent = nullptr;
		node( const T& val_ ) : val(val_) {}
		node( T&& val_ ) : val( val_ ) {}
	};
public:
	using self = fib_heap<T, Comp>;

	class ptr
	{
		// 保证在指向的元素没有删除前是有效的,即使发生了merge,swap等骚操作
		friend class self;
	private:
		node* p;
	public:
		ptr( node* p_ = nullptr ) : p( p_ ) {}
		const T& val()
		{
			return p->val;
		}
	};
	
	friend class ptr;
	friend self merge<T, Comp>( self&, self& );
	friend void swap<T, Comp>( self&, self& );

	fib_heap() = default;
	fib_heap( const Comp& );
	fib_heap( const self& );
	fib_heap( self&& ) noexcept;
	~fib_heap();

	self& operator=( const self& rhs );
	self& operator=( self&& rhs ) noexcept;

	ptr push( const T& );
	ptr push( T&& );
	const T& top();
	void pop();
	void decrease_key( ptr, const T& new_key );

	void clear()
	{
		clean_all();
		size_ = 0;
		min = nullptr;
	}

	bool empty() 
	{
		return size_ == 0;
	}

	size_t size()
	{
		return size_;
	}

private:
	node* min = nullptr;
	size_t size_ = 0;
	Comp smaller;

	void push_helper( node* );
	void merge_list( node*, node* );

	/*在toberemoved所在的链表中将其删除，若删除后链表为空，则返回nullptr，否则返回toBeRemoved的next*/
	node* remove( node* toBeRemoved );

	/*将toBeAdded加入到list中。若list为空，返回新的唯一节点，若不空，返回list*/
	node* add( node* list, node* toBeAdded );

	/*在这里, child仅仅代表一个节点，而不是一个链表*/
	void be_child( node* child, node* parent );
	void consolidate();
	void cut( node* child, node* parent );
	void cascading_cut( node* );

	//清除所有节点
	void clean_all();
	void clean_helper( node* );
	void copy_helper( node**, node*, node* parent );
};

template<typename T, typename C>
fib_heap<T, C>::fib_heap( const C& comp_ )
	: comp( comp_ )
{

}

template<typename T, typename C>
fib_heap<T, C>::fib_heap( const fib_heap<T, C>& rhs )
{
	copy_helper( &min, rhs.min, nullptr );
	size_ = rhs.size_;
	smaller = rhs.smaller;
}

template<typename T, typename C>
fib_heap<T, C>::fib_heap( fib_heap<T, C>&& rhs ) noexcept
	: min( rhs.min ), smaller( std::move( rhs.smaller ) ), size_( rhs.size_ )
{
	rhs.min = nullptr;
}

template<typename T, typename C>
auto fib_heap<T, C>::operator=( const self& rhs ) -> self&
{
	clean_all();
	smaller = rhs.smaller;
	copy_helper( &min, rhs.min, nullptr );
	size_ = rhs.size_;
	return *this;
}

template<typename T, typename C>
auto fib_heap<T, C>::operator=( self&& rhs ) noexcept -> self&
{
	clean_all();
	min = rhs.min;
	smaller = std::move( rhs.smaller );
	size_ = rhs.size_;
	rhs.min = nullptr;
	return *this;
}

template<typename T, typename C>
fib_heap<T, C>::~fib_heap()
{
	clean_all();
}


template<typename T, typename C>
inline auto fib_heap<T, C>::push( const T& val ) -> ptr
{
	node *n  = new node( val );
	push_helper( n );
	return ptr( n );
}

template<typename T, typename C>
inline auto fib_heap<T, C>::push( T&& val ) ->ptr
{
	node* n = new node( val );
	push_helper( n );
	return ptr( n );
}

template<typename T, typename C>
inline const T& fib_heap<T, C>::top()
{
	return min->val;
}

template<typename T, typename C>
void fib_heap<T, C>::pop()
{
	if( min ) {
		auto childlist = min->child;
		if( childlist ) {
			auto index = childlist;
			do {
				index->parent = nullptr;
				index = index->next;
			} while( index != childlist );
			merge_list( min, childlist );
		}

		auto rest = remove( min );
		delete min;
		min = rest;
		if( min ) {
			consolidate();
		}

		--size_;
	} else {
		// throw an exception??
	}
}

template<typename T, typename C>
void fib_heap<T, C>::decrease_key( ptr pos, const T& new_key )
{
	auto pnode = pos.p;
	if( !smaller( pnode->val, new_key ) ) {
		// throw an exception?
		
	}

	pnode->val = new_key;
	auto parent = pnode->parent;
	if( parent && smaller( pnode->val, parent->val ) ) {
		cut( pnode, parent );
		cascading_cut( parent );
	}
	if( smaller( pnode->val, min->val ) ) {
		min = pnode;
	}
}

template<typename T, typename C>
void fib_heap<T, C>::push_helper( node* n )
{
	if( !min ) {
		min = n;
		n->prev = n->next = n;
	} else {
		n->prev = min->prev;
		n->next = min;
		min->prev->next = n;
		min->prev = n;
		if( smaller( n->val, min->val ) ) {
			min = n;
		}
	}
	++size_;
}


template<typename T, typename C>
void fib_heap<T, C>::consolidate()
{
	assert( min );
	size_t sz = log( size_ ) / log( 1.618 ) + 1;
	std::vector<node*> arr( sz, nullptr );
	auto index = min;
	do {
		// for each node in the root list
		auto x = index;
		auto next = index->next;
		auto d = index->degree;
		while( arr[d] ) {
			auto y = arr[d];
			if( smaller( y->val, x->val ) ) {
				std::swap( x, y );
			}
			be_child( y, x );
			arr[d] = nullptr;
			++d;
		}
		arr[d] = x;
		index = next;
	} while( index != min );

	min = nullptr;

	for( auto pnode : arr ) {
		if( pnode ) {
			min = add( min, pnode );
			if( smaller( pnode->val, min->val ) ) {
				min = pnode;
			}
		}
	}
}

template<typename T, typename C>
void fib_heap<T, C>::cut( node* child, node* parent )
{
	parent->child = remove( child );
	--parent->degree;
	min = add( min, child );
	child->parent = nullptr;
	child->mark = false;
}

template<typename T, typename C>
void fib_heap<T, C>::cascading_cut( node* one )
{
	node* p = one->parent;
	if( p ) {
		if( !one->mark ) {
			one->mark = true;
		} else {
			cut( one, p );
			cascading_cut( p );
		}
	}
}

template<typename T, typename C>
inline void fib_heap<T, C>::merge_list( node* li1, node* li2 )
{
	if( li1 && li2 ) {
		auto p = li2->prev;
		li2->prev->next = li1;
		li2->prev = li1->prev;
		li1->prev->next = li2;
		li1->prev = p;
	}
}

template<typename T, typename C>
inline auto fib_heap<T, C>::remove( node* p ) -> node*
{
	if( p == p->next ) {
		return nullptr;
	} else {
		auto right = p->next;
		right->prev = p->prev;
		p->prev->next = right;
		return right;
	}
}

template<typename T, typename C>
inline auto fib_heap<T, C>::add( node* list, node* p ) -> node*
{
	if( !list ) {
		p->next = p->prev = p;
		return p;
	} else {
		p->next = list;
		p->prev = list->prev;
		list->prev->next = p;
		list->prev = p;
		return list;
	}
}

template<typename T, typename C>
inline void fib_heap<T, C>::be_child( node* child, node* parent )
{
	child->parent = parent;
	child->mark = false;
	++parent->degree;
	parent->child = add( parent->child, child );
}

template<typename T, typename C>
inline void fib_heap<T, C>::clean_all()
{
	clean_helper( min );
	size_ = 0;
}

template<typename T, typename C>
void fib_heap<T, C>::clean_helper( node* li )
{
	if( li ) {
		auto index = li;
		do {
			auto next = index->next;
			clean_helper( index->child );
			delete index;
			index = next;
		} while( index != li );
	}
}

template<typename T, typename C>
void fib_heap<T, C>::copy_helper( node** lstart, node* rstart, node* parent )
{
	if( rstart ) {
		*lstart = new node( *rstart );
		node* lprev = *lstart;
		node* rindex = rstart->next;
		while( rindex != rstart ) {
			node* lnew = new node( *rindex );
			lnew->parent = parent;
			lprev->next = lnew;
			lnew->prev = lprev;
			lprev = lnew;
			rindex = rindex->next;
			copy_helper( &(lnew->child), rindex->child, lnew );
		}
		lprev->next = *lstart;
		(*lstart)->prev = lprev;
	} else {
		*lstart = nullptr;
	}
}

template<typename T, typename C>
void swap( fib_heap<T, C>& lhs, fib_heap<T, C>& rhs )
{
	using std::swap;
	swap( lhs.min, rhs.min );
	swap( lhs.size_, rhs.size_ );
	swap( lhs.smaller, rhs.smaller );
}

template<typename T, typename C>
fib_heap<T, C> merge( fib_heap<T, C>& lhs, fib_heap<T, C>& rhs )
{
	fib_heap<T, C> newone;
	newone.min = lhs.min;
	if( !lhs.min || ( rhs.min && lhs.smaller( rhs.min->val, lhs.min->val ) ) ) {
		newone.min = rhs.min;
	}
	newone.merge_list( lhs.min, rhs.min );

	newone.size_ = lhs.size_ + rhs.size_;
	newone.smaller = lhs.smaller;

	lhs.min = nullptr;
	lhs.size_ = 0;
	rhs.min = nullptr;
	rhs.size_ = 0;

	return newone;
}

#endif
