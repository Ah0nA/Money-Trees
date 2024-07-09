#pragma once

#include <vector>
#include <string>

struct CompanyRanking
{
    int company_id;
    float esg_score;
    std::string comapany_name;
};

struct UserPortfolioRanking{
    int user_id;
    std::string user_name;
    float portfolio_esg_score;
};

std::vector<CompanyRanking> getCompanyRankings();
std::vector<UserPortfolioRanking> getUserPortfolioRankings();