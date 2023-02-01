#include <stdlib.h>  
#include <stdio.h>  
#include <stddef.h>  
#include <sys/socket.h>  
#include <sys/un.h>  
#include <errno.h>  
#include <string.h>  
#include <unistd.h>  
#include <ctype.h>   
#include <pthread.h>

#define MAXLINE 80

//define global vlaue
struct thread_client{
	struct sockaddr_un caddr;
	int fdc;
};
char *socket_path = "/home/johnson/echo_design/domain_socket/server.socket";  
static int num_client = 0;
int listenfd;
const int max_client = 10;
int client_mode[20];
static int process_num = 0;
static pthread_mutex_t id_mutex_lock;

//define function
void init_client_mode();
int get_client_ID();
void mode_analyse(char *string,int ID);
void process_data(char *string , int len, int mode);
void* run_client(void* arg);
void* server_control(void* arg);


int main(void)  
{  
    struct sockaddr_un serun;  
    int size,connfd;  

    //init client mode
    init_client_mode();

    //create
    if ((listenfd = socket(AF_UNIX, SOCK_STREAM, 0)) < 0) {  
        perror("socket error");  
        exit(1);  
    } 

    //bind
    memset(&serun, 0, sizeof(serun));  
    serun.sun_family = AF_UNIX;  
    strcpy(serun.sun_path, socket_path);  
    size = offsetof(struct sockaddr_un, sun_path) + strlen(serun.sun_path);  
    unlink(socket_path);  
    if (bind(listenfd, (struct sockaddr *)&serun, size) < 0) {  
        perror("bind error");  
        exit(1);  
    }  
    printf("UNIX domain socket bound\n");  
    
    //listen
    if (listen(listenfd, 20) < 0) {  
        perror("listen error");  
        exit(1);          
    }  
    printf("Accepting connections ...\n");  
    
    //Server thread
    pthread_t thread_server;
	pthread_create(&thread_server, NULL, server_control , NULL);

    //mutex lock
    int ret = pthread_mutex_init(&id_mutex_lock, NULL);
    if (ret != 0) {
        printf("mutex init failed\n");
        return -1;
    }

    while (1)
    {
        /* Accept */
        struct sockaddr_un cliun;
        socklen_t cliun_len;  
        cliun_len = sizeof(cliun);
        connfd = accept(listenfd, (struct sockaddr *)&cliun, &cliun_len);
        
        if (connfd < 0){
            perror("accept error");  
            exit(1);
        } 
        
        struct thread_client c;
		c.caddr = cliun;
        c.fdc = connfd;
        pthread_t thread_c;
		pthread_create(&thread_c, NULL, run_client, &c);
		pthread_detach(thread_c);
    }
    pthread_mutex_destroy(&id_mutex_lock);
    close(connfd);    
    close(listenfd);  
    return 0;  
}


void init_client_mode(){
    client_mode[0] = 0;
    for(int t=1;t<max_client+1;t++){
        client_mode[t] = -1;
    }
}

int get_client_ID(){
    for(int t = 1;t<max_client+1;t++){
        if(client_mode[t] == -1){
            client_mode[t] = client_mode[0];
            return t;
        }
    }
    return -1;
}

void mode_analyse(char *string,int ID){
    sleep(0.2);
    if(strncmp(string,"normal",6) == 0){
            client_mode[ID]= 0;
    }else if(strncmp(string,"upper",5) == 0){
            client_mode[ID] = 1;
    }else if(strncmp(string,"lower",5) == 0){
            client_mode[ID] = 2;
    }else{
        printf("Invalid input\n");
    }
} 

void process_data(char *string , int len, int mode){
    process_num = process_num + len;
    process_num--;
    if(mode == 0){
        return;
    }else if(mode == 1){
        for(int i = 0; i < len; i++) {  
            string[i] = toupper(string[i]);  
        } 
    }else if(mode == 2){
        for(int i = 0; i < len; i++) {  
            string[i] = tolower(string[i]);  
        } 
    }  
}

void* run_client(void* arg) {

    //Get id
    pthread_mutex_lock(&id_mutex_lock);
    int ID = get_client_ID();
    pthread_mutex_unlock(&id_mutex_lock);

	struct thread_client c = *(struct thread_client*)arg;
    char buf[MAXLINE]; 
	int fdc = c.fdc;
    num_client++;

    //Send ID message
    if(ID == -1){
        char *mesg = "Unable to establish a connection with the server";
        write(fdc, mesg, strlen(mesg));
        num_client--;
        return 0; 
    }else{
        char str1[20] = "Client id is ";
        char strID[20] = {0};
        sprintf(strID,"%d",ID);
        strncat(str1, strID,2);
        write(fdc, str1, strlen(str1)); 
    }
    
	//begin
	while(1) {
        int read_len = read(fdc, buf, sizeof(buf));  
        if (read_len < 0) {  
            perror("read error");  
            break;  
        } else if(read_len == 0) {  
            printf("EOF\n");  
            break;  
        }  
        if(strncmp(buf,"quit",4) == 0){
            num_client--;
            pthread_mutex_lock(&id_mutex_lock);
            client_mode[ID] = -1;
            pthread_mutex_unlock(&id_mutex_lock);
            printf("Disconnected from the client %d!\n",ID);
            return 0;
        }
        //Turn the message to specific case
        
        process_data(buf,read_len,client_mode[ID]);
        write(fdc, buf, read_len);  
    }
    num_client--;
    client_mode[ID] = -1;
    return 0; 
}

void* server_control(void* arg) {
	char buff[1024];
	while (1) {
        printf("Server > ");
		memset(buff, 0, sizeof(buff));
		fgets(buff,sizeof(buff)-1,stdin);
        if (strncmp(buff, "mode", 4) == 0) {
            int point = 5;
			char client_id[2];
            int num_point = 0;
            int ID;
            while (buff[point] != ' ')
            {
                client_id[num_point] = buff[point];
                num_point++;
                point++; 
            }
            point++;
            if(num_point == 2){
                ID = (client_id[0]-48)*10+(client_id[1]-48);                
            }else{
                ID = (client_id[0]-48);
            }
            char mode[7];
            num_point = 0;
            while (buff[point] != 0)
            {
                mode[num_point] = buff[point];
                num_point++;
                point++; 
            }
            mode[num_point] = 0;
            mode_analyse(mode,ID);
        }else if (strncmp(buff, "quit", 4) == 0) {
			printf("quit OK!\n");
            close(listenfd);
            exit(0);
			break;
		}else if(strncmp(buff, "show", 4) == 0){
            printf("Server info:\n");
            if(client_mode[0] == 0){
                printf("    Global mode: normal\n");
            }else if(client_mode[0] == 1){
                printf("    Global mode: upper\n");
            }else if(client_mode[0] == 2){
                printf("    Global mode: lower\n");
            }
            printf("    Characters: %d\n",process_num);
            printf("    Client: %d\n",num_client);
        }else{
            printf("Invalid input\n");
        }
	}
}

