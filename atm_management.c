#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <time.h>
#include <math.h>

#define MAX_ACCOUNTS 100
#define DATA_FILE "accounts.dat"
#define NAME_LENGTH 50
#define PIN_LENGTH 5
#define MAX_TRANSACTIONS 5

typedef struct {
    char type[12];
    double amount;
    time_t date;
} Transaction;

typedef struct {
    char name[NAME_LENGTH];
    long long accountNumber;
    char pin[PIN_LENGTH];
    double balance;
    int blocked;
    int transactionCount;
    Transaction transactions[MAX_TRANSACTIONS];
} Account;

void readLine(const char *message, char *buffer, size_t size);
long long readLongLong(const char *message);
double readAmount(const char *message, int allowZero);
int readPin(const char *message, char pin[]);
int loadAccounts(Account accounts[]);
void saveAccounts(Account accounts[], int count);
int findAccount(Account accounts[], int count, long long accountNumber);
void createAccount(Account accounts[], int *count);
void login(Account accounts[], int count);
void accountMenu(Account accounts[], int count, int index);
void checkBalance(const Account *account);
void deposit(Account accounts[], int count, int index);
void withdraw(Account accounts[], int count, int index);
void changePin(Account accounts[], int count, int index);
void showStatement(const Account *account);
void addTransaction(Account *account, const char *type, double amount);
void printMainMenu(void);

void readLine(const char *message, char *buffer, size_t size) {
    size_t length;

    printf("%s", message);

    if (fgets(buffer, (int)size, stdin) == NULL) {
        buffer[0] = '\0';
        return;
    }

    length = strlen(buffer);

    if (length > 0 && buffer[length - 1] == '\n') {
        buffer[length - 1] = '\0';
    } else {
        int ch;
        while ((ch = getchar()) != '\n' && ch != EOF) {
        }
    }
}

long long readLongLong(const char *message) {
    char input[100];
    char *end;
    long long value;

    while (1) {
        readLine(message, input, sizeof(input));

        value = strtoll(input, &end, 10);

        while (isspace((unsigned char)*end)) {
            end++;
        }

        if (input[0] != '\0' && *end == '\0') {
            return value;
        }

        printf("Please enter a valid whole number.\n");
    }
}

double readAmount(const char *message, int allowZero) {
    char input[100];
    char *end;
    double amount;

    while (1) {
        readLine(message, input, sizeof(input));

        amount = strtod(input, &end);

        while (isspace((unsigned char)*end)) {
            end++;
        }

        if (input[0] != '\0' && *end == '\0' && isfinite(amount)
            && (allowZero ? amount >= 0 : amount > 0)) {
            return amount;
        }

        if (allowZero) {
            printf("Enter a valid amount (zero or more).\n");
        } else {
            printf("Enter an amount greater than zero.\n");
        }
    }
}

int readPin(const char *message, char pin[]) {
    char input[30];
    int i;

    while (1) {
        readLine(message, input, sizeof(input));

        if (strlen(input) == 4) {
            for (i = 0; i < 4 && isdigit((unsigned char)input[i]); i++) {
            }

            if (i == 4) {
                strcpy(pin, input);
                return 1;
            }
        }

        printf("PIN must contain exactly 4 digits.\n");
    }
}

int loadAccounts(Account accounts[]) {
    FILE *file = fopen(DATA_FILE, "rb");
    int count;

    if (file == NULL) {
        return 0;
    }

    count = (int)fread(accounts, sizeof(Account), MAX_ACCOUNTS, file);
    fclose(file);

    return count;
}

void saveAccounts(Account accounts[], int count) {
    FILE *file = fopen(DATA_FILE, "wb");

    if (file == NULL) {
        printf("Unable to save account data.\n");
        return;
    }

    fwrite(accounts, sizeof(Account), (size_t)count, file);
    fclose(file);
}

int findAccount(Account accounts[], int count, long long accountNumber) {
    int i;

    for (i = 0; i < count; i++) {
        if (accounts[i].accountNumber == accountNumber) {
            return i;
        }
    }

    return -1;
}

