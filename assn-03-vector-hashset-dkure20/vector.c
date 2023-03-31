#include "vector.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <search.h>

void VectorNew(vector *v, int elemSize, VectorFreeFunction freeFn, int initialAllocation){
    assert(elemSize>0);
    v->elemSize = elemSize;
    assert(initialAllocation>=0);
    if(initialAllocation==0)initialAllocation = 4;
    v->alloc_len = initialAllocation;
    v->log_len = 0; 
    v->freeFn = freeFn;
    v->memory = malloc(elemSize * v->alloc_len);
    assert(v->memory!=NULL);
}
void VectorDispose(vector *v){
    if(v->freeFn!=NULL){
        for(int i=0; i< v->log_len; i++){
            void* elemAdress = (char*)v->memory + (v->elemSize * i);
            v->freeFn(elemAdress);
         }
    }
    free(v->memory);
}

int VectorLength(const vector *v){ 
    return v->log_len;
}

void *VectorNth(const vector *v, int position){
    assert(position >=0 && position < v->log_len);
    void* elemAdress = (char*)v->memory + position * v->elemSize;
    return elemAdress;
}

void VectorReplace(vector *v, const void *elemAddr, int position){
    assert(position>=0 && position < v->log_len);
    void* current = (char*) v->memory + position * v->elemSize;
    if(v->freeFn!=NULL){
        v->freeFn(current);
    }
    memcpy(current, elemAddr,v->elemSize);
}

void VectorInsert(vector *v, const void *elemAddr, int position){
    assert(position>=0 && position <= v->log_len);
    if(v->log_len == v->alloc_len){
        v->alloc_len *=2;
        v->memory = realloc(v->memory, v->alloc_len * v->elemSize);
        assert(v->memory!=NULL);
    }
    void* nextEl = (char*)v->memory + (position+1) * v->elemSize;
    void* cur = (char*)v->memory + position * v->elemSize;
    memmove(nextEl,cur, (v->log_len - position) * v->elemSize);
    v->log_len++;
    memcpy(cur,elemAddr,v->elemSize);
}

void VectorAppend(vector *v, const void *elemAddr){
    VectorInsert(v,elemAddr, v->log_len);
}

void VectorDelete(vector *v, int position){
    assert(position >=0 && position < v->log_len);
   void* elemAdress = (char*)v->memory + position * v->elemSize;
   if(v->freeFn!=NULL) v->freeFn(elemAdress);
   void* src = (char*) v->memory + (position +1) * v->elemSize;
   int numberOfBytes = (v->log_len - position - 1) * v->elemSize;
   if(numberOfBytes >0) 
   memmove(elemAdress,src, numberOfBytes);
   v->log_len--;
}

void VectorSort(vector *v, VectorCompareFunction compare){
    assert(compare != NULL);
    qsort(v->memory, v->log_len, v->elemSize,compare);
}

void VectorMap(vector *v, VectorMapFunction mapFn, void *auxData){
    assert(mapFn!=NULL);
    for(int i=0; i<v->log_len; i++){
        mapFn((char*)v->memory + i * v->elemSize, auxData);
    }
}

static const int kNotFound = -1;
int VectorSearch(const vector *v, const void *key, VectorCompareFunction searchFn, int startIndex, bool isSorted){
    assert(startIndex >= 0 && startIndex <= v->log_len);
    assert(key!=NULL);
    assert(searchFn!=NULL);
    if(isSorted){
        void* starting = (char*) v->memory + startIndex* v->elemSize;
        void* foundElem = bsearch(key, starting, v->log_len, v->elemSize,searchFn);
        if(foundElem==NULL) return kNotFound;
        return ((char*) foundElem - (char*) v->memory) / v->elemSize;
    }else{
        for(int i= startIndex; i < v->log_len; i++){
            void* current = (char*) v->memory + i * v->elemSize;
            if(searchFn(current,key)==0) return i;
        }
        return kNotFound;
    }
} 
