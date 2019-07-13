#include "bytebuffer.h"

#ifdef OS_WINDOWS
#	define HARBOL_LIB
#endif


HARBOL_EXPORT struct HarbolByteBuf *harbol_bytebuffer_new(void)
{
	struct HarbolByteBuf *buf = calloc(1, sizeof *buf);
	if( buf != NULL )
		*buf = harbol_bytebuffer_create();
	return buf;
}

HARBOL_EXPORT struct HarbolByteBuf harbol_bytebuffer_create(void)
{
	struct HarbolByteBuf buf = {NULL, 0, 0};
	return buf;
}

HARBOL_EXPORT bool harbol_bytebuffer_clear(struct HarbolByteBuf *const buf)
{
	if( buf->Table==NULL )
		return false;
	else {
		free(buf->Table), buf->Table = NULL;
		buf->Len = buf->Count = 0;
		return true;
	}
}

HARBOL_EXPORT bool harbol_bytebuffer_free(struct HarbolByteBuf **const bufref)
{
	if( *bufref==NULL )
		return false;
	else {
		const bool res = harbol_bytebuffer_clear(*bufref);
		free(*bufref), *bufref=NULL;
		return res;
	}
}

HARBOL_EXPORT size_t harbol_bytebuffer_len(const struct HarbolByteBuf *const buf)
{
	return buf->Len;
}

HARBOL_EXPORT size_t harbol_bytebuffer_count(const struct HarbolByteBuf *const buf)
{
	return buf->Count;
}

HARBOL_EXPORT uint8_t *harbol_bytebuffer_get_buffer(const struct HarbolByteBuf *const buf)
{
	return buf->Table;
}


#ifndef HARBOL_BYTEBUFFER_INSERTION
#	define HARBOL_BYTEBUFFER_INSERTION \
	if( buf->Count + sizeof val >= buf->Len ) \
		harbol_generic_vector_resizer(buf, buf->Count + sizeof val, sizeof *buf->Table); \
	memcpy(&buf->Table[buf->Count], &val, sizeof val); \
	buf->Count += sizeof val; \
	return true;
#endif

HARBOL_EXPORT bool harbol_bytebuffer_insert_byte(struct HarbolByteBuf *const buf, const uint8_t val)
{
	HARBOL_BYTEBUFFER_INSERTION
}

HARBOL_EXPORT bool harbol_bytebuffer_insert_int16(struct HarbolByteBuf *const buf, const uint16_t val)
{
	HARBOL_BYTEBUFFER_INSERTION
}

HARBOL_EXPORT bool harbol_bytebuffer_insert_int32(struct HarbolByteBuf *const buf, const uint32_t val)
{
	HARBOL_BYTEBUFFER_INSERTION
}

HARBOL_EXPORT bool harbol_bytebuffer_insert_int64(struct HarbolByteBuf *const buf, const uint64_t val)
{
	HARBOL_BYTEBUFFER_INSERTION
}

HARBOL_EXPORT bool harbol_bytebuffer_insert_ptr(struct HarbolByteBuf *const buf, const uintptr_t val)
{
	HARBOL_BYTEBUFFER_INSERTION
}


HARBOL_EXPORT bool harbol_bytebuffer_insert_float32(struct HarbolByteBuf *const buf, const float val)
{
	HARBOL_BYTEBUFFER_INSERTION
}

HARBOL_EXPORT bool harbol_bytebuffer_insert_float64(struct HarbolByteBuf *const buf, const double val)
{
	HARBOL_BYTEBUFFER_INSERTION
}

HARBOL_EXPORT bool harbol_bytebuffer_insert_floatlong(struct HarbolByteBuf *const buf, const long double val)
{
	HARBOL_BYTEBUFFER_INSERTION
}

