// 2017.9.22
// shitong
// 
#include <cassert>
#include <cstdio>
#include <iostream>
#include <fstream>
#include <queue>
#include <vector>
#include <string>
#include <bitset>
#include <memory>
#include <cstdlib>
#include <algorithm>

enum file_mode
{
    BINARY, CHARACTER
};

enum need_to_do
{
    COMPRESS, DECOMPRESS, VIEW
};

class file_ibuffer
{
public:
    file_ibuffer( FILE* src, std::size_t buffer_size = 32 * 1024 )
		: buffer( buffer_size ), pos( 0 ), curr_size( 0 )
	{
	    fin = src;
	    std::fseek( fin, 0, SEEK_END );
	    file_size = std::ftell( fin );
	    std::fseek( fin, 0, SEEK_SET );

	    curr_size = std::fread( buffer.data(), sizeof( char ), buffer.size(), fin );
	}

    file_ibuffer( const file_ibuffer& ) = delete;
    file_ibuffer& operator=( const file_ibuffer& ) = delete;
    file_ibuffer( file_ibuffer&& rhs ) noexcept
		: fin( rhs.fin ), buffer( std::move(rhs.buffer)),
		  pos( rhs.pos ), curr_size( rhs.curr_size ), file_size( rhs.file_size )
	{
	    rhs.fin = nullptr;
	}

    // ~file_ibuffer()
    // 	{
	    
    // 	}

    void rewind()
	{
	    std::fseek( fin, 0, SEEK_SET );
	}

    bool has_next() const
	{
	    if( pos < curr_size ) {
			return true;
	    } else {
			assert( pos == curr_size );
			return std::ftell( fin ) < file_size;
	    }
	}

    uint8_t next()
	{
	    if( pos >= curr_size ) {
			assert( pos == curr_size );
			pos = 0;
			curr_size = std::fread( buffer.data(), sizeof( char ), buffer.size(), fin );
	    }

	    return buffer[pos++];
	}

    void read( char* buf, std::size_t sz )
	{
	    for( auto i = 0; i < sz; ++i ) {
			buf[i] = next();
	    }
	}

    long fsize() const
	{
	    return file_size;
	}

private:
    std::FILE* fin;
    std::vector<char> buffer;
    std::size_t pos;
    std::size_t curr_size;
    long file_size;
};

class file_obuffer
{
public:
    explicit file_obuffer( std::FILE* dest, file_mode md, std::size_t buffer_size = 32 * 1024 )
		: buffer( buffer_size ), pos( 0 ), mode(md),
		  fout(dest)
	{
	}

    file_obuffer( const file_obuffer& ) = delete;
    file_obuffer& operator==( const file_obuffer& ) = delete;

    file_obuffer( file_obuffer&& rhs )
		: fout( rhs.fout ), buffer( std::move(rhs.buffer) ), pos( rhs.pos ),
		  mode( rhs.mode )
	{
	    rhs.fout = nullptr;
	}

    ~file_obuffer()
	{
	    flush();
	}

    void write( uint8_t b )
	{
	    buffer[pos++] = b;
	    if( pos == buffer.size() ) {
			if (mode == BINARY)
				std::fwrite( buffer.data(), sizeof( char ), buffer.size(), fout );
			else {
				assert(mode == CHARACTER);
				std::vector<char> tmp(buffer);
				tmp.push_back('\0');
				std::fprintf(fout, "%s", tmp.data());
			}
			pos = 0;
	    }
	}

    void write( char const* data, std::size_t size )
	{
	    for( auto i = 0; i < size; ++i ) {
			write( data[i] );
	    }
	}

    void flush()
	{
	    if (mode == BINARY)
			std::fwrite( buffer.data(), sizeof( char ), pos, fout );
	    else {
			std::vector<char> tmp( buffer.begin(),
								   buffer.begin() + pos );
			tmp.push_back('\0');
			std::fprintf(fout, "%s", tmp.data());
	    }
	       
	    pos = 0;
	}

private:
    std::FILE* fout;
    std::vector<char> buffer;
    std::size_t pos;
    file_mode mode;
};

//constexpr int BUFFSIZE = 1024

static int maxlen = 0; // for debug

