#ifndef ORDER_STATISTIC_TREE_H
#define ORDER_STATISTIC_TREE_H
#include <cassert>
#include <iterator>
template <typename T, typename Comp = std::less<T>>
class order_statistic_tree
{
	using self = order_statistic_tree<T, Comp>;
	using color_t = bool;

	static constexpr color_t red = false;
	static constexpr color_t black = true;

	struct tree_node
	{
		T val;
		struct tree_node* left;
		struct tree_node* right;
		struct tree_node* parent;
		color_t color;
		size_t subsize;
		tree_node( const T& _val ) :
			val( _val ), left( nullptr ), right( nullptr ), parent( nullptr ),
			subsize( 1 ), color( red ) {}
		tree_node( T&& _val ) :
			val( std::move(_val) ), left( nullptr ), right( nullptr ), parent( nullptr ),
			subsize( 1 ), color( red ) {}
	};

	friend void swap( order_statistic_tree<T, Comp>&, order_statistic_tree<T, Comp>& );
public:
	class iterator : public std::iterator<std::bidirectional_iterator_tag, T, size_t>
	{
		friend class self;
	private:
		const tree_node* curr;
		const self* tree;
	public:
		//using iterator_category = std::bidirectional_iterator_tag;
		//using value_type = T;
		// TODO: add non-const support
		iterator( const tree_node* p, const self* tp ) : curr( p ), tree( tp ) {};

		iterator& operator++();
		iterator operator++( int );
		iterator& operator--();
		iterator operator--( int );
		const T* operator->();
		const T& operator*();

		bool operator==( const iterator& rhs )
		{
			return curr == rhs.curr;
		}

		bool operator!=( const iterator& rhs )
		{
			return curr != rhs.curr;
		}
		/*
		iterator& to_left();
		iterator& to_right();
		*/
	};

	friend class tree_iterator;

	using rank_type = int;
	static constexpr rank_type npos = -1;

	order_statistic_tree()
	{
		nil = new tree_node( T() );
		nil->left = nil->right = nil->parent = nil;
		nil->color = black;
		nil->subsize = 0;
		root = pmax = pmin = nil;
	}

	order_statistic_tree( const Comp& );

	template<typename It>
	order_statistic_tree( It b, It e );

	order_statistic_tree( std::initializer_list<int> li )
		: order_statistic_tree()
	{
		for( auto it = li.begin(); it != li.end(); ++it ) {
			insert( *it );
		}
	}

	order_statistic_tree( const self& rhs );

	order_statistic_tree( self&& rhs );

	~order_statistic_tree();

	self& operator=( const self& rhs );

	self& operator=( self&& rhs );

	std::size_t size() const
	{
		return root->subsize;
	}

	bool empty() const
	{
		return root == nil;
	}

	/*void traverse()
	{
		traverse_helper( root );
	}

	void traverse_helper( tree_node* t )
	{
		if( t != nil ) {
			cout << t->val << " ";
			traverse_helper( t->left );
			traverse_helper( t->right );
		}
	}*/

	iterator insert( const T& key )
	{
		tree_node* n = new tree_node( key );
		n->left = n->right = n->parent = nil;
		insert_helper( n );
		return iterator( n, this );
	}

	iterator insert( T&& key )
	{
		tree_node* n = new tree_node( std::move(key) );
		n->left = n->right = n->parent = nil;
		insert_helper( n );
		return iterator( n, this );
	}

	template<typename It>
	void insert( It begin, It end )
	{
		for( auto i = begin; i != end; ++i ) {
			insert( *i );
		}
	}

	bool del( const T& key );
	bool del( const iterator& );
	void clear()
	{
		free_all_except_nil();
	}

	iterator find( const T& key ) const;
	iterator find_by_rank( rank_type ) const;
	rank_type rank( const T& key ) const;
	rank_type rank( const iterator& ) const;

	iterator begin() const;
	iterator end() const;

	//void check_subsize()
	//{
	//	// for debug
	//	check_helper( root );
	//}

	//void check_helper( tree_node* p )
	//{
	//	// for debug
	//	if( p != nil ) {
	//		assert( p->subsize == p->left->subsize + p->right->subsize + 1 );
	//		check_helper( p->left );
	//		check_helper( p->right );
	//	}
	//}

private:
	tree_node* root;
	tree_node* nil;
	tree_node* pmin;
	tree_node* pmax;
	Comp smaller;

