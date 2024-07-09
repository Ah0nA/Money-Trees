#pragma once

#include "esg_impact_calculation.h"
#include "portfolio_operations.h"
#include <vector>
#include <algorithm>
#include <string>

struct CompanyRanking
{
    int company_id;
    float esg_score;
    int rank;
};

struct UserPortfolioRanking{
    int user_id;
    float overall_esg_score;
    int rank;
};

std::vector<CompanyRanking> rankCompaniesByESGScore();
std::vector<UserPortfolioRanking> rankUserPortfolios();
int getUserPortfolioRank(int user_id);