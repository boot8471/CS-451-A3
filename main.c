#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <pthread.h>
#include <semaphore.h>

#define MAX_STUDENTS_PER_TAXI 4

sem_t taxi_waiting;
sem_t student_ready;
sem_t mutex;

int waiting_students = 0;
int *student_queue = NULL;
int front = 0;
int rear = 0;

int total_students;
int total_taxis;
int party_time;

void* student_thread(void* id) {
    int student_id = (int)(long)id;

    int party_duration = rand() % party_time + 1;
    printf("Student %d: I am at the party. It is way more fun than what I expected...\n", student_id);
    sleep(party_duration);

    printf("Student %d: I am done partying and waiting for a taxi.\n", student_id);


    sem_wait(&mutex);
    student_queue[rear++] = student_id;  // adding students to the queue
    waiting_students++;
    printf("Student %d arrived at the curb. Total students waiting: %d\n", student_id, waiting_students);

    if (waiting_students == MAX_STUDENTS_PER_TAXI) {
        sem_post(&taxi_waiting);
    }
    sem_post(&mutex);
    sem_wait(&student_ready);
    //printf("Student %d: Caught a taxi!\n", student_id);

    pthread_exit(NULL);
}

void* taxi_thread(void* id)
 {
    int taxi_id = (int)(long)id;
    int taxi_sleep = rand() % party_time + 1;

    while (1)
      {
        // Taxi arrives and waits for students
        printf("Taxi %d: I arrived at the curb.\n", taxi_id);

        sem_wait(&taxi_waiting);
        sem_wait(&mutex);

       // sleep(taxi_sleep); //optional sleep to test if it would still pick the first 4 students that arrived if 5 were waiting

        printf("Taxi %d: I have students ", taxi_id);
        for (int i = 0; i < MAX_STUDENTS_PER_TAXI; i++)
          {
            int student_id = student_queue[front++];
            printf("%d", student_id);

            if (i < MAX_STUDENTS_PER_TAXI - 1)
            {
                printf(", ");
            }

            sem_post(&student_ready);
        }

        waiting_students -= MAX_STUDENTS_PER_TAXI;
        printf(". Time to drive....BYE\n");
        sem_post(&mutex);

        break;
    }

    pthread_exit(NULL);
}

int main(int argc, char* argv[]) {

    for (int i = 1; i < argc; i++)
      {
        if (argv[i][0] == '-' && argv[i][1] == 's')
            total_students = atoi(argv[++i]);

        if (argv[i][0] == '-' && argv[i][1] == 't')
            total_taxis = atoi(argv[++i]);

        if (argv[i][0] == '-' && argv[i][1] == 'm')
            party_time = atoi(argv[++i]);
      }

    // Validate inputs
    if (total_students != total_taxis * MAX_STUDENTS_PER_TAXI || total_students == 0 || total_taxis == 0)
      {
        fprintf(stderr, "Invalid number of students or taxis. Please try again!\n");
        exit(EXIT_FAILURE);
    }

    // Dynamically allocate memory for the student queue
    student_queue = (int *)malloc(total_students * sizeof(int));
    if (student_queue == NULL)
      {
        fprintf(stderr, "Memory allocation failed!\n");
        exit(EXIT_FAILURE);
      }


    sem_init(&taxi_waiting, 0, 0);
    sem_init(&student_ready, 0, 0);
    sem_init(&mutex, 0, 1);

    srand(time(NULL));

    //Creating both threads
    pthread_t student_threads[total_students];
    pthread_t taxi_threads[total_taxis];

    for (int i = 0; i < total_students; i++)
        pthread_create(&student_threads[i], NULL, student_thread, (void*)(long)i);

    for (int i = 0; i < total_taxis; i++)
      {
        pthread_create(&taxi_threads[i], NULL, taxi_thread, (void*)(long)i);
        pthread_join(taxi_threads[i], NULL); // Ensure taxi leaves before the next arrives
      }

    //waiting for threads to finish
    for (int i = 0; i < total_students; i++)
        pthread_join(student_threads[i], NULL);

    //Clears the queue and then deletes the semaphores.
    free(student_queue);
    sem_destroy(&taxi_waiting);
    sem_destroy(&student_ready);
    sem_destroy(&mutex);

    return 0;
}
