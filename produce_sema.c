#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>

#define CAPACITY 4
typedef struct {
    int value;
    pthread_mutex_t mutex;
    pthread_cond_t cond;
} sema_t;

typedef struct{
    int in;
    int out;
    int space[CAPACITY];
    sema_t mutext_sema;
    sema_t empty_buffer_sema;
    sema_t full_buffer_sema;
}buff;

buff buffer1, buffer2;

void sema_init(sema_t *sema, int value)//initlize the semaphore
{
    sema->value = value;
    pthread_cond_init(&sema->cond, NULL);
    pthread_mutex_init(&sema->mutex, NULL);
}

void sema_wait(sema_t *sema)
{
    pthread_mutex_lock(&sema->mutex);
    --sema->value;
    while (sema->value < 0)
        pthread_cond_wait(&sema->cond, &sema->mutex);
    
    pthread_mutex_unlock(&sema->mutex);
}

void sema_signal(sema_t *sema)
{
    pthread_mutex_lock(&sema->mutex);
    
    ++sema->value;
    
    pthread_cond_signal(&sema->cond);
    pthread_mutex_unlock(&sema->mutex);
}
int get_item(buff *buffer)
{
    int item;
    sema_wait(&buffer->full_buffer_sema);
    sema_wait(&buffer->mutext_sema);
    
    item = buffer->space[buffer->out ];
    
    buffer->out = (buffer->out + 1) % CAPACITY;
    
    sema_signal(&buffer->mutext_sema);
    sema_signal(&buffer->empty_buffer_sema);
    return item;
}

void put_item(buff *buffer, int item)
{
    sema_wait(&buffer->empty_buffer_sema);
    sema_wait(&buffer->mutext_sema);
    
    buffer->space[buffer->in] = item;
    buffer->in = (buffer->in + 1) % CAPACITY;
    
    sema_signal(&buffer->mutext_sema);
    sema_signal(&buffer->full_buffer_sema);
    printf("Produce item: %c\n", item);
}


void produce()
{
    int item;
    for(int i = 0; i< CAPACITY * 2; i++)
    {
        item = 'a' + i;
        put_item(&buffer1, item);
    }
}

void *consume(void *arg)
{
    for (int i =0; i< CAPACITY * 2; i++)
        printf("Consume item: %c\n", get_item(&buffer1));
        
    return NULL;
}

int main(){
    pthread_t consumer_id;
    
    sema_init(&buffer1.mutext_sema, 1);
    sema_init(&buffer1.empty_buffer_sema, CAPACITY - 1);
    sema_init(&buffer1.full_buffer_sema, 0);
    
    pthread_create(&consumer_id, NULL, consume, NULL);
    
    produce();
    pthread_join(consumer_id, NULL);
    return 0;

}