	void insert_helper( tree_node* );
	void del_helper( tree_node* );

	void insert_fixer( tree_node* n );
	void del_fixer( tree_node* );
	void free_all_except_nil();

	tree_node* find_helper( const T& key ) const;
	void copy_helper( tree_node* prev, tree_node**, tree_node*, tree_node* rnil );
	void free_helper( tree_node* ) const;

	tree_node* find_min( const tree_node* ) const;
	tree_node* find_max( const tree_node* ) const;
	tree_node* predecessor( const tree_node* ) const;
	tree_node* successor( const tree_node* ) const;
	void transplant( tree_node* a, tree_node* b );
	void rotate_left( tree_node* a );
	void rotate_right( tree_node* a );
};

template<typename T, typename Comp>
template<typename It>
order_statistic_tree<T, Comp>::order_statistic_tree( It begin, It end )
	: order_statistic_tree()
{
	for( auto i = begin; i != end; ++i ) {
		insert( *i );
	}
}

template<typename T, typename Comp>
order_statistic_tree<T, Comp>::order_statistic_tree( const Comp& c )
	: order_statistic_tree()
{
	smaller( c );
}

template<typename T, typename Comp>
order_statistic_tree<T, Comp>::order_statistic_tree( const self& rhs )
	: smaller( rhs.smaller ), nil( new tree_node(T()) )
{
	nil->parent = nil->left = nil->right = nil;
	nil->color = black;
	nil->subsize = 0;
	copy_helper( nil, &root, rhs.root, rhs.nil );
	if( root == nil ) {
		pmin = pmax = nil;
	} else {
		tree_node* index = root;
		while( index->left != nil ) {
			index = index->left;
		}
		pmin = index;
		index = root;
		while( index->right != nil ) {
			index = index->right;
		}
		pmax = index;
	}
}

template<typename T, typename Comp>
order_statistic_tree<T, Comp>::order_statistic_tree( self&& rhs )
	: root( rhs.root ), pmax( rhs.pmax ), pmin( rhs.pmin ), nil( rhs.nil ), 
	  smaller( std::move(rhs.smaller) )
{
	// TODO : may be optimized : use nullptr to replace an empty new node
	rhs.root = rhs.pmax = rhs.pmin = rhs.nil = new tree_node(T());
}

template<typename T, typename Comp>
inline order_statistic_tree<T, Comp>::~order_statistic_tree()
{
	free_all_except_nil();
	delete nil;
}

template<typename T, typename Comp>
order_statistic_tree<T, Comp>& order_statistic_tree<T, Comp>::operator=( const self& rhs )
{
	free_all_except_nil();
	smaller = rhs.smaller;
	copy_helper( nil, &root, rhs.root, rhs.nil );
	if( root == nil ) {
		pmin = pmax = nil;
	} else {
		tree_node* index = root;
		while( index->left != nil ) {
			index = index->left;
		}
		pmin = index;
		index = root;
		while( index->right != nil ) {
			index = index->right;
		}
		pmax = index;
	}
	return *this;
}

template<typename T, typename Comp>
order_statistic_tree<T, Comp>& order_statistic_tree<T, Comp>::operator=( self&& rhs )
{
	free_all_except_nil();
	delete nil;

	smaller = std::move( rhs.smaller );
	root = rhs.root;
	pmin = rhs.pmin;
	pmax = rhs.pmax;
	nil = rhs.nil;
	rhs.root = rhs.pmax = rhs.pmin = rhs.nil = new tree_node( T() );
	return *this;
}

template<typename T, typename Comp>
bool order_statistic_tree<T, Comp>::del( const T& val )
{
	tree_node* n = find_helper( val );
	if( n == nil ) {
		return false;
	} else {
		del_helper( n );
		return true;
	}
}

template<typename T, typename Comp>
bool order_statistic_tree<T, Comp>::del( const iterator& it )
{
	del_helper( it.curr );
	return true;
}

template<typename T, typename Comp>
auto order_statistic_tree<T, Comp>::find( const T& key ) const -> iterator
{
	auto p = find_helper( key );
	return iterator( p, this );
}

template<typename T, typename Comp>
auto order_statistic_tree<T, Comp>::find_by_rank( rank_type rank ) const -> iterator
{
	if( rank <= 0 || rank > root->subsize ) {
		return iterator( nil, this );
	}

	auto curr = root;
	while( rank != curr->left->subsize + 1 ) {
		if( rank < curr->left->subsize + 1 ) {
			curr = curr->left;
		} else {
			rank -= curr->left->subsize + 1;
			curr = curr->right;
		}
	}

	return iterator( curr, this );
}

