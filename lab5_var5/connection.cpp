#include <iostream>
#include <WinSock2.h>
#include <thread>
#include <atomic>
#include <chrono>

#pragma comment(lib, "ws2_32.lib")

using namespace std;

// Функція для обробки з'єднання на сокеті та взаємодії з клієнтом
void AcceptConnection(SOCKET listenSocket, atomic<int>& result, int x, string name) {
    cout << name << " Waiting for a connection on socket." << endl;
    SOCKET clientSocket = accept(listenSocket, NULL, NULL);
    if (clientSocket == INVALID_SOCKET) {
        cerr << name << " Accept failed: " << WSAGetLastError() << endl;
        return;
    }

    // Встановлюємо сокет в блокуючий режим
    u_long mode = 0;  // блокуючий режим
    ioctlsocket(clientSocket, FIONBIO, &mode);

    cout << name << " Connection accepted. Sending data..." << endl;
    send(clientSocket, reinterpret_cast<char*>(&x), sizeof(x), 0); // Відправляємо x

    fd_set readfds;
    FD_ZERO(&readfds);
    FD_SET(clientSocket, &readfds);

    timeval timeout;
    timeout.tv_sec = 10;  // Таймаут 10 секунд
    timeout.tv_usec = 0;

    cout << name << " Waiting for result..." << endl;
    int recvResult = 0;

    while (true) {
        // Скидуємо налаштування FD_SET для нового циклу select
        FD_ZERO(&readfds);
        FD_SET(clientSocket, &readfds);

        // Встановлюємо таймаут знову перед кожним викликом select
        timeout.tv_sec = 10;  // Таймаут 10 секунд
        timeout.tv_usec = 0;

        // Чекаємо на завершення операції вводу-виводу
        int selectResult = select(0, &readfds, NULL, NULL, &timeout);

        if (selectResult > 0) {
            recvResult = recv(clientSocket, reinterpret_cast<char*>(&result), sizeof(result), 0);
            if (recvResult > 0) {
                cout << name << " Received result: " << result << endl;
                break;
            }
        }

        cout << name << " No data received within 10 seconds. What would you like to do?" << endl;
        cout << "1) Continue waiting, 2) Stop, 3) Continue without asking" << endl;
        int choice;
        cin >> choice;
        if (choice == 2) {
            cout << name << " Stopping..." << endl;
            break;
        }
        else if (choice == 3) {
            timeout.tv_sec = LONG_MAX;  // Встановлюємо довгий таймаут
        }
    }

    closesocket(clientSocket);
}

int main() {
    WSADATA wsaData;
    int startupRes = WSAStartup(MAKEWORD(2, 2), &wsaData);
    if (startupRes != 0) {
        cerr << "WSAStartup failed: " << startupRes << endl;
        return -1;
    }

    cout << "WSAStartup succeeded." << endl;

    // Слухаємо f
    SOCKET listenSocketF = socket(AF_INET, SOCK_STREAM, 0);
    sockaddr_in serverAddrF = {};
    serverAddrF.sin_family = AF_INET;
    serverAddrF.sin_port = htons(8081);
    serverAddrF.sin_addr.s_addr = INADDR_ANY;
    if (bind(listenSocketF, reinterpret_cast<sockaddr*>(&serverAddrF), sizeof(serverAddrF)) == SOCKET_ERROR) {
        cerr << "Bind failed for F: " << WSAGetLastError() << endl;
        WSACleanup();
        return -1;
    }
    listen(listenSocketF, 1);

    cout << "Listening for F on port 8081." << endl;

    // Слухаємо g
    SOCKET listenSocketG = socket(AF_INET, SOCK_STREAM, 0);
    sockaddr_in serverAddrG = {};
    serverAddrG.sin_family = AF_INET;
    serverAddrG.sin_port = htons(8082);
    serverAddrG.sin_addr.s_addr = INADDR_ANY;
    if (bind(listenSocketG, reinterpret_cast<sockaddr*>(&serverAddrG), sizeof(serverAddrG)) == SOCKET_ERROR) {
        cerr << "Bind failed for G: " << WSAGetLastError() << endl;
        WSACleanup();
        return -1;
    }
    listen(listenSocketG, 1);

    cout << "Listening for G on port 8082." << endl;

    int x;
    cout << "Enter x: ";
    cin >> x;

    atomic<int> fResult = -1;
    atomic<int> gResult = -1;

    // Створюємо потоки для прийому з'єднань
    thread fThread(AcceptConnection, listenSocketF, ref(fResult), x, "F");
    thread gThread(AcceptConnection, listenSocketG, ref(gResult), x, "G");

    // Чекаємо на завершення потоків
    fThread.join();
    gThread.join();

    // Інтерпретація результатів за трьохзначною логікою Кліні
    string finalResult;
    if (fResult == 0 || gResult == 0) {
        finalResult = "false";  // False, якщо одне зі значень — false
    }
    else if (fResult == -1 || gResult == -1) {
        cout << "One or both functions are undefined due to timeout." << endl;
        finalResult = "undefined";
    }
    else {
        finalResult = "true";  // True, якщо обидва значення визначені і не false
    }

    cout << "Final result (Kleene logic): " << finalResult << endl;

    closesocket(listenSocketF);
    closesocket(listenSocketG);
    WSACleanup();

    return 0;
}