HARBOL_EXPORT bool harbol_bytebuffer_insert_cstr(struct HarbolByteBuf *const restrict buf, const char cstr[restrict])
{
	if( cstr==NULL )
		return false;
	else {
		const size_t cstr_len = strlen(cstr);
		if( buf->Count + cstr_len + 1 >= buf->Len )
			harbol_generic_vector_resizer(buf, buf->Count + cstr_len + 1, sizeof *buf->Table);
		strncpy((char *)&buf->Table[buf->Count], cstr, cstr_len);
		buf->Count += cstr_len;
		buf->Table[buf->Count++] = '\0';
		return true;
	}
}

HARBOL_EXPORT bool harbol_bytebuffer_insert_obj(struct HarbolByteBuf *const restrict buf, const void *const obj, const size_t len)
{
	if( buf->Count + len >= buf->Len )
		harbol_generic_vector_resizer(buf, buf->Count + len, sizeof *buf->Table);
	memcpy(&buf->Table[buf->Count], obj, len);
	buf->Count += len;
	return true;
}

HARBOL_EXPORT bool harbol_bytebuffer_insert_zeros(struct HarbolByteBuf *const buf, const size_t amount)
{
	if( buf->Count + amount >= buf->Len )
		harbol_generic_vector_resizer(buf, buf->Count + amount, sizeof *buf->Table);
	
	memset(&buf->Table[buf->Count], 0, amount);
	buf->Count += amount;
	return true;
}

HARBOL_EXPORT bool harbol_bytebuffer_del(struct HarbolByteBuf *const buf, const uindex_t index, const size_t range)
{
	// if the entire range is the entire buffer, just clear everything.
	if( index==0 && index + range >= buf->Count )
		return harbol_bytebuffer_clear(buf);
	else {
		const size_t total_range = range==0 ? 1 : range;
		const size_t
			i = index + total_range,
			j = index
		;
		if( i < buf->Count ) {
			buf->Count -= total_range;
			memmove(&buf->Table[j], &buf->Table[i], buf->Count-j);
			return true;
		} else {
			// if i goes out of range, zero everything after and lower the count.
			memset(&buf->Table[j], 0, buf->Count-j);
			buf->Count -= buf->Count-j;
			return true;
		}
	}
}

HARBOL_EXPORT bool harbol_bytebuffer_to_file(const struct HarbolByteBuf *const buf, FILE *const file)
{
	if( buf->Table==NULL )
		return false;
	else {
		const size_t bytes_written = fwrite(buf->Table, sizeof *buf->Table, buf->Count, file);
		return bytes_written==buf->Count;
	}
}

HARBOL_EXPORT bool harbol_bytebuffer_insert_from_file(struct HarbolByteBuf *const buf, FILE *const file)
{
	fseek(file, 0, SEEK_END);
	const long int file_size = ftell(file);
	if( file_size <= 0 )
		return false;
	else {
		rewind(file);
		if( buf->Count + file_size >= buf->Len )
			harbol_generic_vector_resizer(buf, buf->Count + file_size, sizeof *buf->Table);
		
		const size_t bytes_read = fread(&buf->Table[buf->Count], sizeof *buf->Table, file_size, file);
		buf->Count += bytes_read;
		return bytes_read==(size_t)file_size;
	}
}

HARBOL_EXPORT bool harbol_bytebuffer_append(struct HarbolByteBuf *const bufA, struct HarbolByteBuf *const bufB)
{
	if( bufB->Table==NULL )
		return false;
	else {
		if( bufA->Count + bufB->Count >= bufA->Len )
			harbol_generic_vector_resizer(bufA, bufA->Count + bufB->Count, sizeof *bufA->Table);
		
		memcpy(&bufA->Table[bufA->Count], bufB->Table, bufB->Count);
		bufA->Count += bufB->Count;
		return true;
	}
}

HARBOL_EXPORT bool harbol_bytebuffer_copy(struct HarbolByteBuf *const bufA, struct HarbolByteBuf *const bufB)
{
	if( bufB->Table==NULL )
		return false;
	else {
		if( bufB->Count != bufA->Count )
			harbol_generic_vector_resizer(bufA, bufB->Count, sizeof *bufA->Table);
		
		memcpy(&bufA->Table[0], &bufA->Table[0], bufB->Count);
		bufA->Count = bufB->Count;
		return true;
	}
}
