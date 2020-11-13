/*
 *    UNX511 - Assignment 1 - Nov.2020
 *    
 *    networkMonitor.cpp
 * 
 *    Student name  : Yuan-Hsi Lee
 *    Student number: 106936180
 *    Student email : ylee174@myseneca.ca
 * 
 * */


#include <iostream>
#include <signal.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/un.h>
#include <unistd.h>
#include <vector>

using namespace std;

char socket_path[]="/tmp/assignment1";
const int BUF_LEN=255;
const int INTERFACE_NUM=2;
bool is_running;

static void signalHandler(int signum)
{
    switch(signum) {
        case SIGINT:
            cout<<"signalHandler("<<signum<<"): SIGINT"<<endl;
            is_running=false;
            break;
       default:
            cout<<"signalHandler("<<signum<<"): unknown"<<endl;
    }
}
char *interfaceNames[2] = {"lo", "wlp2s0"};
char *args1[3] = {"./interfaceMonitor1", "lo", NULL};
char *args2[3] = {"./interfaceMonitor2", "wlp2s0", NULL};
char *args3[2] = {"./interfaceMonitor1", NULL};

int main()
{
    //Create the socket for inter-process communications
    struct sockaddr_un addr;
    char buf[BUF_LEN];
    int len;
    int master_fd,max_fd,cl[INTERFACE_NUM],rc;
    fd_set active_fd_set;
    fd_set read_fd_set;
    int ret;
    int numClients=0;
    pid_t interfacePid[INTERFACE_NUM];

    //Set up a signal handler to terminate the program gracefully
    struct sigaction action;
    action.sa_handler = signalHandler;
    sigemptyset(&action.sa_mask);
    action.sa_flags = 0;
    sigaction(SIGINT, &action, NULL);


    //Create the socket
    memset(&addr, 0, sizeof(addr));
    if ( (master_fd = socket(AF_UNIX, SOCK_STREAM, 0)) < 0) {
        cout << "server: " << strerror(errno) << endl;
        exit(-1);
    }

    addr.sun_family = AF_UNIX;
    //Set the socket path to a local socket file
    strncpy(addr.sun_path, socket_path, sizeof(addr.sun_path)-1);
    unlink(socket_path);

    //Bind the socket to this local socket file
    if (bind(master_fd, (struct sockaddr*)&addr, sizeof(addr)) == -1) {
        cout << "server: " << strerror(errno) << endl;
        close(master_fd);
        exit(-1);
    }

    cout<<"Waiting for the interface..."<<endl;

    interfacePid[0] = fork();
    if (interfacePid[0] == 0) {
        execvp(args1[0], args1); //execvp the ./interface <interfaceName>
    }
    if(ret==-1) {
        cout<<"server: Write Error"<<endl;
        cout<<strerror(errno)<<endl;
    }
    cout << "Starting the monitor for the interface " << interfaceNames[0] << endl;

    sleep(0.5);

    interfacePid[1] = fork();
    if (interfacePid[1] == 0) {
        execvp(args2[0], args2); //execvp the ./interface <interfaceName>
    }
    if(ret==-1) {
        cout<<"server: Write Error"<<endl;
        cout<<strerror(errno)<<endl;
    }
    cout << "Starting the monitor for the interface " << interfaceNames[1] << endl;
    
    //Listen for a client to connect to this local socket file
    if (listen(master_fd, INTERFACE_NUM) == -1) {
        cout << "network: " << strerror(errno) << endl;
        unlink(socket_path);
        close(master_fd);
        exit(-1);
    }

    sleep(2);

    FD_ZERO(&read_fd_set);
    FD_ZERO(&active_fd_set);
    //Add the master_fd to the socket set
    FD_SET(master_fd, &active_fd_set);
    max_fd = master_fd;//We will select from max_fd+1 sockets (plus one is due to a new connection)
    is_running=true;

    while(is_running) {
        //Block until an input arrives on one or more sockets
        read_fd_set = active_fd_set;
        ret=select(max_fd+1, &read_fd_set, NULL, NULL, NULL);//Select from up to max_fd+1 sockets
        if (ret < 0) {
            cout << "network: " << strerror(errno) << endl;
        } else { //Service all the sockets with input pending
            if (FD_ISSET (master_fd, &read_fd_set))
            { //Connection request on the master socket
                cl[numClients] = accept(master_fd, NULL, NULL); //Accept the new connection
                if (cl[numClients] < 0) {
                    cout << "network: " << strerror(errno) << endl;
                } else {
                    cout<<"network: incoming connection is "<<cl[numClients] << endl;
                    FD_SET (cl[numClients], &active_fd_set);//Add the new connection to the set
                    if(max_fd<cl[numClients]) max_fd=cl[numClients];//Update the maximum fd
                    ++numClients;
                    if (numClients == INTERFACE_NUM) {
                        for (int i = 0; i < numClients ; ++i) {
                            len = sprintf(buf, interfaceNames[i]) + 1;
                            ret = write(cl[i], buf, len);
                        }
                    }

                }
            }
            else//Data arriving on an already-connected socket
            {
                for (int i = 0; i < numClients; ++i) {//Find which client sent the data
                    if (FD_ISSET (cl[i], &read_fd_set)) {
                        ret = read(cl[i],buf,BUF_LEN);//Read the data from that client
                        if(ret==-1) {
                            cout<<"server: Read Error"<<endl;
                            cout<<strerror(errno)<<endl;
                        }
                        sleep(0.5);
                        cout<<"server: read(sock:"<<cl[i]<<", buf:"<<buf<<")"<<endl;
                        if (strcmp(buf, "Ready") == 0) { //get the Ready msg from client
                            
                            len = sprintf(buf, "Monitor")+1;
                            ret = write(cl[i], buf, len);

                            if (ret == -1) {
                                cout << "write error" << endl;
                                cout << strerror(errno) << endl;
                            }
                        }
                        if (strcmp(buf, "Link Down") == 0) {
                            cout << "I know link is down!" << endl;
                            len = sprintf(buf, "Set Link Up")+1;
                            ret = write(cl[i], buf, len);
                            if (ret == -1) {
                                cout << "write error" << endl;
                                cout << strerror(errno) << endl;
                            }
                        }
                    }
                }
            }
        }
    }

    for(int i=0; i<numClients; ++i) {//Request each client to quit
        cout<<"server: request interface "<<i+1<<" to quit"<<endl;
        len = sprintf(buf, "Quit")+1;
        ret = write(cl[i], buf, len);
        if(ret==-1) {
            cout<<"server: Write Error"<<endl;
            cout<<strerror(errno)<<endl;
        }
	sleep(1);//Give the clients a change to quit
        FD_CLR(cl[i], &active_fd_set);//Remove the socket from the set of active sockets
        close(cl[i]);//Close the socket connection
    }

    //Close the master socket
    close(master_fd);
    //Remove the socket file from /tmp
    unlink(socket_path);
    return 0;
}
