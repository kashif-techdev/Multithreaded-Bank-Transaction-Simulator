#include <iostream>
#include <pthread.h>
#include <vector>
#include <cstdlib>
#include <unistd.h>
#include <ctime>
#include <iomanip>
#include <sstream>
#include <fstream>
#include <limits>

using namespace std;

// ANSI Color Codes for beautiful terminal output
namespace Colors {
    const string RESET = "\033[0m";
    const string BOLD = "\033[1m";
    const string DIM = "\033[2m";
    
    // Text Colors
    const string RED = "\033[31m";
    const string GREEN = "\033[32m";
    const string YELLOW = "\033[33m";
    const string BLUE = "\033[34m";
    const string MAGENTA = "\033[35m";
    const string CYAN = "\033[36m";
    const string WHITE = "\033[37m";
    
    // Background Colors
    const string BG_BLUE = "\033[44m";
    const string BG_GREEN = "\033[42m";
    const string BG_YELLOW = "\033[43m";
    const string BG_RED = "\033[41m";
    const string BG_CYAN = "\033[46m";
    
    // Bright Colors
    const string BRIGHT_GREEN = "\033[92m";
    const string BRIGHT_YELLOW = "\033[93m";
    const string BRIGHT_BLUE = "\033[94m";
    const string BRIGHT_MAGENTA = "\033[95m";
    const string BRIGHT_CYAN = "\033[96m";
}

// Configuration structure
struct Config {
    int numAccounts;
    double initialBalance;
    int numClients;
    int transactionsPerClient;
    double minAmount;
    double maxAmount;
};

// Transaction record structure
struct Transaction {
    int clientId;
    int accountNumber;
    string type;  // "DEPOSIT" or "WITHDRAW" or "TRANSFER"
    double amount;
    double balanceAfter;
    time_t timestamp;
};

// Bank Account structure
struct BankAccount {
    int accountNumber;
    double balance;
    pthread_mutex_t lock;
    vector<Transaction> transactionHistory;
};

// Global variables
vector<BankAccount> accounts;
pthread_mutex_t logMutex = PTHREAD_MUTEX_INITIALIZER;
Config config;
int totalTransactionsCompleted = 0;
pthread_mutex_t progressMutex = PTHREAD_MUTEX_INITIALIZER;

// Function to clear screen (works on Linux/Unix)
void clearScreen() {
    cout << "\033[2J\033[H";
}

// Function to print a beautiful header
void printHeader() {
    cout << Colors::CYAN << Colors::BOLD;
    cout << "\n╔══════════════════════════════════════════════════════════════════════════════╗\n";
    cout << "║" << string(76, ' ') << "║\n";
    cout << "║" << string(20, ' ') << "🏦 MULTITHREADED BANK TRANSACTION SIMULATOR 🏦" << string(20, ' ') << "║\n";
    cout << "║" << string(76, ' ') << "║\n";
    cout << "╚══════════════════════════════════════════════════════════════════════════════╝\n";
    cout << Colors::RESET;
}

// Function to print a section separator
void printSeparator(const string& title = "") {
    if (title.empty()) {
        cout << Colors::DIM << string(80, '─') << Colors::RESET << endl;
    } else {
        int padding = (78 - title.length()) / 2;
        cout << Colors::CYAN << "┌" << string(78, '─') << "┐" << Colors::RESET << endl;
        cout << Colors::CYAN << "│" << Colors::RESET 
             << string(padding, ' ') << Colors::BOLD << Colors::YELLOW << title 
             << Colors::RESET << string(78 - padding - title.length(), ' ') 
             << Colors::CYAN << "│" << Colors::RESET << endl;
        cout << Colors::CYAN << "└" << string(78, '─') << "┘" << Colors::RESET << endl;
    }
}

// Function to get integer input with validation
int getIntInput(const string& prompt, int min = 1, int max = 100) {
    int value;
    while (true) {
        cout << Colors::BRIGHT_CYAN << "➤ " << Colors::RESET << prompt;
        if (cin >> value && value >= min && value <= max) {
            cin.ignore(numeric_limits<streamsize>::max(), '\n');
            return value;
        } else {
            cout << Colors::RED << "❌ Invalid input! Please enter a number between " 
                 << min << " and " << max << "." << Colors::RESET << endl;
            cin.clear();
            cin.ignore(numeric_limits<streamsize>::max(), '\n');
        }
    }
}

