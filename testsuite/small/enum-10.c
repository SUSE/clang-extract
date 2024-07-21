/* { dg-options "-DCE_EXTRACT_FUNCTIONS=f -DCE_NO_EXTERNALIZATION" }*/
/* { dg-xfail } */

enum {
  NB_POSNR = 0,
  NB_LENGTH,
  NB_RESPOS,
  NB_TRKLEN,
  NB_SSNR,
  NB_CURSS,
  NB_SSPOS,
  NB_CHANS,
  NB_MIXG,
  NB_SPEEDMULT,
  NB_DRUMPADMODE,
  NB_END
};
enum {
  INB_INS = 0,
  INB_VOL,
  INB_WAVELEN,
  INB_ATTACK,
  INB_AVOL,
  INB_DECAY,
  INB_DVOL,
  INB_SUSTAIN,
  INB_RELEASE,
  INB_RVOL,
  INB_VIBDELAY,
  INB_VIBDEPTH,
  INB_VIBSPEED,
  INB_SQRLOWER,
  INB_SQRUPPER,
  INB_SQRSPEED,
  INB_FLTLOWER,
  INB_FLTUPPER,
  INB_FLTSPEED,
  INB_PERFSPEED,
  INB_PERFLEN,
  INB_HARDCUT,
  INB_RELCUT,
  INB_END
};

int a[NB_END], b[INB_END];
void f() { int d = b[2]; }

/* { dg-final { scan-tree-dump "b[INB_END];" } } */
/* { dg-final { scan-tree-dump "NB_END" } } */
/* { dg-final { scan-tree-dump "INB_END" } } */