class huffman_tree
{
    friend void printfreelist( huffman_tree& ); //for debug
public:
    static constexpr int FREELISTLEN = 512;
    struct chinfo
    {
		static constexpr int BSSIZE = 511; // bitset size
		int bitcnt;
		std::bitset<BSSIZE> bits;
    };
    struct comp;

    friend struct comp;

    using huffman_ptr = int16_t;
    struct huffman_node
    {
		int32_t freq;
		huffman_ptr left;
		huffman_ptr right;
		bool is_leaf;
		uint8_t byte = 0;
    };

    explicit huffman_tree( int freq[] );
    explicit huffman_tree( std::ifstream & );
    huffman_tree( huffman_ptr r, huffman_node ( &arr )[FREELISTLEN] )
	{
	    root = r;
	    std::copy( std::begin( arr ), std::end( arr ), std::begin( freelist ) );
	}
    huffman_tree( const huffman_tree& rhs ) = delete;
    huffman_tree& operator=( const huffman_tree& rhs ) = delete;

    void get_coding_info( chinfo[] );

    char const* data() const
	{
	    return reinterpret_cast<char const*>( freelist );
	}

    std::size_t freelist_bytes() const
	{
	    return FREELISTLEN * sizeof(huffman_node);
	}

    huffman_ptr get_root() const {
		return root;
    }

    huffman_ptr get_left( huffman_ptr p ) const {
		return freelist[p].left;
    }

    huffman_ptr get_right( huffman_ptr p ) const {
		return freelist[p].right;
    }

    bool is_leaf_ptr( huffman_ptr p ) const {
		return freelist[p].is_leaf;
    }

    uint8_t get_byte( huffman_ptr p ) const {
		return freelist[p].byte;
    }
private:
    huffman_node freelist[FREELISTLEN];
    huffman_ptr root;
    huffman_ptr huff_alloc();
    void huff_free( huffman_ptr );
    void init_freelist();
    void traverse( huffman_ptr, chinfo[], std::bitset<chinfo::BSSIZE> &, int len, uint8_t bit ) const;
};

struct huffman_tree::comp
{
    huffman_tree& tree;
    explicit comp( huffman_tree& t ) : tree( t ) {}
    bool operator()( huffman_tree::huffman_ptr p1, huffman_tree::huffman_ptr p2 ) const {
		return tree.freelist[p1].freq > tree.freelist[p2].freq;
    }
};



void output_usage_and_quit( char* );
std::string proc_cmdflags( int, char*[], need_to_do* );
std::string check_filename( std::string, need_to_do );

uint8_t get_bit_by_offset( uint8_t c, int pos )
{
    assert( pos < 8 * sizeof( uint8_t ) );
    uint8_t mask = 1 << pos;
    return (c & mask) >> pos;
}

void printfreelist( huffman_tree& tree )
{
    std::ofstream testfile( "freelist.txt", std::ios::out );
    std::queue<huffman_tree::huffman_ptr> Q;
    Q.push( tree.root );
    while( !Q.empty() ) {
		auto outptr = Q.front();
		Q.pop();

		testfile << "subscript: " << (int)outptr
				 << "\nbyte: " << (int)tree.freelist[outptr].byte
				 << "\nfrequency: " << tree.freelist[outptr].freq
				 << "\nis leaf: " << tree.freelist[outptr].is_leaf
				 << "\nleft: " << (int)tree.freelist[outptr].left
				 << "   right: " << (int)tree.freelist[outptr].right
				 << "\n"
				 << std::endl;

		if( tree.freelist[outptr].left != 0 ) {
			Q.push( tree.freelist[outptr].left );
		}
		if( tree.freelist[outptr].right != 0 ) {
			Q.push( tree.freelist[outptr].right );
		}
    }
}

void printchinfo( huffman_tree::chinfo info[], int sz )
{
    std::ofstream file( "chinfo.txt", std::ios::out );
    for( int i = 0; i < sz; ++i ) {
		file << "subscript: " << i
			 << "\nbyte: " << (int)i
			 << "\nbit lenght: " << info[i].bitcnt
			 << "\nbits: ";
		for( int j = 0; j < info[i].bitcnt; ++j ) {
			file << info[i].bits[j];
		}
		file << "\n" << std::endl;
    }
}

void compress( file_ibuffer&& ifile, file_obuffer&& ofile );
void decompress( file_ibuffer&& ifile, file_obuffer&& ofile );

