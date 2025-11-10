#include <bits/stdc++.h>
#include <windows.h>
#include <conio.h>
using namespace std;

// ------------------- Console color & animation utils -------------------
HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);

enum Color {
    BLUE = FOREGROUND_BLUE | FOREGROUND_INTENSITY,
    GOLD = FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_INTENSITY,
    WHITE = FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE,
    CYAN = FOREGROUND_GREEN | FOREGROUND_BLUE | FOREGROUND_INTENSITY,
    RESET = WHITE
};

void setColor(WORD attr) { SetConsoleTextAttribute(hConsole, attr); }

void slowPrint(const string &s, unsigned int ms = 8) {
    for (char c : s) {
        cout << c << flush;
        Sleep(ms);
    }
}

void printCentered(const string &s, int width = 46) {
    int pad = max(0, (width - (int)s.size()) / 2);
    cout << string(pad, ' ') << s << "\n";
}

void drawBoxTop(int width) {
    cout << " ";
    for (int i = 0; i < width; i++) cout << "=";
    cout << "\n";
}

void drawBoxBottom(int width) {
    cout << " ";
    for (int i = 0; i < width; i++) cout << "=";
    cout << "\n";
}

void pause_console() {
    cout << "\n";
    setColor(GOLD);
    cout << "Press any key to continue...";
    setColor(WHITE);
    _getch();
}

void clearScreen() {
    system("cls");
}

// ------------------- Abstract base class: Account -------------------
class Account {
protected:
    int accountNumber;
    string ownerName;
    double balance;

public:
    Account(int accNo, string name, double bal)
        : accountNumber(accNo), ownerName(move(name)), balance(bal) {}

    virtual ~Account() = default;

    virtual void deposit(double amount) = 0;
    virtual bool withdraw(double amount) = 0;
    virtual void display() const = 0;

    int getAccountNumber() const { return accountNumber; }
    string getOwnerName() const { return ownerName; }
    double getBalance() const { return balance; }

protected:
    void addToBalance(double amt) { balance += amt; }
    bool subtractFromBalance(double amt) {
        if (amt > balance) return false;
        balance -= amt;
        return true;
    }
};

// ------------------- Derived classes -------------------
class SavingAccount : public Account {
private:
    double interestRate;
public:
    SavingAccount(int accNo, string name, double bal, double rate = 0.03)
        : Account(accNo, move(name), bal), interestRate(rate) {}

    void deposit(double amount) override {
        if (amount <= 0) return;
        addToBalance(amount);
    }

    bool withdraw(double amount) override {
        return subtractFromBalance(amount);
    }

    void display() const override {
        setColor(BLUE);
        cout << "Account No: " << accountNumber << " (Saving)\n";
        setColor(WHITE);
        cout << "Owner     : " << ownerName << "\n";
        cout << "Balance   : " << fixed << setprecision(2) << balance << "\n";
        cout << "Interest  : " << (interestRate * 100) << "%\n";
        setColor(WHITE);
    }
};

class CurrentAccount : public Account {
private:
    double overdraftLimit;
public:
    CurrentAccount(int accNo, string name, double bal, double overdraft = 500.0)
        : Account(accNo, move(name), bal), overdraftLimit(overdraft) {}

    void deposit(double amount) override {
        if (amount <= 0) return;
        addToBalance(amount);
    }

    bool withdraw(double amount) override {
        if (amount <= 0) return false;
        if ((balance - amount) < -overdraftLimit) return false;
        balance -= amount;
        return true;
    }

    void display() const override {
        setColor(BLUE);
        cout << "Account No: " << accountNumber << " (Current)\n";
        setColor(WHITE);
        cout << "Owner     : " << ownerName << "\n";
        cout << "Balance   : " << fixed << setprecision(2) << balance << "\n";
        cout << "Overdraft : " << overdraftLimit << "\n";
        setColor(WHITE);
    }
};

// ------------------- Account Manager -------------------
class AccountManager {
private:
    vector<unique_ptr<Account>> accounts;
    const string dataFile = "accounts.txt";
    int nextAccountNumber = 1001;

