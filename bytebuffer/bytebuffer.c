#include "bytebuffer.h"

#ifdef OS_WINDOWS
#	define HARBOL_LIB
#endif


HARBOL_EXPORT struct HarbolByteBuf *harbol_bytebuffer_new(void)
{
	struct HarbolByteBuf *buf = harbol_alloc(1, sizeof *buf);
	if( buf != NULL )
		*buf = harbol_bytebuffer_create();
	return buf;
}

HARBOL_EXPORT struct HarbolByteBuf harbol_bytebuffer_create(void)
{
	struct HarbolByteBuf buf = EMPTY_HARBOL_BYTEBUF;
	return buf;
}

HARBOL_EXPORT bool harbol_bytebuffer_clear(struct HarbolByteBuf *const buf)
{
	if( buf->table==NULL )
		return false;
	else {
		harbol_free(buf->table);
		*buf = (struct HarbolByteBuf)EMPTY_HARBOL_BYTEBUF;
		return true;
	}
}

HARBOL_EXPORT bool harbol_bytebuffer_free(struct HarbolByteBuf **const bufref)
{
	if( *bufref==NULL )
		return false;
	else {
		const bool res = harbol_bytebuffer_clear(*bufref);
		harbol_free(*bufref), *bufref=NULL;
		return res;
	}
}

HARBOL_EXPORT size_t harbol_bytebuffer_len(const struct HarbolByteBuf *const buf)
{
	return buf->len;
}

HARBOL_EXPORT size_t harbol_bytebuffer_count(const struct HarbolByteBuf *const buf)
{
	return buf->count;
}

HARBOL_EXPORT uint8_t *harbol_bytebuffer_get_buffer(const struct HarbolByteBuf *const buf)
{
	return buf->table;
}


#ifndef HARBOL_BYTEBUFFER_INSERTION
#	define HARBOL_BYTEBUFFER_INSERTION \
	if( buf->count + sizeof val >= buf->len ) \
		harbol_generic_vector_resizer(buf, buf->count + sizeof val, sizeof *buf->table); \
	memcpy(&buf->table[buf->count], &val, sizeof val); \
	buf->count += sizeof val; \
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


HARBOL_EXPORT bool harbol_bytebuffer_insert_float32(struct HarbolByteBuf *const buf, const float32_t val)
{
	HARBOL_BYTEBUFFER_INSERTION
}

HARBOL_EXPORT bool harbol_bytebuffer_insert_float64(struct HarbolByteBuf *const buf, const float64_t val)
{
	HARBOL_BYTEBUFFER_INSERTION
}

HARBOL_EXPORT bool harbol_bytebuffer_insert_floatmax(struct HarbolByteBuf *const buf, const floatmax_t val)
{
	HARBOL_BYTEBUFFER_INSERTION
}

HARBOL_EXPORT bool harbol_bytebuffer_insert_cstr(struct HarbolByteBuf *const restrict buf, const char cstr[restrict])
{
	if( cstr==NULL )
		return false;
	else {
		const size_t cstr_len = strlen(cstr);
		if( buf->count + cstr_len + 1 >= buf->len )
			harbol_generic_vector_resizer(buf, buf->count + cstr_len + 1, sizeof *buf->table);
		strncpy((char *)&buf->table[buf->count], cstr, cstr_len);
		buf->count += cstr_len;
		buf->table[buf->count++] = '\0';
		return true;
	}
}

HARBOL_EXPORT bool harbol_bytebuffer_insert_obj(struct HarbolByteBuf *const restrict buf, const void *const obj, const size_t len)
{
	if( buf->count + len >= buf->len )
		harbol_generic_vector_resizer(buf, buf->count + len, sizeof *buf->table);
	memcpy(&buf->table[buf->count], obj, len);
	buf->count += len;
	return true;
}

HARBOL_EXPORT bool harbol_bytebuffer_insert_zeros(struct HarbolByteBuf *const buf, const size_t amount)
{
	if( buf->count + amount >= buf->len )
		harbol_generic_vector_resizer(buf, buf->count + amount, sizeof *buf->table);
	
	memset(&buf->table[buf->count], 0, amount);
	buf->count += amount;
	return true;
}

HARBOL_EXPORT bool harbol_bytebuffer_del(struct HarbolByteBuf *const buf, const uindex_t index, const size_t range)
{
	// if the entire range is the entire buffer, just clear everything.
	if( index==0 && index + range >= buf->count )
		return harbol_bytebuffer_clear(buf);
	else {
		const size_t total_range = range==0 ? 1 : range;
		const size_t
			i = index + total_range,
			j = index
		;
		if( i < buf->count ) {
			buf->count -= total_range;
			memmove(&buf->table[j], &buf->table[i], buf->count-j);
			return true;
		} else {
			// if i goes out of range, zero everything after and lower the count.
			memset(&buf->table[j], 0, buf->count-j);
			buf->count -= buf->count-j;
			return true;
		}
	}
}

HARBOL_EXPORT bool harbol_bytebuffer_to_file(const struct HarbolByteBuf *const buf, FILE *const file)
{
	if( buf->table==NULL )
		return false;
	else {
		const size_t bytes_written = fwrite(buf->table, sizeof *buf->table, buf->count, file);
		return bytes_written==buf->count;
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
		if( buf->count + file_size >= buf->len )
			harbol_generic_vector_resizer(buf, buf->count + file_size, sizeof *buf->table);
		
		const size_t bytes_read = fread(&buf->table[buf->count], sizeof *buf->table, file_size, file);
		buf->count += bytes_read;
		return bytes_read==(size_t)file_size;
	}
}

HARBOL_EXPORT bool harbol_bytebuffer_append(struct HarbolByteBuf *const bufA, const struct HarbolByteBuf *const bufB)
{
	if( bufB->table==NULL )
		return false;
	else {
		if( bufA->count + bufB->count >= bufA->len )
			harbol_generic_vector_resizer(bufA, bufA->count + bufB->count, sizeof *bufA->table);
		
		memcpy(&bufA->table[bufA->count], bufB->table, bufB->count);
		bufA->count += bufB->count;
		return true;
	}
}

HARBOL_EXPORT bool harbol_bytebuffer_copy(struct HarbolByteBuf *const bufA, const struct HarbolByteBuf *const bufB)
{
	if( bufB->table==NULL )
		return false;
	else {
		if( bufB->count != bufA->count )
			harbol_generic_vector_resizer(bufA, bufB->count, sizeof *bufA->table);
		
		memcpy(&bufA->table[0], &bufB->table[0], bufB->count);
		bufA->count = bufB->count;
		return true;
	}
}
