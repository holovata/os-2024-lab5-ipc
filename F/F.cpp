#include <iostream>
#include <WinSock2.h>
#include <Ws2tcpip.h> // Додано для використання inet_pton
#include <cstring>

#pragma comment(lib, "ws2_32.lib") // Підключення бібліотеки WinSock

using namespace std;

// Функція F обчислює значення в залежності від вхідного параметра x
int F(int x) {
    int y = 5;
    if (x < 0) {
        // Якщо x менше 0, функція входить у безкінечний цикл,
        // моделюючи "зависання" програми
        while (true) {
            Sleep(1000); // Затримка, щоб зменшити навантаження на процесор
        }
    }
    else if (x > y) {
        return 1; // Якщо x більше y, повертаємо 1
    }
    else {
        return 0; // В інших випадках повертаємо 0
    }
}

int main() {
    WSADATA wsaData;
    cout << "F Initializing Winsock..." << endl;
    // Ініціалізація Winsock
    int startupRes = WSAStartup(MAKEWORD(2, 2), &wsaData);
    if (startupRes != 0) {
        cerr << "F WSAStartup failed: " << startupRes << endl;
        return -1; // Помилка ініціалізації Winsock
    }

    sockaddr_in serverAddr;
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(8081); // Використовуємо окремий порт для f

    // Конвертація IP-адреси з текстової у двійкову форму
    cout << "F Converting IP address..." << endl;
    inet_pton(AF_INET, "127.0.0.1", &serverAddr.sin_addr);

    cout << "F Creating socket..." << endl;
    // Створення сокета
    SOCKET serverSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (serverSocket == INVALID_SOCKET) {
        cerr << "F Error at socket(): " << WSAGetLastError() << endl;
        WSACleanup();
        return -1; // Помилка створення сокета
    }

    cout << "F Connecting to server..." << endl;
    // Встановлення з'єднання з сервером
    if (connect(serverSocket, reinterpret_cast<sockaddr*>(&serverAddr), sizeof(serverAddr)) == SOCKET_ERROR) {
        cerr << "F Connect failed: " << WSAGetLastError() << endl;
        closesocket(serverSocket);
        WSACleanup();
        return -1; // Помилка з'єднання
    }

    cout << "F Receiving data..." << endl;
    // Очікування на отримання даних
    int x;
    if (recv(serverSocket, reinterpret_cast<char*>(&x), sizeof(x), 0) == SOCKET_ERROR) {
        cerr << "F Receive failed: " << WSAGetLastError() << endl;
        closesocket(serverSocket);
        WSACleanup();
        return -1; // Помилка отримання даних
    }

    // Обчислення результату функції F на основі отриманих даних
    int result = F(x);
    cout << "F Sending result back: " << result << endl;
    // Відправлення результату назад на сервер
    send(serverSocket, reinterpret_cast<char*>(&result), sizeof(result), 0);

    // Закриття сокета та очищення ресурсів
    closesocket(serverSocket);
    WSACleanup();
    cout << "F Connection closed, cleanup complete." << endl;
    return 0;
}
