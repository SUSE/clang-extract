/* { dg-options "-DCE_EXTRACT_FUNCTIONS=f -DCE_NO_EXTERNALIZATION" }*/

struct NCDObject_s {
  int data_int;
  void *data_ptr;
  void *method_user;
};
struct NCDObject_s NCDObject_build();
void f () {
    NCDObject_build();
}

/* { dg-final { scan-tree-dump "struct NCDObject_s {\n *int data_int;" } } */
