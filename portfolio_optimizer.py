import json
from typing import Dict, List, Tuple
import numpy as np
import argparse
from Prism import get_context, send_portfolio, get_my_current_information

class PortfolioOptimizer:
    def __init__(self):
        self.context = None
        self.risk_factors = {
            'age': 0.5,  # Higher age = more conservative
            'employment_status': 0.3,  # Unemployed = more conservative
            'income': 0.2  # Lower income = more conservative
        }
        self.sector_weights = {}  # Will be populated based on context
        self.budget = 0
        self.disliked_stocks = set()
        self.risk_tolerance = 0.5  # Default risk tolerance
        self.disliked_sectors = set()  # Sectors to avoid
        self.start_date = None
        self.end_date = None
        self.salary = 0

    def parse_context(self, context: str) -> None:
        """Parse the context string into structured data"""
        try:
            data = json.loads(context)
            self.context = data
            
            # Extract client preferences from message
            message = data.get('message', '')
            
            # Parse age
            age_str = message.split('is ')[1].split(' years')[0]
            self.age = int(age_str)
            
            # Parse budget
            budget_str = message.split('budget of $')[1].split('.')[0]
            self.budget = float(budget_str)
            
            # Parse dates
            start_date_str = message.split('started investing on ')[1].split(' and')[0]
            end_date_str = message.split('ended on ')[1].split('.')[0]
            self.start_date = start_date_str
            self.end_date = end_date_str
            
            # Parse salary
            salary_str = message.split('salary of $')[1].split('.')[0]
            self.salary = float(salary_str)
            
            # Parse disliked sectors
            if 'avoids' in message:
                disliked_sectors_str = message.split('avoids ')[1].split('.')[0]
                self.disliked_sectors = set(s.strip() for s in disliked_sectors_str.split(','))
            
            # Calculate risk tolerance based on client profile
            age_factor = min(1.0, self.age / 100)
            employment_factor = 0.3  # Default to employed
            income_factor = min(1.0, self.salary / 200000)  # Normalize salary to 0-1 range
            
            self.risk_tolerance = (
                age_factor * self.risk_factors['age'] +
                employment_factor * self.risk_factors['employment_status'] +
                income_factor * self.risk_factors['income']
            )
            
            print(f"Parsed context:")
            print(f"Age: {self.age}")
            print(f"Budget: ${self.budget}")
            print(f"Salary: ${self.salary}")
            print(f"Start Date: {self.start_date}")
            print(f"End Date: {self.end_date}")
            print(f"Disliked Sectors: {self.disliked_sectors}")
            print(f"Risk Tolerance: {self.risk_tolerance}")
            
        except (json.JSONDecodeError, IndexError, ValueError) as e:
            print(f"Error parsing context: {e}")
            print(f"Message content: {message}")
            raise

    def calculate_stock_weights(self, strategy: str = 'balanced') -> List[Tuple[str, float]]:
        """Calculate optimal stock weights based on constraints"""
        if not self.context:
            raise ValueError("Context not initialized")
            
        # Define sector classifications
        conservative_sectors = ['Utilities', 'Consumer Staples', 'Healthcare']
        aggressive_sectors = ['Technology', 'Consumer Discretionary', 'Communication Services']
        moderate_sectors = ['Financials', 'Industrials', 'Materials']
        
        # Sample stocks for each category
        conservative_stocks = [('PG', 1), ('JNJ', 1), ('KO', 1)]
        moderate_stocks = [('JPM', 1), ('MMM', 1), ('CAT', 1)]
        aggressive_stocks = [('AAPL', 1), ('MSFT', 1), ('GOOGL', 1)]
        
        portfolio = []
        
        if strategy == 'balanced':
            # Standard balanced approach
            if self.risk_tolerance < 0.5:
                conservative_weight = 0.6
                moderate_weight = 0.3
                aggressive_weight = 0.1
            elif self.risk_tolerance > 0.7:
                conservative_weight = 0.2
                moderate_weight = 0.3
                aggressive_weight = 0.5
            else:
                conservative_weight = 0.4
                moderate_weight = 0.4
                aggressive_weight = 0.2
                
        elif strategy == 'aggressive':
            # All-in on aggressive stocks
            conservative_weight = 0.1
            moderate_weight = 0.2
            aggressive_weight = 0.7
            
        elif strategy == 'conservative':
            # All-in on conservative stocks
            conservative_weight = 0.8
            moderate_weight = 0.15
            aggressive_weight = 0.05
            
        elif strategy == 'tech_heavy':
            # Focus on technology sector
            conservative_weight = 0.1
            moderate_weight = 0.1
            aggressive_weight = 0.8
            
        elif strategy == 'equal_weight':
            # Equal weight across all stocks
            num_stocks = len(conservative_stocks) + len(moderate_stocks) + len(aggressive_stocks)
            weight = 1.0 / num_stocks
            for stock, _ in conservative_stocks + moderate_stocks + aggressive_stocks:
                portfolio.append((stock, self.budget * weight))
            return portfolio
            
        elif strategy == 'random':
            # Random allocation
            weights = np.random.dirichlet(np.ones(9), size=1)[0]
            all_stocks = conservative_stocks + moderate_stocks + aggressive_stocks
            for (stock, _), weight in zip(all_stocks, weights):
                portfolio.append((stock, self.budget * weight))
            return portfolio
            
        # Allocate budget to each category
        conservative_budget = self.budget * conservative_weight
        moderate_budget = self.budget * moderate_weight
        aggressive_budget = self.budget * aggressive_weight
        
        # Distribute within each category
        for stock, _ in conservative_stocks:
            portfolio.append((stock, conservative_budget / len(conservative_stocks)))
            
        for stock, _ in moderate_stocks:
            portfolio.append((stock, moderate_budget / len(moderate_stocks)))
            
        for stock, _ in aggressive_stocks:
            portfolio.append((stock, aggressive_budget / len(aggressive_stocks)))
        
        return portfolio

    def optimize_portfolio(self, strategy: str) -> None:
        """Main optimization function"""
        # Get context from API
        success, context = get_context()
        if not success:
            print(f"Error getting context: {context}")
            return
            
        # Parse context
        self.parse_context(context)
        
        # Calculate portfolio using selected strategy
        print(f"\nUsing strategy: {strategy}")
        portfolio = self.calculate_stock_weights(strategy)
        
        # Send portfolio to API
        success, response = send_portfolio(portfolio)
        if success:
            try:
                score = float(response)
                print(f"Score: {score}")
            except ValueError:
                print(f"Could not parse score from response: {response}")
        else:
            print(f"Error submitting portfolio: {response}")
        
        print("\nPortfolio allocation:")
        for stock, amount in portfolio:
            print(f"{stock}: ${amount:.2f}")

def main():
    # Define available strategies
    available_strategies = [
        'balanced',    # Standard balanced approach based on risk tolerance
        'aggressive',  # Heavy focus on growth stocks
        'conservative', # Heavy focus on stable stocks
        'tech_heavy',  # Extreme focus on technology
        'equal_weight', # Equal distribution across all stocks
        'random'       # Random allocation
    ]
    
    # Set up argument parser
    parser = argparse.ArgumentParser(description='Portfolio Optimization')
    parser.add_argument('--strategy', type=str, choices=available_strategies,
                      default='balanced', help='Portfolio strategy to use')
    
    args = parser.parse_args()
    
    # Run optimizer with selected strategy
    optimizer = PortfolioOptimizer()

if __name__ == "__main__":
    main() 