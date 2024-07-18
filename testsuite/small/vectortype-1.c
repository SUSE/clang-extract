/* { dg-options "-Wno-everything -DCE_EXTRACT_FUNCTIONS=f -DCE_NO_EXTERNALIZATION" }*/

typedef float __v8sf __attribute__ ((__vector_size__ (32)));

typedef unsigned int __v8su __attribute__ ((__vector_size__ (32)));

typedef float __m256 __attribute__ ((__vector_size__ (32), __aligned__(32)));

typedef long long __m256i __attribute__((__vector_size__(32), __aligned__(32)));

__m256 __attribute__((__nodebug__, __min_vector_width__(256)))
f (__m256i __A) {
  return (__m256)__builtin_convertvector((__v8su)__A, __v8sf);
}

/* { dg-final { scan-tree-dump "typedef float __v8sf __attribute__ \(\(__vector_size__ \(32\)\)\);" } } */
