#include <iostream>
#include <cstring>
#include <cstdlib>
#include <unistd.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <thread>
#include <vector>
#include <queue>
#include <atomic>
#include <algorithm>
#include <sstream>

using namespace std;

// Constants
const int MAX_CONNECTIONS = 5;
const int MESSAGE_SIZE = 1024;

// Global variables
atomic<int> logical_clock(0);
priority_queue<pair<int, int>, vector<pair<int, int>>, greater<pair<int, int>>> request_queue;
int listening_port;
vector<int> peer_ports;
int replies = 0;

// Peer class for handling process operations
class Peer {
public:
    Peer(int port) : sockfd(0), listening_port(port) {}

    void startListening() {
        // Create socket
        sockfd = socket(AF_INET, SOCK_STREAM, 0);
        if (sockfd < 0) {
            cerr << "ERROR opening socket\n";
            exit(1);
        }

        // Bind socket to port
        struct sockaddr_in serv_addr;
        bzero((char *) &serv_addr, sizeof(serv_addr));
        serv_addr.sin_family = AF_INET;
        serv_addr.sin_addr.s_addr = INADDR_ANY;
        serv_addr.sin_port = htons(listening_port);

        if (bind(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0) {
            cerr << "ERROR on binding\n";
            exit(1);
        }

        // Listen for incoming connections
        listen(sockfd, MAX_CONNECTIONS);

        while (true) {
            acceptConnection();
        }
    }

    void acceptConnection() {
        struct sockaddr_in cli_addr;
        socklen_t clilen = sizeof(cli_addr);
        int newsockfd = accept(sockfd, (struct sockaddr *) &cli_addr, &clilen);
        if (newsockfd < 0) {
            cerr << "ERROR on accept\n";
            exit(1);
        }

        // Handle incoming messages in a separate thread
        thread t(&Peer::handleMessages, this, newsockfd);
        t.detach();
    }

    void handleMessages(int newsockfd) {
        char buffer[MESSAGE_SIZE];
        bzero(buffer, MESSAGE_SIZE);
        int n = read(newsockfd, buffer, MESSAGE_SIZE);
        if (n < 0) {
            cerr << "ERROR reading from socket\n";
            close(newsockfd);
            return;
        }

        // Process incoming message
        processMessage(string(buffer), newsockfd);

        // Close connection
        close(newsockfd);
    }

    void processMessage(const string& message, int sender_port) {
        stringstream ss(message);
        string msg_type;
        int timestamp, port;
        ss >> msg_type >> timestamp >> port;

        if (msg_type == "request") {
            logical_clock = max(logical_clock.load(), timestamp) + 1;
            request_queue.push({timestamp, sender_port});
            sendReply(sender_port);
        } else if (msg_type == "reply") {
            if (port == listening_port) {
                replies++;
                if (replies == peer_ports.size() - 1 && request_queue.top().second == listening_port) {
                    executeCriticalSection();
                }
            }
        } else if (msg_type == "release") {
            if (port == listening_port) {
                request_queue.pop();
                if (!request_queue.empty() && request_queue.top().second == listening_port) {
                    executeCriticalSection();
                }
            }
        }
    }

    void sendRequest() {
        logical_clock++;
        for (int port : peer_ports) {
            if (port == listening_port) continue;
            sendMessage("request", logical_clock, port);
        }
        request_queue.push({logical_clock, listening_port});
    }

    void sendReply(int receiver_port) {
        sendMessage("reply", logical_clock, receiver_port);
    }

    void sendRelease() {
        request_queue.pop();
        for (int port : peer_ports) {
            if (port == listening_port) continue;
            sendMessage("release", logical_clock, port);
        }
    }

    void sendMessage(const string& msg_type, int timestamp, int receiver_port) {
        int sockfd = socket(AF_INET, SOCK_STREAM, 0);
        if (sockfd < 0) {
            cerr << "ERROR opening socket\n";
            return;
        }

        struct sockaddr_in serv_addr;
        bzero((char *) &serv_addr, sizeof(serv_addr));
        serv_addr.sin_family = AF_INET;
        serv_addr.sin_port = htons(receiver_port);
        serv_addr.sin_addr.s_addr = inet_addr("10.10.82.229"); // Change to receiver's IP address

        cerr << "Attempting to connect to IP: " << inet_ntoa(serv_addr.sin_addr) << " Port: " << ntohs(serv_addr.sin_port) << endl;

        if (connect(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0) {
            cerr << "ERROR connecting\n";
            return;
        }

        stringstream ss;
        ss << msg_type << " " << timestamp << " " << listening_port;
        string message = ss.str();

        if (write(sockfd, message.c_str(), message.length()) < 0) {
            cerr << "ERROR writing to socket\n";
        }

        close(sockfd);
    }

    void executeCriticalSection() {
        cout << "Entered critical section!\n";
        // Perform critical section operations
        usleep(2000000); // Simulate critical section execution time

        // Send release message to all peers
        sendRelease();
    }

private:
    int sockfd;
    int listening_port;
};

int main() {
    cout << "Enter listening port number: ";
    cin >> listening_port;

    int num_peers;
    cout << "Enter number of peers: ";
    cin >> num_peers;

    for (int i = 0; i < num_peers; ++i) {
        int peer_port;
        cout << "Enter port for peer " << i + 1 << ": ";
        cin >> peer_port;
        peer_ports.push_back(peer_port);
    }

    Peer peer(listening_port);
    thread listening_thread(&Peer::startListening, &peer);
    listening_thread.detach();

    while (true) {
        int option;
        cout << "Select operation:\n";
        cout << "1. Request critical section\n";
        cout << "2. Exit\n";
        cin >> option;

        if (option == 1) {
            peer.sendRequest();
        } else if (option == 2) {
            exit(0);
        } else {
            cout << "Invalid option. Try again.\n";
        }
    }

    return 0;
}
