//Guy Reuveni 206398596
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include <pthread.h>
#include <semaphore.h>
#include <unistd.h>

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
    // Index of the front element in the queue
    int front;
    // Index of the rear element in the queue
    int rear;
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

/**
 * The function inserts an item into a queue maintained by a producer, using semaphores to ensure synchronization.
 *
 * @param producer A pointer to a struct representing the producer, which contains information about the queue and
 * synchronization primitives used to coordinate access to the queue.
 * @param item A pointer to the item that needs to be inserted into the queue.
 */
void insertItem(Producer *producer, char *item) {
    // Wait for an available slot in the queue
    sem_wait(&producer->slots);
    // Acquire the mutex to access the queue
    sem_wait(&producer->mutex);
    // Insert the item into the queue
    producer->queue[producer->rear] = item;
    producer->rear = (producer->rear + 1) % producer->queue_size;
    sem_post(&producer->mutex);  // Release the mutex
    sem_post(&producer->items);  // Signal that an item has been inserted
}

/**
 * This function removes an item from a queue maintained by a producer and returns the removed item.
 *
 * @param producer A pointer to a struct representing the producer, which contains information about the queue and
 * synchronization primitives used to manage the queue.
 *
 * @return The function `removeItem` returns a pointer to the item that was removed from the queue.
 */
char *removeItem(Producer *producer) {
    char *item;
    sem_wait(&producer->items);  // Wait for an item in the queue
    sem_wait(&producer->mutex);  // Acquire the mutex to access the queue

    // Remove the item from the queue
    item = producer->queue[producer->front];
    producer->front = (producer->front + 1) % producer->queue_size;
    sem_post(&producer->mutex);  // Release the mutex
    sem_post(&producer->slots);  // Signal that a slot is available
    return item;
}


/**
 * The function counts the number of digits in a given integer.
 *
 * @param num The input integer for which we want to count the number of digits.
 *
 * @return The function `countDigitsOfNum` returns the number of digits in the input integer `num`. If `num` is 0, it
 * returns 1 as a special case.
 */
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


/**
 * The function creates a new producer with specified attributes and initializes semaphores and memory allocation.
 *
 * @param producer_id An integer representing the unique identifier for the producer being created.
 * @param product_num The number of products that the producer will produce.
 * @param queue_size The size of the queue that the producer will use to store the products it produces.
 *
 * @return The function `createProducer` is returning a pointer to a `Producer` struct.
 */
Producer *createProducer(int producer_id, int product_num, int queue_size) {
    Producer *producer = malloc(sizeof(*producer));
    if (producer == NULL)
        exit(1);
    producer->id = producer_id-1;
    producer->product_num = product_num;
    producer->queue_size = queue_size;
    producer->sports = 0;
    producer->news = 0;
    producer->weather = 0;
    producer->front = 0;
    producer->rear = 0;
    // Initialize items semaphore to 0 (unavailable)
    sem_init(&producer->mutex, 0, 1);
    // Initialize mutex semaphore to 1 (available)
    sem_init(&producer->items, 0, 0);
    // Initialize slots semaphore to the queue size
    sem_init(&producer->slots, 0, queue_size);
    producers = realloc(producers, (producerCount + 1) * sizeof(Producer));
    if (producers == NULL)
        exit(1);
    producer->queue = malloc(producer->queue_size * sizeof(char *));
    if (producer->queue == NULL)
        exit(1);
    return producer;
}

/**
 * The function reads a configuration file, creates producers based on the file's contents, and prints their information.
 *
 * @param file A pointer to a file that contains configuration information for the program.
 *
 * @return the value of `coEditorCapacity`, which is the integer value read from the configuration file.
 */
int readConfFile(FILE *file) {
    int producer_args, argOne, argTwo, argThree;
    while ((producer_args = fscanf(file, "%d\n%d\n%d\n\n", &argOne, &argTwo, &argThree)) && producer_args == 3) {
        Producer *producer = createProducer(argOne, argTwo, argThree);
        producers[producerCount] = *producer;
        producerCount++;
        free(producer);
    }
    int coEditorCapacity = argOne;
    return coEditorCapacity;
}