template<typename T, typename Comp>
auto order_statistic_tree<T, Comp>::rank( const T& key ) const -> rank_type
{
	rank_type r = 0;
	auto curr = root;
	while( curr != nil && (smaller( key, curr->val ) || smaller( curr->val, key )) ) {
		// not equal
		if( smaller( key, curr->val ) ) {
			curr = curr->left;
		} else {
			r += curr->left->subsize + 1;
			curr = curr->right;
		}
	}

	if( curr == nil ) {
		return npos;
	} else {
		return r + 1 + curr->left->subsize;
	}
}

template<typename T, typename Comp>
auto order_statistic_tree<T, Comp>::rank( const iterator& it ) const -> rank_type
{
	auto x = it.curr;
	rank_type r = x->left->subsize + 1;
	while( x != root ) {
		if( x == x->parent->right ) {
			r += x->parent->left->subsize + 1;
		}
		x = x->parent;
	}

	return r;
}

template<typename T, typename Comp>
inline auto order_statistic_tree<T, Comp>::begin() const -> iterator
{
	return iterator( pmin, this );
}

template<typename T, typename Comp>
inline auto order_statistic_tree<T, Comp>::end() const -> iterator
{
	return iterator( nil, this );
}

template<typename T, typename Comp>
void order_statistic_tree<T, Comp>::insert_helper( tree_node* nn )
{
	assert( nn->color == red );
	tree_node* p = nil;
	tree_node* curr = root;
	while( curr != nil ) {
		curr->subsize += 1;
		p = curr;
		if( smaller( nn->val, curr->val ) ) {
			curr = curr->left;
		} else {
			// recieve duplicate key
			curr = curr->right;
		}
	}


	assert( curr == nil );
	nn->parent = p;
	if( p == nil ) {
		assert( root == nil );
		root = nn;
		pmax = root;
		pmin = root;
	} else {
		if( smaller( nn->val, p->val ) ) {
			p->left = nn;
		} else {
			p->right = nn;
		}

		if( smaller( nn->val, pmin->val ) ) {
			pmin = nn;
		} else if( !smaller( nn->val, pmax->val ) ) {
			pmax = nn;
		}
	}
	//T min = pmin->val;
	insert_fixer( nn );
	//assert( pmin->val == min );
	//cout << "root size: " << root->subsize << endl;
}

template<typename T, typename Comp>
void order_statistic_tree<T, Comp>::insert_fixer( tree_node* n )
{
	
	tree_node* p;
	tree_node* w;
	while( n->parent->color == red ) {
		p = n->parent;
		if( p == p->parent->left ) {
			w = p->parent->right;
			if( w->color == red ) {
				p->color = black;
				w->color = black;
				p->parent->color = red;
				n = p->parent;
			} else {
				if( n == p->right ) {
					n = p;
					rotate_left( n );
					p = n->parent;
				}
				p->color = black;
				p->parent->color = red;
				rotate_right( p->parent );
			}
		} else {
			// p == p->parent->right
			w = p->parent->left;
			if( w->color == red ) {
				p->color = black;
				w->color = black;
				p->parent->color = red;
				n = p->parent;
			} else {
				if( n == p->left ) {
					n = p;
					rotate_right( n );
					p = n->parent;
				}
				p->color = black;
				p->parent->color = red;
				rotate_left( p->parent );
			}
		}

	}
	root->color = black;
	

	/*tree_node* w;
	while( n->parent->color == red ) {
		if( n->parent == n->parent->parent->left ) {
			w = n->parent->parent->right;
			if( w->color == red ) {
				n->parent->color = black;
				w->color = black;
				n->parent->parent->color = red;
				n = n->parent->parent;
			} else {
				if( n == n->parent->right ) {
					n = n->parent;
					rotate_left( n );
				}
				n->parent->color = black;
				n->parent->parent->color = red;
				rotate_right( n->parent->parent );
			}
		} else {
			assert( n->parent == n->parent->parent->right );
			w = n->parent->parent->left;
			if( w->color == red ) {
				n->parent->color = black;
				w->color = black;
				n->parent->parent->color = red;
				n = n->parent->parent;
			} else {
				if( n == n->parent->left ) {
					n = n->parent;
					rotate_right( n );
				}
				n->parent->color = black;
				n->parent->parent->color = red;
				rotate_left( n->parent->parent );
			}
		}
		assert( nil->color == black );
	}
	root->color = black;*/
}

