void test_print_tree(lsm* tree);

int test_put(lsm* tree, int data_size, int buffer_size, bool sorted, bool timing);

int test_get(lsm* tree, int data_size, int nops, bool timing);

int test_delete(lsm* tree, int data_size, int nops, bool timing);

int test_update(lsm* tree, int data_size, int nops, bool timing);

// int test_throughput(lsm* tree, int data_size, int buffer_size, bool sorted, int nops, float put_prob, float update_prob, bool timing);