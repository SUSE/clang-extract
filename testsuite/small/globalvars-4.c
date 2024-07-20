/* { dg-options "-DCE_EXTRACT_FUNCTIONS=f -DCE_NO_EXTERNALIZATION" }*/

extern double pow (double __x, double __y) __attribute__ ((__nothrow__ ));
extern double __pow (double __x, double __y) __attribute__ ((__nothrow__ ));

static float stbi__h2l_gamma_i=1.0f/2.2f, stbi__h2l_scale_i=1.0f;

void f() {
    float z = (float) pow(stbi__h2l_scale_i, stbi__h2l_gamma_i) * 255 + 0.5f;
}

/* { dg-final { scan-tree-dump "static float stbi__h2l_gamma_i=1.0f/2.2f, stbi__h2l_scale_i=1.0f;" } } */
/* { dg-final { scan-tree-dump-not "static float stbi__h2l_gamma_i=1.0f/2.2f;" } } */
