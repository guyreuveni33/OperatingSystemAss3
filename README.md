# OperatingSystemAss3

## Code Overview

The C code simulates a news broadcasting system with concurrent producers, dispatchers, co-editors, and a screen manager. It aims to demonstrate concurrent programming and synchronization concepts.

### Components

1. **Producers**: Producers generate strings representing news stories with specific formats and categories (SPORTS, NEWS, WEATHER). These stories are placed in producer-specific queues.

2. **Dispatcher**: The dispatcher receives messages from producer queues and categorizes them into SPORTS, NEWS, or WEATHER queues. It operates using a Round Robin algorithm and signals when all producers are done.

3. **Co-Editors**: Each category (SPORTS, NEWS, WEATHER) has a co-editor. Co-editors receive messages from the dispatcher, simulate editing by blocking briefly, and pass messages to a shared queue for the screen manager.

4. **Screen Manager**: The screen manager displays messages received from co-editors in the order they arrive. After displaying all messages and receiving three "DONE" messages, it displays a "DONE" statement.

### Bounded Buffer

The code implements bounded buffers for producer and co-editor queues. These buffers support `insert` and `remove` operations. Producers use these buffers to store and pass messages to the dispatcher, while co-editors use them to share messages with the screen manager.

### Configuration File

The assignment reads a configuration file to determine the number of producers, the number of products each producer generates, and the queue size. This file format helps configure the system for different scenarios.

## Key Functions

1. `insertItem` and `removeItem`: These functions insert and remove items from a producer's queue, ensuring thread-safe access using semaphores.

2. `createProducer`: Creates a producer with specified attributes, including the queue size and the number of products to produce.

3. `createProduct`: Generates random news stories for a producer, formats them according to the assignment, and inserts them into the producer's queue.

4. `insertCoEditor` and `removeCoEditor`: These functions handle the insertion and removal of messages in co-editor queues, ensuring thread safety.

5. `dispatcher`: This function categorizes and dispatches messages to co-editor queues based on their content.

6. `insertScreen` and `removeScreen`: These functions manage the screen manager's queue, allowing thread-safe insertion and removal of messages.

7. `printToScreen`: Continuously displays messages on the screen manager until it receives three "DONE" messages.

## How to Use the Code

1. Open your terminal or command prompt.

2. Navigate to the directory where you want to clone the repository.

3. Run the following command to clone the repository to your local machine:
git clone https://github.com/guyreuveni33/OperatingSystemAss3.git

4.Use the cd command to access the cloned directory

5.Compile the Code:
gcc main.c -lpthread

6. Execute the program with a configuration file as a command-line argument: `./a.out confFileExample.txt`.

7. The program will read the configuration, create producer threads, co-editor threads, a dispatcher thread, and a screen manager thread.

8. Producers will generate news stories and send them to the dispatcher via bounded buffers.

8. The dispatcher will categorize messages and pass them to the appropriate co-editors.

9. Co-editors will simulate editing and send messages to the screen manager.

10. The screen manager will display the news and terminate when it receives three "DONE" messages.
