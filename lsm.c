#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <stdbool.h>
#include <string.h>
#include <time.h>
#include <sys/stat.h>
#include "lsm.h"

void check_file_ret(FILE* f, int r){
    if(r == 0 ){
        if(ferror(f)){
            perror("ferror\n");
        }else if(feof(f)){
            perror("EOF found\n");
        }
    }
}
lsm* init_lsm(size_t buffer_size, bool sorted){
    lsm* tree;
    tree = malloc(sizeof(lsm));
    if(!tree){
        perror("init_lsm : block is null \n");
        return NULL;
    }
    tree->block_size = buffer_size;
    tree->k = 2;
    tree->next_empty = 0;
    tree->node_size = sizeof(node);
    tree->block = malloc(tree->block_size*tree->node_size);
    if(!tree->block){
        perror("init_lsm : the block is null \n");
        return NULL;
    }
    tree->diskname = "disk_storage.txt";
    tree->sorted = sorted;
    return tree;
}

void merge(node *whole, node* left, int left_size, node* right,int right_size){
    int l=0, r=0, i=0;
    while(l<left_size && r<right_size){
        whole[i++] = (left[l].key < right[r].key) ? left[l++] : right[r++];
    }
    while(l<left_size){
        whole[i++] = left[l++];
    }
    while(r<right_size){
        whole[i++] = right[r++];
    }
}


void merge_sort(node *block , int n){
    assert(block!=NULL);
    if(n<2){
        return;
    }
    int mid, i;
    mid = n/2;
    node *left, *right;

    left = (node*)malloc(mid*sizeof(node));
    right = (node*)malloc((n-mid)*sizeof(node));

    memcpy(left, block, sizeof(node) * mid);
    memcpy(right, &block[mid], sizeof(node) * (n-mid));

    merge_sort(left, mid);
    merge_sort(right, n-mid);
    merge(block, left, mid, right, n-mid);
    free(left);
    free(right);
}

void destruct_lsm(lsm* tree){
    free(tree->block);
    free(tree);
}

int put(const keyType* key, const valType* val, lsm* tree){
    int r = 0;
    if(tree->next_empty == tree->block_size){
        r = write_to_disk(tree);
    }
    node n;
    n.key = *key;
    n.val = *val;
    tree->block[tree->next_empty] = n;
    tree->next_empty += 1;
    return 0;
}

int write_to_disk(lsm* tree){
    node * complete_data = NULL;
    size_t num_elements = 0;
    int r;

    if(tree->sorted){
        merge_sort(tree->block, tree->next_empty);
    }

    struct stat s;
    int file_exists = stat(tree->diskname, &s);
    if(file_exists == 0){
        FILE* fr = fopen(tree->diskname, "r");
        r = fread(&num_elements, sizeof(size_t), 1, fr);
        check_file_ret(fr, r);

        node * file_data = NULL;
        file_data = malloc(sizeof(node)*num_elements);
        assert(file_data);

        r = fread(file_data, sizeof(node), num_elements, fr);
        check_file_ret(fr, r);

        if(fclose(fr)){
            perror("put : File read close error \n");
        }

        complete_data = malloc(sizeof(node) * (num_elements + tree->next_empty));
        merge(complete_data, file_data, num_elements, tree->block , tree->next_empty);
        num_elements+= tree->next_empty;
        free(file_data);
    }

    FILE* fw = fopen(tree->diskname, "w");
    if(complete_data==NULL){
        complete_data = tree->block;
    }
    if(num_elements<=0){
        num_elements = tree->next_empty;
    }

    if(fseek(fw, 0, SEEK_SET)){
        perror("put : file write seek error\n");
    }
    if(!fwrite(&num_elements, sizeof(size_t), 1, fw)){
        perror("put : file write data count error\n");
    }
    if(fseek(fw, sizeof(size_t), SEEK_SET)){
        perror("put : file write seek error\n");
    }
    if(!fwrite(complete_data, sizeof(node), num_elements, fw)){
        perror("put : file write data error\n");
    }

    // free(complete_data);
    tree->next_empty = 0;
    if(fclose(fw)){
        perror("put : File write close error \n");
    }
    return 0;
}

node* get(const keyType key, lsm* tree){
    nodei *ni = search_buffer(&key, tree);
    if(ni!=NULL){
        return ni->node;
    }
    ni = search_disk(&key, tree);
    if(ni!=NULL){
        return ni->node;
    }
    return NULL;
}

 
nodei* search_buffer(const keyType* key, lsm* tree){
    for (int i = 0; i < tree->next_empty; i++)
    {
        if(tree->block[i].key == *key){
            nodei* nodei = malloc(sizeof(nodei));
            nodei->node = malloc(sizeof(node));
            nodei->node->key = tree->block[i].key;
            nodei->node->val = tree->block[i].val;
            nodei->index = i;
            return nodei;
        }
    }
    return NULL;
}

