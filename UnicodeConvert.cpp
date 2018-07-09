#include "UnicodeConvert.h"
#include <cassert>

namespace slt
{

uint32_t Utf8ToUnicode( uint8_t code )
{
	return code;
}

uint32_t Utf8ToUnicode( uint8_t code1_, uint8_t code2_ )
{
	uint32_t code1 = code1_, code2 = code2_;
	uint32_t ret = ((code1 & 0x1F) << 6) | (code2 & 0x3F);
	assert( ret >= 0x80 && ret <= 0x7FF );
	return ret;
}

uint32_t Utf8ToUnicode( uint8_t code1_, uint8_t code2_, uint8_t code3_ )
{
	uint32_t code1 = code1_, code2 = code2_, code3 = code3_;
	uint32_t ret = ((code1 & 0xF) << 12) | ((code2 & 0x3F) << 6) | (code3 & 0x3F);
	assert( ret >= 0x800 && ret <= 0xFFFF );
	return ret;
}

uint32_t Utf8ToUnicode( uint8_t code1_, uint8_t code2_, uint8_t code3_, uint8_t code4_ )
{
	uint32_t code1 = code1_, code2 = code2_, code3 = code3_, code4 = code4_;
	uint32_t ret = ((code1 & 0x7) << 18) | ((code2 & 0x3F) << 12) | ((code3 & 0x3F) << 6)
		| (code4 & 0x3F);
	assert( ret >= 0x10000 && ret <= 0x10FFFF );
	return ret;
}

group UnicodeToUtf8( uint32_t code )
{
	assert( code <= 0x10FFFF );
	group ret;
	if (code <= 0x7F) {
		/*buffer.push_back( code );*/
		ret.res[0] = code;
		ret.sz = 1;
	} else if (code <= 0x7FF) {
		/*buffer.push_back( (code >> 6) | 0xC0 );
		buffer.push_back( (code & 0x3F) | 0x80 );*/
		ret.res[0] = (code >> 6) | 0xC0;
		ret.res[1] = (code & 0x3F) | 0x80;
		ret.sz = 2;
	} else if (code <= 0xFFFF) {
		/*buffer.push_back( (code >> 12) | 0xE0 );
		buffer.push_back( ((code >> 6) & 0x3F) | 0x80 );
		buffer.push_back( (code & 0x3F) | 0x80 );*/
		ret.res[0] = (code >> 12) | 0xE0;
		ret.res[1] = ((code >> 6) & 0x3F) | 0x80;
		ret.res[2] = (code & 0x3F) | 0x80;
		ret.sz = 3;
	} else {
		/*buffer.push_back( (code >> 18) | 0xF0 );
		buffer.push_back( ((code >> 12) & 0x3F) | 0x80 );
		buffer.push_back( ((code >> 6) & 0x3F) | 0x80 );
		buffer.push_back( (code & 0x3F) | 0x80 );*/
		ret.res[0] = (code >> 18) | 0xF0;
		ret.res[1] = ((code >> 12) & 0x3F) | 0x80;
		ret.res[2] = ((code >> 6) & 0x3F) | 0x80;
		ret.res[3] = (code & 0x3F) | 0x80;
		ret.sz = 4;
	}

	return ret;
}

uint32_t Utf16ToUnicode( uint16_t code )
{
	assert( code <= 0xFFFF );
	return code;
}

uint32_t Utf16ToUnicode( uint16_t code1_, uint16_t code2_ )
{
	assert( code1_ >= 0xD800 && code1_ <= 0xDBFF );
	assert( code2_ >= 0xDC00 && code2_ <= 0xDFFF );

	uint32_t code1 = code1_, code2 = code2_;
	uint32_t ret{};
	ret = ((code1 & 0x3FF) << 10) + (code2 & 0x3FF) + 0x10000;
	assert( ret >= 0x10000 && ret <= 0x10FFFF );
	return ret;
}

group UnicodeToUtf16( uint32_t unicode )
{
	group ret;
	if (unicode <= 0xFFFF) {
		//buffer.push_back( unicode );
		ret.res[0] = unicode;
		ret.sz = 1;
	} else {
		assert( unicode <= 0x10FFFF );
		unicode -= 0x10000;
		/*buffer.push_back( (unicode >> 10) | 0xD800 );
		buffer.push_back( (unicode & 0x3FF) | 0xDC00 );*/
		ret.res[0] = (unicode >> 10) | 0xD800;
		ret.res[1] = (unicode & 0x3FF) | 0xDC00;
		ret.sz = 2;
	}
	return ret;
}

//template<typename InIter, typename OutIter>
//void shi::Utf8_addBOM( InIter inBegin, InIter inEnd, OutIter outBegin )
//{
//	static_assert( sizeof *inBegin == 1 && sizeof *outBegin == 1 );
//	*outBegin++ = 0xEF;
//	*outBegin++ = 0xBB;
//	*outBegin++ = 0xBF;
//	while (inBegin != inEnd) {
//		*outBegin++ = *inBegin++;
//	}
//}
//
//template <typename InIter, typename OutIter>
//void Utf8_delBOM( InIter inBegin, InIter inEnd, OutIter outBegin )
//{
//	static_assert( sizeof *inBegin == 1 && sizeof *outBegin == 1 );
//	auto c1 = *inBegin++;
//	auto c2 = *inBegin++;
//	auto c3 = *inBegin++;
//	assert( c1 == 0xEF && c2 == 0xBB && c3 == 0xBF );
//	while (inBegin != inEnd) {
//		*outBegin++ = *inBegin++;
//	}
//}




} // namespace shi
