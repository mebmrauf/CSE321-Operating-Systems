#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <semaphore.h>
#include <unistd.h>
#include <time.h>

#define totalStudent 10
#define waitingChair 3

struct Queue {
    int studentID[waitingChair];
    int frontIndex;
    int rearIndex;
    int currentEl;
};

pthread_mutex_t waitingList;
pthread_mutex_t statsIO;
sem_t availableStudent;
sem_t availableST;

struct Queue waitingStudents;

int currentlyWaiting = 0;
int servedStudent = 0;
int leftStudent = 0;
int stIDs[totalStudent];

void queueInitialization(struct Queue *qu) {
    (*qu).frontIndex = 0;
    (*qu).rearIndex  = 0;
    (*qu).currentEl  = 0;
}

void queueEnque(struct Queue *qu, int studentID) {
    if ((*qu).currentEl == waitingChair) return;
    (*qu).studentID[(*qu).rearIndex] = studentID;
    (*qu).rearIndex = ((*qu).rearIndex + 1) % waitingChair;
    (*qu).currentEl++;
}

int queueDeque(struct Queue *qu) {
    if ((*qu).currentEl == 0) return -1;
    int studentID = (*qu).studentID[(*qu).frontIndex];
    (*qu).frontIndex = ((*qu).frontIndex + 1) % waitingChair;
    (*qu).currentEl--;
    return studentID;
}

void *stWork(void *arg) {
    (void)arg;

    while (1) {
        sem_wait(&availableStudent);
        pthread_mutex_lock(&waitingList);

        if (waitingStudents.currentEl == 0 && servedStudent >= totalStudent) {
            pthread_mutex_unlock(&waitingList);
            break;
        }
        if (waitingStudents.currentEl == 0) {
            pthread_mutex_unlock(&waitingList);
            continue;
        }

        int studentID = queueDeque(&waitingStudents);
        if (studentID >= 0) {
            currentlyWaiting--;
            printf("A waiting student started getting consultation\n");
            printf("Number of students now waiting: %d\n", currentlyWaiting);
            printf("ST giving consultation\n");
            printf("Student %d is getting consultation\n", studentID);
        }
        pthread_mutex_unlock(&waitingList);

        if (studentID >= 0) {
            sleep(1 + rand() % 2);
            pthread_mutex_lock(&statsIO);
            printf("Student %d finished getting consultation and left\n", studentID);
            pthread_mutex_lock(&waitingList);
            servedStudent++;
            int processedNow = servedStudent;
            int stillWaiting = waitingStudents.currentEl;
            pthread_mutex_unlock(&waitingList);
            printf("Number of served students: %d\n", processedNow);
            pthread_mutex_unlock(&statsIO);
            sem_post(&availableST);
            if (processedNow >= totalStudent && stillWaiting == 0) {
                break;
            }
        }
    }
    return NULL;
}

void *stStudent(void *arg) {
    int studentID = *(int*)arg;
    sleep(rand() % 3);

    pthread_mutex_lock(&waitingList);
    if (waitingStudents.currentEl < waitingChair) {
        queueEnque(&waitingStudents, studentID);
        currentlyWaiting++;
        printf("Student %d started waiting for consultation\n", studentID);
        pthread_mutex_unlock(&waitingList);
        sem_post(&availableStudent);
        sem_wait(&availableST);
    } else {
        pthread_mutex_unlock(&waitingList);
        pthread_mutex_lock(&statsIO);
        printf("No chairs remaining in lobby. Student %d Leaving.....\n", studentID);
        printf("Student %d finished getting consultation and left\n", studentID);
        pthread_mutex_lock(&waitingList);
        leftStudent++;
        servedStudent++;
        int processedNow = servedStudent;
        pthread_mutex_unlock(&waitingList);
        printf("Number of served students: %d\n", processedNow);
        pthread_mutex_unlock(&statsIO);
        sem_post(&availableStudent);
    }
    return NULL;
}

int main(void) {
    queueInitialization(&waitingStudents);
    pthread_mutex_init(&waitingList, NULL);
    pthread_mutex_init(&statsIO, NULL);
    sem_init(&availableStudent, 0, 0);
    sem_init(&availableST, 0, 0);

    pthread_t threadST;
    pthread_t threadStudent[totalStudent];

    for (int ix = 0; ix < totalStudent; ix++) {
        stIDs[ix] = ix;
    }

    pthread_create(&threadST, NULL, stWork, NULL);

    for (int ix = 0; ix < totalStudent; ix++) {
        pthread_create(&threadStudent[ix], NULL, stStudent, &stIDs[ix]);
    }

    for (int ix = 0; ix < totalStudent; ix++) {
        pthread_join(threadStudent[ix], NULL);
    }
    pthread_join(threadST, NULL);
    sem_destroy(&availableStudent);
    sem_destroy(&availableST);
    pthread_mutex_destroy(&statsIO);
    pthread_mutex_destroy(&waitingList);

    return 0;
}