    void loadFromFile() {
        ifstream fin(dataFile);
        if (!fin) return;

        string line;
        while (getline(fin, line)) {
            if (line.empty()) continue;
            stringstream ss(line);
            string token;
            vector<string> parts;
            while (getline(ss, token, '|')) parts.push_back(token);
            if (parts.size() < 4) continue;

            int accNo = stoi(parts[0]);
            string type = parts[1];
            string name = parts[2];
            double bal = stod(parts[3]);

            if (type == "S") accounts.emplace_back(make_unique<SavingAccount>(accNo, name, bal));
            else accounts.emplace_back(make_unique<CurrentAccount>(accNo, name, bal));

            if (accNo >= nextAccountNumber) nextAccountNumber = accNo + 1;
        }
    }

    void saveToFile() {
        ofstream fout(dataFile, ios::trunc);
        if (!fout) {
            setColor(GOLD);
            cerr << "Warning: could not write to " << dataFile << "\n";
            setColor(WHITE);
            return;
        }
        for (auto &p : accounts) {
            if (dynamic_cast<SavingAccount*>(p.get())) {
                fout << p->getAccountNumber() << "|S|" << p->getOwnerName() << "|" << p->getBalance() << "\n";
            } else {
                fout << p->getAccountNumber() << "|C|" << p->getOwnerName() << "|" << p->getBalance() << "\n";
            }
        }
    }

    Account* findAccount(int accNo) {
        for (auto &p : accounts)
            if (p->getAccountNumber() == accNo)
                return p.get();
        return nullptr;
    }

public:
    AccountManager() { loadFromFile(); }
    ~AccountManager() { saveToFile(); }

    int createAccount(char type, const string &name, double initialDeposit) {
        int accNo = nextAccountNumber++;
        if (type == 'S') accounts.emplace_back(make_unique<SavingAccount>(accNo, name, initialDeposit));
        else accounts.emplace_back(make_unique<CurrentAccount>(accNo, name, initialDeposit));
        saveToFile();
        return accNo;
    }

    bool depositToAccount(int accNo, double amount) {
        Account* acc = findAccount(accNo);
        if (!acc) return false;
        acc->deposit(amount);
        saveToFile();
        return true;
    }

    bool withdrawFromAccount(int accNo, double amount) {
        Account* acc = findAccount(accNo);
        if (!acc) return false;
        bool ok = acc->withdraw(amount);
        saveToFile();
        return ok;
    }

    bool showAccount(int accNo) {
        Account* acc = findAccount(accNo);
        if (!acc) return false;
        acc->display();
        return true;
    }

    void listAllAccounts() const {
        setColor(GOLD);
        cout << "\nAll Accounts:\n";
        setColor(WHITE);
        for (auto &p : accounts) {
            if (dynamic_cast<SavingAccount*>(p.get())) {
                setColor(BLUE); cout << "[S] "; setColor(WHITE);
            } else { setColor(BLUE); cout << "[C] "; setColor(WHITE); }
            cout << p->getAccountNumber() << " - " << p->getOwnerName() 
                 << " (" << fixed << setprecision(2) << p->getBalance() << ")\n";
        }
        setColor(WHITE);
    }
};

// ------------------- Fancy Console UI -------------------
void showFancyHeader() {
    clearScreen();
    setColor(BLUE);
    drawBoxTop(46);
    printCentered("BANK MANAGEMENT SYSTEM", 46);
    printCentered("— OOP C++ Demo —", 46);
    drawBoxBottom(46);
    setColor(GOLD);
    printCentered("Royal Blue & Gold Theme", 46);
    setColor(WHITE);
}

int mainMenu() {
    showFancyHeader();
    setColor(BLUE);
    cout << "\n  1. Open New Account\n";
    cout << "  2. Deposit Money\n";
    cout << "  3. Withdraw Money\n";
    cout << "  4. Show Account Details\n";
    cout << "  5. List All Accounts\n";
    cout << "  6. Exit\n";
    setColor(GOLD);
    cout << "\n Select option: ";
    setColor(WHITE);
    int ch;
    if (!(cin >> ch)) {
        cin.clear();
        cin.ignore(numeric_limits<streamsize>::max(), '\n');
        return -1;
    }
    cin.ignore(numeric_limits<streamsize>::max(), '\n');
    return ch;
}

