import time
import subprocess
import sys
from datetime import datetime
import os

def run_optimizer():
    """Run the portfolio optimizer script"""
    try:
        # Get the directory of the current script
        script_dir = os.path.dirname(os.path.abspath(__file__))
        
        # Path to the portfolio optimizer script
        optimizer_script = os.path.join(script_dir, "portfolio_optimizer.py")
        
        # Run the script and capture output
        result = subprocess.run(
            [sys.executable, optimizer_script],
            capture_output=True,
            text=True
        )
        
        # Log the output
        timestamp = datetime.now().strftime("%Y-%m-%d %H:%M:%S")
        with open("optimizer_log.txt", "a", encoding="utf-8") as log_file:
            log_file.write(f"\n[{timestamp}] Run started\n")
            log_file.write(result.stdout)
            if result.stderr:
                log_file.write(f"Errors:\n{result.stderr}")
            log_file.write(f"[{timestamp}] Run completed\n")
        
        return True
    except Exception as e:
        timestamp = datetime.now().strftime("%Y-%m-%d %H:%M:%S")
        with open("optimizer_log.txt", "a", encoding="utf-8") as log_file:
            log_file.write(f"\n[{timestamp}] Error: {str(e)}\n")
        return False

def main():
    print("Starting Portfolio Optimizer Auto-Runner")
    print("Running every 1 seconds")
    print("Press Ctrl+C to stop")
    
    try:
        while True:
            start_time = time.time()
            
            # Run the optimizer
            success = run_optimizer()
            
            if success:
                print(f"[{datetime.now().strftime('%Y-%m-%d %H:%M:%S')}] Run completed successfully")
            else:
                print(f"[{datetime.now().strftime('%Y-%m-%d %H:%M:%S')}] Error occurred, check optimizer_log.txt")
            
            # Calculate time to wait
            elapsed_time = time.time() - start_time
            wait_time = max(0, 1 - elapsed_time)
            
            if wait_time > 0:
                time.sleep(wait_time)
            
    except KeyboardInterrupt:
        print("\nStopping auto-run  ner...")
        sys.exit(0)

if __name__ == "__main__":
    main() 