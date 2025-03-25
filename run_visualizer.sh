#!/bin/bash
# Activate the virtual environment
source philo_venv/bin/activate

# Run the visualizer
python visualizer.py "$@"
