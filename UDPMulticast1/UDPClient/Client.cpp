#define WIN32_LEAN_AND_MEAN

#include <ws2tcpip.h>
#include <windows.h>
#include <iostream>
#include <string>

using namespace std;

#pragma comment (lib, "Ws2_32.lib")

#define DEFAULT_BUFLEN 4096

#define SERVER_IP "127.0.0.1"
#define DEFAULT_PORT "8888"

SOCKET client_socket;
int coloR;

COORD point;
HANDLE h = GetStdHandle(STD_OUTPUT_HANDLE);

CONSOLE_SCREEN_BUFFER_INFO screenBufferInfo;

string colorChange(string message, int* colorMessage)
{
    size_t space_index = message.find(' ');

    if (space_index != string::npos && isdigit(message[0]))
    {
        *colorMessage = stoi(message.substr(0, space_index));
        return message.substr(space_index + 1);
    }

    return message;
}

DWORD WINAPI Sender(void* param)
{
    while (true) 
    {
        char query[DEFAULT_BUFLEN];

        GetConsoleScreenBufferInfo(h, &screenBufferInfo);
        point = screenBufferInfo.dwCursorPosition;
        SetConsoleCursorPosition(h, point);
        SetConsoleTextAttribute(h, coloR);

        cin.getline(query, DEFAULT_BUFLEN);

        send(client_socket, query, strlen(query), 0);
    }
}

DWORD WINAPI Receiver(void* param)
{
    while (true) 
    {
        int colorMessage;
        char response[DEFAULT_BUFLEN];
        int result = recv(client_socket, response, DEFAULT_BUFLEN, 0);

        response[result] = '\0';

        string tempResponse = response;
        string message = colorChange(tempResponse, &colorMessage);

        GetConsoleScreenBufferInfo(h, &screenBufferInfo);
        point = screenBufferInfo.dwCursorPosition;
        SetConsoleCursorPosition(h, point);
        SetConsoleTextAttribute(h, colorMessage);

        cout << message << "\n";

        SetConsoleTextAttribute(h, coloR);
    }
}

int main()
{
    system("title Client");

    WSADATA wsaData;
    int iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
    if (iResult != 0) 
    {
        printf("WSAStartup failed with error: %d\n", iResult);

        return 1;
    }

    addrinfo hints = {};
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = IPPROTO_TCP;

    addrinfo* result = nullptr;
    iResult = getaddrinfo(SERVER_IP, DEFAULT_PORT, &hints, &result);

    if (iResult != 0)
    {
        printf("getaddrinfo failed with error: %d\n", iResult);
        WSACleanup();
        return 2;
    }

    addrinfo* ptr = nullptr;

    for (ptr = result; ptr != NULL; ptr = ptr->ai_next)
    {
        client_socket = socket(ptr->ai_family, ptr->ai_socktype, ptr->ai_protocol);

        if (client_socket == INVALID_SOCKET) {
            printf("socket failed with error: %ld\n", WSAGetLastError());
            WSACleanup();
            return 3;
        }

        iResult = connect(client_socket, ptr->ai_addr, (int)ptr->ai_addrlen);
        if (iResult == SOCKET_ERROR) {
            closesocket(client_socket);
            client_socket = INVALID_SOCKET;
            continue;
        }

        break;
    }

    freeaddrinfo(result);

    if (client_socket == INVALID_SOCKET) 
    {
        printf("Unable to connect to server!\n");
        WSACleanup();
        return 5;
    }

    // Enter nickname and color
    cout << "Enter your nickname: ";
    string nickname;
    getline(cin, nickname);
    send(client_socket, nickname.c_str(), nickname.size(), 0);

    do
    {
        cout << "Enter your color (1-15): ";
        cin >> coloR;
    } while (coloR > 15 || coloR < 0);
    
    send(client_socket, to_string(coloR).c_str(), to_string(coloR).size(), 0);

    CreateThread(0, 0, Sender, 0, 0, 0);
    CreateThread(0, 0, Receiver, 0, 0, 0);

    Sleep(INFINITE);
}
