
#include <windows.h>
#include <iostream>
#include <sql.h>
#include <sqlext.h>

// Function to show detailed SQL error messages
void showError(unsigned int handleType, const SQLHANDLE& handle) {
    SQLWCHAR SQLState[1024];
    SQLWCHAR message[1024];
    SQLINTEGER nativeError;
    SQLSMALLINT textLength;
    SQLRETURN retCode;
    int i = 1; // Record number starts from 1

    // Loop to get all diagnostic records
    while (SQL_SUCCEEDED(retCode = SQLGetDiagRecW(handleType, handle, i++, SQLState, &nativeError, message, sizeof(message) / sizeof(SQLWCHAR), &textLength))) {
        std::wcerr << L"SQL State: " << SQLState << L"\n"
                   << L"SQL Error Message: " << message << L"\n"
                   << L"Native Error Code: " << nativeError << L"\n";
    }
}

// Function to connect to the database
SQLHANDLE connectToDatabase() {
    SQLHANDLE sqlEnvHandle;
    SQLHANDLE sqlConnHandle;

    // Allocate environment handle
    if (SQLAllocHandle(SQL_HANDLE_ENV, SQL_NULL_HANDLE, &sqlEnvHandle) != SQL_SUCCESS) {
        std::wcerr << L"Failed to allocate environment handle.\n";
        return nullptr;
    }

    // Set environment attribute
    if (SQLSetEnvAttr(sqlEnvHandle, SQL_ATTR_ODBC_VERSION, (SQLPOINTER)SQL_OV_ODBC3, 0) != SQL_SUCCESS) {
        std::wcerr << L"Failed to set environment attribute.\n";
        SQLFreeHandle(SQL_HANDLE_ENV, sqlEnvHandle);
        return nullptr;
    }

    // Allocate connection handle
    if (SQLAllocHandle(SQL_HANDLE_DBC, sqlEnvHandle, &sqlConnHandle) != SQL_SUCCESS) {
        std::wcerr << L"Failed to allocate connection handle.\n";
        SQLFreeHandle(SQL_HANDLE_ENV, sqlEnvHandle);
        return nullptr;
    }

    // Connect to the data source
    SQLWCHAR retConString[1024];
    switch (SQLDriverConnectW(sqlConnHandle, NULL, 
        (SQLWCHAR*)L"DSN=MoneyTreesDSN;UID=Flavia;PWD=Darken&5h33p !;", 
        SQL_NTS, retConString, 1024, NULL, SQL_DRIVER_NOPROMPT)) {
    case SQL_SUCCESS:
        std::wcout << L"Connection successful.\n";
        break;
    case SQL_SUCCESS_WITH_INFO:
        std::wcout << L"Connection successful with warnings.\n";
        break;
    case SQL_INVALID_HANDLE:
    case SQL_ERROR:
    default:
        showError(SQL_HANDLE_DBC, sqlConnHandle);
        SQLFreeHandle(SQL_HANDLE_DBC, sqlConnHandle);
        SQLFreeHandle(SQL_HANDLE_ENV, sqlEnvHandle);
        return nullptr;
    }

    return sqlConnHandle;
}

// Function to execute a SQL query
void executeQuery(SQLHANDLE sqlConnHandle, const wchar_t* query) {
    SQLHANDLE sqlStmtHandle;
    if (SQLAllocHandle(SQL_HANDLE_STMT, sqlConnHandle, &sqlStmtHandle) != SQL_SUCCESS) {
        std::wcerr << L"Failed to allocate statement handle.\n";
        return;
    }

    if (SQLExecDirectW(sqlStmtHandle, (SQLWCHAR*)query, SQL_NTS) != SQL_SUCCESS) {
        showError(SQL_HANDLE_STMT, sqlStmtHandle);
    } else {
        std::wcout << L"Query executed successfully.\n";
    }

    SQLFreeHandle(SQL_HANDLE_STMT, sqlStmtHandle);
}

// Function to fetch and display data
void fetchData(SQLHANDLE sqlConnHandle, const wchar_t* query) {
    SQLHANDLE sqlStmtHandle;
    if (SQLAllocHandle(SQL_HANDLE_STMT, sqlConnHandle, &sqlStmtHandle) != SQL_SUCCESS) {
        std::wcerr << L"Failed to allocate statement handle.\n";
        return;
    }

    if (SQLExecDirectW(sqlStmtHandle, (SQLWCHAR*)query, SQL_NTS) != SQL_SUCCESS) {
        showError(SQL_HANDLE_STMT, sqlStmtHandle);
    } else {
        SQLWCHAR data[256];
        SQLLEN dataLen;
        while (SQLFetch(sqlStmtHandle) == SQL_SUCCESS) {
            SQLGetData(sqlStmtHandle, 1, SQL_C_WCHAR, data, sizeof(data) / sizeof(SQLWCHAR), &dataLen);
            std::wcout << L"Data: " << data << L"\n";
        }
    }

    SQLFreeHandle(SQL_HANDLE_STMT, sqlStmtHandle);
}

int main() {
    // Connect to the database
    SQLHANDLE sqlConnHandle = connectToDatabase();
    if (!sqlConnHandle) {
        return -1;
    }

    // // Example query to create ESG data table
    // const wchar_t* createESGTableQuery = L"CREATE TABLE esg_data (company_id INT PRIMARY KEY, company_name NVARCHAR(255), esg_score FLOAT, updated_at DATETIME)";
    // executeQuery(sqlConnHandle, createESGTableQuery);

    // // Example query to create portfolios table
    // const wchar_t* createPortfoliosTableQuery = L"CREATE TABLE portfolios (user_id INT, company_id INT, shares_owned INT, PRIMARY KEY (user_id, company_id))";
    // executeQuery(sqlConnHandle, createPortfoliosTableQuery);

    // // Example query to create historical data table
    // const wchar_t* createHistoricalTableQuery = L"CREATE TABLE historical_data (company_id INT, date DATE, esg_score FLOAT, stock_price FLOAT)";
    // executeQuery(sqlConnHandle, createHistoricalTableQuery);

    // // Example query to insert ESG data
    // const wchar_t* insertESGDataQuery = L"INSERT INTO esg_data (company_id, company_name, esg_score, updated_at) VALUES (1, 'TestCompany', 85.6, '2023-07-01 12:00:00')";
    // executeQuery(sqlConnHandle, insertESGDataQuery);

    // // Example query to fetch ESG data
    // fetchData(sqlConnHandle, L"SELECT * FROM esg_data");

    // Clean up
    SQLFreeHandle(SQL_HANDLE_DBC, sqlConnHandle);
    return 0;
}