int main( int argc, char* argv[] )  // for command line
{
    need_to_do ntd;
    auto file_name = proc_cmdflags( argc, argv, &ntd );

    auto file_closer = [](FILE* file) { std::fclose(file); };
    if( ntd == COMPRESS ) {
		// unique_ptr as file guard
		std::unique_ptr<FILE, decltype(file_closer)>
			src(std::fopen(file_name.c_str(), "rb"), file_closer);
		std::unique_ptr<FILE, decltype(file_closer)>
			dest(std::fopen((file_name + ".huff").c_str(), "wb"), file_closer);
		compress( file_ibuffer(src.get()), file_obuffer(dest.get(), BINARY));
    } else {
		auto dest_filename { file_name };
		dest_filename.erase( dest_filename.size() - 5 ); // erase ".huff"
		std::unique_ptr<FILE, decltype(file_closer)>
			src(std::fopen(file_name.c_str(), "rb"), file_closer);

		if (ntd == DECOMPRESS) {
			std::unique_ptr<FILE, decltype(file_closer)>
				dest(std::fopen(dest_filename.c_str(), "wb"), file_closer);
			decompress( file_ibuffer( src.get() ),
						file_obuffer( dest.get(), BINARY ) );
		} else if (ntd == VIEW) {
			decompress( file_ibuffer( src.get() ), file_obuffer(stdout, CHARACTER) );
		} else {
			assert(0 && "no this option");
		}
    }
}

void compress( file_ibuffer&& src, file_obuffer&& dest )
{
    /*file_ibuffer src( s );
      file_obuffer dest( d );*/
    int freq[256] = { 0 };
    while( src.has_next() ) {
		++freq[src.next()];
    }
    src.rewind();

    huffman_tree ht( freq );
    huffman_tree::chinfo info[256];
    ht.get_coding_info( info );

    // 1.
    auto size = src.fsize();
    dest.write( reinterpret_cast<char const*>(&size), sizeof( long ) );

    // 2.
    auto root = ht.get_root();
    dest.write( reinterpret_cast<char const*>(&root), sizeof root );

    // 3.
    dest.write( ht.data(), ht.freelist_bytes() );

    // 4.
    uint8_t code = 0;
    int offset = 0;
    while( src.has_next() ) {
		auto byte = src.next();
		for (auto i = 0; i < info[byte].bitcnt; ++i) {
			uint8_t tmp = info[byte].bits[i];
			code = code | (tmp << offset);
			offset++;
			if (offset == 8 * sizeof( uint8_t )) {
				dest.write( code );
				code = 0;
				offset = 0;
			}
		}
    }

    if (offset != 0) {
		dest.write( code );
		//the last byte, maybe some useless 0 in it
    }
}

void decompress( file_ibuffer&& src, file_obuffer&& dest )
{
    /*file_ibuffer src( ifile );
      file_obuffer dest( ofile );*/

    long byte_cnt{};
    src.read( reinterpret_cast<char*>(&byte_cnt), sizeof( long ) );
    huffman_tree::huffman_node freelist[huffman_tree::FREELISTLEN];
    huffman_tree::huffman_ptr root{};
    src.read( reinterpret_cast<char*>(&root), sizeof root );
    src.read( reinterpret_cast<char*>(freelist), sizeof( freelist ) );
    huffman_tree ht( root, freelist );

    uint8_t byte = src.next();
    auto offset = 0;
    for( int i = 0; i < byte_cnt; ++i ) {
		huffman_tree::huffman_ptr curr = ht.get_root();
		while (!ht.is_leaf_ptr( curr )) {
			uint8_t b = get_bit_by_offset( byte, offset );
			if( b == 0 ) {
				curr = ht.get_left( curr );
			} else {
				assert( b == 1 );
				curr = ht.get_right( curr );
			}
			++offset;
			if( offset == 8 * sizeof(uint8_t) ) {
				if (src.has_next()) {
					byte = src.next();
				}
				offset = 0;
			}
		}

		dest.write( ht.get_byte( curr ) );
    }
    // dest.flush();
    // dtor will do this
}

void output_usage_and_quit( char* name )
{
    std::cerr << "Syntax error" << std::endl;
    std::cerr << "Usage: " << name << " [-c/-d] [src/dst]" << std::endl;
    std::exit( 1 );
}