// Function to get double input with validation
double getDoubleInput(const string& prompt, double min = 0.01) {
    double value;
    while (true) {
        cout << Colors::BRIGHT_CYAN << "➤ " << Colors::RESET << prompt;
        if (cin >> value && value >= min) {
            cin.ignore(numeric_limits<streamsize>::max(), '\n');
            return value;
        } else {
            cout << Colors::RED << "❌ Invalid input! Please enter a number >= " 
                 << min << "." << Colors::RESET << endl;
            cin.clear();
            cin.ignore(numeric_limits<streamsize>::max(), '\n');
        }
    }
}

// Function to get user configuration
Config getUserConfig() {
    Config cfg;
    
    printHeader();
    cout << Colors::BRIGHT_MAGENTA << Colors::BOLD << "\n📋 CONFIGURATION SETUP\n" << Colors::RESET;
    printSeparator("Please provide the following information");
    cout << endl;
    
    cfg.numAccounts = getIntInput("Number of Bank Accounts (1-10): ", 1, 10);
    cfg.initialBalance = getDoubleInput("Initial Balance per Account ($): ", 0.01);
    cfg.numClients = getIntInput("Number of Client Threads (1-20): ", 1, 20);
    cfg.transactionsPerClient = getIntInput("Transactions per Client (1-50): ", 1, 50);
    cfg.minAmount = getDoubleInput("Minimum Transaction Amount ($): ", 0.01);
    cfg.maxAmount = getDoubleInput("Maximum Transaction Amount ($): ", cfg.minAmount);
    
    return cfg;
}

// Function to log transaction with beautiful formatting
void logTransaction(int clientId, int accountNumber, const string& type, 
                    double amount, double balanceAfter) {
    pthread_mutex_lock(&logMutex);
    
    Transaction trans;
    trans.clientId = clientId;
    trans.accountNumber = accountNumber;
    trans.type = type;
    trans.amount = amount;
    trans.balanceAfter = balanceAfter;
    trans.timestamp = time(nullptr);
    
    // Add to account history
    for (auto& acc : accounts) {
        if (acc.accountNumber == accountNumber) {
            acc.transactionHistory.push_back(trans);
            break;
        }
    }
    
    // Update progress
    totalTransactionsCompleted++;
    int totalTransactions = config.numClients * config.transactionsPerClient;
    int progress = (totalTransactionsCompleted * 100) / totalTransactions;
    
    // Print transaction with colors
    time_t now = time(nullptr);
    tm* localTime = localtime(&now);
    
    // Choose color based on transaction type
    string typeColor;
    string icon;
    if (type.find("DEPOSIT") != string::npos) {
        typeColor = Colors::GREEN;
        icon = "💰";
    } else if (type.find("WITHDRAW") != string::npos) {
        typeColor = Colors::YELLOW;
        icon = "💸";
    } else {
        typeColor = Colors::BLUE;
        icon = "🔄";
    }
    
    cout << Colors::DIM << "[" << put_time(localTime, "%H:%M:%S") << "]" << Colors::RESET << " "
         << Colors::BRIGHT_CYAN << "Client " << setw(2) << clientId << Colors::RESET << " | "
         << typeColor << icon << " " << type << Colors::RESET
         << " $" << Colors::BOLD << setw(10) << fixed << setprecision(2) << amount << Colors::RESET
         << " | " << Colors::MAGENTA << "Account " << accountNumber << Colors::RESET
         << " | " << Colors::BRIGHT_GREEN << "Balance: $" << setw(12) << fixed << setprecision(2) 
         << balanceAfter << Colors::RESET
         << " [" << Colors::BRIGHT_YELLOW << progress << "%" << Colors::RESET << "]" << endl;
    
    pthread_mutex_unlock(&logMutex);
}

// Function to deposit money
bool deposit(int clientId, int accountIndex, double amount) {
    pthread_mutex_lock(&accounts[accountIndex].lock);
    
    accounts[accountIndex].balance += amount;
    double newBalance = accounts[accountIndex].balance;
    
    pthread_mutex_unlock(&accounts[accountIndex].lock);
    
    logTransaction(clientId, accounts[accountIndex].accountNumber, 
                   "DEPOSIT  ", amount, newBalance);
    return true;
}