int main() {
    AccountManager manager;

    // Animated intro
    setColor(BLUE);
    clearScreen();
    drawBoxTop(60);
    setColor(BLUE);
    slowPrint("   WELCOME TO\n", 6);
    setColor(GOLD);
    slowPrint("   BANK MANAGEMENT SYSTEM\n", 6);
    setColor(WHITE);
    drawBoxBottom(60);
    pause_console();

    while (true) {
        int choice = mainMenu();
        if (choice == -1) { setColor(GOLD); cout << "Invalid input. Try again."; setColor(WHITE); Sleep(800); continue; }

        if (choice == 1) {
            setColor(GOLD); cout << "\nOpen New Account\n"; setColor(WHITE);
            cout << "Enter owner name: "; string name; getline(cin, name);
            cout << "Account type (S = Saving, C = Current): "; char t; cin >> t; t = toupper(t); cin.ignore(numeric_limits<streamsize>::max(), '\n');
            cout << "Initial deposit amount: "; double amt; cin >> amt; cin.ignore(numeric_limits<streamsize>::max(), '\n');
            if (amt < 0) { setColor(GOLD); cout << "Invalid amount.\n"; setColor(WHITE); pause_console(); continue; }
            int accNo = manager.createAccount(t, name, amt);
            setColor(GOLD); cout << "\nAccount created successfully. Account Number: "; setColor(BLUE); cout << accNo << "\n"; setColor(WHITE);
            pause_console();
        }
        else if (choice == 2) {
            setColor(GOLD); cout << "\nDeposit Money\n"; setColor(WHITE);
            cout << "Enter account number: "; int acc; cin >> acc; cin.ignore(numeric_limits<streamsize>::max(), '\n');
            cout << "Amount to deposit: "; double amt; cin >> amt; cin.ignore(numeric_limits<streamsize>::max(), '\n');
            if (amt <= 0) { setColor(GOLD); cout << "Invalid amount.\n"; setColor(WHITE); pause_console(); continue; }
            if (manager.depositToAccount(acc, amt)) { setColor(BLUE); cout << "Deposit successful."; setColor(WHITE); }
            else { setColor(GOLD); cout << "Account not found."; setColor(WHITE); }
            pause_console();
        }
        else if (choice == 3) {
            setColor(GOLD); cout << "\nWithdraw Money\n"; setColor(WHITE);
            cout << "Enter account number: "; int acc; cin >> acc; cin.ignore(numeric_limits<streamsize>::max(), '\n');
            cout << "Amount to withdraw: "; double amt; cin >> amt; cin.ignore(numeric_limits<streamsize>::max(), '\n');
            if (amt <= 0) { setColor(GOLD); cout << "Invalid amount.\n"; setColor(WHITE); pause_console(); continue; }
            bool ok = manager.withdrawFromAccount(acc, amt);
            if (ok) { setColor(BLUE); cout << "Withdrawal successful."; setColor(WHITE); }
            else { setColor(GOLD); cout << "Withdrawal failed (insufficient funds or account not found)."; setColor(WHITE); }
            pause_console();
        }
        else if (choice == 4) {
            setColor(GOLD); cout << "\nShow Account Details\n"; setColor(WHITE);
            cout << "Enter account number: "; int acc; cin >> acc; cin.ignore(numeric_limits<streamsize>::max(), '\n');
            if (!manager.showAccount(acc)) { setColor(GOLD); cout << "Account not found."; setColor(WHITE); }
            pause_console();
        }
        else if (choice == 5) {
            manager.listAllAccounts();
            pause_console();
        }
        else if (choice == 6) {
            setColor(GOLD); cout << "\nExiting..." << endl; setColor(WHITE);
            break;
        }
        else {
            setColor(GOLD); cout << "Invalid choice." << endl; setColor(WHITE);
            pause_console();
        }
    }
    return 0;
}