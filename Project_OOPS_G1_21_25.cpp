#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
#include <stdexcept>
#include <vector>
#include <algorithm>

using namespace std;

// Base class: Account
class Account {
protected:
    string accountHolder;
    int accountNumber;
    double balance;

public:
    Account(const string &name, int number, double initialBalance){
        accountHolder=name; 
        accountNumber=number; 
        balance=initialBalance;
        if (initialBalance < 0) {
            throw invalid_argument("Initial balance cannot be negative.");
        }
    }

    virtual void deposit(double amount) {
        if (amount <= 0) throw invalid_argument("Deposit amount must be positive.");
        balance += amount;
    }

    virtual void withdraw(double amount) {
        if (amount <= 0) throw invalid_argument("Withdrawal amount must be positive.");
        if (amount > balance) throw runtime_error("Insufficient balance.");
        balance -= amount;
    }

    virtual void display() const {
        cout << "Account Holder: " << accountHolder << "\n";
        cout << "Account Number: " << accountNumber << "\n";
        cout << "Balance: " << balance << "\n";
    }

    virtual string getType() const = 0; // Pure virtual function for account type
    virtual double getAttribute() const = 0; // To get derived class-specific attribute

    int getAccountNumber() const { return accountNumber; }
    double getBalance() const { return balance; }
    void setBalance(double newBalance) { balance = newBalance; }
    string getAccountHolder() {return accountHolder; }
    virtual ~Account() {}
};

// Derived class: SavingsAccount
class SavingsAccount : public Account {
    double interestRate; // in percentage

public:
    SavingsAccount(const string &name, int number, double initialBalance, double rate)
    : Account(name, number, initialBalance) {
            interestRate=rate;
        }

    void display() const {
        cout << "[Savings Account]\n";
        Account::display();
        cout << "Interest Rate: " << interestRate << "%\n";
    }

    string getType() const { return "Savings"; }
    double getAttribute() const { return interestRate; }
};

// Derived class: CurrentAccount
class CurrentAccount : public Account {
    double overdraftLimit;

public:
    CurrentAccount(const string &name, int number, double initialBalance, double limit)
        : Account(name, number, initialBalance) {
            overdraftLimit=limit;
        }

    void withdraw(double amount) {
        if (amount <= 0) throw invalid_argument("Withdrawal amount must be positive.");
        if (amount > balance + overdraftLimit) throw runtime_error("Overdraft limit exceeded.");
        balance -= amount;
    }

    void display() const {
        cout << "[Current Account]\n";
        Account::display();
        cout << "Overdraft Limit: " << overdraftLimit << "\n";
    }

    string getType() const { return "Current"; }
    double getAttribute() const { return overdraftLimit; }
};

// Global container for accounts
vector<Account*> accounts;

// Function to write all accounts back to the file
void updateAccountsFile(const string &filename) {
    ofstream file(filename, ios::trunc); // Open in truncation mode to overwrite the file
    if (!file) throw runtime_error("Unable to open file for writing.");

    for (const auto &account : accounts) {
        if (account->getType() == "Savings") {
            file << account->getAccountNumber() << "|" << account->getAccountHolder() << "|" << account->getBalance() <<
             "|Savings|" << account->getAttribute() << endl;
        } 
        else if (account->getType() == "Current") {
            file << account->getAccountNumber() << "|" << account->getAccountHolder() << "|" << account->getBalance()
             << "|Current|" << account->getAttribute() << endl;
        }
    }
    file.close();
}

