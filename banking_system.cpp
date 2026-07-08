/*
    BANKING SYSTEM - C++ OOP Project
    ---------------------------------
    Classes: Transaction, Account, Customer, Bank
    Features: create customer/account, deposit, withdraw, transfer,
              transaction history, view account info
*/

#include <iostream>
#include <vector>
#include <string>
#include <ctime>
#include <iomanip>
#include <algorithm>
using namespace std;

// ----------------------------------------------------
// TRANSACTION CLASS
// ----------------------------------------------------
class Transaction {
private:
    int transactionID;
    string type;          // "Deposit", "Withdraw", "Transfer-Out", "Transfer-In"
    double amount;
    string timestamp;
    string accountNumber;

    string getCurrentTime() {
        time_t now = time(0);
        char buf[30];
        struct tm* tstruct = localtime(&now);
        strftime(buf, sizeof(buf), "%Y-%m-%d %H:%M:%S", tstruct);
        return string(buf);
    }

public:
    Transaction(int id, string t, double amt, string accNo) {
        transactionID = id;
        type = t;
        amount = amt;
        accountNumber = accNo;
        timestamp = getCurrentTime();
    }

    void displayTransaction() const {
        cout << "  [ID: " << setw(4) << transactionID << "] "
             << setw(14) << left << type
             << " | Amount: Rs. " << setw(10) << fixed << setprecision(2) << amount
             << " | " << timestamp << endl;
    }

    double getAmount() const { return amount; }
    string getType() const { return type; }
    int getID() const { return transactionID; }
};

// ----------------------------------------------------
// ACCOUNT CLASS
// ----------------------------------------------------
class Account {
private:
    string accountNumber;
    string accountType;      // "Savings" or "Current"
    double balance;
    vector<Transaction> transactionHistory;
    static int transactionCounter;   // shared across all accounts, gives unique IDs

public:
    Account() : balance(0) {}   // default constructor (needed for vector storage)

    Account(string accNo, string type, double initialBalance) {
        accountNumber = accNo;
        accountType = type;
        balance = (initialBalance >= 0) ? initialBalance : 0;
    }

    // ---- Deposit ----
    bool deposit(double amount) {
        if (amount <= 0) {
            cout << "Error: Deposit amount must be positive.\n";
            return false;
        }
        balance += amount;
        transactionCounter++;
        transactionHistory.push_back(Transaction(transactionCounter, "Deposit", amount, accountNumber));
        return true;
    }

    // ---- Withdraw ----
    bool withdraw(double amount) {
        if (amount <= 0) {
            cout << "Error: Withdrawal amount must be positive.\n";
            return false;
        }
        if (amount > balance) {
            cout << "Error: Insufficient balance. Current balance: Rs. " << balance << endl;
            return false;
        }
        balance -= amount;
        transactionCounter++;
        transactionHistory.push_back(Transaction(transactionCounter, "Withdraw", amount, accountNumber));
        return true;
    }

    // ---- Used internally by transfer ----
    void recordTransfer(double amount, bool isOutgoing) {
        transactionCounter++;
        string label = isOutgoing ? "Transfer-Out" : "Transfer-In";
        transactionHistory.push_back(Transaction(transactionCounter, label, amount, accountNumber));
    }

    void adjustBalance(double amount) {
        // amount can be negative (deduction) or positive (addition)
        balance += amount;
    }

    void showRecentTransactions(int count) const {
        if (transactionHistory.empty()) {
            cout << "  No transactions yet.\n";
            return;
        }
        int total = transactionHistory.size();
        int start = max(0, total - count);
        cout << "  Showing last " << (total - start) << " transaction(s):\n";
        for (int i = total - 1; i >= start; i--) {
            transactionHistory[i].displayTransaction();
        }
    }

    void displayAccountInfo() const {
        cout << "  Account No: " << accountNumber
             << " | Type: " << accountType
             << " | Balance: Rs. " << fixed << setprecision(2) << balance << endl;
    }

    double getBalance() const { return balance; }
    string getAccountNumber() const { return accountNumber; }
    string getAccountType() const { return accountType; }
};