/**
 * The function creates a product for a given producer by generating a random number and allocating memory for a string
 * based on the random number.
 *
 * @param producer A pointer to a struct of type Producer, which contains information about the producer and their
 * products.
 */
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
                if (s == NULL)
                    exit(1);
                sprintf(s, "Producer %d %s %d", producer->id, getProductType(randomNumber), producer->sports);
                producer->sports++;
                break;
            case 1:
                s = malloc(strlen("producer ") + countDigitsOfNum(producer->id) + strlen(" ") + strlen("NEWS") +
                           strlen(" ") + countDigitsOfNum(producer->news) +
                           1); // Allocate memory for the string
                if (s == NULL)
                    exit(1);
                sprintf(s, "Producer %d %s %d", producer->id, getProductType(randomNumber), producer->news);
                producer->news++;
                break;
            case 2:
                s = malloc(strlen("producer ") + countDigitsOfNum(producer->id) + strlen(" ") + strlen("WEATHER") +
                           strlen(" ") + countDigitsOfNum(producer->weather) +
                           1); // Allocate memory for the string
                if (s == NULL)
                    exit(1);
                sprintf(s, "Producer %d %s %d", producer->id, getProductType(randomNumber), producer->weather);
                producer->weather++;
                break;
            default:
                break;
        }
        insertItem(producer, s);  // Insert the item into the queue
    }
    insertItem(producer, "DONE");  // Insert the DONE string into the queue
}


/**
 * This function creates threads for each producer and calls the createProduct function for each thread.
 *
 * @param threads An array of pthread_t data type that will store the thread IDs of the created threads.
 */
void runThreads_Producers(pthread_t threads[]) {
    for (int i = 0; i < producerCount; i++)
        pthread_create(&threads[i], NULL, (void *) createProduct, &producers[i]);
}

void createCoEditor() {
/**
 * The function creates an array of three CoEditor pointers, initializes their mutex semaphores to 1, and sets their size
 * and queue values to 0 and NULL respectively.
 */
    // Allocate memory for the array of CoEditor pointers
    coEditors = malloc(3 * sizeof(CoEditor));
    if (coEditors == NULL) {
        exit(1);
    }
    for (int i = 0; i < 3; i++) {
        CoEditor *tempEditor = malloc(sizeof(CoEditor));
        if (tempEditor == NULL) {
            exit(1);
        }
        // Initialize mutex semaphore to 1 (available)
        sem_init(&tempEditor->mutex, 0, 1);
        tempEditor->size = 0;
        tempEditor->queue = NULL;
        coEditors[i] = *tempEditor;
        free(tempEditor);
    }
}

/**
 * The function inserts a new string into a queue in a thread-safe manner.
 *
 * @param coEditor a pointer to a CoEditor struct, which contains information about a collaborative editor, including a
 * queue of strings and a mutex lock for thread safety.
 * @param s s is a pointer to a character array (string) that represents the string to be added to the CoEditor's queue.
 */
void insertCoEditor(CoEditor *coEditor, char *s) {
    // Acquire the mutex lock to ensure thread safety
    sem_wait(&coEditor->mutex);
    // Increase the size of the queue
    coEditor->size++;
    // Reallocate memory for the updated queue size
    coEditor->queue = realloc(coEditor->queue, coEditor->size * sizeof(char *));
    if (coEditor->queue == NULL)
        exit(1);
    // Allocate memory for the new string in the queue
    coEditor->queue[coEditor->size - 1] = malloc(strlen(s) + 1);
    if (coEditor->queue[coEditor->size - 1] == NULL)
        exit(1);
    // Copy the string into the queue
    strcpy(coEditor->queue[coEditor->size - 1], s);
    // Release the mutex lock
    sem_post(&coEditor->mutex);

}

/**
 * This function removes the first element from a queue in a CoEditor struct and returns it as a string.
 *
 * @param coEditor CoEditor is a struct that contains a queue of strings and a mutex for synchronization. The function
 * `removeCoEditor` takes a pointer to a CoEditor struct as its parameter.
 *
 * @return The function `removeCoEditor` returns a pointer to a character array (string) that represents the first element
 * of the `queue` array in the `CoEditor` struct that is passed as an argument to the function.
 */
