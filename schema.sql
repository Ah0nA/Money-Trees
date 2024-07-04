-- Users table
CREATE TABLE users (
    user_id INT PRIMARY KEY,
    email VARCHAR(255) UNIQUE,
    username VARCHAR(100)
);

-- ESG data table
CREATE TABLE esg_data (
    company_id INT PRIMARY KEY,
    company_name VARCHAR(100),
    esg_score FLOAT,
    updated_at DATETIME
);

-- Portfolios table
CREATE TABLE portfolios (
    user_id INT,
    company_id INT,
    shares_owned INT,
    PRIMARY KEY(user_id, company_id),
    FOREIGN KEY(user_id) REFERENCES users(user_id),
    FOREIGN KEY(company_id) REFERENCES esg_data(company_id)
);

-- Historical data table
CREATE TABLE historical_data (
    company_id INT,
    date DATE,
    esg_score FLOAT,
    stock_price FLOAT,
    PRIMARY KEY(company_id, date),  -- Assuming a company's ESG score and stock price change can happen multiple times on different dates
    FOREIGN KEY(company_id) REFERENCES esg_data(company_id)
);
