#define WIN32_LEAN_AND_MEAN

#include <winsock2.h>
#include <windows.h>
#include <iostream>
#include <vector>
#include <string>
using namespace std;

#define MAX_CLIENTS 10
#define DEFAULT_BUFLEN 4096

#pragma comment(lib, "ws2_32.lib")
#pragma warning(disable:4996)

struct ClientInfo 
{
    SOCKET socket;
    string nickname;
    int color;
};

SOCKET server_socket;

vector<string> history;
vector<ClientInfo> clients;

int main() 
{
    system("title Server");

    puts("Start server... DONE.");

    WSADATA wsa;

    if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0)
    {
        printf("Failed. Error Code: %d", WSAGetLastError());

        return 1;
    }

    if ((server_socket = socket(AF_INET, SOCK_STREAM, 0)) == INVALID_SOCKET) 
    {
        printf("Could not create socket: %d", WSAGetLastError());

        return 2;
    }

    sockaddr_in server;
    server.sin_family = AF_INET;
    server.sin_addr.s_addr = INADDR_ANY;
    server.sin_port = htons(8888);

    if (bind(server_socket, (sockaddr*)&server, sizeof(server)) == SOCKET_ERROR) 
    {
        printf("Bind failed with error code: %d", WSAGetLastError());

        return 3;
    }

    listen(server_socket, MAX_CLIENTS);

    puts("Server is waiting for incoming connections...\nPlease, start one or more client-side app.");

    fd_set readfds;
    SOCKET client_sockets[MAX_CLIENTS] = {};

    while (true) 
    {
        FD_ZERO(&readfds);

        FD_SET(server_socket, &readfds);

        for (int i = 0; i < MAX_CLIENTS; i++) {
            SOCKET s = client_sockets[i];
            if (s > 0) {
                FD_SET(s, &readfds);
            }
        }

        if (select(0, &readfds, NULL, NULL, NULL) == SOCKET_ERROR)
        {
            printf("select function call failed with error code : %d", WSAGetLastError());

            return 4;
        }

        SOCKET new_socket;
        sockaddr_in address;

        int addrlen = sizeof(sockaddr_in);

        if (FD_ISSET(server_socket, &readfds)) 
        {

            if ((new_socket = accept(server_socket, (sockaddr*)&address, &addrlen)) < 0) 
            {
                perror("accept function error");

                return 5;
            }


            char client_nickname[DEFAULT_BUFLEN];
            int nickname_length = recv(new_socket, client_nickname, DEFAULT_BUFLEN, 0);
            client_nickname[nickname_length] = '\0';


            char client_color[DEFAULT_BUFLEN];
            int color_length = recv(new_socket, client_color, DEFAULT_BUFLEN, 0);
            client_color[color_length] = '\0';

            ClientInfo new_client;
            new_client.socket = new_socket;
            new_client.nickname = string(client_nickname);
            new_client.color = atoi(client_color);

            for (int i = 0; i < history.size(); i++)
            {
                send(new_socket, history[i].c_str(), history[i].size(), 0);
            }

            clients.push_back(new_client);

            printf("New connection, socket fd is %d, ip is: %s, port: %d\n", new_socket, inet_ntoa(address.sin_addr), ntohs(address.sin_port));

            for (int i = 0; i < MAX_CLIENTS; i++) 
            {
                if (client_sockets[i] == 0)
                {
                    client_sockets[i] = new_socket;
                    printf("Adding to list of sockets at index %d\n", i);
                    break;
                }
            }
        }

        for (int i = 0; i < MAX_CLIENTS; i++) 
        {
            SOCKET s = client_sockets[i];
            if (FD_ISSET(s, &readfds))
            {
                sockaddr_in address;
                int addrlen = sizeof(sockaddr_in);

                char client_message[DEFAULT_BUFLEN];

                int client_message_length = recv(s, client_message, DEFAULT_BUFLEN, 0);
                client_message[client_message_length] = '\0';

                string check_exit = client_message;

                if (check_exit == "off")
                {
                    cout << "Client #" << i << " is off\n";
                    client_sockets[i] = 0;
                }

                string temp = to_string(clients[i].color) + " " + clients[i].nickname + ": " + client_message +"\n";
                history.push_back(temp);

                for (int j = 0; j < MAX_CLIENTS; j++) 
                {
                    if (client_sockets[j] != 0)
                    {
                        if (i != j) 
                        {
                            string message_with_info = to_string(clients[i].color) + " " + clients[i].nickname + ": " + client_message;

                            send(client_sockets[j], message_with_info.c_str(), message_with_info.size(), 0);
                        }
                    }
                }
            }
        }
    }

    WSACleanup();
}