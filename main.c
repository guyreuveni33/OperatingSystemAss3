#include <stdio.h>
#include <stdlib.h>

typedef struct {
    int id;
    int queue_size;
    char **queue;
} Producer;

Producer *producers = NULL;

Producer createProducer(int producer_id,int queue_size,int count){
    Producer producer;
    producer.id = producer_id;
    producer.queue_size = queue_size;
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
    return producer;
}

int readConfFile(FILE *file) {
    int producer_args, argOne, argTwo, argThree, count = 0;
    while ((producer_args = fscanf(file, "%d\n%d\n%d\n\n", &argOne, &argTwo, &argThree)) && producer_args == 3) {
        Producer producer=createProducer(argOne,argThree,count);
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
    readConfFile(file);

    return 0;
}
