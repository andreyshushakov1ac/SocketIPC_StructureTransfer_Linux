#include <iostream>
#include <ctime>
#include <sys/wait.h>
#include "unistd.h" // syst calls, pr-ss cntrl
#include <string.h> 
/* ith IPv4 and IPv6 network addresses. Some of the functions that can be used from this library include 
inet_pton() to convert the string representation of an IP address to a binary representation) and 
inet_ntop() to convert the binary representation of an IP address to a string representation). */
#include <arpa/inet.h>  
using namespace std;

//#################################//################################

 struct Header 
{
    int dataSizeBytes;
    time_t currentTime;
}; 
struct Body 
{
    char gg[14]; 
};
struct End 
{
    uint16_t crc16; 
}; 
//#################################
struct Packet 
{   
    Header H1;
    Body B1;
    End E1;
};
//#################################
    
    // Calculate CRC16
uint16_t calculateCRC16(const char* data, int dataSize);
    // Covert time to dd.mm.yy form 
string formatTime(time_t currentTime);

int TcpServer();
int TcpClient();

//===============================================================================
int main()
{

        // Create new process
    pid_t pid = fork();
    if (pid == -1)
    {
        cerr << "Ошибка при создании процесса" << endl;
        return 1;
    }
        // Сhild (TCP-SERVER)
    else if (pid == 0) 
    {
        TcpServer();
    }
        // Parent (TCP-CLIENT)
    else 
    {
        TcpClient();
            // Waiting for child (he will sleep just after being connected)
        int status; waitpid(pid, &status, 0);
    }



//
}//main
//==================================================================================================================================





int TcpClient()
{
    
    sleep(1); // for server to be in time with socket opening

    int sock = 0;                   // fd
    struct sockaddr_in serv_addr;   // keeping addr of server

        // Open socket
    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) // IP4, ...
    {                                                     
        cerr << "Socket creation error" << endl; 
        return 1;
    }

        // Set server socket
    serv_addr.sin_family = AF_INET;   // IP4
    serv_addr.sin_port = htons(8080); //  the htons function is used to convert the port to network byte order (the byte order used in network packets). In this case, the port is set to 8080. The Host format is a format for representing data on a specific computer architecture (for example, x86/x86_64). In the host format, integers, including ports, are usually represented in Little-Endian byte order, where the lowest byte comes first. This differs from the network byte order (Big-Endian), which is used in network protocols.
    if (inet_pton(AF_INET, "127.0.0.1", &serv_addr.sin_addr) <= 0) 
    {                                                                       
        cerr << "Invalid address/ Address not supported" << endl; 
        return 1;
    }
        
        // Connection
    if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
    {                                                 
        cerr << "Connection Failed" << endl; 
         close (sock);   
        return 1;
    }


//######################
    // Create message and send it to server

        // Packet creation and init
    Packet msg; 
    strcpy(msg.B1.gg, "Hello, world!");
    msg.H1.dataSizeBytes = sizeof(msg.B1.gg);
    msg.H1.currentTime = time(nullptr); 
    msg.E1.crc16 = calculateCRC16(reinterpret_cast<const char*>(&msg.B1), sizeof(Body));

        // Convert struct to bytes array and send to server
    char sendToSocket[sizeof(msg)];
    memcpy(sendToSocket, &msg, sizeof(msg));
    int bytes_received = send(sock, sendToSocket , sizeof(msg), 0);  // 0 - TCP without any flags

    cout << "\nData packet is sent by CLIENT in process 1"<<endl;
    cout<<"Packet size: "<< sizeof(Packet)<<endl;      
    cout<< "Client sent: "<<  msg.B1.gg   <<endl;
    cout<< "Time sent: "<<  formatTime(msg.H1.currentTime) <<endl;
    cout<< "CRC16: "<<  msg.E1.crc16   <<endl<<endl;         
   
    close (sock);

    return 0; // Завершение программы
    
    //==============
}

