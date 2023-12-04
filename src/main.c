#include <stdio.h>
#include <string.h>

#if defined(WIN32) || defined(_WIN32) || defined(__WIN32) && !defined(__CYGWIN__)
#pragma comment(lib, "ws2_32")
#include <winsock2.h>
#include <windows.h>
#include <ws2tcpip.h>
// using https://learn.microsoft.com/en-us/windows/win32/api/winsock2/
// and   https://learn.microsoft.com/en-us/windows/win32/winsock/creating-a-socket-for-the-client
#else
#include <sys/socket.h>
#endif

int main(int argc, char **argv)
{
    if (argc != 2)
    {
        fprintf(stderr, "improper usage: requires one verb\n");
        return 1;
    }

    WSADATA wsadata;

    if (WSAStartup(MAKEWORD(2, 2), &wsadata) != NO_ERROR)
    {
        fprintf(stderr, "WSAStartup failed\n");
        return 1;
    }

    struct addrinfo *result = NULL,
                    *ptr = NULL,
                    hints;

    ZeroMemory(&hints, sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = 0;
    hints.ai_protocol = 0;

    if (getaddrinfo("conjugator.reverso.net", "80", &hints, &result) != NO_ERROR)
    {
        fprintf(stderr, "getaddrinfo failed\n");
        WSACleanup();
        return 1;
    }

    SOCKET sock = INVALID_SOCKET;

    for (ptr = result; ptr != NULL; ptr = ptr->ai_next)
    {
        sock = socket(ptr->ai_family, ptr->ai_socktype, ptr->ai_protocol);

        if (sock == INVALID_SOCKET)
        {
            continue;
        }

        if (connect(sock, ptr->ai_addr, (int)ptr->ai_addrlen) != SOCKET_ERROR)
        {
            break;
        }

        closesocket(sock);
    }

    freeaddrinfo(result);

    if (ptr == NULL)
    {
        fprintf(stderr, "could not connect\n");
        WSACleanup();
        return 1;
    }

    size_t verb_length = strlen(argv[1]);
    char *request = malloc(95 + verb_length);

    strcpy(request, "GET /conjugation-french-verb-");
    strcpy(request + 29, argv[1]);
    strcpy(request + 29 + verb_length, ".html HTTP/2\r\nHost: conjugator.reverso.net\r\nConnection: close\r\n\r\n");
    size_t req_length = strlen(request);

    // char *request = malloc(91);
    // strncpy(request, "GET /conjugation-french.html HTTP/1.1\r\nHost: conjugator.reverso.net\r\nConnection: close\r\n\r\n", 91);
    // size_t req_length = 91;

    // printf("sending %zu bytes:\n%s\n", req_length, request);

    // for (int i = 0; i < req_length; i++)
    // {
    //     printf("%d ", *(request + i));
    // }

    if (send(sock, request, req_length, MSG_OOB) == SOCKET_ERROR)
    {
        fprintf(stderr, "could not complete write\n");
        free(request);
        shutdown(sock, SD_BOTH);
        WSACleanup();
        return 1;
    }

    free(request);

    size_t buf_size = 102400;
    char *buf = malloc(buf_size);

    size_t nread = recv(sock, buf, buf_size, MSG_WAITALL);

    if (nread == SOCKET_ERROR)
    {
        fprintf(stderr, "read failed\n");
        free(buf);
        shutdown(sock, SD_BOTH);
        WSACleanup();
        return 1;
    }

    shutdown(sock, SD_BOTH);
    WSACleanup();

    printf("received %zu bytes:\n%s", nread, buf);

    free(buf);
    return 0;
}