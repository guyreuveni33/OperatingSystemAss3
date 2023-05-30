
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include <pthread.h>
#include <semaphore.h>
#include <unistd.h>

int lineCounter = 0;
int lineCounter2 = 0;
int lineCounter3 = 0;
int screenCount = 0;
typedef struct {
    int size;
    char **queue;
    sem_t mutex;
} CoEditor;


typedef struct {
    int currentCapacity;
    int maxCapacity;
    char **queue;
    sem_t full;
    sem_t empty;
    pthread_mutex_t mutex;
} Screen;


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
CoEditor *coEditors = NULL;
Screen *screen = NULL;

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
    // int semaphoreValue;
    // sem_getvalue(&producer->slots, &semaphoreValue);
    // printf("Insert - Slots semaphore value: %d\n", semaphoreValue);
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


Producer *createProducer(int producer_id, int product_num, int queue_size) {
    Producer *producer = malloc(sizeof(*producer));
    producer->id = producer_id;
    producer->product_num = product_num;
    producer->queue_size = queue_size;
    producer->sports = 0;
    producer->news = 0;
    producer->weather = 0;
    producer->front = 0;
    producer->rear = 0;
    sem_init(&producer->mutex, 0, 1);    // Initialize mutex semaphore to 1 (available)
    sem_init(&producer->items, 0, 0);    // Initialize items semaphore to 0 (unavailable)
    sem_init(&producer->slots, 0, queue_size);   // Initialize slots semaphore to the queue size
//    printf("Semaphore values for producer %d:\n", producer->id);
//    int mutexValue, itemsValue, slotsValue;
//    sem_getvalue(&producer->mutex, &mutexValue);
//    sem_getvalue(&producer->items, &itemsValue);
//    sem_getvalue(&producer->slots, &slotsValue);
//    printf("Mutex: %d\n", mutexValue);
//    printf("Items: %d\n", itemsValue);
//    printf("Slots: %d\n", slotsValue);
    producers = realloc(producers, (producerCount + 1) * sizeof(Producer));
    if (producers == NULL) {
        printf("Memory allocation failed.\n");
        //TODO return error
    }
    producer->queue = malloc(producer->queue_size * sizeof(char *));
    if (producer->queue == NULL) {
        printf("Memory allocation failed.\n");
        //TODO return error
    }
    return producer;
}

int readConfFile(FILE *file) {
    int producer_args, argOne, argTwo, argThree;
    while ((producer_args = fscanf(file, "%d\n%d\n%d\n\n", &argOne, &argTwo, &argThree)) && producer_args == 3) {
        Producer *producer = createProducer(argOne, argTwo, argThree);
        producers[producerCount] = *producer;
        producerCount++;
        free(producer);
    }
    int coEditorCapacity = argOne;
    for (int i = 0; i < producerCount; i++) {
        printf("Producer ID: %d\n", producers[i].id);
        printf("Queue Size: %d\n", producers[i].queue_size);
        printf("-------------\n");
    }
    return coEditorCapacity;
}

void createProduct(Producer *producer) {
    char *s = NULL;
    for (int i = 0; i < producer->product_num; i++) {
        // Generate a random number between 0 and 2
        int randomNumber = rand() % 3;
        switch (randomNumber) {
            case 0:
                s = malloc(strlen("producer ") + countDigitsOfNum(producer->id) + strlen(" ") + strlen("SPORTS") +
                           strlen(" ") + countDigitsOfNum(producer->sports) +
                           1); // Allocate memory for the string
                sprintf(s, "Producer %d %s %d", producer->id, getProductType(randomNumber), producer->sports);
                producer->sports++;
                break;
            case 1:
                s = malloc(strlen("producer ") + countDigitsOfNum(producer->id) + strlen(" ") + strlen("NEWS") +
                           strlen(" ") + countDigitsOfNum(producer->news) +
                           1); // Allocate memory for the string
                sprintf(s, "Producer %d %s %d", producer->id, getProductType(randomNumber), producer->news);
                producer->news++;
                break;
            case 2:
                s = malloc(strlen("producer ") + countDigitsOfNum(producer->id) + strlen(" ") + strlen("WEATHER") +
                           strlen(" ") + countDigitsOfNum(producer->weather) +
                           1); // Allocate memory for the string
                sprintf(s, "Producer %d %s %d", producer->id, getProductType(randomNumber), producer->weather);
                producer->weather++;
                break;
            default:
                break;
        }
        insertItem(producer, s);  // Insert the item into the queue
        //free(s);
        // printf("%s\n", s);
        //removeItem(producer);
    }
    insertItem(producer, "DONE");  // Insert the DONE string into the queue
}


void runThreads_Producers(pthread_t threads[]) {
    for (int i = 0; i < producerCount; i++) {
        //    createProduct(producers[i]);
        pthread_create(&threads[i], NULL, (void *) createProduct, &producers[i]);
    }

}

