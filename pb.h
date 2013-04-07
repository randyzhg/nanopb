#ifndef _PB_H_
#define _PB_H_

/* pb.h: Common parts for nanopb library.
 * Most of these are quite low-level stuff. For the high-level interface,
 * see pb_encode.h or pb_decode.h
 */

#define NANOPB_VERSION nanopb-0.2.1-dev

#ifdef PB_SYSTEM_HEADER
#include PB_SYSTEM_HEADER
#else
#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include <string.h>
#endif

/* Macro for defining packed structures (compiler dependent).
 * This just reduces memory requirements, but is not required.
 */
#if defined(__GNUC__) || defined(__clang__)
    /* For GCC and clang */
#   define PB_PACKED_STRUCT_START
#   define PB_PACKED_STRUCT_END
#   define pb_packed __attribute__((packed))
#elif defined(__ICCARM__)
    /* For IAR ARM compiler */
#   define PB_PACKED_STRUCT_START _Pragma("pack(push, 1)")
#   define PB_PACKED_STRUCT_END _Pragma("pack(pop)")
#   define pb_packed
#elif defined(_MSC_VER) && (_MSC_VER >= 1500)
    /* For Microsoft Visual C++ */
#   define PB_PACKED_STRUCT_START __pragma(pack(push, 1))
#   define PB_PACKED_STRUCT_END __pragma(pack(pop))
#   define pb_packed
#else
    /* Unknown compiler */
#   define PB_PACKED_STRUCT_START
#   define PB_PACKED_STRUCT_END
#   define pb_packed
#endif

/* Handly macro for suppressing unreferenced-parameter compiler warnings.    */
#ifndef UNUSED
#define UNUSED(x) (void)(x)
#endif

/* Compile-time assertion, used for checking compatible compilation options.
 * If this fails on your compiler for some reason, use #define STATIC_ASSERT
 * to disable it. */
#ifndef STATIC_ASSERT
#define STATIC_ASSERT(COND,MSG) typedef char STATIC_ASSERT_MSG(MSG, __LINE__, __COUNTER__)[(COND)?1:-1];
#define STATIC_ASSERT_MSG(MSG, LINE, COUNTER) STATIC_ASSERT_MSG_(MSG, LINE, COUNTER)
#define STATIC_ASSERT_MSG_(MSG, LINE, COUNTER) static_assertion_##MSG##LINE##COUNTER
#endif

/* Number of required fields to keep track of
 * (change here or on compiler command line). */
#ifndef PB_MAX_REQUIRED_FIELDS
#define PB_MAX_REQUIRED_FIELDS 64
#endif

#if PB_MAX_REQUIRED_FIELDS < 64
#error You should not lower PB_MAX_REQUIRED_FIELDS from the default value (64).
#endif

/* List of possible field types. These are used in the autogenerated code.
 * Least-significant 4 bits tell the scalar type
 * Most-significant 4 bits specify repeated/required/packed etc.
 * 
 * INT32 and UINT32 are treated the same, as are (U)INT64 and (S)FIXED*
 * These types are simply casted to correct field type when they are
 * assigned to the memory pointer.
 * SINT* is different, though, because it is zig-zag coded.
 */

typedef uint8_t pb_type_t;

/************************
 * Field contents types *
 ************************/

/* Numeric types */
#define PB_LTYPE_VARINT 0x00 /* int32, uint32, int64, uint64, bool, enum */
#define PB_LTYPE_SVARINT 0x01 /* sint32, sint64 */
#define PB_LTYPE_FIXED32 0x02 /* fixed32, sfixed32, float */
#define PB_LTYPE_FIXED64 0x03 /* fixed64, sfixed64, double */

/* Marker for last packable field type. */
#define PB_LTYPE_LAST_PACKABLE 0x03

/* Byte array with pre-allocated buffer.
 * data_size is the length of the allocated PB_BYTES_ARRAY structure. */
#define PB_LTYPE_BYTES 0x04

/* String with pre-allocated buffer.
 * data_size is the maximum length. */
#define PB_LTYPE_STRING 0x05

/* Submessage
 * submsg_fields is pointer to field descriptions */
#define PB_LTYPE_SUBMESSAGE 0x06

/* Number of declared LTYPES */
#define PB_LTYPES_COUNT 7
#define PB_LTYPE_MASK 0x0F

/**************************
 * Field repetition rules *
 **************************/

#define PB_HTYPE_REQUIRED 0x00
#define PB_HTYPE_OPTIONAL 0x10
#define PB_HTYPE_REPEATED 0x20
#define PB_HTYPE_MASK     0x30

/********************
 * Allocation types *
 ********************/
 
#define PB_ATYPE_STATIC   0x00
#define PB_ATYPE_CALLBACK 0x40
#define PB_ATYPE_MASK     0xC0

#define PB_ATYPE(x) ((x) & PB_ATYPE_MASK)
#define PB_HTYPE(x) ((x) & PB_HTYPE_MASK)
#define PB_LTYPE(x) ((x) & PB_LTYPE_MASK)

