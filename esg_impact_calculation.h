#pragma once 

#include <vector>

struct ESGImpact{
    int company_id;
    float esg_score;
    int shares_owned;
    float esg_impact_points;
    float weighted_esg_score;

};

std::vector<ESGImpact> calculateESGImpact(int user_id);
float calculateOverallPortfolioESGSCore(const std::vector<ESGImpact> esgImpacts);