std::string proc_cmdflags( int argc, char* argv[], need_to_do* ntd )
{
    if( argc != 3 ) {
		output_usage_and_quit( argv[0] );
		std::exit( 0 );
    } else {
		char* p = argv[1];
		if( *p != '-' ) {
			output_usage_and_quit( argv[0] );
		} else {
			++p;
			if( *p == 'c' ) {
				*ntd = COMPRESS;
			} else if( *p == 'd' ) {
				*ntd = DECOMPRESS;
			} else if( *p == 'v' ) {
				*ntd = VIEW;
			} else {
				output_usage_and_quit( argv[0] );
			}

			++p;
			if( *p != '\0' ) {
				output_usage_and_quit( argv[0] );
			}
		}

		return check_filename( std::string( argv[2] ), *ntd );
    }
}

std::string check_filename( std::string name, need_to_do ntd )
{
    if( ntd == COMPRESS ) {
		/*
		  if( name.find(".txt") == std::string::npos ) {
		  std::cerr << "unexpected file format" << std::endl;
		  }
		  std::exit( 1 );
		*/
    } else {
		auto pos = name.find( ".huff", name.size() - 5 );
		if( pos == std::string::npos ) {
			std::cerr << "unexpected file format" << std::endl;
			std::exit( 1 );
		}
    }

    return name;
}

void huffman_tree::init_freelist()
{
    // the first element of this list is remained as a header
    int i;
    for( i = 0; i < FREELISTLEN - 1; ++i ) {
		freelist[i].left = i + 1;
    }
    freelist[i].left = 0;
}

huffman_tree::huffman_ptr
huffman_tree::huff_alloc()
{
    huffman_ptr p;
    p = freelist[0].left;
    assert( p != 0 );
    freelist[0].left = freelist[p].left;
    return p;
}

void huffman_tree::huff_free( huffman_ptr p )
{
    assert( p != 0 );
    freelist[p].left = freelist[0].left;
    freelist[0].left = p;
}

huffman_tree::huffman_tree( int freq[] )
{
    init_freelist();
    comp cmp( *this );
    std::priority_queue<huffman_ptr,
						std::vector<huffman_ptr>, decltype(cmp)> pq( cmp );

    for( unsigned int i = 0; i < 256; ++i ) {
		huffman_ptr p = huff_alloc();
		freelist[p].byte = i;
		freelist[p].freq = freq[i];
		freelist[p].is_leaf = true;
		freelist[p].left = 0;
		freelist[p].right = 0;
		pq.push( p );
    }

    for( int i = 1; i <= 255; ++i ) {
		huffman_ptr out1 = pq.top();
		pq.pop();
		huffman_ptr out2 = pq.top();
		pq.pop();

		huffman_ptr newp = huff_alloc();
		freelist[newp].is_leaf = false;
		freelist[newp].freq = freelist[out1].freq + freelist[out2].freq;
		freelist[newp].left = out1;
		freelist[newp].right = out2;

		pq.push( newp );
    }

    root = pq.top();

    /*printfreelist( *this );*/
}

void huffman_tree::get_coding_info( chinfo info[] )
{
    std::bitset<chinfo::BSSIZE> bset;
    traverse( freelist[root].left, info, bset, 1, 0 );
    traverse( freelist[root].right, info, bset, 1, 1 );
}

void huffman_tree::traverse( huffman_ptr p, chinfo info[],
							 std::bitset<chinfo::BSSIZE>& bset, int len, uint8_t bit ) const
{
    if( len > maxlen )
		maxlen = len;

    //std::cerr << len << " ";

    assert( p != 0 );
    assert( len - 1 <= chinfo::BSSIZE );
    if( !freelist[p].is_leaf ) {
		bset[len - 1] = bit;
		traverse( freelist[p].left, info, bset, len + 1, 0 );
		traverse( freelist[p].right, info, bset, len + 1, 1 );
    } else {
		bset[len - 1] = bit;
		info[freelist[p].byte].bitcnt = len;
		info[freelist[p].byte].bits = bset;
    }
}

huffman_tree::huffman_tree( std::ifstream& ifile )
{
    huffman_ptr r;
    ifile.read( reinterpret_cast<char*>(&r), sizeof( huffman_ptr ) );
    root = r;
    ifile.read( reinterpret_cast<char*>(freelist), FREELISTLEN * sizeof( huffman_node ) );
}