void addTransaction(Account *account, const char *type, double amount) {
    int i;
    int position;

    if (account->transactionCount < MAX_TRANSACTIONS) {
        position = account->transactionCount;
        account->transactionCount++;
    } else {
        for (i = 1; i < MAX_TRANSACTIONS; i++) {
            account->transactions[i - 1] = account->transactions[i];
        }
        position = MAX_TRANSACTIONS - 1;
    }

    snprintf(account->transactions[position].type,
             sizeof(account->transactions[position].type), "%s", type);
    account->transactions[position].amount = amount;
    account->transactions[position].date = time(NULL);
}

void createAccount(Account accounts[], int *count) {
    Account newAccount;
    char confirmPin[PIN_LENGTH];

    if (*count >= MAX_ACCOUNTS) {
        printf("\nThe account limit has been reached.\n");
        return;
    }

    printf("\n========== CREATE ACCOUNT ==========\n");

    readLine("Enter your full name: ", newAccount.name, sizeof(newAccount.name));

    while (strlen(newAccount.name) == 0) {
        printf("Name cannot be empty.\n");
        readLine("Enter your full name: ", newAccount.name, sizeof(newAccount.name));
    }

    while (1) {
        newAccount.accountNumber = readLongLong("Choose an account number: ");

        if (newAccount.accountNumber <= 0) {
            printf("Account number must be positive.\n");
        } else if (findAccount(accounts, *count, newAccount.accountNumber) != -1) {
            printf("This account number is already registered.\n");
        } else {
            break;
        }
    }

    readPin("Set a 4-digit PIN: ", newAccount.pin);

    while (1) {
        readPin("Confirm your PIN: ", confirmPin);

        if (strcmp(newAccount.pin, confirmPin) == 0) {
            break;
        }

        printf("PINs do not match. Please try again.\n");
    }

    newAccount.balance = readAmount("Enter opening balance: Rs. ", 1);
    newAccount.blocked = 0;
    newAccount.transactionCount = 0;

    accounts[*count] = newAccount;
    (*count)++;

    saveAccounts(accounts, *count);

    printf("\nAccount created successfully.\n");
    printf("Your account number is: %lld\n", newAccount.accountNumber);
}

void checkBalance(const Account *account) {
    printf("\nAvailable balance: Rs. %.2f\n", account->balance);
}

void deposit(Account accounts[], int count, int index) {
    double amount = readAmount("\nEnter amount to deposit: Rs. ", 0);

    accounts[index].balance += amount;
    addTransaction(&accounts[index], "Deposit", amount);
    saveAccounts(accounts, count);

    printf("Rs. %.2f deposited successfully.\n", amount);
    printf("Updated balance: Rs. %.2f\n", accounts[index].balance);
}

void withdraw(Account accounts[], int count, int index) {
    double amount = readAmount("\nEnter amount to withdraw: Rs. ", 0);

    if (amount > accounts[index].balance) {
        printf("Insufficient balance. Withdrawal cancelled.\n");
        return;
    }

    accounts[index].balance -= amount;
    addTransaction(&accounts[index], "Withdraw", amount);
    saveAccounts(accounts, count);

    printf("Please collect your cash: Rs. %.2f\n", amount);
    printf("Remaining balance: Rs. %.2f\n", accounts[index].balance);
}

void changePin(Account accounts[], int count, int index) {
    char oldPin[PIN_LENGTH];
    char newPin[PIN_LENGTH];
    char confirmPin[PIN_LENGTH];

    readPin("\nEnter your current PIN: ", oldPin);

    if (strcmp(oldPin, accounts[index].pin) != 0) {
        printf("Current PIN is incorrect. PIN was not changed.\n");
        return;
    }

    readPin("Enter your new 4-digit PIN: ", newPin);
    readPin("Confirm your new PIN: ", confirmPin);

    if (strcmp(newPin, confirmPin) != 0) {
        printf("PINs do not match. PIN was not changed.\n");
        return;
    }

    if (strcmp(newPin, oldPin) == 0) {
        printf("New PIN must be different from the current PIN.\n");
        return;
    }

    strcpy(accounts[index].pin, newPin);
    saveAccounts(accounts, count);

    printf("Your PIN has been changed successfully.\n");
}

