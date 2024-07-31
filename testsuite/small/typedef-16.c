/* { dg-options "-DCE_EXTRACT_FUNCTIONS=f -DCE_NO_EXTERNALIZATION" }*/

typedef long unsigned int size_t;

typedef struct _MagickWand
  MagickWand;

typedef enum
{
  MagickFalse = 0,
  MagickTrue = 1
} MagickBooleanType;

typedef enum
{
  UndefinedChannel = 0x0000,
  RedChannel = 0x0001,
  GrayChannel = 0x0001,
} ChannelType;

extern __attribute__ ((visibility ("default"))) MagickBooleanType
  MagickAdaptiveBlurImage(MagickWand *,const double,const double),
  MagickAdaptiveResizeImage(MagickWand *,const size_t,const size_t),
  MagickAdaptiveSharpenImage(MagickWand *,const double,const double);

MagickBooleanType f(MagickWand *wand, const ChannelType channel, const double radius, const double sigma) {
 MagickBooleanType status;
 status = MagickAdaptiveSharpenImage(wand, radius, sigma);
 return status;
}

/* { dg-final { scan-tree-dump "typedef long unsigned int size_t;" } } */
/* { dg-final { scan-tree-dump "MagickAdaptiveSharpenImage\(MagickWand *\*,const double, *const double\);" } } */
