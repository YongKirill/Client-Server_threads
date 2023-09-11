#include <iostream>
#include <stdlib.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netdb.h>
#include <netinet/in.h>
#include <stdio.h>
#include <string.h>
#include <thread>

class Server
{
private:

    #define ERROR_S "SERVER ERROR: "
    
    int Port = 1600;
    int client;
    int server;
    int buffer = 0;
    bool is_exit = false;

    struct sockaddr_in server_address;

    void create_socket()
    {
        while (true)
        {
            
            client = socket (AF_INET, SOCK_STREAM, 0);
            if (client<0)
            {
                std::cout << "================= " << ERROR_S  << "establishing socket error =================\n";                       //================= SERVER ERROR: establishing socket error =================
                exit (0);
            }
            std::cout << "\n============ Server: Socket for server was successfuly created ============\n\n";                             //============ Server: Socket for server was successfuly created ============
        
            server_address.sin_port = Port;
            server_address.sin_family = AF_INET;
            server_address.sin_addr.s_addr = htons(INADDR_ANY);

            int ret = bind(client, reinterpret_cast<struct sockaddr*>(&server_address),
                sizeof(server_address));

            if (ret<0)
            {
                std::cout << "===== " << ERROR_S << "binding conection. Socket has alredy establishing ====\n";                         //===== SERVER ERROR: binding conection. Socket has alredy establishing ====
                Port ++;
                close(client);
                continue;
            }

            
            std::cout << "====================== The server is using port: " << Port <<" =====================\n"                        //====================== The server is using port: 1600 =====================
                      << "=========== Enter this value when connecting a client =====================\n\n";                              //=========== Enter this value when connecting a client =====================

            break;
        }
    }

    void conecting_client ()
    {
        while (true)
        {
            socklen_t size = sizeof(server_address);
            std::cout << "======================= SERVER: Listining clients... ======================\n\n";                               //======================= SERVER: Listining clients... ======================
            listen(client, 1);

            //Соединение
            server = accept(client, reinterpret_cast<struct sockaddr*>(&server_address), &size);
            if (server <0)
            {
                std::cout << "================= " << ERROR_S << "Can't accepting client. ===================" << std::endl               //================= SERVER ERROR: Can't accepting client. ===================
                         << "\n============================ Reconecting....   ============================\n";                           //============================ Reconecting....   ============================
                continue;
            }
            std::cout << "========================= Connected to the client =========================\n";                                //========================= Connected to the client =========================
            std::cout << "========================== Receiving messages... ==========================\n\n";                               //========================== Receiving messages... ==========================

            break;
        }
    }
    
    void receiving_messages()
    {
        while (!is_exit)
        {
            int check_connection = 1;
            //Проверка соединения 
            send(server, &check_connection, 4,0);

            int bytesReceived = recv(server, &buffer, 4, 0);
            if(bytesReceived <= 0)
            {
                std::cout << "\n========================== Client disconnected! ===========================" << std::endl                  //========================== Client disconnected! ===========================
                          << "===================== Searching for a new connection... ===================\n\n";                            //===================== Searching for a new connection... ===================
                conecting_client();
                continue;
            }
            else
            {
                data_processing();
            }
            
        }
    }

    void data_processing()
    {
        if (buffer == -1) is_exit = true;
        else
        {
            if ( (buffer>99) && (buffer%32==0) )
            {
                std::cout<<"Data received!"<<std::endl;
            }
            else
            {
                std::cout<<"ERROR! The number contains less than two digits and is not a multiple of 32!"<<std::endl;
            }
        }
        
    }

public:
    Server()
    {
        create_socket();
        conecting_client ();        
        receiving_messages();
    }

    ~Server()
    {
        std::cout<<"\n================================= GoodBye! ================================"<<std::endl;                 //================================= GoodBye! ================================

        close (client);
        close (server);
    }
    
};

int main (int argc, char* argv[])
{
    Server one;
    return 0;
}