void showStatement(const Account *account) {
    int i;
    char dateText[30];
    struct tm *timeInfo;

    printf("\n============== MINI STATEMENT ==============\n");
    printf("%-20s %-12s %-15s\n", "Date and time", "Type", "Amount (Rs.)");
    printf("--------------------------------------------\n");

    if (account->transactionCount == 0) {
        printf("No transactions have been made yet.\n");
        return;
    }

    for (i = 0; i < account->transactionCount; i++) {
        timeInfo = localtime(&account->transactions[i].date);

        if (timeInfo != NULL) {
            strftime(dateText, sizeof(dateText), "%d-%m-%Y %H:%M",
                     timeInfo);
        } else {
            strcpy(dateText, "Date unavailable");
        }

        printf("%-20s %-12s %15.2f\n",
               dateText,
               account->transactions[i].type,
               account->transactions[i].amount);
    }

    printf("--------------------------------------------\n");
}

void accountMenu(Account accounts[], int count, int index) {
    long long choice;

    while (1) {
        printf("\n====================================\n");
        printf("          ATM MAIN MENU\n");
        printf("====================================\n");
        printf("Welcome, %s\n", accounts[index].name);
        printf("1. Check Balance\n");
        printf("2. Deposit Money\n");
        printf("3. Withdraw Money\n");
        printf("4. Change PIN\n");
        printf("5. Mini Statement\n");
        printf("6. Logout\n");
        printf("====================================\n");

        choice = readLongLong("Enter your choice: ");

        switch (choice) {
            case 1:
                checkBalance(&accounts[index]);
                break;
            case 2:
                deposit(accounts, count, index);
                break;
            case 3:
                withdraw(accounts, count, index);
                break;
            case 4:
                changePin(accounts, count, index);
                break;
            case 5:
                showStatement(&accounts[index]);
                break;
            case 6:
                printf("\nYou have been logged out successfully.\n");
                return;
            default:
                printf("Invalid choice. Please select from 1 to 6.\n");
        }
    }
}

void login(Account accounts[], int count) {
    long long accountNumber;
    char enteredPin[PIN_LENGTH];
    int index;
    int attempts = 0;

    if (count == 0) {
        printf("\nNo accounts found. Please create an account first.\n");
        return;
    }

    printf("\n============== LOGIN ==============\n");
    accountNumber = readLongLong("Enter account number: ");

    index = findAccount(accounts, count, accountNumber);

    if (index == -1) {
        printf("Account number not found.\n");
        return;
    }

    if (accounts[index].blocked) {
        printf("This account is blocked due to 3 incorrect PIN attempts.\n");
        return;
    }

    while (attempts < 3) {
        readPin("Enter your 4-digit PIN: ", enteredPin);

        if (strcmp(enteredPin, accounts[index].pin) == 0) {
            printf("\nLogin successful. Welcome, %s!\n", accounts[index].name);
            accountMenu(accounts, count, index);
            return;
        }

        attempts++;

        if (attempts < 3) {
            printf("Incorrect PIN. %d attempt(s) remaining.\n", 3 - attempts);
        }
    }

    accounts[index].blocked = 1;
    saveAccounts(accounts, count);

    printf("\nToo many incorrect attempts. Your account has been blocked.\n");
}

void printMainMenu(void) {
    printf("\n====================================\n");
    printf("        ATM MANAGEMENT SYSTEM\n");
    printf("====================================\n");
    printf("1. Create New Account\n");
    printf("2. Login to Account\n");
    printf("3. Exit\n");
    printf("====================================\n");
}

int main(void) {
    Account accounts[MAX_ACCOUNTS];
    int count = loadAccounts(accounts);
    long long choice;

    printf("\nWelcome to the ATM Management System.\n");

    while (1) {
        printMainMenu();
        choice = readLongLong("Enter your choice: ");

        switch (choice) {
            case 1:
                createAccount(accounts, &count);
                break;
            case 2:
                login(accounts, count);
                break;
            case 3:
                printf("\nThank you for using our ATM. Goodbye!\n");
                return 0;
            default:
                printf("Invalid choice. Please select 1, 2 or 3.\n");
        }
    }
}