void createCoEditor() {
    coEditors = malloc(3 * sizeof(CoEditor ));  // Allocate memory for the array of CoEditor pointers
    if (coEditors == NULL) {
        exit(1);
    }
    for (int i = 0; i < 3; i++) {
        CoEditor *tempEditor = malloc(sizeof(CoEditor));
        if (tempEditor == NULL) {
            exit(1);
        }
        sem_init(&tempEditor->mutex, 0, 1);  // Initialize mutex semaphore to 1 (available)
        tempEditor->size = 0;
        tempEditor->queue = NULL;
        coEditors[i] = *tempEditor;
        free(tempEditor);
    }
}

void insertCoEditor(CoEditor *coEditor, char *s) {
    // Acquire the mutex lock to ensure thread safety
    sem_wait(&coEditor->mutex);
    // Increase the size of the queue
    coEditor->size++;
    // Reallocate memory for the updated queue size
    coEditor->queue = realloc(coEditor->queue, coEditor->size * sizeof(char *));
    if (coEditor->queue == NULL) {
        printf("1111Memory allocation failed.\n");
        // TODO: handle error
    }
    // Allocate memory for the new string in the queue
    coEditor->queue[coEditor->size - 1] = malloc(strlen(s) + 1);
    if (coEditor->queue[coEditor->size - 1] == NULL) {
        printf("111Memory allocation failed.\n");
        // TODO: handle error
    }
    // Copy the string into the queue
    strcpy(coEditor->queue[coEditor->size - 1], s);
    lineCounter++;
    //printf("Added string:%d %s\n", lineCounter, s);
    // Release the mutex lock
    sem_post(&coEditor->mutex);

}

char *removeCoEditor(CoEditor *coEditor) {
    if (coEditor == NULL || coEditor->queue == NULL) {
        ///TODO CHANGED HERE TO RETURN EMPTY STRING AND NOT NULL
        return "";
    }
    sem_wait(&coEditor->mutex);
    char *str = coEditor->queue[0];
    lineCounter2++;
    //printf("Remove string:%d %s\n", lineCounter2, str);

    for (int i = 1; i < coEditor->size; i++) {
        coEditor->queue[i - 1] = coEditor->queue[i];
    }
    coEditor->queue[coEditor->size - 1] = NULL;
    coEditor->size--;
    coEditor->queue = (char **) realloc(coEditor->queue, (coEditor->size) * sizeof(char *));
    sem_post(&coEditor->mutex);
    return str;
}

void dispatcher() {
    const char *categoryMap[] = {"WEATHER", "NEWS", "SPORTS"};
    for (int start = 0, end = 0; end != producerCount;
         start = (start + 1) % producerCount) {
        if (producers[start].queue_size == 0) {
            continue;
        }
        char *s = removeItem(&producers[start]);
        if (s == NULL || strlen(s) == 0) {
            continue;
        }
        if (strcmp(s, "DONE") == 0) {
            producers[start].queue_size = 0;
            end++;
         //  free(s);
            continue;
        }
        // Iterate through the NEWS or SPORTS or WEATHER strings and check for a match
        for (int i = 0; i < 3; i++) {
            if (strstr(s, categoryMap[i]) != NULL) {
                insertCoEditor(&coEditors[i], s);
                break;
            }
        }
          free(s);
    }
    insertCoEditor(&coEditors[0], "DONE");
    insertCoEditor(&coEditors[1], "DONE");
    insertCoEditor(&coEditors[2], "DONE");
}



void insertScreen(char *item) {
    sem_wait(&(screen)->empty);
    pthread_mutex_lock(&(screen)->mutex);
    (screen)->queue[(screen)->currentCapacity] = (char *) malloc(strlen(item) + 1);
    if ((screen)->queue[(screen)->currentCapacity] == NULL) {
        exit(1);
    }
    strcpy((screen)->queue[(screen)->currentCapacity], item);
    screenCount++;
    //printf("screen managerrrrrrrrrrr %d: %s\n", screenCount, item);
    (screen)->currentCapacity++;
    pthread_mutex_unlock(&(screen)->mutex);
    sem_post(&(screen)->full);

    // Free the allocated memory
  //  free(item);
}

void screenManger(CoEditor *coEditor) {
    while (1) {
        char *item = removeCoEditor(coEditor);
        ///TODO CHANGED HERE FROM BREAK TO CONTINUE
        if (strlen(item) == 0)
            continue;
        if (strcmp(item, "DONE") == 0) {
            free(item);
            break;
        }
        usleep(100 * 1000);
        insertScreen(item);
        free(item);
    }
    insertScreen("DONE");
}

void createScreenManger(int screenCapacity) {
    screen = malloc(sizeof(Screen));
    screen->currentCapacity = 0;
    screen->maxCapacity = screenCapacity;
    // Allocate memory for the queue array
    screen->queue = (char **) malloc(screenCapacity * sizeof(char *));
    // Initialize the semaphores
    sem_init(&(screen->full), 0, 0);
    sem_init(&(screen->empty), 0, screenCapacity);
    // Initialize the mutex
    pthread_mutex_init(&(screen->mutex), NULL);
}