/* This structure is used in auto-generated constants
 * to specify struct fields.
 * You can change field sizes if you need structures
 * larger than 256 bytes or field tags larger than 256.
 * The compiler should complain if your .proto has such
 * structures. Fix that by defining PB_FIELD_16BIT or
 * PB_FIELD_32BIT.
 */
PB_PACKED_STRUCT_START
typedef struct _pb_field_t pb_field_t;
struct _pb_field_t {

#if !defined(PB_FIELD_16BIT) && !defined(PB_FIELD_32BIT)
    uint8_t tag;
    pb_type_t type;
    uint8_t data_offset; /* Offset of field data, relative to previous field. */
    int8_t size_offset; /* Offset of array size or has-boolean, relative to data */
    uint8_t data_size; /* Data size in bytes for a single item */
    uint8_t array_size; /* Maximum number of entries in array */
#elif defined(PB_FIELD_16BIT) && !defined(PB_FIELD_32BIT)
    uint16_t tag;
    pb_type_t type;
    uint8_t data_offset;
    int8_t size_offset;
    uint16_t data_size;
    uint16_t array_size;
#else
    uint32_t tag;
    pb_type_t type;
    uint8_t data_offset;
    int8_t size_offset;
    uint32_t data_size;
    uint32_t array_size;
#endif
    
    /* Field definitions for submessage
     * OR default value for all other non-array, non-callback types
     * If null, then field will zeroed. */
    const void *ptr;
} pb_packed;
PB_PACKED_STRUCT_END

/* This structure is used for 'bytes' arrays.
 * It has the number of bytes in the beginning, and after that an array.
 * Note that actual structs used will have a different length of bytes array.
 */
struct _pb_bytes_array_t {
    size_t size;
    uint8_t bytes[1];
};

typedef struct _pb_bytes_array_t pb_bytes_array_t;

/* This structure is used for giving the callback function.
 * It is stored in the message structure and filled in by the method that
 * calls pb_decode.
 *
 * The decoding callback will be given a limited-length stream
 * If the wire type was string, the length is the length of the string.
 * If the wire type was a varint/fixed32/fixed64, the length is the length
 * of the actual value.
 * The function may be called multiple times (especially for repeated types,
 * but also otherwise if the message happens to contain the field multiple
 * times.)
 *
 * The encoding callback will receive the actual output stream.
 * It should write all the data in one call, including the field tag and
 * wire type. It can write multiple fields.
 *
 * The callback can be null if you want to skip a field.
 */
typedef struct _pb_istream_t pb_istream_t;
typedef struct _pb_ostream_t pb_ostream_t;
typedef struct _pb_callback_t pb_callback_t;
struct _pb_callback_t {
#ifdef PB_OLD_CALLBACK_STYLE
    /* Deprecated since nanopb-0.2.1 */
    union {
        bool (*decode)(pb_istream_t *stream, const pb_field_t *field, void *arg);
        bool (*encode)(pb_ostream_t *stream, const pb_field_t *field, const void *arg);
    } funcs;
#else
    /* New function signature, which allows modifying arg contents in callback. */
    union {
        bool (*decode)(pb_istream_t *stream, const pb_field_t *field, void **arg);
        bool (*encode)(pb_ostream_t *stream, const pb_field_t *field, void * const *arg);
    } funcs;
#endif    
    
    /* Free arg for use by callback */
    void *arg;
};

/* Wire types. Library user needs these only in encoder callbacks. */
typedef enum {
    PB_WT_VARINT = 0,
    PB_WT_64BIT  = 1,
    PB_WT_STRING = 2,
    PB_WT_32BIT  = 5
} pb_wire_type_t;

/* These macros are used to declare pb_field_t's in the constant array. */
#define pb_membersize(st, m) (sizeof ((st*)0)->m)
#define pb_arraysize(st, m) (pb_membersize(st, m) / pb_membersize(st, m[0]))
#define pb_delta(st, m1, m2) ((int)offsetof(st, m1) - (int)offsetof(st, m2))
#define pb_delta_end(st, m1, m2) (int)(offsetof(st, m1) == offsetof(st, m2) \
                                  ? offsetof(st, m1) \
                                  : offsetof(st, m1) - offsetof(st, m2) - pb_membersize(st, m2))
#define PB_LAST_FIELD {0,(pb_type_t) 0,0,0,0,0,0}

/* Required fields are the simplest. They just have delta (padding) from
 * previous field end, and the size of the field. Pointer is used for
 * submessages and default values.
 */
#define PB_REQUIRED_STATIC(tag, st, m, pm, ltype, ptr) \
    {tag, PB_ATYPE_STATIC | PB_HTYPE_REQUIRED | ltype, \
    pb_delta_end(st, m, pm), 0, pb_membersize(st, m), 0, ptr}

