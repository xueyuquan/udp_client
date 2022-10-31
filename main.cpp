#include <iostream>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <sys/stat.h>
#include <dirent.h>
#include <sys/mman.h>
#include <sys/wait.h>
#include <signal.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/msg.h>
#include <sys/sem.h>
#include <pthread.h>
#include <semaphore.h>
#include <poll.h>
#include <sys/epoll.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
using namespace std;

#define SEND_PORT 29954
#define RECV_PORT 29955
#define SERVERIP  "192.168.0.151"
#define NUM_SIGNAL  104
unsigned char sbuf[8] = {0x01, 0x03, 0x00, 0x00, 0x00, 0x67, 0x04, 0x19};

class UDP_Client{
public:
        UDP_Client(const char *server_ip, int sendport, int recvport);
        //~UDP_Client();
        int createUDPsocket();
        void bindSocketAddr(int type);      //type 1:send 0:recv
        int getsocket();
        void sendDatatoServer(int sockfd, unsigned char* sendBuf, int Buflen);
        void recvDatafromServer(int sockfd, unsigned  char* recvBuf, int Buflen);
        uint16_t chartoword(unsigned char ch, unsigned char cl);
        uint16_t getdata(int index);//index 1~104
        socklen_t getsscklen_t();
        int getbindstatus();
private:
    struct sockaddr_in struSendAddr;
    struct  sockaddr_in strRecvAddr;
    const char* strServerIP;
    int socketfd;
    int on;
    socklen_t addr_len;
    uint16_t data[NUM_SIGNAL];
    int bindstatus;
};

UDP_Client::UDP_Client(const char *server_ip, int sendport, int recvport) {
    this->strServerIP = server_ip;
    bzero(&struSendAddr, sizeof(struSendAddr));
    bzero(&strRecvAddr, sizeof(strRecvAddr));
    //设置发送地址信息、IP信息
    this->struSendAddr.sin_family = AF_INET;
    this->struSendAddr.sin_port = htons(sendport);
    this->struSendAddr.sin_addr.s_addr = inet_addr(strServerIP);
    //设置接收地址信息、IP信息
    this->strRecvAddr.sin_family = AF_INET;
    this->strRecvAddr.sin_port = htons(recvport);
    this->strRecvAddr.sin_addr.s_addr = htonl(INADDR_ANY);
    this->on = 1;
    this->addr_len = sizeof(struct sockaddr);
}

void UDP_Client::sendDatatoServer(int sockfd, unsigned char *sendBuf, int Buflen) {
    int ret = 0;
    if(Buflen != sendto(sockfd, sendBuf, Buflen, 0, (struct sockaddr*)&struSendAddr, addr_len)){
        perror("sento server failed");
        return;
    }else {
        cout << "send data to Server successfully!" << endl;
    }
}

int UDP_Client::createUDPsocket() {
    this->socketfd = socket(AF_INET, SOCK_DGRAM, 0);//创建udp的套接字
    if(socketfd < 0){
        perror("failed create UDP socket!");
        return socketfd;
    }
    //设置端口复用
    setsockopt(socketfd, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on));
    return 0;
}
uint16_t UDP_Client::getdata(int index)
{
    return data[index-1];
}

socklen_t UDP_Client::getsscklen_t() {
    return this->addr_len;
}

int UDP_Client::getsocket() {
    return socketfd;
}

void UDP_Client::bindSocketAddr(int type) {
    int ret = 0;
    if(type){
        //绑定地址信息，ip信息
        ret = bind(socketfd, (struct sockaddr*)&struSendAddr, addr_len);
        if (ret < 0){
            perror("sbind sendsocket failed");
            bindstatus = 0;
            return;
        }else {
            bindstatus = 1;
        }
    } else{
        //绑定地址信息，ip信息
        ret = bind(socketfd, (struct sockaddr*)&strRecvAddr, addr_len);
        if (ret < 0){
            perror("sbind recvsocket failed");
            bindstatus = 0;
            return;
        }else {
            bindstatus = 1;
        }
    }
}
int UDP_Client::getbindstatus(){
    return  bindstatus;
}

void UDP_Client::recvDatafromServer(int sockfd, unsigned char *recvBuf, int Buflen) {
    uint16_t word[104] = {0};

    int ret = 0;
    ret = recvfrom(sockfd, recvBuf, Buflen, 0, (struct sockaddr*)&strRecvAddr, &addr_len);
    if(ret < 0){
        perror("recvfrom failed");
    }
    for(int i = 0; i < Buflen; ++i){
        printf("%02x ", recvBuf[i]);
    }
    cout << endl;
    for(int i = 0; i < Buflen; ++i)
    {
        if(recvBuf[i] == 0x01 && (recvBuf[i+1] == 0x03)){
//            printf("%02x ", recvBuf[3]);
//            printf("%02x ", recvBuf[4]);
            for(int j = 0; j < NUM_SIGNAL; ++j){
                data[i+j] = chartoword(recvBuf[3+i+j*2], recvBuf[4+i+j*2]);
                printf("%d ", data[j]);
            }
        }
    }
    cout << endl;

}

uint16_t UDP_Client::chartoword(unsigned char ch, unsigned char cl){
    return (ch << 8) | cl;
}



int main(int argc, char const *argv[])
{
    unsigned char rbuf_mb[426] = {0};
    UDP_Client udp_client1(SERVERIP, SEND_PORT, RECV_PORT);
    if(udp_client1.createUDPsocket() == 0)
    {
        cout << "create UDP socket successfully!" << endl;
    }
    udp_client1.bindSocketAddr(0);
    //循环发送信息给服务端
    while (1)
    {
        if(udp_client1.getbindstatus()){
            udp_client1.sendDatatoServer(udp_client1.getsocket(), sbuf, 8);
            cout << "xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx" << endl;

            udp_client1.recvDatafromServer(udp_client1.getsocket(), rbuf_mb, 213);

            cout << "xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx" << endl;
            for(int i = 0; i < 104; i++){
                cout << udp_client1.getdata(i+1) << " ";
            }
            cout << endl;
            sleep(1);
        }
        if(!udp_client1.getbindstatus()){
            udp_client1.bindSocketAddr(0);
        }
    }
    close(udp_client1.getsocket());
    return 0;
}