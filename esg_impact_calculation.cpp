#include "esg_impact_calculation.h"
#include "portfolio_operations.h"
#include "portfolio_operations.cpp"
#include <sql.h>
#include <sqlext.h>
#include <iostream>
#include <vector>

float calculateTotalPortfolioValue(const std::vector<ESGImpact>& esgImpacts)
{
    float totalValue = 0.0;
    for (const ESGImpact &impact : esgImpacts)
    {
        totalValue += impact.shares_owned * impact.esg_score;
    }
    return totalValue;
}

std::vector<ESGImpact> calculateESGImpact(int user_id)
{
    SQLHANDLE henv, hdbc, hstmt;
    SQLRETURN ret;
    std::vector<ESGImpact> esgImpacts;

    if (!connectToDatabase(henv, hdbc))
    {
        return esgImpacts;
    }

    ret = SQLAllocHandle(SQL_HANDLE_STMT, hdbc, &hstmt);
    if (ret != SQL_SUCCESS && ret != SQL_SUCCESS_WITH_INFO)
    {
        std::cerr << "Error Allocating Handle";
        SQLFreeHandle(SQL_HANDLE_STMT, hstmt);
        SQLDisconnect(hdbc);
        SQLFreeHandle(SQL_HANDLE_DBC, hdbc);
        SQLFreeHandle(SQL_HANDLE_ENV, henv);
        return esgImpacts;
    }
    
    std::string sqlQuery = "SELECT p.company_id, p.shares_owned, e.esg_score "
                           "FROM portfolios p "
                           "JOIN esg_data e ON p.company_id = e.company_id "
                           "WHERE p.user_id = ?";
    
    ret = SQLPrepare(hstmt, (SQLWCHAR *)sqlQuery.c_str(), SQL_NTS);
    if (ret != SQL_SUCCESS && ret != SQL_SUCCESS_WITH_INFO)
    {
        std::cerr << "Error Preparing SQL Statements" << std::endl;
        SQLFreeHandle(SQL_HANDLE_STMT, hstmt);
        SQLDisconnect(hdbc);
        SQLFreeHandle(SQL_HANDLE_DBC, hdbc);
        SQLFreeHandle(SQL_HANDLE_ENV, henv);
        return esgImpacts;
    }

    ret = SQLBindParameter(hstmt, 1, SQL_PARAM_INPUT, SQL_C_LONG, SQL_INTEGER, 0, 0, (SQLPOINTER)&user_id, 0, NULL);
    if (ret != SQL_SUCCESS && ret != SQL_SUCCESS_WITH_INFO)
    {
        std::cerr << "Error binding parameter 1" << std::endl;
    }
    
    ret = SQLExecute(hstmt);
    if (ret != SQL_SUCCESS && ret != SQL_SUCCESS_WITH_INFO)
    {
        std::cerr << "Error Executing SQL Statements" << std::endl;
        SQLFreeHandle(SQL_HANDLE_STMT, hstmt);
        SQLDisconnect(hdbc);
        SQLFreeHandle(SQL_HANDLE_DBC, hdbc);
        SQLFreeHandle(SQL_HANDLE_ENV, henv);
        return esgImpacts;
    }

    ESGImpact esgImpact;

    while (SQLFetch(hstmt) == SQL_SUCCESS)
    {
        SQLGetData(hstmt, 1, SQL_C_LONG, &esgImpact.company_id, 0, NULL);
        SQLGetData(hstmt, 2, SQL_C_LONG, &esgImpact.shares_owned, 0, NULL);
        SQLGetData(hstmt, 3, SQL_C_FLOAT, &esgImpact.esg_score, 0, NULL);

        esgImpact.esg_impact_points = esgImpact.shares_owned * esgImpact.esg_score;
        esgImpacts.push_back(esgImpact);
    }

    float totalPortfolioValue = calculateTotalPortfolioValue(esgImpacts);
    for (ESGImpact &impact : esgImpacts)
    {
        float proportion = (impact.shares_owned * impact.esg_score) / totalPortfolioValue;
        impact.weighted_esg_score = proportion * impact.esg_score;
    }

    SQLFreeHandle(SQL_HANDLE_STMT, hstmt);
    SQLDisconnect(hdbc);
    SQLFreeHandle(SQL_HANDLE_DBC, hdbc);
    SQLFreeHandle(SQL_HANDLE_ENV, henv);

    return esgImpacts;
}

float calculateOverallPortfolioESGScore(const std::vector<ESGImpact>& esgImpacts)
{
    float overallESGScore = 0.0;
    for (const ESGImpact &impact : esgImpacts)
    {
        overallESGScore += impact.weighted_esg_score;
    }
    return overallESGScore;
}
