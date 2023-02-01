#include "client.h"
#include "kclient.h"


int main(int argc, char *argv[])
{

    if (argc != 2) {
        printf("[Error]args:Please input -u or -k\n");
        return 0;
    }
    if (strcmp(argv[1], "-u") == 0) {
        client_run_socket();
    }
    else if (strcmp(argv[1], "-k") == 0) {
        kclient_run_socket();
    }
    else {
        printf("[error]argv:Please input -u or -k\n");
    }
    return 0;
}