template<typename T, typename Comp>
void order_statistic_tree<T, Comp>::del_helper( tree_node* dn )
{
	// TODO : fix pmin and pmax
	if( dn == nil ) {
		return;
	}

	if( dn == root ) {
		pmin = pmax = nil;
	} else if( dn == pmin ) {
		pmin = successor( pmin );
	} else if( dn == pmax ) {
		pmax = predecessor( pmax );
	}

	tree_node* y; 
	tree_node* z;
	color_t orig_color;
	y = dn;
	orig_color = y->color;
	if( dn->left == nil ) {
		z = y->right;
		transplant( y, z );
	} else if( dn->right == nil ) {
		z = y->left;
		transplant( y, z );
	} else {
		y = find_min( dn->right );
		orig_color = y->color;
		z = y->right;
		if( y->parent == dn ) {
			z->parent = y;
		} else {
			transplant( y, z );
			y->right = dn->right;
			dn->right->parent = y;
		}
		y->color = dn->color;
		y->subsize = dn->subsize;
		y->left = dn->left;
		dn->left->parent = y;
		transplant( dn, y );
	}

	delete dn; // release the resource;
	
	// fix subsize:
	tree_node* index = z->parent;
	while( index != nil ) {
		index->subsize -= 1;
		index = index->parent;
	}

	if( orig_color == black ) {
		del_fixer( z );
	}
}

template<typename T, typename Comp>
void order_statistic_tree<T, Comp>::del_fixer( tree_node* x )
{
	while( x != root && x->color != red ) {

		tree_node* bro;
		if( x == x->parent->left ) {
			bro = x->parent->right;
			if( bro->color == red ) {
				bro->color = black;
				x->parent->color = red;
				rotate_left( x->parent );
				bro = x->parent->right;
			}
			// now bro->color == black
			if( bro->left->color == black && bro->right->color == black ) {
				bro->color = red;
				x = x->parent;
			} else {
				if( bro->right->color == black ) {
					bro->color = red;
					bro->left->color = black;
					bro = bro->left;
					rotate_right( bro->parent );
				}
				bro->color = x->parent->color;
				x->parent->color = black;
				bro->right->color = black;
				rotate_left( x->parent );
				x = root;
			}
		} else {
			bro = x->parent->left;
			if( bro->color == red ) {
				bro->color = black;
				x->parent->color = red;
				rotate_right( x->parent );
				bro = x->parent->left;
			}
			// now bro->color == black
			if( bro->left->color == black && bro->right->color == black ) {
				bro->color = red;
				x = x->parent;
			} else {
				if( bro->left->color == black ) {
					bro->color = red;
					bro->right->color = black;
					bro = bro->right;
					rotate_left( bro->parent );
				}
				bro->color = x->parent->color;
				x->parent->color = black;
				bro->left->color = black;
				rotate_right( x->parent );
				x = root;
			}
		}
	}

	x->color = black;
}

template<typename T, typename Comp>
void order_statistic_tree<T, Comp>::free_all_except_nil()
{
	free_helper( root );
	root = pmax = pmin = nil;
}

template<typename T, typename Comp>
void order_statistic_tree<T, Comp>::transplant( tree_node* a, tree_node* b )
{
	/*transplant b to a's original position*/
	if( a->parent == nil ) {
		root = b;
	} else if( a->parent->left == a ) {
		a->parent->left = b;
	} else {
		a->parent->right = b;
	}
	b->parent = a->parent;
}

template<typename T, typename Comp>
void order_statistic_tree<T, Comp>::rotate_left( tree_node* a )
{
	auto b = a->right;
	a->right = b->left;
	if( b->left != nil ) {
		b->left->parent = a;
	}
	b->left = a;
	if( a->parent == nil ) {
		root = b;
	} else if( a == a->parent->left ) {
		a->parent->left = b;
	} else {
		a->parent->right = b;
	}
	b->parent = a->parent;
	a->parent = b;
	b->subsize = a->subsize;
	a->subsize = a->left->subsize + a->right->subsize + 1;
}

