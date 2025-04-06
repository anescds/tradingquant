#include <iostream>
#include <string>
#include <vector>
#include <map>
#include <set>
#include <random>
#include <algorithm>
#include <cmath>
#include <sstream>
#include <iomanip>
#include "prism_client.h"

// Simple JSON-like structure using standard C++ types
struct PortfolioContext {
    std::string message;
    double budget;
    int age;
    double salary;
    std::string start_date;
    std::string end_date;
    std::set<std::string> disliked_sectors;
};

class PortfolioOptimizer {
private:
    PortfolioContext context;
    std::map<std::string, double> risk_factors = {
        {"age", 0.5},        // Higher age = more conservative
        {"employment_status", 0.3},  // Unemployed = more conservative
        {"income", 0.2}      // Lower income = more conservative
    };
    std::map<std::string, double> sector_weights;
    double budget = 0;
    std::set<std::string> disliked_stocks;
    double risk_tolerance = 0.5;  // Default risk tolerance
    std::set<std::string> disliked_sectors;
    std::string start_date;
    std::string end_date;
    double salary = 0;
    int age = 0;

public:
    PortfolioOptimizer() = default;

    void parseContext(const std::string& contextStr) {
        context.message = contextStr;
        
        // Extract client preferences from message
        std::string message = contextStr;
        
        // Parse age
        size_t agePos = message.find("is ") + 3;
        size_t yearsPos = message.find(" years", agePos);
        if (agePos != std::string::npos && yearsPos != std::string::npos) {
            try {
                std::string ageStr = message.substr(agePos, yearsPos - agePos);
                std::cout << "Parsing age from: '" << ageStr << "'" << std::endl;
                age = std::stoi(ageStr);
            } catch (const std::exception& e) {
                std::cerr << "Error parsing age: " << e.what() << std::endl;
            }
        }
        
        // Parse budget
        size_t budgetPos = message.find("budget of $") + 10;
        size_t dotPos = message.find(".", budgetPos);
        if (budgetPos != std::string::npos && dotPos != std::string::npos) {
            try {
                std::string budgetStr = message.substr(budgetPos, dotPos - budgetPos);
                // Remove any non-numeric characters except decimal point
                budgetStr.erase(std::remove_if(budgetStr.begin(), budgetStr.end(), 
                    [](char c) { return !std::isdigit(c) && c != '.'; }), budgetStr.end());
                std::cout << "Parsing budget from: '" << budgetStr << "'" << std::endl;
                budget = std::stod(budgetStr);
            } catch (const std::exception& e) {
                std::cerr << "Error parsing budget: " << e.what() << std::endl;
            }
        }
        
        // Parse dates
        size_t startDatePos = message.find("Started investing on ") + 20;
        size_t andPos = message.find(" and", startDatePos);
        if (startDatePos != std::string::npos && andPos != std::string::npos) {
            start_date = message.substr(startDatePos, andPos - startDatePos);
            // Clean up any extra spaces
            start_date.erase(0, start_date.find_first_not_of(" \t"));
            start_date.erase(start_date.find_last_not_of(" \t") + 1);
        }
        
        size_t endDatePos = message.find("ended on ") + 9;
        size_t endDotPos = message.find(".", endDatePos);
        if (endDatePos != std::string::npos && endDotPos != std::string::npos) {
            end_date = message.substr(endDatePos, endDotPos - endDatePos);
            // Clean up any extra spaces
            end_date.erase(0, end_date.find_first_not_of(" \t"));
            end_date.erase(end_date.find_last_not_of(" \t") + 1);
        }
        
        // Parse salary
        size_t salaryPos = message.find("salary of $") + 10;
        size_t salaryDotPos = message.find(".", salaryPos);
        if (salaryPos != std::string::npos && salaryDotPos != std::string::npos) {
            try {
                std::string salaryStr = message.substr(salaryPos, salaryDotPos - salaryPos);
                // Remove any non-numeric characters except decimal point
                salaryStr.erase(std::remove_if(salaryStr.begin(), salaryStr.end(), 
                    [](char c) { return !std::isdigit(c) && c != '.'; }), salaryStr.end());
                std::cout << "Parsing salary from: '" << salaryStr << "'" << std::endl;
                salary = std::stod(salaryStr);
            } catch (const std::exception& e) {
                std::cerr << "Error parsing salary: " << e.what() << std::endl;
            }
        }
        
        // Parse disliked sectors
        if (message.find("avoids") != std::string::npos) {
            size_t avoidsPos = message.find("avoids ") + 7;
            size_t avoidsDotPos = message.find(".", avoidsPos);
            if (avoidsPos != std::string::npos && avoidsDotPos != std::string::npos) {
                std::string sectorsStr = message.substr(avoidsPos, avoidsDotPos - avoidsPos);
                // Split sectors by comma and add to set
                size_t pos = 0;
                while ((pos = sectorsStr.find(',')) != std::string::npos) {
                    std::string sector = sectorsStr.substr(0, pos);
                    sector.erase(0, sector.find_first_not_of(" \t"));
                    sector.erase(sector.find_last_not_of(" \t") + 1);
                    disliked_sectors.insert(sector);
                    sectorsStr.erase(0, pos + 1);
                }
                if (!sectorsStr.empty()) {
                    sectorsStr.erase(0, sectorsStr.find_first_not_of(" \t"));
                    sectorsStr.erase(sectorsStr.find_last_not_of(" \t") + 1);
                    disliked_sectors.insert(sectorsStr);
                }
            }
        }
        
        // Calculate risk tolerance based on client profile
        double age_factor = 1.0 - (std::min(1.0, static_cast<double>(age) / 100.0));  // Invert age factor
        double employment_factor = 0.7;  // Higher for employed
        double income_factor = std::min(1.0, salary / 100000.0);  // Normalize salary to 0-1 range
        
        risk_tolerance = (
            age_factor * risk_factors["age"] +
            employment_factor * risk_factors["employment_status"] +
            income_factor * risk_factors["income"]
        );
        
        // Ensure risk tolerance is between 0.3 and 0.8
        risk_tolerance = std::max(0.3, std::min(0.8, risk_tolerance));
        
        std::cout << "\nParsed context:" << std::endl;
        std::cout << "Age: " << age << std::endl;
        std::cout << "Budget: $" << budget << std::endl;
        std::cout << "Salary: $" << salary << std::endl;
        std::cout << "Start Date: " << start_date << std::endl;
        std::cout << "End Date: " << end_date << std::endl;
        std::cout << "Disliked Sectors: ";
        for (const auto& sector : disliked_sectors) {
            std::cout << sector << " ";
        }
        std::cout << std::endl;
        std::cout << "Risk Tolerance: " << risk_tolerance << std::endl;
    }