nodei* search_disk(const keyType* key, lsm* tree){
    int r; 
    FILE * fr = fopen(tree->diskname, "r");
    if(fr == NULL){
        perror("Search disk read error \n");
        return NULL;
    }
    size_t num_elements = 0;
    node* file_data;
    r = fread(&num_elements, sizeof(size_t), 1, fr);
    check_file_ret(fr, r);
    file_data = malloc(sizeof(node)* num_elements);
    r = fread(file_data, sizeof(node), num_elements, fr);
    check_file_ret(fr, r);

    for (int i = 0; i < num_elements; i++){
        if(file_data[i].key == *key){
            nodei* nodei = malloc(sizeof(nodei));
            nodei->node = malloc(sizeof(node));
            nodei->node->key = file_data[i].key;
            nodei->node->val = file_data[i].val;
            nodei->index = i;
            if(fclose(fr)){
                perror("Search disk close error \n");
            }
            return nodei;
        }
    }
    free(file_data);
    if(fclose(fr)){
        perror("Search disk close error \n");
    }
    return NULL;
}

int delete(const keyType* key, lsm* tree){
    nodei *ni = search_buffer(key, tree); //in buffer
    if(ni!=NULL){
        tree->next_empty -= 1;
        memmove(&tree->block[ni->index], &tree->block[ni->index+1], tree->block_size-ni->index);
        return 0;
    }
    //in disk
    ni = search_disk(key, tree);
    if(ni==NULL){
        printf("Delete key %d is not available\n", *key);
        return 0;
    }
    assert(ni);
    
    int r = 0;
    FILE * fr = fopen(tree->diskname, "r");
    if(fr == NULL){
        perror("Delete: disk read error \n");
        return -1;
    }
    size_t num_elements = 0;
    node* file_data;
    r = fread(&num_elements, sizeof(size_t), 1, fr);
    check_file_ret(fr, r);

    file_data = malloc(sizeof(node)* num_elements);
    assert(file_data);

    r = fread(file_data, sizeof(node), num_elements, fr);
    check_file_ret(fr, r);
    
    if(fclose(fr)){
        perror("delete : File read close error \n");
    }

    num_elements -= 1;
    memmove(&file_data[ni->index], &file_data[ni->index+1], num_elements-ni->index);
    FILE* fw = fopen(tree->diskname, "w");
    
    if(fseek(fw, 0, SEEK_SET)){
        perror("delete : file write seek error\n");
    }
    if(!fwrite(&num_elements, sizeof(size_t), 1, fw)){
        perror("delete : file write data count error\n");
    }
    if(fseek(fw, sizeof(size_t), SEEK_SET)){
        perror("delete : file write seek error\n");
    }
    if(!fwrite(file_data, sizeof(node), num_elements, fw)){
        perror("delete : file write data error\n");
    }

    if(fclose(fw)){
        perror("put : File write close error \n");
    }
    return 0;
}

int update(const keyType* key, const valType* val, lsm* tree){
    int r;
    nodei * ni = search_buffer(key, tree);
    if(ni!=NULL){
        node n;
        n.key = *key;
        n.val = *val;
        tree->block[ni->index] = n;
    }else{
        r = delete(key, tree);
        r = put(key, val, tree);
    }
    return 0;
}

void print_buffer_data(lsm* tree){
    printf("buffer data\n");
    for (int i = 0; i < tree->next_empty; i++){
        print_node(&tree->block[i]);
    }
}

void print_disk_data(lsm* tree){
    int r;
    printf("disk data\n");
    FILE * fr = fopen(tree->diskname, "r");
    if(fr == NULL){
        perror("Search disk read error \n");
        return;
    }
    size_t num_elements = 0;
    node* file_data;
    r = fread(&num_elements, sizeof(size_t), 1, fr);
    check_file_ret(fr, r);
    file_data = malloc(sizeof(node)* num_elements);
    if(file_data == NULL){
        perror("print_disk_data: unsuccessful allocation \n");
        return;
    }
    r = fread(file_data, sizeof(node), num_elements, fr);
    check_file_ret(fr, r);

    for (int i = 0; i < num_elements; i++){
        print_node(&file_data[i]);
    }
    free(file_data);
    if(fclose(fr)){
        perror("Search disk close error \n");
    }
}
void print_node(node* node){
    if(node == NULL){
        printf("Key not present\n");
        return;
    }
    printf("Key : %d  --- Value : %d\n", node->key, node->val);
}