/* Optional fields add the delta to the has_ variable. */
#define PB_OPTIONAL_STATIC(tag, st, m, pm, ltype, ptr) \
    {tag, PB_ATYPE_STATIC | PB_HTYPE_OPTIONAL | ltype, \
    pb_delta_end(st, m, pm), \
    pb_delta(st, has_ ## m, m), \
    pb_membersize(st, m), 0, ptr}

/* Repeated fields have a _count field and also the maximum number of entries. */
#define PB_REPEATED_STATIC(tag, st, m, pm, ltype, ptr) \
    {tag, PB_ATYPE_STATIC | PB_HTYPE_REPEATED | ltype, \
    pb_delta_end(st, m, pm), \
    pb_delta(st, m ## _count, m), \
    pb_membersize(st, m[0]), \
    pb_arraysize(st, m), ptr}

/* Callbacks are much like required fields except with special datatype. */
#define PB_REQUIRED_CALLBACK(tag, st, m, pm, ltype, ptr) \
    {tag, PB_ATYPE_CALLBACK | PB_HTYPE_REQUIRED | ltype, \
    pb_delta_end(st, m, pm), 0, pb_membersize(st, m), 0, ptr}

#define PB_OPTIONAL_CALLBACK(tag, st, m, pm, ltype, ptr) \
    {tag, PB_ATYPE_CALLBACK | PB_HTYPE_OPTIONAL | ltype, \
    pb_delta_end(st, m, pm), 0, pb_membersize(st, m), 0, ptr}
    
#define PB_REPEATED_CALLBACK(tag, st, m, pm, ltype, ptr) \
    {tag, PB_ATYPE_CALLBACK | PB_HTYPE_REPEATED | ltype, \
    pb_delta_end(st, m, pm), 0, pb_membersize(st, m), 0, ptr}

/* The mapping from protobuf types to LTYPEs is done using these macros. */
#define PB_LTYPE_MAP_BOOL       PB_LTYPE_VARINT
#define PB_LTYPE_MAP_BYTES      PB_LTYPE_BYTES
#define PB_LTYPE_MAP_DOUBLE     PB_LTYPE_FIXED64
#define PB_LTYPE_MAP_ENUM       PB_LTYPE_VARINT
#define PB_LTYPE_MAP_FIXED32    PB_LTYPE_FIXED32
#define PB_LTYPE_MAP_FIXED64    PB_LTYPE_FIXED64
#define PB_LTYPE_MAP_FLOAT      PB_LTYPE_FIXED32
#define PB_LTYPE_MAP_INT32      PB_LTYPE_VARINT
#define PB_LTYPE_MAP_INT64      PB_LTYPE_VARINT
#define PB_LTYPE_MAP_MESSAGE    PB_LTYPE_SUBMESSAGE
#define PB_LTYPE_MAP_SFIXED32   PB_LTYPE_FIXED32
#define PB_LTYPE_MAP_SFIXED64   PB_LTYPE_FIXED64
#define PB_LTYPE_MAP_SINT32     PB_LTYPE_SVARINT
#define PB_LTYPE_MAP_SINT64     PB_LTYPE_SVARINT
#define PB_LTYPE_MAP_STRING     PB_LTYPE_STRING
#define PB_LTYPE_MAP_UINT32     PB_LTYPE_VARINT
#define PB_LTYPE_MAP_UINT64     PB_LTYPE_VARINT

/* This is the actual macro used in field descriptions.
 * It takes these arguments:
 * - Field tag number
 * - Field type:   BOOL, BYTES, DOUBLE, ENUM, FIXED32, FIXED64,
 *                 FLOAT, INT32, INT64, MESSAGE, SFIXED32, SFIXED64
 *                 SINT32, SINT64, STRING, UINT32 or UINT64
 * - Field rules:  REQUIRED, OPTIONAL or REPEATED
 * - Allocation:   STATIC or CALLBACK
 * - Message name
 * - Field name
 * - Previous field name (or field name again for first field)
 * - Pointer to default value or submsg fields.
 */

#define PB_FIELD(tag, type, rules, allocation, message, field, prevfield, ptr) \
    PB_ ## rules ## _ ## allocation(tag, message, field, prevfield, \
                                    PB_LTYPE_MAP_ ## type, ptr)

/* These macros are used for giving out error messages.
 * They are mostly a debugging aid; the main error information
 * is the true/false return value from functions.
 * Some code space can be saved by disabling the error
 * messages if not used.
 */
#ifdef PB_NO_ERRMSG
#define PB_RETURN_ERROR(stream,msg) return false
#define PB_GET_ERROR(stream) "(errmsg disabled)"
#else
#define PB_RETURN_ERROR(stream,msg) \
    do {\
        if ((stream)->errmsg == NULL) \
            (stream)->errmsg = (msg); \
        return false; \
    } while(0)
#define PB_GET_ERROR(stream) ((stream)->errmsg ? (stream)->errmsg : "(none)")
#endif

#endif