char *removeScreen() {
        sem_wait(&(screen)->full);
        pthread_mutex_lock(&(screen)->mutex);
        char *story = screen->queue[0];
        for (int i = 0; i < screen->maxCapacity - 1; i++) {
            screen->queue[i] = screen->queue[i + 1];
        }
    screen->queue[screen->maxCapacity -1] = NULL;
    screen->currentCapacity--;
        pthread_mutex_unlock(&(screen)->mutex);
        sem_post(&(screen)->empty);
        return story;

}

void printToScreen() {
    {
        int doneCounter = 0;
        while (doneCounter != 3) {
            char *item = removeScreen();
            if (item == NULL || strlen(item) == 0) {
                continue;
            }
            if (strcmp(item, "DONE") == 0) {
                doneCounter++;
                 free(item);
                continue;
            }
            lineCounter3++;
            printf("%d :%s\n",lineCounter3, item);
            free(item);
        }
        printf("DONE\n");
    }
}

void freeGlobals() {
    // Free memory for producers
    if (producers != NULL) {
        for (int i = 0; i < producerCount; i++) {
            Producer *producer = &producers[i];
            if (producer->queue != NULL) {
                for (int j = 0; j < producer->queue_size; j++) {
                    free(producer->queue[j]);
                }
                free(producer->queue);
            }
            sem_destroy(&producer->mutex);
            sem_destroy(&producer->items);
            sem_destroy(&producer->slots);
        }
        free(producers);
    }

    // Free memory for coEditors
    if (coEditors != NULL) {
        for (int i = 0; i < screen->maxCapacity; i++) {
            CoEditor *coEditor = &coEditors[i];
            if (coEditor->queue != NULL) {
                for (int j = 0; j < coEditor->size; j++) {
                    free(coEditor->queue[j]);
                }
                free(coEditor->queue);
            }
            sem_destroy(&coEditor->mutex);
        }
        free(coEditors);
    }

    // Free memory for screen
    if (screen != NULL) {
        if (screen->queue != NULL) {
            for (int i = 0; i < screen->maxCapacity; i++) {
                free(screen->queue[i]);
            }
            free(screen->queue);
        }
        sem_destroy(&screen->full);
        sem_destroy(&screen->empty);
        pthread_mutex_destroy(&screen->mutex);
    }
}

void freeMemory() {
    // Free memory for producers
    for (int i = 0; i < producerCount; i++) {
        sem_destroy(&producers[i].items);
        sem_destroy(&producers[i].slots);
        sem_destroy(&producers[i].mutex);
        for (int j = 0; j < producers[i].queue_size; j++) {
            free(producers[i].queue[j]);
        }
        free(producers[i].queue);
    }
    free(producers);

    // Free memory for coEditors
    for (int i = 0; i < 3; i++) {
        sem_destroy(&coEditors[i].mutex);
        for (int j = 0; j < coEditors[i].size; j++) {
            free(coEditors[i].queue[j]);
        }
        free(coEditors[i].queue);
    }
    free(coEditors);

    // Free memory for screenManager
    pthread_mutex_destroy(&screen->mutex);
    sem_destroy(&screen->full);
    sem_destroy(&screen->empty);
    for (int i = 0; i < screen->currentCapacity; i++) {
        free(screen->queue[i]);
    }
    free(screen->queue);
    free(screen);
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
    int screenCapacity = readConfFile(file);
    //create threads by number of producer + 3 co-Editor + dispatcher
    pthread_t threads[producerCount + 4];
    runThreads_Producers(threads);
    createCoEditor();
    pthread_create(&threads[producerCount], NULL, (void *) dispatcher, NULL);
    createScreenManger(screenCapacity);
    pthread_create(&threads[producerCount + 1], NULL, (void *) screenManger, &coEditors[0]);

    pthread_create(&threads[producerCount + 2], NULL, (void *) screenManger, &coEditors[1]);
    pthread_create(&threads[producerCount + 3], NULL, (void *) screenManger, &coEditors[2]);

    printToScreen();
    for (int i = 0; i < producerCount + 4; i++) {
        pthread_join(threads[i], NULL);
    }
    freeMemory();

//    screenManger(&coEditors[0]);
//    screenManger(&coEditors[1]);
//    screenManger(&coEditors[2]);
//    for (int i = 0; i < coEditors[0].size; i++) {
//      printf("%d: %s\n", i+1, coEditors[0].queue[i]);
//    }
//    for (int j = 0; j< 3 ; j++) {
//    for (int i = 0; i < coEditors[j].size; i++) {
//        printf("%d: %s\n", i+1, coEditors->queue[i]);
//    }
//    }
//    char *item;
//    while ((item = removeItem(&coEditor[0])) != NULL) {
//        printf("Item removed: %s\n", item);
//        //free(item);  // Free the memory allocated for the item
//    }

//    for (int i = 0; i < producerCount; i++) {
//        sem_destroy(&producers[i].slots);
//        sem_destroy(&producers[i].items);
//        sem_destroy(&producers[i].mutex);
//        //free(producers[i].queue);
//    }
    //freeGlobals();
    fclose(file);
    return 0;
}
