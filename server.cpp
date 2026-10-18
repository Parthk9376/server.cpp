
#include <iostream>
#include <vector>
#include <memory>
#include <string>
#include <sstream>
#include <map>
#include <algorithm>
#include <cstdlib>
#include <cstring>
#include <cctype>
#include <stdexcept>
#include <csignal>
#include <mutex>

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>

using namespace std;

// ============================================================
// BASE CLASS: Table
// ============================================================
class Table {
protected:
    int tableId;
    int capacity;
    bool occupied;

public:
    Table(int id, int seats, bool isOccupied)
        : tableId(id), capacity(seats), occupied(isOccupied) {}

    virtual ~Table() = default;

    int getId() const { return tableId; }
    int getCapacity() const { return capacity; }
    bool isOccupied() const { return occupied; }

    virtual void display() const = 0;
};

// ============================================================
// DERIVED CLASS: FoodCourtTable
// OPERATOR OVERLOADING: == compares table IDs
// ============================================================
class FoodCourtTable : public Table {
public:
    FoodCourtTable(int id, int seats, bool isOccupied)
        : Table(id, seats, isOccupied) {}

    bool operator==(const FoodCourtTable& other) const {
        return tableId == other.tableId;
    }

    bool occupy() {
        if (occupied) return false;
        occupied = true;
        return true;
    }

    bool release() {
        if (!occupied) return false;
        occupied = false;
        return true;
    }

    virtual string type() const {
        return "Regular";
    }

    void display() const override {
        cout << "Table " << tableId
             << " | Seats: " << capacity
             << " | Type: " << type()
             << " | Status: "
             << (occupied ? "OCCUPIED" : "AVAILABLE")
             << '\n';
    }
};

// ============================================================
// DERIVED CLASS: PremiumTable
// Demonstrates multilevel inheritance:
// Table -> FoodCourtTable -> PremiumTable
// ============================================================
class PremiumTable : public FoodCourtTable {
private:
    bool nearWindow;

public:
    PremiumTable(int id, int seats, bool isOccupied, bool window)
        : FoodCourtTable(id, seats, isOccupied),
          nearWindow(window) {}

    string type() const override {
        return nearWindow ? "Premium (Window)" : "Premium";
    }

    void display() const override {
        FoodCourtTable::display();
    }
};

// ============================================================
// FOOD COURT MANAGEMENT SYSTEM
// ============================================================
class FoodCourtSystem {
private:
    vector<unique_ptr<FoodCourtTable>> tables;
    mutable mutex dataMutex;

    FoodCourtTable* findTableUnlocked(int id) {
        // Uses overloaded == operator to compare table IDs.
        FoodCourtTable key(id, 0, false);

        for (auto& table : tables) {
            if (*table == key) {
                return table.get();
            }
        }
        return nullptr;
    }

public:
    FoodCourtSystem() {
        tables.push_back(make_unique<FoodCourtTable>(1, 2, false));
        tables.push_back(make_unique<FoodCourtTable>(2, 4, true));
        tables.push_back(make_unique<FoodCourtTable>(3, 4, false));
        tables.push_back(make_unique<FoodCourtTable>(4, 6, false));
        tables.push_back(make_unique<FoodCourtTable>(5, 2, false));

        tables.push_back(make_unique<PremiumTable>(6, 4, false, true));
        tables.push_back(make_unique<PremiumTable>(7, 6, true, true));
        tables.push_back(make_unique<PremiumTable>(8, 4, false, false));

        tables.push_back(make_unique<FoodCourtTable>(9, 2, false));
        tables.push_back(make_unique<FoodCourtTable>(10, 8, false));
    }

    string listTables() {
        lock_guard<mutex> lock(dataMutex);
        ostringstream out;

        out << "========== MALLSEAT FINDER ==========\n";
        for (const auto& table : tables) {
            out << "Table " << table->getId()
                << " | Seats: " << table->getCapacity()
                << " | " << table->type()
                << " | "
                << (table->isOccupied() ? "OCCUPIED" : "AVAILABLE")
                << '\n';
        }
        return out.str();
    }

    string findAvailable(int groupSize) {
        lock_guard<mutex> lock(dataMutex);
        ostringstream out;

        if (groupSize < 1 || groupSize > 100) {
            return "Enter a group size between 1 and 100.\n";
        }

        out << "Available tables for " << groupSize
            << " person(s):\n";

        bool found = false;

        for (const auto& table : tables) {
            if (!table->isOccupied() &&
                table->getCapacity() >= groupSize) {
                out << "Table " << table->getId()
                    << " | Seats: " << table->getCapacity()
                    << " | " << table->type() << '\n';
                found = true;
            }
        }

        if (!found) {
            out << "No suitable table is available.\n";
        }

        return out.str();
    }

