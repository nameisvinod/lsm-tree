#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <stdbool.h>
#include <string.h>
#include <time.h>
#include <sys/stat.h>
#include "lsm.h"
#include "test.h"

void test_print_tree(lsm* tree){
  struct stat s; 
  if(stat(tree->diskname, &s)){
    perror("print: fstat \n");
  }  
   if(s.st_size == 0 && tree->next_empty != 0){
    printf("data fits in the buffer\n");
    print_buffer_data(tree);
  }
  if(s.st_size > 0 && tree->next_empty == 0){
    printf("data is only on disk\n");
    print_disk_data(tree);
  }
  if(s.st_size >  0 && tree->next_empty != 0){
    printf("data is in buffer & on disk\n");
    print_buffer_data(tree);
    print_disk_data(tree);
  }
}

int test_put(lsm* tree, int data_size, int buffer_size, bool sorted, bool timing){
    int r;
    clock_t start, end;
    start = clock();
    for (int i = 0; i < data_size; i++){
        keyType k;
        valType v;

        k = (keyType) i;
        v = (valType) i;
        r = put(&k, &v, tree);
        assert(r==0);
    }
    end = clock();
    if(timing){
        double time_elapsed = (double)(end - start)/CLOCKS_PER_SEC;
        printf("time to put %d data set is %f, \n", data_size, time_elapsed);
    }
    return r;   
}

int test_get(lsm* tree, int data_size, int nops, bool timing){
    int r;
    clock_t start, end;
    start = clock();
    for (int i = 0; i < nops; i++){
        keyType k;
        valType v;
        k = (keyType) (rand() % data_size-1);
        node* n = get(k, tree);
    }
    end = clock();
    if(timing){
        double time_elapsed = (double)(end - start)/CLOCKS_PER_SEC;
        printf("time for get %d nops set is %f, \n", nops, time_elapsed);
    }
    return r;   
}

int test_delete(lsm* tree, int data_size, int nops, bool timing){
  int r = 0; 
  clock_t start, end;
  start = clock();
  for(int i = 0; i < nops; i++){
    keyType k;
    k = (keyType)((rand() % data_size)+10);
    printf("deleting key: %d \n", k);
    r = delete(&k, tree);
  }
  end = clock();
  if(timing){
    double time_elapsed = (double)end-start/CLOCKS_PER_SEC;
    printf("time for delete %d nops set is %f, \n", nops, time_elapsed);
  }
  return r; 
}

int test_update(lsm* tree, int data_size, int nops, bool timing){
    clock_t start, end;
    int r ;
    for(int i = 0; i < nops; i++){
        keyType k;
        valType v;
        k = (keyType)((rand() % data_size)+10);
        v = (valType)(rand() % data_size-1);
        printf("updating key: %d \n", k);
        int r = update(&k, &v, tree);
        assert(r==0);
    }
    end = clock();
    if(timing){
        double time_elapsed = (double)end-start/CLOCKS_PER_SEC;
        printf("time for update %d nops set is %f, \n", nops, time_elapsed);
    }
    return r;
}


int main(int argc, char const *argv[])
{
    assert(argc >= 5);

    int r;
    int data_size = atoi(argv[1]);
    int buffer_size = atoi(argv[2]);
    int nops = atoi(argv[3]);
    char testing[4];
    strcpy(testing, argv[4]);
    bool sorted = false;
    if (argc == 6)
    {
        sorted = true;
    }
    // set probabilities for throughput.
    float put_prob = 33.0;
    float update_prob = 33.0;
    if (argc == 8)
    {
        put_prob = atoi(argv[6]);
        update_prob = atoi(argv[7]);
    }
    lsm *tree;
    tree = init_lsm(buffer_size, sorted);
    if (strcmp(testing, "put") == 0)
    {
        r = test_put(tree, data_size, buffer_size, sorted, true);
    }
    else if (strcmp(testing, "get") == 0)
    {
        r = test_put(tree, data_size, buffer_size, sorted, false);
        r = test_get(tree, data_size, nops, true);
    }
    else if (strcmp(testing, "upd") == 0)
    {
        r = test_put(tree, data_size, buffer_size, sorted, false);
        r = test_update(tree, data_size, nops, true);
    }

    else if (strcmp(testing, "thr") == 0)
    {
        // r = test_put(tree, data_size, buffer_size, sorted, false);
        // r = test_throughput(tree, data_size, buffer_size, sorted, nops, put_prob, update_prob, true);
    }else{
        r = test_put(tree, data_size, buffer_size, sorted, false);
        test_print_tree(tree);
    }
    return 0;
}
