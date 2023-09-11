#include <iostream>
#include <string>
#include <algorithm>
#include <set>
#include <queue>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netdb.h>
#include <netinet/in.h>

class Client
{
private:
#define SERVER_IP "127.0.0.1"
#define ERROR_S "CLIENT ERROR: "

    int Port;
    int client;
    sockaddr_in server_address;

    bool check_Port_num(std::string& input)
    {
        try
        {
            Port = std::stoi(input);
        }
        catch (const std::invalid_argument&)
        {
            std::cout << "============== Type error! Enter an integer from 1024 to 65535 ==============\n";
            return false;

        }
        catch (const std::out_of_range&)
        {
            std::cerr << "============ Error: The entered number is too large or too small. ===========" << std::endl;
            return false;
        }

        if ((Port > 1023) && (Port < 65536))
        {
            return true;
        }
        else
        {
            std::cout << "==================== Enter an integer from 1024 to 65535 =====================\n";
            return false;
        }
    }

    void enter_Port_num()
    {
        std::string input = "0";
        std::cout << "==================== What port is the server running on? ===================\n";
        do
        {
            std::cin >> input;
        } while (!check_Port_num(input));
    }

    void create_socket()
    {
        client = socket(AF_INET, SOCK_STREAM, 0);
        if (client < 0)
        {
            std::cout << "\n================= " << ERROR_S << "establishing socket error ==================\n\n"; 
            exit(0);
        }
        std::cout << "\n============= Server: Socket for server was successfuly created ============\n\n";

        server_address.sin_port = Port;
        server_address.sin_family = AF_INET;
        inet_pton(AF_INET, SERVER_IP, &server_address.sin_addr);

        std::cout << "=================== Client socked created!    Port: " << ntohs(server_address.sin_port) << " ===================\n\n";
    }
    
    // Подключение к серверу
    void connection_to_server()
    {
        while (true)
        {
            if (connect(client, reinterpret_cast<sockaddr *>(&server_address), sizeof(server_address)) < 0)
            {
                std::cout << "============================ Error connecting! =============================" << std::endl 
                          << "============================= Reconecting....   ============================\n\n";    
                close(client);
                create_socket();

                // Пауза перед следующей попыткой переподключения
                std::this_thread::sleep_for(std::chrono::seconds(5));
                continue;
            }

            std::cout << "========= Connectionton to server "  << inet_ntoa(server_address.sin_addr)
                      << " with port number: " << Port << " =========\n";
            break;
        }
        std::cout << "\nEnter string:\nString must not be longer than 64 characters and contain only digits\n";
        std::cout << "Enter \"exit\" to end the program\n\n";
    }

public:
    Client()
    {
        enter_Port_num();
        create_socket();
        connection_to_server();
    }

    void send_message(int sum)
    {
        int a;
        int bytesReceived = recv(client, &a, sizeof(a), 0);

        while (bytesReceived <= 0)
        {
            connection_to_server();
            bytesReceived = recv(client, &a, sizeof(a), 0);
        }
        send(client, &sum, 4, 0);
    }

    ~Client()
    {
        close(client);
        std::cout << "\n================================= GoodBye! =================================\n"; //================================= GoodBye! =================================
    }
};

class SharedBuffer
{
private:
    std::queue<std::string> buffer;
    std::mutex mtx;
    std::condition_variable cv;

public:
    void addData(std::string data)
    {
        std::unique_lock<std::mutex> lock(mtx);
        buffer.push(data);
        cv.notify_one(); // Уведомить ждущий поток
    }

    std::string getData()
    {
        std::unique_lock<std::mutex> lock(mtx);
        cv.wait(lock, [this]()
                { return !buffer.empty(); }); // Ждать, пока буфер не станет не пустым
        std::string data = buffer.front();
        buffer.pop();
        return data;
    }
};

class first_thread
{
private:
    std::string line;

    std::string check_parity_size()
    {
        if (line.size() > 64)
        {
            return "String size greater than 64 characters";
        }
        if (!std::all_of(line.begin(), line.end(), ::isdigit))
        {
            return "A string is not just numbers!";
        }
        return line;
    }

    void sorting()
    {
        std::multiset<int, std::greater<int>> sort;
        for (int i = 0; i < line.size(); i++)
        {
            int a = (int)line[i];
            sort.insert(a);
        }
        line = "";
        for (auto el : sort)
        {
            line += (char)(el);
        }
    }
    
    void replace_even_digit()
    {
        std::string new_str = "";
        for (int i = 0; i < line.size(); i++)
        {
            int a = (int)line[i] - 48;
            if (a % 2 == 1)
            {
                new_str += line[i];
            }
            else
            {
                new_str += "KB";
            }
        }
        line = new_str;
    }

public:
    first_thread(SharedBuffer *ShB)
    {
        bool is_exit = false;
        while (true)
        {
            while (true)
            {
                getline(std::cin, line);
                if (line == "exit")
                {
                    (*ShB).addData(line);
                    is_exit = true;
                    break;
                }

                std::string check = check_parity_size();
                if (check != line) { std::cout << check << "\n\n"; }
                else { break; }
            }
            if (is_exit) { break; }
            sorting();
            replace_even_digit();
            (*ShB).addData(line);
        }
    }
};

class second_thread
{
private:
    std::string line;
    int sum = 0;

    void sum_num()
    {
        for (int i = 0; i < line.size(); i++)
        {
            int buff = (int)line[i];
            if (buff >= 48 && buff <= 57)
            {
                sum += buff - 48;
            }
        }
    }

public:
    second_thread(SharedBuffer *ShB, Client *clnt)
    {
        while (true)
        {
            line = (*ShB).getData();
            if (line == "exit")
            {
                (*clnt).send_message(-1);
                break;
            }
            if (line != "")
            {
                print();
                sum_num();
                (*clnt).send_message(sum);
                sum = 0;
            }
        }
    }

    void print()
    {
        std::cout << line << "\n\n";
    }
};

int main()
{
    SharedBuffer ShB;
    Client clnt;

    std::thread t1([&]()
                   { first_thread first(&ShB); });
    std::thread t2([&]()
                   { second_thread second(&ShB, &clnt); });

    t1.join();
    t2.join();

    return 0;
}