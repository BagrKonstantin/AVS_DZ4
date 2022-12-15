#include <iostream>
#include <pthread.h>
#include <unistd.h>
#include <semaphore.h>
#include "queue"


sem_t barber_is_free;       //симафор для блокировки парикмахера
sem_t customer_is_waiting;  // симафор для блокировки пользователей

pthread_mutex_t is_writing;        // мьютекс для того что бы обезопасить ввывод

std::queue<int> queue;      // очередь клиентов

int customers;              // кол-во клиентов
int max_cut_time;
int max_people_time;


bool is_file = false;
FILE *input;
FILE *output;


void print(const char *str, int val) {
    pthread_mutex_lock(&is_writing);
    if (is_file) {
        if (val == 0) {
            fprintf(output, str);
        } else {
            fprintf(output, str, val );
        }
    } else {
        if (val == 0) {
            printf(str);
        } else {
            printf(str, val);
        }
    }
    pthread_mutex_unlock(&is_writing);

}


void *barber_function(void *arg) {
    int servedCustomerCount = 0;
    while (servedCustomerCount != customers) {
        if (queue.empty()) {                    // если клиентов нет, то парикмахер засыпает
            print("Barber: Zzzz...\n", 0);
        }
        sem_post(&barber_is_free);              // сообщаем что парикмахер свободен
        sem_wait(&customer_is_waiting);         // ждёт клиента


        print("Barber: customer %d is being served\n", queue.front()); // говорим что стрижём
        usleep(rand() % max_cut_time * 1000000);   // стрижка

        servedCustomerCount++;
        queue.pop();                    // удаляем клиента из очереди
    }

    pthread_exit(nullptr);
}

void *customer_function(void *arg) {
    int client_number = *((int *) arg);

    print("Customer %d: Can i get a haircut?\n", client_number);   // говорим что хотим стрижку
    if (queue.front() != client_number) print("Customer %d: Zzzz...\n", client_number); // засыпает если не первый в очереди
    while (queue.front() != client_number); // ждём своей очереди
    sem_post(&customer_is_waiting);         // сообщаем, что готовы стричся
    sem_wait(&barber_is_free);              // ждём парикмахера
    while (queue.front() == client_number); // на windows без этой строчки вывод очень сильно путается. На линуксе её можно закомментить.

    print("Customer %d: Thank you! \n", client_number);    // говорим спасибо что подстригли

    pthread_exit(nullptr);
}


void *make_customer_function(void *arg) {// имитируем очередь
    int *clientNumbers = new int[customers];
    for (int i = 0; i < customers; ++i) {
        clientNumbers[i] = i + 1;
        pthread_t customer_thread;
        pthread_create(&customer_thread, nullptr, customer_function, &clientNumbers[i]);       // создание потока клиента
        queue.push(i + 1);
        usleep(rand() % max_people_time * 1000000);
    }
    pthread_exit(nullptr);
}





int main(int argc, char * argv[]) {
    if (argc == 1) {
        std::cin >> customers >> max_cut_time >> max_people_time;
    } else if (argc == 4) {
        customers = std::stoi(argv[1]);
        max_cut_time = std::stoi(argv[2]);
        max_people_time = std::stoi(argv[3]);
    } else {
        is_file = true;
        input = fopen(argv[1], "r");
        output = fopen(argv[2], "w");
        fscanf(input, "%d", &customers);
        fscanf(input, "%d", &max_cut_time);
        fscanf(input, "%d", &max_people_time);
        fclose(input);
    }


    srand(time(nullptr));


    queue = std::queue<int>();
    pthread_t barber;
    pthread_t customer_maker;


    sem_init(&customer_is_waiting, 0, 0);
    sem_init(&barber_is_free, 0, 0);

    pthread_mutex_init(&is_writing, nullptr);

    pthread_create(&barber, nullptr, barber_function, nullptr); // создание потока парикмахера

    pthread_create(&customer_maker, nullptr, make_customer_function, nullptr);// создание потока создающего очередь

    pthread_join(barber, nullptr);
    pthread_join(customer_maker, nullptr);

    fclose(output);
}
