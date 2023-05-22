#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>

typedef struct {
    int sports;
    int news;
    int weather;
    int id;
    int queue_size;
    int product_num;
    char **queue;
} Producer;

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

void updateProductCount(Producer *producer, int randomNumber,char **s) {
    switch (randomNumber) {
        case 0:
            *s = malloc(strlen("producer ") + countDigitsOfNum(producer->id) + strlen(" ") + strlen("SPORTS") +
                       strlen(" ") + countDigitsOfNum(producer->sports) +
                       strlen("\0")); // Allocate memory for the string
            sprintf(*s, "Producer %d %s %d", producer->id, getProductType(randomNumber), producer->sports);
            producer->sports++;
            break;
        case 1:
            *s = malloc(strlen("producer ") + countDigitsOfNum(producer->id) + strlen(" ") + strlen("NEWS") +
                       strlen(" ") + countDigitsOfNum(producer->news) + strlen("\0")); // Allocate memory for the string
            sprintf(*s, "Producer %d %s %d", producer->id, getProductType(randomNumber), producer->news);
            producer->news++;
            break;
        case 2:
            *s = malloc(strlen("producer ") + countDigitsOfNum(producer->id) + strlen(" ") + strlen("WEATHER") +
                       strlen(" ") + countDigitsOfNum(producer->weather) +
                       strlen("\0")); // Allocate memory for the string
            sprintf(*s, "Producer %d %s %d", producer->id, getProductType(randomNumber), producer->weather);
            producer->weather++;
            break;
        default:
            break;
    }
}


Producer createProducer(int producer_id, int product_num, int queue_size, int count) {
    Producer producer;
    producer.id = producer_id;
    producer.product_num = product_num;
    producer.queue_size = queue_size;
    producer.sports = 0;
    producer.news = 0;
    producer.weather = 0;
    producers = realloc(producers, (count + 1) * sizeof(Producer));
    if (producers == NULL) {
        printf("Memory allocation failed.\n");
        //TODO return error
    }
    producer.queue = malloc(producer.queue_size * sizeof(char *));
    if (producer.queue == NULL) {
        printf("Memory allocation failed.\n");
        //TODO return error
    }
    // Generate a random number between 0 and 2
    char *productArray[producer.product_num];
    char *s= NULL; ;
    int counter = 0;
    for (int i = 0; i < producer.product_num; ++i) {
        int randomNumber = rand() % 3;
        updateProductCount(&producer, randomNumber,&s);
        productArray[i]=s;
        counter++;
        printf("%s\n",s);
    }
    return producer;
}

int readConfFile(FILE *file) {
    int producer_args, argOne, argTwo, argThree, count = 0;
    while ((producer_args = fscanf(file, "%d\n%d\n%d\n\n", &argOne, &argTwo, &argThree)) && producer_args == 3) {
        Producer producer = createProducer(argOne, argTwo, argThree, count);
        producers[count] = producer;
        count++;
    }
    for (int i = 0; i < count; i++) {
        printf("Producer ID: %d\n", producers[i].id);
        printf("Queue Size: %d\n", producers[i].queue_size);
        printf("-------------\n");
    }
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
    readConfFile(file);
    return 0;
}
