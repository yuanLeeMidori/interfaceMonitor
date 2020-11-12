#include <iostream>
#include <fstream>
#include <signal.h>
#include <string.h>
#include <net/if.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/un.h>
#include <unistd.h>

using namespace std;

char socket_path[]="/tmp/assignment1";
bool is_running;
const int BUF_LEN=100;
char interface[BUF_LEN];
char statPath[BUF_LEN];

int main(int argc, char *argv[])
{
    //Set up socket communications
    struct sockaddr_un clientAddr;
    char buf[BUF_LEN];
    int len, ret;
    int fd,rc;
    char operstate[] = ""; 
    int carrier_up_count = 0, carrier_down_count = 0;
    int tx_bytes = 0, tx_dropped = 0, tx_errors = 0, tx_packets = 0;
    int rx_bytes = 0, rx_dropped = 0, rx_errors = 0, rx_packets = 0;
    

    // cout<<"client("<<getpid()<<"): running..."<<endl;
    memset(&clientAddr, 0, sizeof(clientAddr));
    //Create the socket
    if ( (fd = socket(AF_UNIX, SOCK_STREAM, 0)) == -1) {
        cout << "client("<<getpid()<<"): "<<strerror(errno)<<endl;
        exit(-1);
    }

    clientAddr.sun_family = AF_UNIX;
    //Set the socket path to a local socket file
    strncpy(clientAddr.sun_path, socket_path, sizeof(clientAddr.sun_path)-1);

    //Connect to the local socket
    if (connect(fd, (struct sockaddr*)&clientAddr, sizeof(clientAddr)) < 0) {
        cout << "client("<<getpid()<<"): " << strerror(errno) << endl;
        close(fd);
        exit(-1);
    }

    is_running=true;
    while(is_running) {
        ret = read(fd,buf,BUF_LEN);//Read something from the server
        if(ret<0) {
            cout<<"client("<<getpid()<<"): error reading the socket"<<endl;
            cout<<strerror(errno)<<endl;
        }
        if(strcmp(buf, "Monitor") == 0) {
            cout << "I receive Monitor msg" << endl;
        }
	    if(strcmp(buf, "lo")==0 || strcmp(buf, "wlp2s0") == 0) {//Server pass the interface name
            len = sprintf(buf, "Ready")+1;
            ret = write(fd, buf, len);//sent "Ready" to server
            if(ret==-1) {
               cout<<"client("<<getpid()<<"): Write Error"<<endl;
               cout<<strerror(errno)<<endl;
            }
            while(is_running) {   
                strncpy(interface, argv[1], BUF_LEN); //get the passing interface name
                ifstream infile;
                sprintf(statPath, "/sys/class/net/%s/operstate", interface); //operstate
                infile.open(statPath);
                if (infile.is_open()) {
                    infile >> operstate;
                    infile.close();
                }
                sprintf(statPath, "/sys/class/net/%s/carrier_up_count", interface); //carrier_up_count
                infile.open(statPath);
                if (infile.is_open()) {
                    infile >> carrier_up_count;
                    infile.close();
                }
                sprintf(statPath, "/sys/class/net/%s/carrier_down_count", interface); //carrier_down_count
                infile.open(statPath);
                if (infile.is_open()) {
                    infile >> carrier_down_count;
                    infile.close();
                }                
                sprintf(statPath, "/sys/class/net/%s/statistics/rx_bytes", interface); //rx_bytes
                infile.open(statPath);
                if(infile.is_open()) {
                    infile>>rx_bytes;
                    infile.close();
                }
                sprintf(statPath, "/sys/class/net/%s/statistics/rx_dropped", interface); //rx_dropped
                infile.open(statPath);
                if (infile.is_open()) {
                    infile >> rx_dropped;
                    infile.close();
                }
                sprintf(statPath, "/sys/class/net/%s/statistics/rx_errors", interface); //rx_errors
                infile.open(statPath);
                if (infile.is_open()) {
                    infile >> rx_errors;
                    infile.close();
                }             
                sprintf(statPath, "/sys/class/net/%s/statistics/rx_packets", interface); //rx_packets
                infile.open(statPath);
                if(infile.is_open()) {
                    infile>>rx_packets;
                    infile.close();
                }
                sprintf(statPath, "/sys/class/net/%s/statistics/tx_bytes", interface); //tx_bytes
                infile.open(statPath);
                if(infile.is_open()) {
                    infile>>tx_bytes;
                    infile.close();
                }
                sprintf(statPath, "/sys/class/net/%s/statistics/tx_dropped", interface); //tx_dropped
                infile.open(statPath);
                if (infile.is_open()) {
                    infile >> tx_dropped;
                    infile.close();
                }
                sprintf(statPath, "/sys/class/net/%s/statistics/tx_errors", interface); //tx_errors
                infile.open(statPath);
                if (infile.is_open()) {
                    infile >> tx_errors;
                    infile.close();
                }            
                sprintf(statPath, "/sys/class/net/%s/statistics/tx_packets", interface); //tx_packets
                infile.open(statPath);
                if(infile.is_open()) {
                    infile>>tx_packets;
                    infile.close();
                }
       
                                                
                cout << "\nInterface: " << interface << " state: " << operstate << " up_count: " << carrier_up_count << " down_count: " << carrier_down_count
                     << "\nrx_bytes: " << rx_bytes << " rx_dropped: " << rx_dropped << " rx_errors: " << rx_errors << " rx_packets: " << rx_packets
                     << "\ntx_bytes: " << tx_bytes << " tx_dropped: " << tx_dropped << " tx_errors: " << tx_errors << " tx_packets: " << tx_packets << endl;

                sleep(1);
            }

        } else if(strcmp(buf, "Quit")==0) {//Server requests the client to terminate
            cout<<"client("<<getpid()<<"): received request to quit"<<endl;
            is_running = false;
        }
    }

    cout<<"client("<<getpid()<<"): stopping..."<<endl;
    close(fd);
    return 0;
}