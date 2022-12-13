#include <iostream>
#include <pthread.h>
#include <unistd.h>
#include <semaphore.h>
#include "queue"



sem_t barber_is_free;
sem_t customer_is_waiting;

std::queue<int> queue;


int customers;




void *barber_function(void *arg) {
    int servedCustomerCount = 0;
    while (servedCustomerCount != customers) {
        if (queue.empty()) {
            printf("Barber: Zzzz...\n");
        }
        sem_wait(&customer_is_waiting);
        sem_post(&barber_is_free);

        printf("Barber: customer %d is being served\n", queue.front());
        usleep(rand() % 5 * 1000000);

        servedCustomerCount++;
        queue.pop();
    }

    pthread_exit(nullptr);
}

void *customer_function(void *arg) {
    int client_number = *((int *) arg);

    printf("Customer %d: Can i get a haircut?\n", client_number);
    if (queue.front() != client_number) printf("Customer %d: Zzzz...\n", client_number);
    while (queue.front() != client_number);
    sem_post(&customer_is_waiting);
    sem_wait(&barber_is_free);
    while (queue.front() == client_number);

    printf("Customer %d: Thank you! \n", client_number);

    pthread_exit(nullptr);
}


void *make_customer_function(void *arg) {
    int *clientNumbers = new int[customers];
    for (int i = 0; i < customers; ++i) {
        clientNumbers[i] = i + 1;
        pthread_t customer_thread;
        pthread_create(&customer_thread, nullptr, customer_function, &clientNumbers[i]);
        queue.push(i + 1);
        usleep(rand() % 5 * 1000000);
    }
    pthread_exit(nullptr);
}

int main() {
    srand(time(nullptr));

    std::cin >> customers;

    queue = std::queue<int>();
    pthread_t barber;
    pthread_t customer_maker;


    sem_init(&customer_is_waiting, 0, 0);
    sem_init(&barber_is_free, 0, 0);

    pthread_create(&barber, nullptr, barber_function, nullptr);

    pthread_create(&customer_maker, nullptr, make_customer_function, nullptr);

    pthread_join(barber, nullptr);
    pthread_join(customer_maker, nullptr);
}
