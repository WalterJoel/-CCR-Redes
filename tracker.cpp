#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

#include <string>
#include <iostream>       // std::cout
#include <thread>         // std::thread, std::this_thread::sleep_for
#include <chrono>         // std::chrono::seconds
#include <vector>
#include <sstream>

using namespace std;
#define PROTOCOL_SIZE 4

string Int_a_String(int num)
{
    stringstream stream;
    string palabra;
    stream << num;
    palabra = stream.str();
    return palabra;
}
struct NodeTracker{
    string port = "";
    string ip   = "";
    NodeTracker(){}
    NodeTracker(string port,string ip){
        this->port = port;
        this->ip   = ip;
    }
};
class Tracker{
    int SERSocket;
    vector<NodeTracker*> listOfPeers;
public:
    Tracker(){}
    void initializeTracker();
    void readAction(int);
    void registerPeer(string,string);
    void showListPeers(int);
    string charToInt(string);
};
void Tracker::initializeTracker(){
    //Variable que me permitirá controlar quienes se conectac primero al tracker
    int cont = 0;
    struct sockaddr_in SERstSockAddr;
    SERSocket = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
    char buffer[256];
    int n;
    if(-1 == SERSocket)
    {
        perror("can not create socket");
        exit(EXIT_FAILURE);
    }
    SERstSockAddr.sin_family      = AF_INET;
    SERstSockAddr.sin_port        = htons(9000);
    SERstSockAddr.sin_addr.s_addr = INADDR_ANY;
    if(-1 == bind(SERSocket,(const struct sockaddr *)&SERstSockAddr, sizeof(struct sockaddr_in)))
    {
        perror("error bind failed");
        close(SERSocket);
        exit(EXIT_FAILURE);
    }
    if(-1 == listen(SERSocket, 10))
    {
        perror("error listen failed");
        close(SERSocket);
        exit(EXIT_FAILURE);
    }
    for(;;)
    {
        int NEWCLISocket = accept(SERSocket, NULL, NULL);
        cout<<"Nuevo cliente"<<endl;
        //////////// PRIMERO LEO LA ACCION ////////////
        char action[2];
        read(NEWCLISocket,action,1);
        if(action[0] == 'R'){
            this->readAction(NEWCLISocket);
        }
        if(action[0] == 'L'){
            this->showListPeers(NEWCLISocket);
        }
        //readAction(NEWCLISocket);
        if(0 > NEWCLISocket) {
            perror("error accept failed");
            close(SERSocket);
            exit(EXIT_FAILURE);
        }
    }
    close(SERSocket);
}
string Tracker::charToInt(string toChar){
    string Char;
    for(int i=0;i<4;i++){
        if(i!=3){
            Char+= Int_a_String( int(toChar[i])) + '.';
        }
        else{
            Char+= Int_a_String( int(toChar[i]));
        }
    }
    return Char;
}

void Tracker::readAction(int id){
    string PORT;
    string IP;
    //Read the size of string_port
    char buffer[100];
    int n = read(id,buffer,PROTOCOL_SIZE);
    buffer[4]='\0';
    int portSize = atoi(buffer);
    //Read the action
    n = read(id,buffer,1);
    char  msgAction = buffer[0];
    buffer[1]='\0';
    //Read the IP
    n = read(id,buffer,4);
    for(int i=0;i<n;i++){
        IP +=buffer[i];
    }

    //Read the port number
    n = read(id,buffer,portSize);
    PORT = string(buffer);

    ////////// ACTION //////////
    if(msgAction == 'R'){
        registerPeer(PORT,IP);
    }
}
void Tracker::registerPeer(string PORT,string IP){
    //Create a new nodeTracker
    NodeTracker * newP = new NodeTracker(PORT,IP);
    listOfPeers.push_back(newP);
    cout<<"Peer Registrado: "<<endl;
}
void Tracker::showListPeers(int id){
    string msg;
    for(int i=0;i< this->listOfPeers.size();i++){
        msg+=this->charToInt( this->listOfPeers[i]->ip )+','+this->listOfPeers[i]->port;
        msg+='\n';
    }
    char complete_msg[5];
    //Completo el tamaño
    sprintf(complete_msg,"%04d",msg.size());  // 0004
    string total = string(complete_msg)+msg;
    cout<<"total: "<<total<<endl;
    write(id,total.c_str(),total.size());
}

int main(void)
{
    Tracker serverTra;
    serverTra.initializeTracker();
    return 0;
}