//#################################//#################################
int TcpServer()
{
        Packet msg ;

        int server_fd{0}, new_socket{0};  
        struct sockaddr_in address;           
        int addrlen = sizeof(address);   
        char buffer[sizeof(msg)]{0};                  
        
            //
        if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0)
        {  //Если функция socket завершилась успешно, она вернет неотрицательное значение (файловый дескриптор сокета)                                 
            perror("Socket creation failed"); // Вывод сообщения об ошибке
            return 1;
        }

        address.sin_family = AF_INET;         // Установка семейства адресов
        address.sin_addr.s_addr = INADDR_ANY; // сокет будет принимать соединения на любом доступном сетевом интерфейсе.
        address.sin_port = htons(8080);       

        if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0) //проверяет успешность привязки сокета server_fd к адресу и порту, указанным в структуре address. Если привязка прошла успешно, функция bind() вернет значение больше или равное нулю
        {                          
            perror("Bind failed"); 
            close(server_fd); close(new_socket);
            return 1;
        }

        if (listen(server_fd, 3) < 0) //дескриптор сокета и максимальное количество ожидающих соединений в очереди.
        {                            // Ожидание подключений на сокете
            perror("Listen failed"); 
            close(server_fd); close(new_socket);
            return 1;
        }

        // accept заполняет структуру address с информацией об адресе клиента, который устанавливает соединение с сервером. Параметр addrlen используется для указания размера этой структуры, чтобы функция accept() могла корректно скопировать данные в соответствующий размер буфера.
        if ((new_socket = accept(server_fd, (struct sockaddr *)&address, (socklen_t *)&addrlen)) < 0)
        {                            // Принятие нового соединения
            perror("Accept failed"); // Вывод сообщения об ошибке
            close(server_fd); close(new_socket);
            return 1;
        }

            // Recieve data from socket
        int bytes_received = recv(new_socket,buffer, sizeof(buffer), 0);
        if (bytes_received < 0) 
        {
            close(server_fd); close(new_socket);
            cerr << "Receive failed" << endl;
            return -1;
        }               
        else            
        if (bytes_received >= sizeof(Packet))
        {    
            sleep(1); // to avoid conflicts in console
            memcpy(&msg, buffer, sizeof(buffer));
            cout<< "The packet is recieved by SERVER in process2"<<
            "\nBytes recived: "<< bytes_received <<
            "\nRecieved message: "<< msg.B1.gg  <<
            "\nRecieved time: "<< formatTime(msg.H1.currentTime)  <<
            "\nRecieved CRC16: "<< msg.E1.crc16<<
            "\nCalculated CRC16: "<< calculateCRC16(reinterpret_cast<const char*>(&msg.B1), sizeof(msg.B1))<< endl<<endl ;
        }
        else 
            cerr << "Received incomplete packet" << endl;

        close(server_fd); close(new_socket);

        return 0; // Завершение программы
}


//###################################################################################
uint16_t calculateCRC16(const char* data, int dataSize) {
    
    // Генераторный полином для CRC16 (x^16 + x^15 + x^2 + 1)
const uint16_t CRC16_POLY = 0xA001;
    
    uint16_t crc = 0xFFFF; // init by max value


    for (int i = 0; i < dataSize; ++i) {
        crc ^= static_cast<uint8_t>(data[i]); // XOR
        for (int j = 0; j < 8; ++j) {
            if (crc & 0x0001) {
                crc = (crc >> 1) ^ CRC16_POLY;
            } else {
                crc >>= 1;
            }
        }
    }
    
    return crc;
}


//####################################
string formatTime(time_t currentTime) {
    struct tm* timeinfo;
    timeinfo = localtime(&currentTime);
    
    int day = timeinfo->tm_mday;
    int month = timeinfo->tm_mon + 1;
    int year = timeinfo->tm_year + 1900;
    
    return to_string(day) + "." + to_string(month) + "." + to_string(year);
}