    std::vector<std::pair<std::string, double>> calculateStockWeights(const std::string& strategy = "balanced") {
        if (context.message.empty()) {
            throw std::runtime_error("Context not initialized");
        }
        
        // Define sector classifications
        std::vector<std::string> conservative_sectors = {"Utilities", "Consumer Staples", "Healthcare"};
        std::vector<std::string> aggressive_sectors = {"Technology", "Consumer Discretionary", "Communication Services"};
        std::vector<std::string> moderate_sectors = {"Financials", "Industrials", "Materials"};
        
        // Sample stocks for each category
        std::vector<std::pair<std::string, double>> conservative_stocks = {
            {"PG", 1}, {"JNJ", 1}, {"KO", 1}
        };
        std::vector<std::pair<std::string, double>> moderate_stocks = {
            {"JPM", 1}, {"MMM", 1}, {"CAT", 1}
        };
        std::vector<std::pair<std::string, double>> aggressive_stocks = {
            {"AAPL", 1}, {"MSFT", 1}, {"GOOGL", 1}
        };
        
        std::vector<std::pair<std::string, double>> portfolio;
        double conservative_weight = 0.0;
        double moderate_weight = 0.0;
        double aggressive_weight = 0.0;
        
        if (strategy == "balanced") {
            if (risk_tolerance < 0.5) {
                conservative_weight = 0.6;
                moderate_weight = 0.3;
                aggressive_weight = 0.1;
            } else if (risk_tolerance > 0.7) {
                conservative_weight = 0.2;
                moderate_weight = 0.3;
                aggressive_weight = 0.5;
            } else {
                conservative_weight = 0.4;
                moderate_weight = 0.4;
                aggressive_weight = 0.2;
            }
        } else if (strategy == "aggressive") {
            conservative_weight = 0.1;
            moderate_weight = 0.2;
            aggressive_weight = 0.7;
        } else if (strategy == "conservative") {
            conservative_weight = 0.8;
            moderate_weight = 0.15;
            aggressive_weight = 0.05;
        } else if (strategy == "tech_heavy") {
            conservative_weight = 0.1;
            moderate_weight = 0.1;
            aggressive_weight = 0.8;
        } else if (strategy == "equal_weight") {
            size_t num_stocks = conservative_stocks.size() + moderate_stocks.size() + aggressive_stocks.size();
            double weight = 1.0 / num_stocks;
            for (const auto& stock : conservative_stocks) {
                portfolio.emplace_back(stock.first, budget * weight);
            }
            for (const auto& stock : moderate_stocks) {
                portfolio.emplace_back(stock.first, budget * weight);
            }
            for (const auto& stock : aggressive_stocks) {
                portfolio.emplace_back(stock.first, budget * weight);
            }
            return portfolio;
        } else if (strategy == "random") {
            std::random_device rd;
            std::mt19937 gen(rd());
            std::gamma_distribution<> dist(1.0, 1.0);
            
            std::vector<double> weights;
            double sum = 0.0;
            for (size_t i = 0; i < 9; ++i) {
                double w = dist(gen);
                weights.push_back(w);
                sum += w;
            }
            
            for (double& w : weights) {
                w /= sum;
            }
            
            size_t idx = 0;
            for (const auto& stock : conservative_stocks) {
                portfolio.emplace_back(stock.first, budget * weights[idx++]);
            }
            for (const auto& stock : moderate_stocks) {
                portfolio.emplace_back(stock.first, budget * weights[idx++]);
            }
            for (const auto& stock : aggressive_stocks) {
                portfolio.emplace_back(stock.first, budget * weights[idx++]);
            }
            return portfolio;
        }
        
        // Allocate budget to each category
        double conservative_budget = budget * conservative_weight;
        double moderate_budget = budget * moderate_weight;
        double aggressive_budget = budget * aggressive_weight;
        
        // Distribute within each category
        for (const auto& stock : conservative_stocks) {
            portfolio.emplace_back(stock.first, conservative_budget / conservative_stocks.size());
        }
        
        for (const auto& stock : moderate_stocks) {
            portfolio.emplace_back(stock.first, moderate_budget / moderate_stocks.size());
        }
        
        for (const auto& stock : aggressive_stocks) {
            portfolio.emplace_back(stock.first, aggressive_budget / aggressive_stocks.size());
        }
        
        return portfolio;
    }

