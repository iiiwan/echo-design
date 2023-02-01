#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>

static pthread_mutex_t g_mutex_lock;
static int g_count = 0;

static void *thread_fun_1(void *data)
{
    while (1)
    {
        pthread_mutex_lock(&g_mutex_lock);
        sleep(2);
        g_count++;
        printf("%s g_count: %d\n", __func__, g_count);
        pthread_mutex_unlock(&g_mutex_lock);
    }
    
    
}

static void *thread_fun_2(void *data)
{
    while (1)
    {
        pthread_mutex_lock(&g_mutex_lock);
        sleep(2);
        g_count++;
        printf("%s g_count: %d\n", __func__, g_count);
        pthread_mutex_unlock(&g_mutex_lock);
    }
}

static void *thread_fun_3(void *data)
{
    while (1)
    {
        pthread_mutex_lock(&g_mutex_lock);
        sleep(2);
        g_count++;
        printf("%s g_count: %d\n", __func__, g_count);
        pthread_mutex_unlock(&g_mutex_lock);
        sleep(1);
    }
}

int main(int argc, char const *argv[])
{
    int ret;
    pthread_t pid[3];

    ret = pthread_mutex_init(&g_mutex_lock, NULL);
    if (ret != 0) {
        printf("mutex init failed\n");
        return -1;
    }

    pthread_create(&pid[0], NULL, thread_fun_1, NULL);
    pthread_create(&pid[1], NULL, thread_fun_2, NULL);
    pthread_create(&pid[2], NULL, thread_fun_3, NULL);

    pthread_join(pid[0], NULL);
    pthread_join(pid[1], NULL);
    pthread_join(pid[2], NULL);

    pthread_mutex_destroy(&g_mutex_lock);

    return 0;
}
