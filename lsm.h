typedef int keyType;
typedef int valType;

typedef struct _node{
    keyType key;
    valType val;
}node;

typedef struct _lsm{
    size_t block_size; //number of nodes for each block
    int k;
    int node_size;
    size_t next_empty;
    node *block;
    char* diskname;
    bool sorted;
}lsm;

typedef struct _nodei{
    node *node;
    int index;
}nodei;


lsm* init_lsm(size_t block_size,bool sorted);

void destruct_lsm(lsm* tree);

void merge(node *whole, node* left, int left_size, node* right,int right_size);

void merge_sort(node *block , int n);

nodei* search_buffer(const keyType* key, lsm* tree);

nodei* search_disk(const keyType* key, lsm* tree);

node* get(const keyType key, lsm* tree);

int write_to_disk(lsm* tree);

int put(const keyType* key, const valType* val, lsm* tree);

int delete(const keyType* key, lsm* tree);

int update(const keyType* key, const valType* val, lsm* tree);

void print_buffer_data(lsm* tree);

void print_disk_data(lsm* tree);

void print_node(node* node);