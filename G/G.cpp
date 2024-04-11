#include <iostream>
#include <WinSock2.h>
#include <Ws2tcpip.h> // Додано для використання inet_pton
#include <cstring>

#pragma comment(lib, "ws2_32.lib") // Підключення бібліотеки WinSock

using namespace std;

// Функція G, яка обчислює значення на основі вхідного параметру x
int G(int x) {
    int y = 10;
    if (x > 13) {
        // Якщо x більше 13, функція зациклюється, моделюючи "зависання"
        while (true) {
            Sleep(1000); // Затримка, щоб уникнути завантаження процесора
        }
    }
    else if (x < y) {
        return 1; // Якщо x менше y, повертаємо 1
    }
    else {
        return 0; // В інших випадках повертаємо 0
    }
}

int main() {
    WSADATA wsaData;
    cout << "G Initializing Winsock..." << endl;
    // Ініціалізація Winsock
    int startupRes = WSAStartup(MAKEWORD(2, 2), &wsaData);
    if (startupRes != 0) {
        cerr << "G WSAStartup failed: " << startupRes << endl;
        return -1; // Помилка ініціалізації
    }

    sockaddr_in serverAddr;
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(8082); // Використання різного порту для g

    cout << "G Converting IP address..." << endl;
    // Конвертація IP-адреси з текстового у двійковий формат
    int ptonRes = inet_pton(AF_INET, "127.0.0.1", &serverAddr.sin_addr);
    if (ptonRes <= 0) {
        if (ptonRes == 0) {
            cerr << "G Not in presentation format." << endl;
        }
        else {
            cerr << "G inet_pton failed: " << WSAGetLastError() << endl;
        }
        WSACleanup();
        return -1; // Помилка конвертації IP-адреси
    }

    cout << "G Creating socket..." << endl;
    // Створення сокета
    SOCKET serverSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (serverSocket == INVALID_SOCKET) {
        cerr << "G Error at socket(): " << WSAGetLastError() << endl;
        WSACleanup();
        return -1; // Помилка створення сокета
    }

    cout << "G Connecting to the server..." << endl;
    // Встановлення з'єднання з сервером
    if (connect(serverSocket, reinterpret_cast<sockaddr*>(&serverAddr), sizeof(serverAddr)) == SOCKET_ERROR) {
        cerr << "G Connect failed: " << WSAGetLastError() << endl;
        closesocket(serverSocket);
        WSACleanup();
        return -1; // Помилка з'єднання
    }

    cout << "G Connection established. Waiting for data..." << endl;
    // Очікування на дані
    int x;
    if (recv(serverSocket, reinterpret_cast<char*>(&x), sizeof(x), 0) == SOCKET_ERROR) {
        cerr << "G Receive failed: " << WSAGetLastError() << endl;
        closesocket(serverSocket);
        WSACleanup();
        return -1; // Помилка отримання даних
    }

    cout << "G Data received. Calculating G(x)..." << endl;
    // Обчислення результату функції G(x)
    int result = G(x);
    cout << "G Sending back result: " << result << endl;
    // Відправлення результату назад на сервер
    send(serverSocket, reinterpret_cast<char*>(&result), sizeof(result), 0);

    // Закриття сокета та очищення ресурсів
    closesocket(serverSocket);
    WSACleanup();
    cout << "G Connection closed, cleanup complete." << endl;
    return 0;
}