// Function to withdraw money
bool withdraw(int clientId, int accountIndex, double amount) {
    bool success = false;
    double currentBalance;
    int accountNumber;
    
    pthread_mutex_lock(&accounts[accountIndex].lock);
    
    accountNumber = accounts[accountIndex].accountNumber;
    currentBalance = accounts[accountIndex].balance;
    
    if (currentBalance >= amount) {
        accounts[accountIndex].balance -= amount;
        double newBalance = accounts[accountIndex].balance;
        pthread_mutex_unlock(&accounts[accountIndex].lock);
        
        logTransaction(clientId, accountNumber, 
                       "WITHDRAW ", amount, newBalance);
        success = true;
    } else {
        pthread_mutex_unlock(&accounts[accountIndex].lock);
        
        pthread_mutex_lock(&logMutex);
        time_t now = time(nullptr);
        tm* localTime = localtime(&now);
        cout << Colors::DIM << "[" << put_time(localTime, "%H:%M:%S") << "]" << Colors::RESET << " "
             << Colors::BRIGHT_CYAN << "Client " << clientId << Colors::RESET << " | "
             << Colors::RED << "⚠️  INSUFFICIENT FUNDS" << Colors::RESET
             << " | Attempted: $" << Colors::BOLD << fixed << setprecision(2) << amount << Colors::RESET
             << " | " << Colors::MAGENTA << "Account " << accountNumber << Colors::RESET
             << " | " << Colors::YELLOW << "Current Balance: $" << fixed << setprecision(2) 
             << currentBalance << Colors::RESET << endl;
        pthread_mutex_unlock(&logMutex);
    }
    
    return success;
}

// Function to transfer money between accounts
bool transfer(int clientId, int fromAccountIndex, int toAccountIndex, double amount) {
    // Lock both accounts in order to prevent deadlock
    int first = min(fromAccountIndex, toAccountIndex);
    int second = max(fromAccountIndex, toAccountIndex);
    
    pthread_mutex_lock(&accounts[first].lock);
    pthread_mutex_lock(&accounts[second].lock);
    
    bool success = false;
    if (accounts[fromAccountIndex].balance >= amount) {
        accounts[fromAccountIndex].balance -= amount;
        accounts[toAccountIndex].balance += amount;
        
        double fromBalance = accounts[fromAccountIndex].balance;
        double toBalance = accounts[toAccountIndex].balance;
        
        pthread_mutex_unlock(&accounts[second].lock);
        pthread_mutex_unlock(&accounts[first].lock);
        
        // Log transfer
        pthread_mutex_lock(&logMutex);
        time_t now = time(nullptr);
        tm* localTime = localtime(&now);
        totalTransactionsCompleted++;
        int totalTransactions = config.numClients * config.transactionsPerClient;
        int progress = (totalTransactionsCompleted * 100) / totalTransactions;
        
        cout << Colors::DIM << "[" << put_time(localTime, "%H:%M:%S") << "]" << Colors::RESET << " "
             << Colors::BRIGHT_CYAN << "Client " << clientId << Colors::RESET << " | "
             << Colors::BLUE << "🔄 TRANSFER" << Colors::RESET
             << " $" << Colors::BOLD << fixed << setprecision(2) << amount << Colors::RESET
             << " | " << Colors::MAGENTA << "Account " << accounts[fromAccountIndex].accountNumber 
             << Colors::RESET << " → " << Colors::MAGENTA << "Account " 
             << accounts[toAccountIndex].accountNumber << Colors::RESET
             << " | From: $" << fixed << setprecision(2) << fromBalance
             << " | To: $" << fixed << setprecision(2) << toBalance
             << " [" << Colors::BRIGHT_YELLOW << progress << "%" << Colors::RESET << "]" << endl;
        pthread_mutex_unlock(&logMutex);
        
        success = true;
    } else {
        pthread_mutex_unlock(&accounts[second].lock);
        pthread_mutex_unlock(&accounts[first].lock);
        
        pthread_mutex_lock(&logMutex);
        time_t now = time(nullptr);
        tm* localTime = localtime(&now);
        cout << Colors::DIM << "[" << put_time(localTime, "%H:%M:%S") << "]" << Colors::RESET << " "
             << Colors::BRIGHT_CYAN << "Client " << clientId << Colors::RESET << " | "
             << Colors::RED << "❌ TRANSFER FAILED" << Colors::RESET
             << " | Insufficient funds in " << Colors::MAGENTA << "Account " 
             << accounts[fromAccountIndex].accountNumber << Colors::RESET << endl;
        pthread_mutex_unlock(&logMutex);
    }
    
    return success;
}

