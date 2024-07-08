#include <portfolio_operations.h>
#include <iostream>
#include <sql.h>
#include <sqlext.h>
#include <vector>

bool connectToDatabase(SQLHANDLE &henv, SQLHANDLE &hdbc) {
    SQLRETURN ret;

    // Allocate environment handle
    ret = SQLAllocHandle(SQL_HANDLE_ENV, SQL_NULL_HANDLE, &henv);
    if (ret != SQL_SUCCESS && ret != SQL_SUCCESS_WITH_INFO) {
        std::cerr << "Error allocating environment handle" << std::endl;
        return false;
    }

    // Set the ODBC version environment attribute
    ret = SQLSetEnvAttr(henv, SQL_ATTR_ODBC_VERSION, (SQLPOINTER)SQL_OV_ODBC3, 0);
    if (ret != SQL_SUCCESS && ret != SQL_SUCCESS_WITH_INFO) {
        std::cerr << "Error setting ODBC version" << std::endl;
        SQLFreeHandle(SQL_HANDLE_ENV, henv);
        return false;
    }

    // Allocate connection handle
    ret = SQLAllocHandle(SQL_HANDLE_DBC, henv, &hdbc);
    if (ret != SQL_SUCCESS && ret != SQL_SUCCESS_WITH_INFO) {
        std::cerr << "Error allocating connection handle" << std::endl;
        SQLFreeHandle(SQL_HANDLE_ENV, henv);
        return false;
    }

    SQLWCHAR connStr[] = L"DRIVER={ODBC Driver 18 for SQL Server};SERVER=moneytrees-sqlserver.database.windows.net;DATABASE=MoneyTreesDatabase;UID=Flavia;PWD=Darken&5h33p!;";

    // Connect to the database
    ret = SQLConnect(hdbc, connStr, SQL_NTS, NULL, 0, NULL, 0);
    if (ret != SQL_SUCCESS && ret != SQL_SUCCESS_WITH_INFO) {
        std::cerr << "Error connecting to the database" << std::endl;
        SQLFreeHandle(SQL_HANDLE_DBC, hdbc);
        SQLFreeHandle(SQL_HANDLE_ENV, henv);
        return false;
    }

    return true;
}

bool insertPortfolioEntry(const Portfolio &portfolio) {
    SQLHANDLE henv, hdbc, hstmt;
    SQLRETURN ret;

    if (!connectToDatabase(henv, hdbc)) {
        return false;
    }

    ret = SQLAllocHandle(SQL_HANDLE_STMT, hdbc, &hstmt);
    if (ret != SQL_SUCCESS && ret != SQL_SUCCESS_WITH_INFO) {
        std::cerr << "Error allocating statement handle" << std::endl;
        SQLDisconnect(hdbc);
        SQLFreeHandle(SQL_HANDLE_DBC, hdbc);
        SQLFreeHandle(SQL_HANDLE_ENV, henv);
        return false;
    }

    std::string sqlQuery = "INSERT INTO portfolios (user_id, company_id, shares_owned) VALUES (?, ?, ?)";
    ret = SQLPrepare(hstmt, (SQLWCHAR *)sqlQuery.c_str(), SQL_NTS);
    if (ret != SQL_SUCCESS && ret != SQL_SUCCESS_WITH_INFO) {
        std::cerr << "Error preparing SQL statement" << std::endl;
        SQLFreeHandle(SQL_HANDLE_STMT, hstmt);
        SQLDisconnect(hdbc);
        SQLFreeHandle(SQL_HANDLE_DBC, hdbc);
        SQLFreeHandle(SQL_HANDLE_ENV, henv);
        return false;
    }

    ret = SQLBindParameter(hstmt, 1, SQL_PARAM_INPUT, SQL_C_LONG, SQL_INTEGER, 0, 0, (SQLPOINTER)&portfolio.user_id, 0, NULL);
    if (ret != SQL_SUCCESS && ret != SQL_SUCCESS_WITH_INFO) {
        std::cerr << "Error binding parameter 1" << std::endl;
    }
    ret = SQLBindParameter(hstmt, 2, SQL_PARAM_INPUT, SQL_C_LONG, SQL_INTEGER, 0, 0, (SQLPOINTER)&portfolio.company_id, 0, NULL);
    if (ret != SQL_SUCCESS && ret != SQL_SUCCESS_WITH_INFO) {
        std::cerr << "Error binding parameter 2" << std::endl;
    }
    ret = SQLBindParameter(hstmt, 3, SQL_PARAM_INPUT, SQL_C_LONG, SQL_INTEGER, 0, 0, (SQLPOINTER)&portfolio.shares_owned, 0, NULL);
    if (ret != SQL_SUCCESS && ret != SQL_SUCCESS_WITH_INFO) {
        std::cerr << "Error binding parameter 3" << std::endl;
    }

    ret = SQLExecute(hstmt);
    if (ret != SQL_SUCCESS && ret != SQL_SUCCESS_WITH_INFO) {
        std::cerr << "Error executing SQL statement" << std::endl;
        SQLFreeHandle(SQL_HANDLE_STMT, hstmt);
        SQLDisconnect(hdbc);
        SQLFreeHandle(SQL_HANDLE_DBC, hdbc);
        SQLFreeHandle(SQL_HANDLE_ENV, henv);
        return false;
    }

    SQLFreeHandle(SQL_HANDLE_STMT, hstmt);
    SQLDisconnect(hdbc);
    SQLFreeHandle(SQL_HANDLE_DBC, hdbc);
    SQLFreeHandle(SQL_HANDLE_ENV, henv);
    return true;
}

