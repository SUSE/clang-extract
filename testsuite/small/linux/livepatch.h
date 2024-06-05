/**
 * KLP_RELOC_SYMBOL_POS - define relocation for external symbols
 *
 * @LP_OBJ_NAME: name of the livepatched object where the symbol is needed
 * @SYM_OBJ_NAME: name of the object where the symbol exists
 * @SYM_NAME: symbol name
 * @SYM_POS: position of the symbol in SYM_OBJ when there are more
 *       symbols of the same name.
 *
 * Use for annotating external symbols used in livepatches which are
 * not exported in vmlinux or are in livepatched modules, see
 * Documentation/livepatch/module-elf-format.rst
 */
#define KLP_RELOC_SYMBOL_POS(LP_OBJ_NAME, SYM_OBJ_NAME, SYM_NAME, SYM_POS)	\
	asm("\".klp.sym.rela." #LP_OBJ_NAME "." #SYM_OBJ_NAME "." #SYM_NAME "," #SYM_POS "\"")

#define KLP_RELOC_SYMBOL(LP_OBJ_NAME, SYM_OBJ_NAME, SYM_NAME)	\
	KLP_RELOC_SYMBOL_POS(LP_OBJ_NAME, SYM_OBJ_NAME, SYM_NAME, 0)
