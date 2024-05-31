/* { dg-options "-DCE_EXTRACT_FUNCTIONS=f -DCE_RENAME_SYMBOLS -DCE_EXPORT_SYMBOLS=symbol" }*/

struct AA {
  int aa;
};

extern struct AA symbol;

int f(void)
{
  typeof(symbol) ls = (typeof(symbol)) { 0 };
  return ls.aa;
}

/* { dg-final { scan-tree-dump "typeof\(\(\*klpe_symbol\)\) ls = \(typeof\(\(\*klpe_symbol\)\)\) " } } */
/* { dg-final { scan-tree-dump-not "typeof\(symbol\)" } } */
