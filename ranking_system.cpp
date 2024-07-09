#include "ranking_system.h"
#include "esg_impact_calculation.h"
#include "esg_impact_calculation.cpp"
#include "portfolio_operations.h"
#include "portfolio_operations.cpp"
#include <sql.h>
#include <sqlext.h>
#include <algorithm>
#include <iostream>

std::vector<CompanyRanking> rankCompaniesByESGScore()
{
    SQLHANDLE henv, hdbc, hstmt;
    SQLRETURN ret;
    std::vector<CompanyRanking> companyRankings;

    if (!connectToDatabase(henv, hdbc))
    {
        return companyRankings;
    }

    ret = SQLAllocHandle(SQL_HANDLE_STMT, hdbc, &hstmt);

    if (ret != SQL_SUCCESS && ret != SQL_SUCCESS_WITH_INFO)
    {
        std::cerr << "Error Allocating Statement Handle" << std::endl;
        SQLFreeHandle(SQL_HANDLE_STMT, hstmt);
        SQLDisconnect(hdbc);
        SQLFreeHandle(SQL_HANDLE_DBC, hdbc);
        SQLFreeHandle(SQL_HANDLE_ENV, henv);
        return companyRankings;
    }

    std::string sqlQuery = "SELECT company_id, esg_score FROM esg_data ORDER BY esg_score DESC";

    ret = SQLExecDirect(hstmt, (SQLWCHAR *)sqlQuery.c_str(), SQL_NTS);

    if (ret != SQL_SUCCESS && ret != SQL_SUCCESS_WITH_INFO)
    {
        std::cerr << "Error Executing SQL Statements" << std::endl;
        SQLFreeHandle(SQL_HANDLE_STMT, hstmt);
        SQLDisconnect(hdbc);
        SQLFreeHandle(SQL_HANDLE_DBC, hdbc);
        SQLFreeHandle(SQL_HANDLE_ENV, henv);
        return companyRankings;
    }

    CompanyRanking ranking;
    int rank = 1;

    while (SQLFetch(hstmt) == SQL_SUCCESS)
    {
        SQLGetData(hstmt, 1, SQL_C_LONG, &ranking.company_id, 0, NULL);
        SQLGetData(hstmt, 2, SQL_C_LONG, &ranking.esg_score, 0, NULL);

        ranking.rank = rank++;
        companyRankings.push_back(ranking);
    }

    SQLFreeHandle(SQL_HANDLE_STMT, hstmt);
    SQLDisconnect(hdbc);
    SQLFreeHandle(SQL_HANDLE_DBC, hdbc);
    SQLFreeHandle(SQL_HANDLE_ENV, henv);

    return companyRankings;
}

std::vector<UserPortfolioRanking> rankUserPortfolios()
{
    SQLHANDLE henv, hdbc, hstmt;
    SQLRETURN ret;
    std::vector<UserPortfolioRanking> userPortfolioRankings;

    if (!connectToDatabase(henv, hdbc))
    {
        return userPortfolioRankings;
    }

    ret = SQLAllocHandle(SQL_HANDLE_STMT, hdbc, &hstmt);

    if (ret != SQL_SUCCESS && ret != SQL_SUCCESS_WITH_INFO)
    {
        std::cerr << "Error Allocating Handle" << std::endl;
        SQLFreeHandle(SQL_HANDLE_STMT, hstmt);
        SQLDisconnect(hdbc);
        SQLFreeHandle(SQL_HANDLE_DBC, hdbc);
        SQLFreeHandle(SQL_HANDLE_ENV, henv);
        return userPortfolioRankings;
    }

    std::string sqlQuery = "SELECT DISTINCT user_id FROM portfolios";

    ret = SQLExecDirect(hstmt, (SQLWCHAR *)sqlQuery.c_str(), SQL_NTS);

    if (ret != SQL_SUCCESS && ret != SQL_SUCCESS_WITH_INFO)
    {
        std::cerr << "Error Executing SQL Statements" << std::endl;
        SQLFreeHandle(SQL_HANDLE_STMT, hstmt);
        SQLDisconnect(hdbc);
        SQLFreeHandle(SQL_HANDLE_DBC, hdbc);
        SQLFreeHandle(SQL_HANDLE_ENV, henv);
        return userPortfolioRankings;
    }

    std::vector<int> userIds;
    int user_id;

    while (SQLFetch(hstmt) == SQL_SUCCESS)
    {
        SQLGetData(hstmt, 1, SQL_C_LONG, &user_id, 0, NULL);
        userIds.push_back(user_id);
    }

    SQLFreeHandle(SQL_HANDLE_STMT, hstmt);

    for (int user_id : userIds)
    {
        std::vector<ESGImpact> esgImpacts = calculateESGImpact(user_id);
        float overallESGScore = calculateOverallPortfolioESGScore(esgImpacts);

        UserPortfolioRanking ranking;
        ranking.user_id = user_id;
        ranking.overall_esg_score = overallESGScore;
        userPortfolioRankings.push_back(ranking);
    }

    std::sort(userPortfolioRankings.begin(), userPortfolioRankings.end(), [](const UserPortfolioRanking &a, const UserPortfolioRanking &b)
              { return a.overall_esg_score > b.overall_esg_score; });

    int rank = 1;

    for (auto &ranking : userPortfolioRankings)
    {
        ranking.rank = rank++;
    }

    SQLDisconnect(hdbc);
    SQLFreeHandle(SQL_HANDLE_DBC, hdbc);
    SQLFreeHandle(SQL_HANDLE_ENV, henv);

    return userPortfolioRankings;
}

int getUserPortfolioRank(int user_id){
    std::vector<UserPortfolioRanking> userPortfolioRankings = rankUserPortfolios();
    for(const auto &ranking : userPortfolioRankings){
        if (ranking.user_id == user_id) {
            return ranking.rank;
        }
    }
    return -1;
}