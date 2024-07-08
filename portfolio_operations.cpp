#include <portfolio_operations.h>
#include <iostream>
#include <sql.h>
#include <sqlext.h>
#include <vector>

bool connectToDatabase(SQLHANDLE &henv, SQLHANDLE &hdbc)
{
    SQLRETURN ret;

    // Allocate environment handle
    ret = SQLAllocHandle(SQL_HANDLE_ENV, SQL_NULL_HANDLE, &henv);
    if (ret != SQL_SUCCESS && ret != SQL_SUCCESS_WITH_INFO)
    {
        std::cerr << "Error allocating environment handle" << std::endl;
        return false;
    }

    // Set the ODBC version environment attribute
    ret = SQLSetEnvAttr(henv, SQL_ATTR_ODBC_VERSION, (SQLPOINTER)SQL_OV_ODBC3, 0);
    if (ret != SQL_SUCCESS && ret != SQL_SUCCESS_WITH_INFO)
    {
        std::cerr << "Error setting ODBC version" << std::endl;
        SQLFreeHandle(SQL_HANDLE_ENV, henv);
        return false;
    }

     // Allocate connection handle
     ret = SQLAllocHandle(SQL_HANDLE_DBC,henv, &hdbc);
    if (ret != SQL_SUCCESS && ret != SQL_SUCCESS_WITH_INFO){
        std::cerr <<"Error allocating connecting handle" <<std::endl;
        SQLFreeHandle(SQL_HANDLE_DBC, hdbc);
        SQLFreeHandle(SQL_HANDLE_ENV, henv);
        return false;
    }

    SQLWCHAR* connStr = (SQLWCHAR*)"DRIVER={ODBC Driver 18 for SQL Server};SERVER=moneytrees-sqlserver.database.windows.net;DATABASE=MoneyTreesDatabase;UID=Flavia;PWD=Darken&5h33p !;";

    //Connect to the database
    ret = SQLConnect(hdbc, connStr, SQL_NTS, NULL, 0, NULL,0 );
    if(ret != SQL_SUCCESS && ret != SQL_SUCCESS_WITH_INFO){
        std::cerr <<"Error connecting to the database" <<std::endl;
        SQLFreeHandle(SQL_HANDLE_ENV, henv);
        return false;
    }
    return true;
}

//Function to insert new portfolio
bool insertPortfoilioEntry(const Portfolio& portfolio){
    SQLHANDLE henv, hdbc, hstmt;
    SQLRETURN ret;

    if (!connectToDatabase(henv, hdbc)){
        return false;
    }
    ret = SQLAllocHandle(SQL_HANDLE_STMT, hdbc, &hstmt);
    if (ret != SQL_SUCCESS && ret != SQL_SUCCESS_WITH_INFO){
        std::cerr << "Error alloccating Statement Handle" <<std::endl;
        SQLDisconnect(hdbc);
        SQLFreeHandle(SQL_HANDLE_DBC, hdbc);
         SQLFreeHandle(SQL_HANDLE_ENV, henv);
         return false;
    } 

    std::string sqlQuery = "INSERT INTO portfoilios (user_id, company_id, shares_owned) VALUES(?, ?, ?)";
    ret = SQLPrepare(hstmt, (SQLWCHAR*) sqlQuery.c_str(), SQL_NTS);
    if(ret != SQL_SUCCESS && ret != SQL_SUCCESS_WITH_INFO){
        std::cerr << "Error preparing SQL statement" <<std::endl;
        SQLFreeHandle(SQL_HANDLE_STMT, hstmt);
        SQLDisconnect(hdbc);
        SQLFreeHandle(SQL_HANDLE_DBC, hdbc);
        SQLFreeHandle(SQL_HANDLE_ENV, henv);
        return false;

    }

    ret = SQLBindParameter(hstmt, 1, SQL_PARAM_INPUT, SQL_C_LONG, SQL_INTEGER, 0, 0, (SQLPOINTER)& portfolio.user_id,0 ,NULL);
    ret = SQLBindParameter(hstmt, 2, SQL_PARAM_INPUT, SQL_C_LONG, SQL_INTEGER,0,0 ,(SQLPOINTER) &portfolio.company_id, 0, NULL);
    ret = SQLBindParameter(hstmt, 3, SQL_PARAM_INPUT, SQL_C_LONG, SQL_INTEGER, 0, 0 ,(SQLPOINTER)& portfolio.shares_owned, 0 ,NULL);


    ret = SQLExecute(hstmt);

    if(ret != SQL_SUCCESS && ret != SQL_SUCCESS_WITH_INFO){
        std::cerr <<"Error executing SQL statement" <<std::endl;
        SQLFreeHandle(SQL_HANDLE_STMT, hstmt);
        SQLDisconnect(hdbc);
        SQLFreeHandle(SQL_HANDLE_DBC, hdbc);
        SQLFreeHandle(SQL_HANDLE_ENV, henv);
    }


}