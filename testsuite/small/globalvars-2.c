/* { dg-options "-DCE_EXTRACT_FUNCTIONS=f -DCE_NO_EXTERNALIZATION" }*/

typedef struct pglist_data {
 int nr_zones;
} pg_data_t;
extern pg_data_t *node_data[];

extern struct pglist_data *node_data[];

void f() {
    struct pglist_data *a;
    node_data[1] = a;
}

/* { dg-final { scan-tree-dump "struct pglist_data;" } } */
/* { dg-final { scan-tree-dump "extern struct pglist_data \*node_data\[\];" } } */
