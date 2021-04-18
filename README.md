# lsm-tree

LSM tree implemented in C.

Array is used for active buffer and disk storage.

Merge sort used for compaction.

Supports four operations 

    get(key, tree)
    put(key, val, tree)
    delete(key , tree)
    update(key, val, tree)
## How to run

### Compile
    gcc test.c lsm.c -o test

### run
    ./test [data_size] [buffer_size] [npos] [operation_type] [sort]


data_size - size of data that needed to be tested

buffer_size - size of the active bufffer

nops - number of operations to be done for get, put, update, delete

operation_type - "get" , "put" , "del", "upd"

sort - true | false
