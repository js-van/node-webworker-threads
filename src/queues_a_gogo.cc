//2011-11 Proyectos Equis Ka, s.l., jorge@jorgechamorro.com
//queues_a_gogo.cc

#include <unistd.h>
#include <string.h>
#include <stdio.h>
#include <pthread.h>
#include <stdlib.h>

enum types {
  kItemTypeNONE,
  kItemTypeNumber,
  kItemTypePointer,
  kItemTypeQUIT
};

struct typeQueueItem {
  int itemType;
  typeQueueItem* next;
  union {
    void* asPtr;
    double asNumber;
  };
};
typedef struct typeQueueItem typeQueueItem;

typedef struct {
  typeQueueItem* first;
  typeQueueItem* last;
  pthread_mutex_t queueLock;
  long int id;
  volatile long int length;
} typeQueue;

static typeQueue* queuesPool= NULL;
static typeQueue* freeItemsQueue= NULL;




static inline void queue_push (typeQueueItem* qitem, typeQueue* queue);
static inline typeQueueItem* queue_pull (typeQueue* queue);
static inline typeQueueItem* nuItem (int itemType, void* item);
static inline void destroyItem (typeQueueItem* qitem);
static typeQueue* nuQueue (long int id);
static void resetQueue (typeQueue* queue);
static void initQueues (void);




static inline void queue_push (typeQueueItem* qitem, typeQueue* queue) {
  qitem->next= NULL;
  
  pthread_mutex_lock(&queue->queueLock);
  if (queue->last) {
    queue->last->next= qitem;
  }
  else {
    queue->first= qitem;
  }
  queue->last= qitem;
  queue->length++;
  pthread_mutex_unlock(&queue->queueLock);
}




static inline typeQueueItem* queue_pull (typeQueue* queue) {
  typeQueueItem* qitem;
  
  pthread_mutex_lock(&queue->queueLock);
  if ((qitem= queue->first)) {
    if (queue->last == qitem) {
      queue->first= queue->last= NULL;
    }
    else {
      queue->first= qitem->next;
    }
    queue->length--;
    //qitem->next= NULL;
  }
  pthread_mutex_unlock(&queue->queueLock);
  
  return qitem;
}




static inline typeQueueItem* nuItem (int itemType, void* item) {
  
  typeQueueItem* qitem= queue_pull(freeItemsQueue);
  if (!qitem) {
    qitem= (typeQueueItem*) calloc(1, sizeof(typeQueueItem));
  }
  
  //qitem->next= NULL;
  qitem->itemType= itemType;
  if (itemType == kItemTypeNumber) {
    qitem->asNumber= *((double*) item);
  }
  else if (itemType == kItemTypePointer) {
    qitem->asPtr= item;
  }
  
  return qitem;
}




static inline void destroyItem (typeQueueItem* qitem) {
  
  if (freeItemsQueue) {
    queue_push(qitem, freeItemsQueue);
  }
  else {
    free(qitem);
  }
}




static typeQueue* nuQueue (long int id) {
  
  typeQueue* queue= NULL;
  typeQueueItem* qitem= NULL;
  if (queuesPool && queuesPool->first) qitem= queue_pull(queuesPool);
  if (qitem) {
    queue= (typeQueue*) qitem->asPtr;
    destroyItem(qitem);
  }
  else {
    queue= (typeQueue*) calloc(1, sizeof(typeQueue));
    pthread_mutex_init(&queue->queueLock, NULL);
  }
  
  queue->id= id;
  queue->length= 0;
  queue->first= queue->last= NULL;
  return queue;
}




static void resetQueue (typeQueue* queue) {
  queue->first= queue->last= NULL;
  queue->length= 0;
}




/*
static void destroyQueue (typeQueue* queue) {
  if (queuesPool) {
    queue_push(nuItem(kItemTypePointer, queue), queuesPool);
  }
  else {
    free(queue);
  }
}
*/




static void initQueues (void) {
  freeItemsQueue= nuQueue(-2);  //MUST be created before queuesPool
  //queuesPool= nuQueue(-1);
}