char *removeCoEditor(CoEditor *coEditor) {
    if (coEditor == NULL || coEditor->queue == NULL) {
        return "";
    }
    sem_wait(&coEditor->mutex);
    char *str = coEditor->queue[0];
    for (int i = 1; i < coEditor->size; i++) {
        coEditor->queue[i - 1] = coEditor->queue[i];
    }
    coEditor->queue[coEditor->size - 1] = NULL;
    coEditor->size--;
    coEditor->queue = (char **) realloc(coEditor->queue, (coEditor->size) * sizeof(char *));
    if (coEditor->size > 0)
        if (coEditor->queue == NULL)
            exit(1);
    sem_post(&coEditor->mutex);
    return str;
}

/**
 * The function dispatches messages from producers to co-editors based on their category.
 */
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
/**
 * This function inserts a string item into a queue in a thread-safe manner using semaphores and mutexes.
 *
 * @param item item is a pointer to a character array (string) that is being inserted into a queue in the insertScreen
 * function.
 */
}


void insertScreen(char *item) {
    sem_wait(&(screen)->empty);
    pthread_mutex_lock(&(screen)->mutex);
    (screen)->queue[(screen)->currentCapacity] = (char *) malloc(strlen(item) + 1);
    if ((screen)->queue[(screen)->currentCapacity] == NULL) {
        exit(1);
    }
    strcpy((screen)->queue[(screen)->currentCapacity], item);
    (screen)->currentCapacity++;
    pthread_mutex_unlock(&(screen)->mutex);
    sem_post(&(screen)->full);
}

/**
 * This function manages the screen by continuously removing items from a CoEditor and inserting them into the screen until
 * it receives the "DONE" signal.
 *
 * @param coEditor The parameter `coEditor` is a pointer to a `CoEditor` object, which is likely a data structure used for
 * coordinating concurrent editing of a document by multiple users.
 */
void screenManger(CoEditor *coEditor) {
    while (1) {
        char *item = removeCoEditor(coEditor);
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

/**
 * The function creates a screen manager with a given capacity and initializes its semaphores and mutex.
 *
 * @param screenCapacity The capacity of the screen manager, which determines the maximum number of screens that can be
 * stored in the queue at any given time.
 */
void createScreenManger(int screenCapacity) {
    screen = malloc(sizeof(Screen));
    if (screen == NULL)
        exit(0);
    screen->currentCapacity = 0;
    screen->maxCapacity = screenCapacity;
    // Allocate memory for the queue array
    screen->queue = (char **) malloc(screenCapacity * sizeof(char *));
    if (screen->queue == NULL)
        exit(0);
    // Initialize the semaphores
    sem_init(&(screen->full), 0, 0);
    sem_init(&(screen->empty), 0, screenCapacity);
    // Initialize the mutex
    pthread_mutex_init(&(screen->mutex), NULL);
}

/**
 * This function removes the first element from a queue in a thread-safe manner and returns it.
 *
 * @return The function `removeScreen()` returns a `char` pointer, which is the first element of the `queue` array in the
 * `screen` struct. This represents the oldest story that was added to the screen.
 */
char *removeScreen() {
    sem_wait(&(screen)->full);
    pthread_mutex_lock(&(screen)->mutex);
    char *story = screen->queue[0];
    for (int i = 0; i < screen->maxCapacity - 1; i++) {
        screen->queue[i] = screen->queue[i + 1];
    }
    screen->queue[screen->maxCapacity - 1] = NULL;
    screen->currentCapacity--;
    pthread_mutex_unlock(&(screen)->mutex);
    sem_post(&(screen)->empty);
    return story;

}

/**
 * The function prints items from the screen until it encounters "DONE" three times.
 */
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
            printf("%s\n", item);
            free(item);
        }
        printf("DONE\n");
    }
}


/**
 * The function frees memory allocated for producers, coEditors, and screenManager.
 */
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
        exit(1);
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
    fclose(file);
    return 0;
}