int Account::transactionCounter = 1000;   // static member initialization

// ----------------------------------------------------
// CUSTOMER CLASS
// ----------------------------------------------------
class Customer {
private:
    string customerID;
    string name;
    string cnic;
    string contact;
    vector<Account> accounts;

public:
    Customer() {}

    Customer(string id, string n, string cnicNo, string contactNo) {
        customerID = id;
        name = n;
        cnic = cnicNo;
        contact = contactNo;
    }

    void addAccount(const Account& acc) {
        accounts.push_back(acc);
    }

    // Returns pointer so caller can modify the ACTUAL account, not a copy
    Account* findAccount(string accNo) {
        for (auto &acc : accounts) {
            if (acc.getAccountNumber() == accNo) {
                return &acc;
            }
        }
        return nullptr;
    }

    void displayCustomerInfo() const {
        cout << "\nCustomer ID: " << customerID
             << " | Name: " << name
             << " | CNIC: " << cnic
             << " | Contact: " << contact << endl;
        cout << "Accounts:\n";
        if (accounts.empty()) {
            cout << "  No accounts yet.\n";
        }
        for (const auto &acc : accounts) {
            acc.displayAccountInfo();
        }
    }

    string getCustomerID() const { return customerID; }
    string getName() const { return name; }
    vector<Account>& getAccounts() { return accounts; }
};

// ----------------------------------------------------
// BANK CLASS - manages all customers, ties everything together
// ----------------------------------------------------
class Bank {
private:
    vector<Customer> customers;
    int customerCounter = 0;
    int accountCounter = 0;

public:
    Customer* createCustomer(string name, string cnic, string contact) {
        customerCounter++;
        string id = "CUST" + to_string(customerCounter);
        customers.push_back(Customer(id, name, cnic, contact));
        cout << "Customer created successfully. Customer ID: " << id << endl;
        return &customers.back();
    }

    Customer* findCustomer(string custID) {
        for (auto &c : customers) {
            if (c.getCustomerID() == custID) return &c;
        }
        return nullptr;
    }

    bool createAccount(string custID, string accType, double initialDeposit) {
        Customer* cust = findCustomer(custID);
        if (cust == nullptr) {
            cout << "Error: Customer not found.\n";
            return false;
        }
        accountCounter++;
        string accNo = "AC" + to_string(1000 + accountCounter);
        cust->addAccount(Account(accNo, accType, initialDeposit));
        cout << "Account created successfully. Account No: " << accNo << endl;
        return true;
    }

    // Finds an account across ALL customers (needed for transfers)
    Account* findAccountGlobally(string accNo) {
        for (auto &c : customers) {
            Account* acc = c.findAccount(accNo);
            if (acc != nullptr) return acc;
        }
        return nullptr;
    }

    bool transfer(string fromAccNo, string toAccNo, double amount) {
        if (amount <= 0) {
            cout << "Error: Transfer amount must be positive.\n";
            return false;
        }
        Account* fromAcc = findAccountGlobally(fromAccNo);
        Account* toAcc = findAccountGlobally(toAccNo);

        if (fromAcc == nullptr || toAcc == nullptr) {
            cout << "Error: One or both accounts not found.\n";
            return false;
        }
        if (fromAcc->getBalance() < amount) {
            cout << "Error: Insufficient balance in source account.\n";
            return false;
        }

        fromAcc->adjustBalance(-amount);
        fromAcc->recordTransfer(amount, true);

        toAcc->adjustBalance(amount);
        toAcc->recordTransfer(amount, false);

        cout << "Transfer of Rs. " << fixed << setprecision(2) << amount
             << " successful from " << fromAccNo << " to " << toAccNo << endl;
        return true;
    }

    void listAllCustomers() const {
        if (customers.empty()) {
            cout << "No customers registered yet.\n";
            return;
        }
        for (const auto &c : customers) {
            c.displayCustomerInfo();
        }
    }

    vector<Customer>& getCustomers() { return customers; }
};