    string occupyTable(int id) {
        lock_guard<mutex> lock(dataMutex);
        FoodCourtTable* table = findTableUnlocked(id);

        if (!table) return "ERROR: Table not found.\n";
        if (!table->occupy()) {
            return "ERROR: Table is already occupied.\n";
        }

        return "SUCCESS: Table " + to_string(id) +
               " is now occupied.\n";
    }

    string releaseTable(int id) {
        lock_guard<mutex> lock(dataMutex);
        FoodCourtTable* table = findTableUnlocked(id);

        if (!table) return "ERROR: Table not found.\n";
        if (!table->release()) {
            return "ERROR: Table is already available.\n";
        }

        return "SUCCESS: Table " + to_string(id) +
               " is now available.\n";
    }

    string statistics() {
        lock_guard<mutex> lock(dataMutex);
        int available = 0;
        int occupied = 0;

        for (const auto& table : tables) {
            if (table->isOccupied()) ++occupied;
            else ++available;
        }

        ostringstream out;
        out << "========== FOOD COURT STATISTICS ==========\n"
            << "Total tables: " << tables.size() << '\n'
            << "Available: " << available << '\n'
            << "Occupied: " << occupied << '\n';
        return out.str();
    }

    void runConsole() {
        int choice;

        while (true) {
            cout << "\n====================================\n"
                 << "       MALLSEAT FINDER\n"
                 << "====================================\n"
                 << "1. View all tables\n"
                 << "2. Find an available table\n"
                 << "3. Occupy a table\n"
                 << "4. Release a table\n"
                 << "5. View statistics\n"
                 << "0. Exit\n"
                 << "Enter choice: ";

            if (!(cin >> choice)) {
                cin.clear();
                cin.ignore(10000, '\n');
                cout << "Invalid input.\n";
                continue;
            }

            if (choice == 0) {
                cout << "Thank you for using MallSeat Finder!\n";
                break;
            }

            int idOrSize;

            switch (choice) {
                case 1:
                    cout << listTables();
                    break;

                case 2:
                    cout << "Enter group size: ";
                    if (cin >> idOrSize)
                        cout << findAvailable(idOrSize);
                    else {
                        cin.clear();
                        cin.ignore(10000, '\n');
                        cout << "Invalid group size.\n";
                    }
                    break;

                case 3:
                    cout << "Enter table ID to occupy: ";
                    if (cin >> idOrSize)
                        cout << occupyTable(idOrSize);
                    else {
                        cin.clear();
                        cin.ignore(10000, '\n');
                        cout << "Invalid table ID.\n";
                    }
                    break;

                case 4:
                    cout << "Enter table ID to release: ";
                    if (cin >> idOrSize)
                        cout << releaseTable(idOrSize);
                    else {
                        cin.clear();
                        cin.ignore(10000, '\n');
                        cout << "Invalid table ID.\n";
                    }
                    break;

                case 5:
                    cout << statistics();
                    break;

                default:
                    cout << "Invalid choice. Try again.\n";
            }
        }
    }
};

// ============================================================
// PLAIN-TEXT HTTP INTERFACE
// No HTML, CSS, JavaScript, or third-party libraries.
// ============================================================

string urlDecode(const string& value) {
    string result;

    for (size_t i = 0; i < value.size(); ++i) {
        if (value[i] == '%' && i + 2 < value.size()) {
            try {
                int n = stoi(value.substr(i + 1, 2), nullptr, 16);
                result += static_cast<char>(n);
                i += 2;
            } catch (...) {
                result += value[i];
            }
        } else if (value[i] == '+') {
            result += ' ';
        } else {
            result += value[i];
        }
    }

    return result;
}

map<string, string> parseQuery(const string& query) {
    map<string, string> params;
    stringstream stream(query);
    string item;

    while (getline(stream, item, '&')) {
        size_t pos = item.find('=');
        if (pos == string::npos) continue;

        params[urlDecode(item.substr(0, pos))] =
            urlDecode(item.substr(pos + 1));
    }

    return params;
}