bool updatePortfolioEntry(const Portfolio &portfolio) {
    SQLHANDLE henv, hdbc, hstmt;
    SQLRETURN ret;

    if (!connectToDatabase(henv, hdbc)) {
        return false;
    }

    ret = SQLAllocHandle(SQL_HANDLE_STMT, hdbc, &hstmt);
    if (ret != SQL_SUCCESS && ret != SQL_SUCCESS_WITH_INFO) {
        std::cerr << "Error allocating statement handle" << std::endl;
        SQLFreeHandle(SQL_HANDLE_STMT, hstmt);
        SQLDisconnect(hdbc);
        SQLFreeHandle(SQL_HANDLE_DBC, hdbc);
        SQLFreeHandle(SQL_HANDLE_ENV, henv);
        return false;
    }

    std::string sqlQuery = "UPDATE portfolios SET shares_owned = ? WHERE user_id = ? AND company_id = ?";
    ret = SQLPrepare(hstmt, (SQLWCHAR *)sqlQuery.c_str(), SQL_NTS);
    if (ret != SQL_SUCCESS && ret != SQL_SUCCESS_WITH_INFO) {
        std::cerr << "Error preparing SQL statement" << std::endl;
        SQLFreeHandle(SQL_HANDLE_STMT, hstmt);
        SQLDisconnect(hdbc);
        SQLFreeHandle(SQL_HANDLE_DBC, hdbc);
        SQLFreeHandle(SQL_HANDLE_ENV, henv);
        return false;
    }

    ret = SQLBindParameter(hstmt, 1, SQL_PARAM_INPUT, SQL_C_LONG, SQL_INTEGER, 0, 0, (SQLPOINTER)&portfolio.shares_owned, 0, NULL);
    if (ret != SQL_SUCCESS && ret != SQL_SUCCESS_WITH_INFO) {
        std::cerr << "Error binding parameter 1" << std::endl;
    }
    ret = SQLBindParameter(hstmt, 2, SQL_PARAM_INPUT, SQL_C_LONG, SQL_INTEGER, 0, 0, (SQLPOINTER)&portfolio.user_id, 0, NULL);
    if (ret != SQL_SUCCESS && ret != SQL_SUCCESS_WITH_INFO) {
        std::cerr << "Error binding parameter 2" << std::endl;
    }
    ret = SQLBindParameter(hstmt, 3, SQL_PARAM_INPUT, SQL_C_LONG, SQL_INTEGER, 0, 0, (SQLPOINTER)&portfolio.company_id, 0, NULL);
    if (ret != SQL_SUCCESS && ret != SQL_SUCCESS_WITH_INFO) {
        std::cerr << "Error binding parameter 3" << std::endl;
    }

    ret = SQLExecute(hstmt);
    if (ret != SQL_SUCCESS && ret != SQL_SUCCESS_WITH_INFO) {
        std::cerr << "Error executing SQL statement" << std::endl;
        SQLFreeHandle(SQL_HANDLE_STMT, hstmt);
        SQLDisconnect(hdbc);
        SQLFreeHandle(SQL_HANDLE_DBC, hdbc);
        SQLFreeHandle(SQL_HANDLE_ENV, henv);
        return false;
    }

    SQLFreeHandle(SQL_HANDLE_STMT, hstmt);
    SQLDisconnect(hdbc);
    SQLFreeHandle(SQL_HANDLE_DBC, hdbc);
    SQLFreeHandle(SQL_HANDLE_ENV, henv);
    return true;
}

