#include <sys/stat.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <string.h>
#include <asm/types.h>
#include <linux/netlink.h>
#include <linux/socket.h>

#define NETLINK_TEST 21

#define MAX_PAYLOAD 1024 /* maximum payload size*/
struct sockaddr_nl src_addr, dest_addr;
struct nlmsghdr *nlh = NULL;
struct iovec iov;
int sock_fd;
struct msghdr msg;

int main(int argc, char* argv[]){
    sock_fd = socket(AF_NETLINK, SOCK_RAW, NETLINK_TEST);
    memset(&msg, 0, sizeof(msg));
    memset(&src_addr, 0, sizeof(src_addr));
    src_addr.nl_family = AF_NETLINK;
    src_addr.nl_pid = getpid(); /* self pid */
    src_addr.nl_groups = 0; /* not in mcast groups */

    bind(sock_fd, (struct sockaddr*)&src_addr, sizeof(src_addr));
    memset(&dest_addr, 0, sizeof(dest_addr));
    dest_addr.nl_family = AF_NETLINK;
    dest_addr.nl_pid = 0; /* For Linux Kernel */
    dest_addr.nl_groups = 0; /* unicast */
    nlh=(struct nlmsghdr *)malloc(NLMSG_SPACE(MAX_PAYLOAD));
    
    /* Fill the netlink message header */
    nlh->nlmsg_len = NLMSG_SPACE(MAX_PAYLOAD);
    nlh->nlmsg_pid = getpid(); /* self pid */
    nlh->nlmsg_flags = 0;

    /* Fill in the netlink message payload */
    while(1){
        /* Read message from terminal */
        char buf[80];
        memset(buf,0,sizeof(buf));
        printf("Client > ");
        gets(buf);
        if(strncmp(buf,"quit",4) == 0){
            break;
        }
        printf("0\n");
        strcpy(NLMSG_DATA(nlh), buf);

        printf("1\n");
        /* Fill in the netlink message payload */
        iov.iov_base = (void *)nlh;
        iov.iov_len = nlh->nlmsg_len;
        msg.msg_name = (void *)&dest_addr;
        msg.msg_namelen = sizeof(dest_addr);
        msg.msg_iov = &iov;
        msg.msg_iovlen = 1;

        /* Send message to kernal */
        sendmsg(sock_fd, &msg, 0);
        printf("2\n");
        
        /* Read message from kernel */
        memset(nlh, 0, NLMSG_SPACE(MAX_PAYLOAD));
        
        recvmsg(sock_fd, &msg, 0);
        printf("Server: %s\n",(char *)NLMSG_DATA(nlh));
    }
    
    /* Close Netlink Socket */
    close(sock_fd);
}