string menuText() {
    return
        "====================================\n"
        "          MALLSEAT FINDER\n"
        "     MALL FOOD COURT MANAGEMENT\n"
        "====================================\n\n"
        "1. View tables:\n"
        "   /?action=list\n\n"
        "2. Find a table for 4 people:\n"
        "   /?action=find&size=4\n\n"
        "3. Occupy table 3:\n"
        "   /?action=occupy&id=3\n\n"
        "4. Release table 3:\n"
        "   /?action=release&id=3\n\n"
        "5. View statistics:\n"
        "   /?action=stats\n\n"
        "Change the URL query parameters to select an action.\n"
        "This is a plain-text interface generated by C++.\n";
}

string handleRequest(FoodCourtSystem& system, const string& target) {
    size_t qpos = target.find('?');
    string path = target.substr(0, qpos);
    string query = qpos == string::npos
                 ? "" : target.substr(qpos + 1);

    if (path == "/health") return "OK\n";

    if (path != "/") return "Not found.\n";

    map<string, string> params = parseQuery(query);

    auto getInt = [&](const string& key, int& result) -> bool {
        auto it = params.find(key);
        if (it == params.end() || it->second.empty()) return false;

        try {
            size_t used = 0;
            int value = stoi(it->second, &used);
            if (used != it->second.size()) return false;
            result = value;
            return true;
        } catch (...) {
            return false;
        }
    };

    auto actionIt = params.find("action");
    string action = actionIt == params.end()
                  ? "" : actionIt->second;

    if (action.empty()) {
        return menuText() + "\n" + system.statistics();
    }

    if (action == "list") return system.listTables();
    if (action == "stats") return system.statistics();

    int value = 0;

    if (action == "find") {
        if (!getInt("size", value))
            return "ERROR: Supply a valid size parameter.\n";
        return system.findAvailable(value);
    }

    if (action == "occupy") {
        if (!getInt("id", value))
            return "ERROR: Supply a valid id parameter.\n";
        return system.occupyTable(value);
    }

    if (action == "release") {
        if (!getInt("id", value))
            return "ERROR: Supply a valid id parameter.\n";
        return system.releaseTable(value);
    }

    return "Unknown action. Open the home page for the menu.\n";
}

void sendAll(int client, const string& message) {
    size_t sent = 0;

    while (sent < message.size()) {
        ssize_t n = send(client, message.data() + sent,
                         message.size() - sent, 0);
        if (n <= 0) return;
        sent += static_cast<size_t>(n);
    }
}

void runServer(FoodCourtSystem& system) {
    signal(SIGPIPE, SIG_IGN);

    const char* portEnv = getenv("PORT");
    int port = portEnv ? atoi(portEnv) : 10000;

    if (port < 1 || port > 65535) {
        cerr << "Invalid PORT value.\n";
        return;
    }

    int server = socket(AF_INET, SOCK_STREAM, 0);
    if (server < 0) {
        perror("socket");
        return;
    }

    int opt = 1;
    setsockopt(server, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    sockaddr_in address{};
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = htonl(INADDR_ANY);
    address.sin_port = htons(static_cast<unsigned short>(port));

    if (bind(server, reinterpret_cast<sockaddr*>(&address),
             sizeof(address)) < 0) {
        perror("bind");
        close(server);
        return;
    }

    if (listen(server, 20) < 0) {
        perror("listen");
        close(server);
        return;
    }

    cout << "MallSeat Finder is listening on port "
         << port << '\n';

    while (true) {
        int client = accept(server, nullptr, nullptr);
        if (client < 0) continue;

        char buffer[8192]{};
        ssize_t bytes = recv(client, buffer, sizeof(buffer) - 1, 0);

        if (bytes > 0) {
            string request(buffer, static_cast<size_t>(bytes));
            istringstream input(request);
            string method, target, version;
            input >> method >> target >> version;

            string body;
            string status = "200 OK";

            if (method != "GET") {
                status = "405 Method Not Allowed";
                body = "Only GET requests are supported.\n";
            } else {
                body = handleRequest(system, target);
            }

            ostringstream response;
            response << "HTTP/1.1 " << status << "\r\n"
                     << "Content-Type: text/plain; charset=utf-8\r\n"
                     << "Content-Length: " << body.size() << "\r\n"
                     << "Connection: close\r\n"
                     << "X-Content-Type-Options: nosniff\r\n"
                     << "\r\n" << body;

            sendAll(client, response.str());
        }

        close(client);
    }

    close(server);
}

int main(int argc, char* argv[]) {
    FoodCourtSystem system;

    if (argc > 1 && string(argv[1]) == "--console") {
        system.runConsole();
    } else {
        runServer(system);
    }

    return 0;
}