bool deletePortfolioEntry(const Portfolio &portfolio) {
    SQLHANDLE henv, hdbc, hstmt;
    SQLRETURN ret;

    if (!connectToDatabase(henv, hdbc)) {
        return false;
    }

    ret = SQLAllocHandle(SQL_HANDLE_STMT, hdbc, &hstmt);
    if (ret != SQL_SUCCESS && ret != SQL_SUCCESS_WITH_INFO) {
        std::cerr << "Error allocating statement handle" << std::endl;
        SQLFreeHandle(SQL_HANDLE_STMT, hstmt);
        SQLDisconnect(hdbc);
        SQLFreeHandle(SQL_HANDLE_DBC, hdbc);
        SQLFreeHandle(SQL_HANDLE_ENV, henv);
        return false;
    }

    std::string sqlQuery = "DELETE FROM portfolios WHERE user_id = ? AND company_id = ?";
    ret = SQLPrepare(hstmt, (SQLWCHAR *)sqlQuery.c_str(), SQL_NTS);
    if (ret != SQL_SUCCESS && ret != SQL_SUCCESS_WITH_INFO) {
        std::cerr << "Error preparing SQL statement" << std::endl;
        SQLFreeHandle(SQL_HANDLE_STMT, hstmt);
        SQLDisconnect(hdbc);
        SQLFreeHandle(SQL_HANDLE_DBC, hdbc);
        SQLFreeHandle(SQL_HANDLE_ENV, henv);
        return false;
    }

    ret = SQLBindParameter(hstmt, 1, SQL_PARAM_INPUT, SQL_C_LONG, SQL_INTEGER, 0, 0, (SQLPOINTER)&portfolio.user_id, 0, NULL);
    if (ret != SQL_SUCCESS && ret != SQL_SUCCESS_WITH_INFO) {
        std::cerr << "Error binding parameter 1" << std::endl;
    }
    ret = SQLBindParameter(hstmt, 2, SQL_PARAM_INPUT, SQL_C_LONG, SQL_INTEGER, 0, 0, (SQLPOINTER)&portfolio.company_id, 0, NULL);
    if (ret != SQL_SUCCESS && ret != SQL_SUCCESS_WITH_INFO) {
        std::cerr << "Error binding parameter 2" << std::endl;
    }

    ret = SQLExecute(hstmt);
    if (ret != SQL_SUCCESS && ret != SQL_SUCCESS_WITH_INFO) {
        std::cerr << "Error executing SQL statement" << std::endl;
        SQLFreeHandle(SQL_HANDLE_STMT, hstmt);
        SQLDisconnect(hdbc);
        SQLFreeHandle(SQL_HANDLE_DBC, hdbc);
        SQLFreeHandle(SQL_HANDLE_ENV, henv);
        return false;
    }

    SQLFreeHandle(SQL_HANDLE_STMT, hstmt);
    SQLDisconnect(hdbc);
    SQLFreeHandle(SQL_HANDLE_DBC, hdbc);
    SQLFreeHandle(SQL_HANDLE_ENV, henv);
    return true;
}

std::vector<Portfolio> getPortfolioForUser(int user_id) {
    SQLHANDLE henv, hdbc, hstmt;
    SQLRETURN ret;
    std::vector<Portfolio> portfolios;

    if (!connectToDatabase(henv, hdbc)) {
        return portfolios;
    }

    ret = SQLAllocHandle(SQL_HANDLE_STMT, hdbc, &hstmt);
    if (ret != SQL_SUCCESS && ret != SQL_SUCCESS_WITH_INFO) {
        std::cerr << "Error allocating handle" << std::endl;
        SQLFreeHandle(SQL_HANDLE_STMT, hstmt);
        SQLDisconnect(hdbc);
        SQLFreeHandle(SQL_HANDLE_DBC, hdbc);
        SQLFreeHandle(SQL_HANDLE_ENV, henv);
        return portfolios;
    }

    std::string sqlQuery = "SELECT user_id, company_id, shares_owned FROM portfolios WHERE user_id = ?";
    ret = SQLPrepare(hstmt, (SQLWCHAR *)sqlQuery.c_str(), SQL_NTS);
    if (ret != SQL_SUCCESS && ret != SQL_SUCCESS_WITH_INFO) {
        std::cerr << "Error preparing SQL statement" << std::endl;
        SQLFreeHandle(SQL_HANDLE_STMT, hstmt);
        SQLDisconnect(hdbc);
        SQLFreeHandle(SQL_HANDLE_DBC, hdbc);
        SQLFreeHandle(SQL_HANDLE_ENV, henv);
        return portfolios;
    }

    ret = SQLBindParameter(hstmt, 1, SQL_PARAM_INPUT, SQL_C_LONG, SQL_INTEGER, 0, 0, (SQLPOINTER)&user_id, 0, NULL);
    if (ret != SQL_SUCCESS && ret != SQL_SUCCESS_WITH_INFO) {
        std::cerr << "Error binding parameter" << std::endl;
    }

    ret = SQLExecute(hstmt);
    if (ret != SQL_SUCCESS && ret != SQL_SUCCESS_WITH_INFO) {
        std::cerr << "Error executing SQL statement" << std::endl;
        SQLFreeHandle(SQL_HANDLE_STMT, hstmt);
        SQLDisconnect(hdbc);
        SQLFreeHandle(SQL_HANDLE_DBC, hdbc);
        SQLFreeHandle(SQL_HANDLE_ENV, henv);
        return portfolios;
    }

    Portfolio portfolio;
    while (SQLFetch(hstmt) == SQL_SUCCESS) {
        SQLGetData(hstmt, 1, SQL_C_LONG, &portfolio.user_id, 0, NULL);
        SQLGetData(hstmt, 2, SQL_C_LONG, &portfolio.company_id, 0, NULL);
        SQLGetData(hstmt, 3, SQL_C_LONG, &portfolio.shares_owned, 0, NULL);
        portfolios.push_back(portfolio);
    }

    SQLFreeHandle(SQL_HANDLE_STMT, hstmt);
    SQLDisconnect(hdbc);
    SQLFreeHandle(SQL_HANDLE_DBC, hdbc);
    SQLFreeHandle(SQL_HANDLE_ENV, henv);
    return portfolios;
}
