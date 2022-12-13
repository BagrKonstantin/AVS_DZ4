#include <iostream>
#include <unistd.h>
#include <pthread.h>

pthread_mutex_t can_take_nex_from_queue;
pthread_mutex_t can_cut;

int customers_amount;
int customers_in_queue = 0;
int should_wait;

bool isExit = true;
bool isWait = false;
bool was_cut = false;


void *hairdresser(void *n) {
    pthread_mutex_unlock(&can_cut);
    while (customers_amount > 0) {
        while (isExit && !isWait && customers_in_queue <= 0) {
            std::cout << "hairdresser: Zzzz... waiting..\n";
            isExit = false;
            while (!isWait);
        }
        while (customers_in_queue > 0) {
            was_cut = true;
            std::cout << "hairdresser: someone came. cutting...\n";
            sleep(2);
            --customers_in_queue;
            --customers_amount;
            was_cut = false;
        }
        isWait = false;
    }
    pthread_mutex_lock(&can_cut);
    return nullptr;
}

void *customer(void *num) {
    int number = *((int *) num);
    pthread_mutex_lock(&can_take_nex_from_queue);
    sleep(should_wait == 0 ? 0 : 4);
    ++customers_in_queue;
    std::cout << "customer " << number << ": Can i get a haircut?\n";
    was_cut = true;
    isWait = true;
    pthread_mutex_unlock(&can_take_nex_from_queue);
    pthread_mutex_lock(&can_cut);
    while (was_cut);
    std::cout << "customer " << number <<": Good bye.\n";
    isExit = true;
    pthread_mutex_unlock(&can_cut);
    return nullptr;
}

int main() {
    std::cin >> customers_amount;
    std::cin >> should_wait;

    pthread_mutex_init(&can_take_nex_from_queue, nullptr);
    pthread_mutex_init(&can_cut, nullptr);

    pthread_t p_dresser;
    pthread_t p_arr[customers_amount];
    int c_arr[customers_amount];

    pthread_create(&p_dresser, nullptr, hairdresser, nullptr);
    for (int i = 0; i < customers_amount; ++i) {
        c_arr[i] = i + 1;
        pthread_create(&p_arr[i], nullptr, customer, (void *) (c_arr + i));
    }
    for (unsigned long i : p_arr)
        pthread_join(i, nullptr);

    return 0;
}
