#include "hashset.h"
#include <assert.h>
#include <stdlib.h>
#include <string.h>

void HashSetNew(hashset *h, int elemSize, int numBuckets,
		HashSetHashFunction hashfn, HashSetCompareFunction comparefn, HashSetFreeFunction freefn){
			assert(elemSize > 0 && numBuckets > 0);
			assert(hashfn!=NULL && comparefn!=NULL);
			h->elemSize = elemSize;
			h->numBuckets = numBuckets;
			h->freeFn = freefn;
			h->compareFn = comparefn;
			h->hashFn = hashfn;
			h->hashVector = (vector*) malloc(sizeof(vector) * numBuckets);
			assert(h->hashVector!=NULL);
			for(int i=0; i<numBuckets; i++){
				VectorNew(&(h->hashVector[i]),elemSize, freefn, 4);
			}
		}

void HashSetDispose(hashset *h){
		for(int i=0; i<h->numBuckets; i++){
			VectorDispose(&h->hashVector[i]);
		}
	free(h->hashVector);
}

int HashSetCount(const hashset *h){
	int sum = 0; 
	for(int i=0; i<h->numBuckets; i++){
		sum += VectorLength(&h->hashVector[i]);
	}
	return sum;
}

void HashSetMap(hashset *h, HashSetMapFunction mapfn, void *auxData){
	assert(mapfn!=NULL && h!=NULL);
	for(int i=0; i<h->numBuckets; i++){
		VectorMap(&h->hashVector[i],mapfn, auxData);
	}
}

void HashSetEnter(hashset *h, const void *elemAddr){
	assert(elemAddr!=NULL);
	int hashIndex = h->hashFn(elemAddr, h->numBuckets);
	assert(hashIndex >= 0 && hashIndex < h->numBuckets);
	vector* curVector =	&h->hashVector[hashIndex];
	if(VectorSearch(curVector,elemAddr,h->compareFn,0,false)<0){
		VectorAppend(curVector,elemAddr);
	}else{
		int found = VectorSearch(curVector,elemAddr,h->compareFn,0,false);
		VectorReplace(curVector, elemAddr,found);
	}
}

void *HashSetLookup(const hashset *h, const void *elemAddr){
	 assert(elemAddr!=NULL);
	 int hashIndex = h->hashFn(elemAddr,h->numBuckets);
	 assert(hashIndex >= 0 && hashIndex < h->numBuckets);
	 vector* curVector = &h->hashVector[hashIndex];
	 int element = VectorSearch(curVector,elemAddr,h->compareFn,0,false);
	 if(element < 0 ) return NULL;
	 return VectorNth(curVector,element);

}