// ----------------------------------------------------
// MENU-DRIVEN MAIN PROGRAM
// ----------------------------------------------------
void showMenu() {
    cout << "\n========== BANKING SYSTEM ==========\n";
    cout << "1. Create Customer\n";
    cout << "2. Create Account\n";
    cout << "3. Deposit\n";
    cout << "4. Withdraw\n";
    cout << "5. Transfer Funds\n";
    cout << "6. View Account Info\n";
    cout << "7. View Recent Transactions\n";
    cout << "8. View All Customers\n";
    cout << "0. Exit\n";
    cout << "=====================================\n";
    cout << "Enter choice: ";
}

int main() {
    Bank bank;
    int choice;

    do {
        showMenu();
        cin >> choice;

        if (cin.fail()) {
            cin.clear();
            cin.ignore(10000, '\n');
            cout << "Invalid input. Please enter a number.\n";
            continue;
        }

        switch (choice) {
            case 1: {
                string name, cnic, contact;
                cin.ignore();
                cout << "Enter name: "; getline(cin, name);
                cout << "Enter CNIC: "; getline(cin, cnic);
                cout << "Enter contact: "; getline(cin, contact);
                bank.createCustomer(name, cnic, contact);
                break;
            }
            case 2: {
                string custID, accType;
                double initial;
                cout << "Enter Customer ID: "; cin >> custID;
                cout << "Enter Account Type (Savings/Current): "; cin >> accType;
                cout << "Enter Initial Deposit: "; cin >> initial;
                bank.createAccount(custID, accType, initial);
                break;
            }
            case 3: {
                string custID, accNo;
                double amount;
                cout << "Enter Customer ID: "; cin >> custID;
                cout << "Enter Account No: "; cin >> accNo;
                cout << "Enter Deposit Amount: "; cin >> amount;
                Customer* cust = bank.findCustomer(custID);
                if (cust) {
                    Account* acc = cust->findAccount(accNo);
                    if (acc) {
                        if (acc->deposit(amount)) cout << "Deposit successful.\n";
                    } else cout << "Account not found.\n";
                } else cout << "Customer not found.\n";
                break;
            }
            case 4: {
                string custID, accNo;
                double amount;
                cout << "Enter Customer ID: "; cin >> custID;
                cout << "Enter Account No: "; cin >> accNo;
                cout << "Enter Withdraw Amount: "; cin >> amount;
                Customer* cust = bank.findCustomer(custID);
                if (cust) {
                    Account* acc = cust->findAccount(accNo);
                    if (acc) {
                        if (acc->withdraw(amount)) cout << "Withdrawal successful.\n";
                    } else cout << "Account not found.\n";
                } else cout << "Customer not found.\n";
                break;
            }
            case 5: {
                string fromAcc, toAcc;
                double amount;
                cout << "Enter From Account No: "; cin >> fromAcc;
                cout << "Enter To Account No: "; cin >> toAcc;
                cout << "Enter Amount: "; cin >> amount;
                bank.transfer(fromAcc, toAcc, amount);
                break;
            }
            case 6: {
                string custID, accNo;
                cout << "Enter Customer ID: "; cin >> custID;
                cout << "Enter Account No: "; cin >> accNo;
                Customer* cust = bank.findCustomer(custID);
                if (cust) {
                    Account* acc = cust->findAccount(accNo);
                    if (acc) acc->displayAccountInfo();
                    else cout << "Account not found.\n";
                } else cout << "Customer not found.\n";
                break;
            }
            case 7: {
                string custID, accNo;
                int count;
                cout << "Enter Customer ID: "; cin >> custID;
                cout << "Enter Account No: "; cin >> accNo;
                cout << "How many recent transactions to show? "; cin >> count;
                Customer* cust = bank.findCustomer(custID);
                if (cust) {
                    Account* acc = cust->findAccount(accNo);
                    if (acc) acc->showRecentTransactions(count);
                    else cout << "Account not found.\n";
                } else cout << "Customer not found.\n";
                break;
            }
            case 8: {
                bank.listAllCustomers();
                break;
            }
            case 0: {
                cout << "Exiting... Thank you!\n";
                break;
            }
            default: {
                cout << "Invalid choice. Try again.\n";
            }
        }
    } while (choice != 0);

    return 0;
}