    void optimizePortfolio(const std::string& strategy) {
        // Initialize Prism client
        PrismClient client;
        
        // Get context from API
        auto [success, contextStr] = client.getContext();
        if (!success) {
            std::cerr << "Error getting context: " << contextStr << std::endl;
            return;
        }
        
        // Parse context
        parseContext(contextStr);
        
        // Calculate portfolio using selected strategy
        std::cout << "\nUsing strategy: " << strategy << std::endl;
        auto portfolio = calculateStockWeights(strategy);
        
        // Send portfolio to API
        auto [sendSuccess, response] = client.sendPortfolio(portfolio);
        if (!sendSuccess) {
            std::cerr << "Error submitting portfolio: " << response << std::endl;
            return;
        }
        
        try {
            double score = std::stod(response);
            std::cout << "Score: " << score << std::endl;
        } catch (const std::exception& e) {
            std::cerr << "Could not parse score from response: " << response << std::endl;
        }
        
        std::cout << "\nPortfolio allocation:" << std::endl;
        double total = 0.0;
        for (const auto& [stock, amount] : portfolio) {
            std::cout << stock << ": $" << std::fixed << std::setprecision(2) << amount << std::endl;
            total += amount;
        }
        std::cout << "Total: $" << std::fixed << std::setprecision(2) << total << std::endl;
    }
};

int main(int argc, char* argv[]) {
    // Define available strategies
    std::vector<std::string> available_strategies = {
        "balanced",     // Standard balanced approach based on risk tolerance
        "aggressive",   // Heavy focus on growth stocks
        "conservative", // Heavy focus on stable stocks
        "tech_heavy",   // Extreme focus on technology
        "equal_weight", // Equal distribution across all stocks
        "random"        // Random allocation
    };
    
    // Default strategy
    std::string strategy = "balanced";
    
    // Parse command line arguments
    if (argc > 1) {
        strategy = argv[1];
        if (std::find(available_strategies.begin(), available_strategies.end(), strategy) == available_strategies.end()) {
            std::cerr << "Invalid strategy. Available strategies are:" << std::endl;
            for (const auto& s : available_strategies) {
                std::cerr << "  " << s << std::endl;
            }
            return 1;
        }
    }
    
    // Run optimizer with selected strategy
    PortfolioOptimizer optimizer;
    optimizer.optimizePortfolio(strategy);
    
    return 0;
} 