// Client thread function
void* clientThread(void* arg) {
    int clientId = *((int*)arg);
    
    for (int i = 0; i < config.transactionsPerClient; i++) {
        int operation = rand() % 3; // 0: deposit, 1: withdraw, 2: transfer
        int accountIndex = rand() % accounts.size();
        double amount = config.minAmount + 
                       (rand() % (int)((config.maxAmount - config.minAmount) * 100)) / 100.0;
        
        switch (operation) {
            case 0: // Deposit
                deposit(clientId, accountIndex, amount);
                break;
                
            case 1: // Withdraw
                withdraw(clientId, accountIndex, amount);
                break;
                
            case 2: // Transfer
                if (accounts.size() > 1) {
                    int toAccountIndex = rand() % accounts.size();
                    while (toAccountIndex == accountIndex) {
                        toAccountIndex = rand() % accounts.size();
                    }
                    transfer(clientId, accountIndex, toAccountIndex, amount);
                }
                break;
        }
        
        // Random delay to simulate real-world transaction time
        usleep((rand() % 200000) + 50000); // 50ms to 250ms
    }
    
    return nullptr;
}

// Function to print final account balances and statistics
void printFinalReport() {
    cout << "\n\n";
    printSeparator("FINAL ACCOUNT REPORT");
    cout << endl;
    
    for (size_t i = 0; i < accounts.size(); i++) {
        auto& acc = accounts[i];
        
        cout << Colors::BRIGHT_BLUE << "┌" << string(78, '─') << "┐" << Colors::RESET << endl;
        cout << Colors::BRIGHT_BLUE << "│" << Colors::RESET 
             << Colors::BOLD << Colors::CYAN << "  Account #" << acc.accountNumber 
             << Colors::RESET << string(60, ' ') << Colors::BRIGHT_BLUE << "│" << Colors::RESET << endl;
        cout << Colors::BRIGHT_BLUE << "├" << string(78, '─') << "┤" << Colors::RESET << endl;
        
        cout << Colors::BRIGHT_BLUE << "│" << Colors::RESET 
             << "  " << Colors::YELLOW << "Final Balance:" << Colors::RESET 
             << string(20, ' ') << Colors::BRIGHT_GREEN << "$" << fixed << setprecision(2) 
             << setw(12) << acc.balance << Colors::RESET 
             << string(40, ' ') << Colors::BRIGHT_BLUE << "│" << Colors::RESET << endl;
        
        cout << Colors::BRIGHT_BLUE << "│" << Colors::RESET 
             << "  " << Colors::YELLOW << "Total Transactions:" << Colors::RESET 
             << string(15, ' ') << Colors::BRIGHT_CYAN << acc.transactionHistory.size() 
             << Colors::RESET << string(40, ' ') << Colors::BRIGHT_BLUE << "│" << Colors::RESET << endl;
        
        if (!acc.transactionHistory.empty()) {
            cout << Colors::BRIGHT_BLUE << "├" << string(78, '─') << "┤" << Colors::RESET << endl;
            cout << Colors::BRIGHT_BLUE << "│" << Colors::RESET 
                 << "  " << Colors::BOLD << Colors::MAGENTA << "Last 5 Transactions:" 
                 << Colors::RESET << string(50, ' ') << Colors::BRIGHT_BLUE << "│" << Colors::RESET << endl;
            
            int start = max(0, (int)acc.transactionHistory.size() - 5);
            for (int j = start; j < acc.transactionHistory.size(); j++) {
                const Transaction& trans = acc.transactionHistory[j];
                tm* localTime = localtime(&trans.timestamp);
                
                string typeColor = (trans.type.find("DEPOSIT") != string::npos) ? Colors::GREEN :
                                  (trans.type.find("WITHDRAW") != string::npos) ? Colors::YELLOW : Colors::BLUE;
                
                cout << Colors::BRIGHT_BLUE << "│" << Colors::RESET 
                     << "    " << Colors::DIM << "[" << put_time(localTime, "%H:%M:%S") << "]" << Colors::RESET
                     << " Client " << Colors::BRIGHT_CYAN << trans.clientId << Colors::RESET
                     << " | " << typeColor << trans.type << Colors::RESET
                     << " $" << fixed << setprecision(2) << setw(8) << trans.amount
                     << " | Balance: $" << fixed << setprecision(2) << setw(10) << trans.balanceAfter
                     << string(20, ' ') << Colors::BRIGHT_BLUE << "│" << Colors::RESET << endl;
            }
        }
        
        cout << Colors::BRIGHT_BLUE << "└" << string(78, '─') << "┘" << Colors::RESET << endl;
        if (i < accounts.size() - 1) cout << endl;
    }
    
    // Summary statistics
    double totalBalance = 0;
    int totalTrans = 0;
    for (auto& acc : accounts) {
        totalBalance += acc.balance;
        totalTrans += acc.transactionHistory.size();
    }
    
    cout << "\n";
    printSeparator("SUMMARY STATISTICS");
    cout << Colors::BRIGHT_GREEN << "  ✓ Total Accounts: " << Colors::RESET << accounts.size() << endl;
    cout << Colors::BRIGHT_GREEN << "  ✓ Total Balance: " << Colors::RESET 
         << "$" << fixed << setprecision(2) << totalBalance << endl;
    cout << Colors::BRIGHT_GREEN << "  ✓ Total Transactions: " << Colors::RESET << totalTrans << endl;
    cout << Colors::BRIGHT_GREEN << "  ✓ Average Balance: " << Colors::RESET 
         << "$" << fixed << setprecision(2) << (totalBalance / accounts.size()) << endl;
}