template<typename T, typename Comp>
void order_statistic_tree<T, Comp>::rotate_right( tree_node* a )
{
	auto b = a->left;
	a->left = b->right;
	if( b->right != nil ) {
		b->right->parent = a;
	}
	b->right = a;
	if( a->parent == nil ) {
		root = b;
	} else if( a == a->parent->left ) {
		a->parent->left = b;
	} else {
		a->parent->right = b;
	}
	b->parent = a->parent;
	a->parent = b;
	b->subsize = a->subsize;
	a->subsize = a->left->subsize + a->right->subsize + 1;
}

template<typename T, typename Comp>
auto order_statistic_tree<T, Comp>::find_helper( const T& key ) const -> tree_node*
{
	auto index = root;
	while( index != nil ) {
		if( !smaller( index->val, key ) && !smaller( key, index->val ) ) {
			return index;
		} else {
			if( smaller( key, index->val ) ) {
				index = index->left;
			} else {
				index = index->right;
			}
		}
	}

	return index;
}

template<typename T, typename Comp>
void order_statistic_tree<T, Comp>::copy_helper( tree_node* prev, tree_node** lpp, tree_node* rp, tree_node* rnil )
{
	if( rp != rnil ) {
		*lpp = new tree_node( rp->val );
		(*lpp)->parent = prev;
		(*lpp)->color = rp->color;
		(*lpp)->subsize = rp->subsize;
		copy_helper( *lpp, &((*lpp)->left),  rp->left, rnil );
		copy_helper( *lpp, &((*lpp)->right), rp->right, rnil );
	} else {
		*lpp = nil;
	}
}

template<typename T, typename Comp>
auto order_statistic_tree<T, Comp>::predecessor( const tree_node* curr ) const -> tree_node*
{
	if( curr->left != nil ) {
		return find_max( curr->left );
	} else {
		tree_node* prev = curr->parent;
		while( prev != nil && prev->left == curr ) {
			curr = prev;
			prev = prev->parent;
		}
		return const_cast<tree_node*>( prev );
	}
}

template<typename T, typename Comp>
auto order_statistic_tree<T, Comp>::successor( const tree_node* curr ) const -> tree_node*
{
	if( curr->right != nil ) {
		return find_min( curr->right );
	} else {
		tree_node* prev = curr->parent;
		while( prev != nil && prev->right == curr ) {
			curr = prev;
			prev = prev->parent;
		}
		return const_cast<tree_node*>( prev );
	}
}

template<typename T, typename Comp>
auto order_statistic_tree<T, Comp>::find_min( const tree_node* n ) const -> tree_node*
{
	while( n->left != nil ) {
		n = n->left;
	}
	return const_cast<tree_node*>( n );
}

template<typename T, typename Comp>
auto order_statistic_tree<T, Comp>::find_max( const tree_node* n ) const -> tree_node*
{
	while( n->right != nil ) {
		n = n->right;
	}
	return const_cast<tree_node*>(n);
}

template<typename T, typename Comp>
void order_statistic_tree<T, Comp>::free_helper( tree_node* n ) const
{
	if( n != nil ) {
		free_helper( n->left );
		free_helper( n->right );
		delete n;
	}
}

template<typename T, typename Comp>
auto order_statistic_tree<T, Comp>::iterator::operator++() -> iterator&
{
	curr = tree->successor( curr );
	return *this;
}

template<typename T, typename Comp>
auto order_statistic_tree<T, Comp>::iterator::operator++( int ) -> iterator
{
	auto temp = *this;
	curr = tree->successor( curr );
	return temp;
}

template<typename T, typename Comp>
auto order_statistic_tree<T, Comp>::iterator::operator--() -> iterator&
{
	curr = tree->predecessor( curr );
	return *this;
}

template<typename T, typename Comp>
auto order_statistic_tree<T, Comp>::iterator::operator--( int ) -> iterator
{
	auto temp = *this;
	curr = tree->predecessor( curr );
	return temp;
}

template<typename T, typename Comp>
const T* order_statistic_tree<T, Comp>::iterator::operator->()
{
	return curr;
}

template<typename T, typename Comp>
const T& order_statistic_tree<T, Comp>::iterator::operator*()
{
	return curr->val;
}

template<typename T, typename Comp>
void swap( order_statistic_tree<T, Comp>& lhs, order_statistic_tree<T, Comp>& rhs )
{
	using std::swap;
	swap( lhs.root, rhs.root );
	swap( lhs.pmin, rhs.pmin );
	swap( lhs.pmax, rhs.pmax );
	swap( lhs.nil, rhs.nil );
	swap( lhs.smaller, rhs.smaller );
}

#endif
