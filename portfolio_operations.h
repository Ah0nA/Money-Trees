#pragma once

#include <string>
#include <vector>

struct Portfolio {
    int user_id;
    int company_id;
    double shares_owned;
    
};

bool insertPortfolioEntry(const Portfolio& portfolio);
bool updatePortfolioEntry(const Portfolio& portfolio);
bool deletePortfolioEntry(int user_id, int company_id);
std::vector<Portfolio> getPortfoilioForUser(int user_id); 
