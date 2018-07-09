#ifndef UNICODECONVERT_H__
#define UNICODECONVERT_H__

#include <cstdint>
#include <tuple>
#include <vector>
#include <cassert>
#include <type_traits>

namespace slt
{

struct group
{
	uint16_t res[4]; // big enough
	size_t sz;
};

constexpr uint8_t utf8BOM[] = { 0xEF, 0xBB, 0xBF };
constexpr uint16_t utf16BOM = 0xFEFF;
constexpr uint32_t utf32BOM = 0xFEFF;

// utilities:
// shouldn't be used directly
uint32_t Utf8ToUnicode( uint8_t );
uint32_t Utf8ToUnicode( uint8_t, uint8_t );
uint32_t Utf8ToUnicode( uint8_t, uint8_t, uint8_t );
uint32_t Utf8ToUnicode( uint8_t, uint8_t, uint8_t, uint8_t );

group UnicodeToUtf8( uint32_t unicode );

uint32_t Utf16ToUnicode( uint16_t );
uint32_t Utf16ToUnicode( uint16_t, uint16_t );

group UnicodeToUtf16( uint32_t );

// commom interfaces:

//default: utf8 without bom
template<typename InIter, typename OutIter>
OutIter Utf8ToUtf16( InIter inBegin, InIter inEnd, OutIter outBegin );

template<typename InIter, typename OutIter>
OutIter Utf16ToUtf8( InIter inBegin, InIter inEnd, OutIter outBegin );


template<typename InIter, typename OutIter>
OutIter Utf8ToUtf32( InIter inBegin, InIter inEnd, OutIter outBegin );

template<typename InIter, typename OutIter>
OutIter Utf32ToUtf8( InIter inBegin, InIter inEnd, OutIter outBegin );


template<typename InIter, typename OutIter>
OutIter Utf16ToUtf32( InIter inBegin, InIter inEnd, OutIter outBegin );

template<typename InIter, typename OutIter>
OutIter Utf32ToUtf16( InIter inBegin, InIter inEnd, OutIter outBegin );



//---------------------------------------------------------------------------
// implementations of function templates
template <typename InIter, typename OutIter>
OutIter Utf8ToUtf16( InIter inBegin, InIter inEnd, OutIter outBegin )
{
	//static_assert(sizeof *inBegin == 1 && sizeof *outBegin == 2);
	/*constexpr uint16_t utf16BOM = 0xFEFF;
	*outBegin++ = utf16BOM; */
	auto iter = inBegin;
	while (iter != inEnd) {
		if (*iter <= 0x7F) {
			group bytes = UnicodeToUtf16( Utf8ToUnicode( *iter++ ) );
			for (auto i = 0u; i < bytes.sz; ++i) {
				*outBegin++ = bytes.res[i];
			}
		} else if (*iter <= 0xDF) {
			auto c1 = *iter++;
			auto c2 = *iter++;
			group bytes = UnicodeToUtf16( Utf8ToUnicode( c1, c2 ) );
			for (auto i = 0u; i < bytes.sz; ++i) {
				*outBegin++ = bytes.res[i];
			}
		} else if (*iter <= 0xEF) {
			auto c1 = *iter++;
			auto c2 = *iter++;
			auto c3 = *iter++;
			group bytes = UnicodeToUtf16( Utf8ToUnicode( c1, c2, c3 ) );
			for (auto i = 0u; i < bytes.sz; ++i) {
				*outBegin++ = bytes.res[i];
			}
		} else {
			assert( *iter <= 0xF7 );
			auto c1 = *iter++;
			auto c2 = *iter++;
			auto c3 = *iter++;
			auto c4 = *iter++;
			group g = UnicodeToUtf16( Utf8ToUnicode( c1, c2, c3, c4 ) );
			for (auto i = 0u; i < g.sz; ++i) {
				*outBegin++ = g.res[i];
			}
		}
	}

	return outBegin;
}

template <typename InIter, typename OutIter>
OutIter Utf16ToUtf8( InIter inBegin, InIter inEnd, OutIter outBegin )
{
	//static_assert(sizeof *inBegin == 2 && sizeof *outBegin == 1);
	//uint16_t utf16BOM = *inBegin;
	auto iter = inBegin;
	while (iter != inEnd) {
		if (*iter >= 0xD800 && *iter <= 0xDBFF) {
			// high surrogate
			auto c1 = *iter++;
			auto c2 = *iter++; assert( c2 >= 0xDC00 && c2 <= 0xDFFF );
			group g = UnicodeToUtf8( Utf16ToUnicode( c1, c2 ) );
			for (auto i = 0; i < g.sz; ++i) {
				*outBegin++ = g.res[i];
			}
		} else {
			group g = UnicodeToUtf8( Utf16ToUnicode( *iter++ ) );
			for (auto i = 0; i < g.sz; ++i) {
				*outBegin++ = g.res[i];
			}
		}
	}

	return outBegin;
}

template <typename InIter, typename OutIter>
OutIter Utf8ToUtf32( InIter inBegin, InIter inEnd, OutIter outBegin )
{
	//assert( sizeof *inBegin == 1 );
	auto iter = inBegin;
	while (iter != inEnd) {
		if (*iter <= 0x7F) {
			*outBegin++ = Utf8ToUnicode( *iter++ );
		} else if (*iter <= 0xDF) {
			auto c1 = *iter++;
			auto c2 = *iter++;
			*outBegin++ = Utf8ToUnicode( c1, c2 );
		} else if (*iter <= 0xEF) {
			auto c1 = *iter++;
			auto c2 = *iter++;
			auto c3 = *iter++;
			*outBegin++ = Utf8ToUnicode( c1, c2, c3 );
		} else {
			assert( *iter <= 0xF7 );
			auto c1 = *iter++;
			auto c2 = *iter++;
			auto c3 = *iter++;
			auto c4 = *iter++;
			*outBegin++ = Utf8ToUnicode( c1, c2, c3, c4 );
		}
	}
	return outBegin;
}
template<typename InIter, typename OutIter>
OutIter Utf32ToUtf8( InIter inBegin, InIter inEnd, OutIter outBegin )
{
	//static_assert(sizeof *inBegin == 4 && sizeof *outBegin == 1);
	auto iter = inBegin;
	while (iter != inEnd) {
		group g = UnicodeToUtf8( *iter++ );
		for (auto i = 0u; i < g.sz; ++i) {
			*outBegin++ = g.res[i];
		}
	}
	return outBegin;
}

template <typename InIter, typename OutIter>
OutIter Utf16ToUtf32( InIter inBegin, InIter inEnd, OutIter outBegin )
{
	//static_assert(sizeof *inBegin == 2 && sizeof *outBegin == 4);
	auto iter = inBegin;
	while (iter != inEnd) {
		if (*iter >= 0xD800 && *iter <= 0xDBFF) {
			auto c1 = *iter++;
			auto c2 = *iter++; assert( c2 >= 0xDC00 && c2 <= 0xDFFF );
			*outBegin++ = Utf16ToUnicode( c1, c2 );
		} else {
			*outBegin++ = Utf16ToUnicode( *iter++ );
		}
	}
	return outBegin;
}

template <typename InIter, typename OutIter>
OutIter Utf32ToUtf16( InIter inBegin, InIter inEnd, OutIter outBegin )
{
	//static_assert(sizeof *inBegin == 4 && sizeof *outBegin == 2);
	auto iter = inBegin;
	while (iter != inEnd) {
		group g = UnicodeToUtf16( *iter++ );
		for (auto i = 0; i < g.sz; ++i) {
			*outBegin++ = g.res[i];
		}
	}
	return outBegin;
}

} // namespace shi



#endif
