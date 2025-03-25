#!/bin/bash

# Colors for output
GREEN='\033[0;32m'
BLUE='\033[0;34m'
YELLOW='\033[1;33m'
RED='\033[0;31m'
NC='\033[0m' # No Color

echo -e "${BLUE}Setting up Philosophers Visualizer environment...${NC}"

# Check if Python is installed
if ! command -v python3 &>/dev/null; then
	echo -e "${RED}Python 3 is not installed. Please install Python 3.${NC}"
	exit 1
fi

# Check Python version
PYTHON_VERSION=$(python3 -c 'import sys; print(f"{sys.version_info.major}.{sys.version_info.minor}")')
echo -e "${GREEN}Found Python version: ${PYTHON_VERSION}${NC}"

# Create a virtual environment
echo -e "${YELLOW}Creating virtual environment...${NC}"
python3 -m venv philo_venv

# Activate the virtual environment
echo -e "${YELLOW}Activating virtual environment...${NC}"
source philo_venv/bin/activate

# Upgrade pip
echo -e "${YELLOW}Upgrading pip...${NC}"
pip install --upgrade pip

# Install required packages
echo -e "${YELLOW}Installing required packages...${NC}"
pip install -r requirements.txt

# Create a run script
echo -e "${YELLOW}Creating run script...${NC}"
cat >run_visualizer.sh <<'EOF'
#!/bin/bash
# Activate the virtual environment
source philo_venv/bin/activate

# Run the visualizer
python visualizer.py "$@"
EOF

chmod +x run_visualizer.sh

echo -e "${GREEN}Setup complete!${NC}"
echo -e "${BLUE}To run the visualizer, use: ${YELLOW}./run_visualizer.sh${NC}"
echo -e "${BLUE}If you want to provide a specific binary: ${YELLOW}./run_visualizer.sh /path/to/philo${NC}"
