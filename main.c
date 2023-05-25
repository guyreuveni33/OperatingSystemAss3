#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include <pthread.h>
#include <semaphore.h>


typedef struct {
    int sports;
    int news;
    int weather;
    int id;
    int queue_size;
    int product_num;
    char **queue;
    int front;  // Index of the front element in the queue
    int rear;   // Index of the rear element in the queue
    // Semaphore for mutual exclusion
    sem_t mutex;
    // Semaphore for signaling available items in the queue,so who want to remove an item from the queue will
    // wait (sem_wait) on the items semaphore until it becomes available
    sem_t items;
    // Semaphore for signaling available slots in the queue
    sem_t slots;
} Producer;

int producerCount = 0;
Producer *producers = NULL;

const char *getProductType(int randomNumber) {
    switch (randomNumber) {
        case 0:
            return "SPORTS";
        case 1:
            return "NEWS";
        case 2:
            return "WEATHER";
        default:
            return "";
    }
}

void insertItem(Producer *producer, char *item) {
    sem_wait(&producer->slots);  // Wait for an available slot in the queue
    sem_wait(&producer->mutex);  // Acquire the mutex to access the queue

    // Insert the item into the queue
    producer->queue[producer->rear] = item;
    producer->rear = (producer->rear + 1) % producer->queue_size;

    sem_post(&producer->mutex);  // Release the mutex
    sem_post(&producer->items);  // Signal that an item has been inserted
    //printf("Item inserted: %s\n", item);  // Print the inserted item

}

char *removeItem(Producer *producer) {
    char *item;

    sem_wait(&producer->items);  // Wait for an item in the queue
    sem_wait(&producer->mutex);  // Acquire the mutex to access the queue

    // Remove the item from the queue
    item = producer->queue[producer->front];
    producer->front = (producer->front + 1) % producer->queue_size;

    sem_post(&producer->mutex);  // Release the mutex
    sem_post(&producer->slots);  // Signal that a slot is available
    //printf("Item removed: %s\n", item);  // Print the removed item

    return item;
}


int countDigitsOfNum(int num) {
    if (num == 0) {
        return 1;  // Special case for zero
    }
    int count = 0;
    while (num != 0) {
        num /= 10;
        count++;
    }
    return count;
}


Producer createProducer(int producer_id, int product_num, int queue_size) {
    Producer producer;
    producer.id = producer_id;
    producer.product_num = product_num;
    producer.queue_size = queue_size;
    producer.sports = 0;
    producer.news = 0;
    producer.weather = 0;
    producer.front = 0;
    producer.rear = 0;
    sem_init(&producer.mutex, 0, 1);    // Initialize mutex semaphore to 1 (available)
    sem_init(&producer.items, 0, 0);    // Initialize items semaphore to 0 (unavailable)
    sem_init(&producer.slots, 0, queue_size);   // Initialize slots semaphore to the queue size
    producers = realloc(producers, (producerCount + 1) * sizeof(Producer));
    if (producers == NULL) {
        printf("Memory allocation failed.\n");
        //TODO return error
    }
    producer.queue = malloc(producer.queue_size * sizeof(char *));
    if (producer.queue == NULL) {
        printf("Memory allocation failed.\n");
        //TODO return error
    }
    return producer;
}

int readConfFile(FILE *file) {
    int producer_args, argOne, argTwo, argThree;
    while ((producer_args = fscanf(file, "%d\n%d\n%d\n\n", &argOne, &argTwo, &argThree)) && producer_args == 3) {
        Producer producer = createProducer(argOne, argTwo, argThree);
        producers[producerCount] = producer;
        producerCount++;
    }
    int coEditorCapacity = argOne;
    for (int i = 0; i < producerCount; i++) {
        printf("Producer ID: %d\n", producers[i].id);
        printf("Queue Size: %d\n", producers[i].queue_size);
        printf("-------------\n");
    }
    return coEditorCapacity;
}


int createProduct(Producer producer) {
    char *s = NULL;
    for (int i = 0; i < producer.product_num; i++) {
        // Generate a random number between 0 and 2
        int randomNumber = rand() % 3;
        switch (randomNumber) {
            case 0:
                s = malloc(strlen("producer ") + countDigitsOfNum(producer.id) + strlen(" ") + strlen("SPORTS") +
                           strlen(" ") + countDigitsOfNum(producer.sports) +
                           1); // Allocate memory for the string
                sprintf(s, "Producer %d %s %d", producer.id, getProductType(randomNumber), producer.sports);
                producer.sports++;
                break;
            case 1:
                s = malloc(strlen("producer ") + countDigitsOfNum(producer.id) + strlen(" ") + strlen("NEWS") +
                           strlen(" ") + countDigitsOfNum(producer.news) +
                           1); // Allocate memory for the string
                sprintf(s, "Producer %d %s %d", producer.id, getProductType(randomNumber), producer.news);
                producer.news++;
                break;
            case 2:
                s = malloc(strlen("producer ") + countDigitsOfNum(producer.id) + strlen(" ") + strlen("WEATHER") +
                           strlen(" ") + countDigitsOfNum(producer.weather) +
                           1); // Allocate memory for the string
                sprintf(s, "Producer %d %s %d", producer.id, getProductType(randomNumber), producer.weather);
                producer.weather++;
                break;
            default:
                break;
        }
        insertItem(&producer, s);  // Insert the item into the queue
        printf("%s\n", s);
         removeItem(&producer);
    }
}


void runThreads_Producers(pthread_t threads[]) {
    for (int i = 0; i < producerCount; i++) {
        createProduct(producers[i]);
    }        //pthread_create(&threads[i], NULL, (void *)createProduct, &producers[i]);

}

int main(int argc, char *argv[]) {
    if (argc != 2) {
        printf("Usage: %s <config_file_path>\n", argv[0]);
        return 1;
    }
    FILE *file = fopen(argv[1], "r");
    if (file == NULL) {
        printf("Failed to open the file.\n");
        return 1;
    }
    // Read the file line by line
    srand(time(NULL));

    int coEditorCapacity = readConfFile(file);


    //create threads by number of producer + 3 co-Editor + dispatcher
  pthread_t threads[producerCount + 4];
    runThreads_Producers(threads);

    for (int i = 0; i < producerCount + 4; i++) {
        //pthread_join(threads[i], NULL);
    }
    return 0;
}