// Function to add an account to the file
void addAccountToFile(const string &filename){
    ofstream file(filename, ios::app);
    if (!file) throw runtime_error("Unable to open file.");

    string name, type;
    int accountNumber;
    double balance, attribute;

    cout << "Enter Account Holder's Name: ";
    cin.ignore();
    getline(cin, name);
    cout << "Enter Account Number: ";
    cin >> accountNumber;
    cout << "Enter Initial Balance: ";
    cin >> balance;
    cout << "Enter Account Type (Savings/Current): ";
    cin >> type;

    Account* account = nullptr;
    if (type == "Savings") {
        cout << "Enter Interest Rate (%): ";
        cin >> attribute;
        account = new SavingsAccount(name, accountNumber, balance, attribute);
        file << accountNumber << "|" << name << "|" << balance << "|Savings|" << attribute << endl;
    } 
    else if (type == "Current") {
        cout << "Enter Overdraft Limit: ";
        cin >> attribute;
        account = new CurrentAccount(name, accountNumber, balance, attribute);
        file << accountNumber << "|" << name << "|" << balance << "|Current|" << attribute << endl;
    } 
    else {
        cout << "Invalid account type.\n";
        return;
    }

    accounts.push_back(account);
    cout << "Account added successfully.\n";
    file.close();
}

// Function to display account details
void displayAccountFromFile(const string &filename) {
    ifstream file(filename);
    if (!file) throw runtime_error("Unable to open file.");

    string line, name, type;
    int accountNumber;
    double balance, attribute;

    for (auto account : accounts) {
        delete account;
    }
    accounts.clear();

    cout << "\n--- Account Details ---\n";
    while (getline(file, line)) {
        stringstream ss(line);
        getline(ss, line, '|');
        accountNumber = stoi(line);
        getline(ss, name, '|');
        ss >> balance;
        ss.ignore();
        getline(ss, type, '|');
        ss >> attribute;

        Account* account = nullptr;
        if (type == "Savings") {
            account = new SavingsAccount(name, accountNumber, balance, attribute);
        } 
        else if (type == "Current") {
            account = new CurrentAccount(name, accountNumber, balance, attribute);
        }

        if (account) {
            accounts.push_back(account);
            account->display();
            cout << "---------------------\n";
        }
    }
    file.close();
}

// Function to perform a transaction
void performTransaction(const string &filename) {
    int accountNumber;
    double amount;
    char transactionType;

    cout << "Enter Account Number: ";
    cin >> accountNumber;

    bool found=false;
    Account* it;
    for(auto t:accounts){
        if(t->getAccountNumber()==accountNumber){
            found=true;
            it=t;
        }
    }

    if(!found){
        cout << "Account not found.\n";
        return; 
    }

    cout << "Enter Transaction Type (D/W): ";
    cin >> transactionType;
    cout << "Enter Amount: ";
    cin >> amount;

    try {
        if (transactionType == 'D' || transactionType == 'd') {
            (it)->deposit(amount);
        } 
        else if (transactionType == 'W' || transactionType == 'w') {
            (it)->withdraw(amount);
        } 
        else {
            cout << "Invalid transaction type.\n";
            return;
        }
        cout << "Transaction successful. Updated details:\n";
        (it)->display();

        // Update the file with the new balances and details
        updateAccountsFile(filename);

    } 
    catch (const exception &e){
        cout << "Transaction failed: " << e.what() << endl;
    }
}


int main() {
    try{
    string filename = "accounts.txt";
    int choice;

    do {
        cout << "\n--- Banking System Menu ---\n";
        cout << "1. Add Account\n";
        cout << "2. Display Accounts\n";
        cout << "3. Perform Transaction\n";
        cout << "4. Exit\n";
        cout << "Enter your choice: ";
        cin >> choice;

        switch (choice) {
        case 1:
            addAccountToFile(filename);
            break;
        case 2:
            displayAccountFromFile(filename);
            break;
        case 3:
            performTransaction(filename);
            break;
        case 4:
            cout << "Exiting...\n";
            break;
        default:
            cout << "Invalid choice. Try again.\n";
        }
    } while (choice != 4);
    }
    catch(const exception &e){
        cout<<e.what()<<endl;
    }
    // Clean up allocated memory
    for (auto account : accounts) {
        delete account;
    }
    return 0;
}