int main() {
    srand(time(nullptr));
    
    // Get user configuration
    config = getUserConfig();
    
    // Clear screen and show initialization
    clearScreen();
    printHeader();
    
    cout << Colors::BRIGHT_MAGENTA << Colors::BOLD << "\n🚀 INITIALIZING SIMULATION\n" << Colors::RESET;
    printSeparator("Account Initialization");
    cout << endl;
    
    // Initialize bank accounts
    for (int i = 0; i < config.numAccounts; i++) {
        BankAccount acc;
        acc.accountNumber = i + 1;
        acc.balance = config.initialBalance;
        pthread_mutex_init(&acc.lock, nullptr);
        accounts.push_back(acc);
        
        cout << Colors::GREEN << "  ✓ " << Colors::RESET 
             << "Account " << Colors::BRIGHT_CYAN << acc.accountNumber << Colors::RESET
             << " initialized with balance: " << Colors::BRIGHT_GREEN 
             << "$" << fixed << setprecision(2) << config.initialBalance << Colors::RESET << endl;
        usleep(100000); // Small delay for visual effect
    }
    
    cout << "\n" << Colors::BRIGHT_YELLOW << "  ⏳ Starting " << config.numClients 
         << " client thread(s)..." << Colors::RESET << endl;
    usleep(500000);
    
    cout << "\n";
    printSeparator("LIVE TRANSACTION LOG");
    cout << endl;
    
    // Create client threads
    pthread_t* threads = new pthread_t[config.numClients];
    int* clientIds = new int[config.numClients];
    
    for (int i = 0; i < config.numClients; i++) {
        clientIds[i] = i + 1;
        if (pthread_create(&threads[i], nullptr, clientThread, &clientIds[i]) != 0) {
            cerr << Colors::RED << "❌ Error creating thread " << i << Colors::RESET << endl;
            return 1;
        }
    }
    
    // Wait for all threads to complete
    for (int i = 0; i < config.numClients; i++) {
        if (pthread_join(threads[i], nullptr) != 0) {
            cerr << Colors::RED << "❌ Error joining thread " << i << Colors::RESET << endl;
            return 1;
        }
    }
    
    // Print final report
    printFinalReport();
    
    // Cleanup mutexes
    for (auto& acc : accounts) {
        pthread_mutex_destroy(&acc.lock);
    }
    pthread_mutex_destroy(&logMutex);
    pthread_mutex_destroy(&progressMutex);
    
    delete[] threads;
    delete[] clientIds;
    
    cout << "\n" << Colors::BRIGHT_GREEN << Colors::BOLD 
         << "  ✨ Simulation completed successfully! ✨" << Colors::RESET << endl;
    cout << endl;
    
    return 0;
}

