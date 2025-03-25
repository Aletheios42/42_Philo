#!/usr/bin/env python3

import os
import sys
import time
import signal
import re
import subprocess
import threading
import tkinter as tk
from tkinter import ttk, messagebox, filedialog
from datetime import datetime
from typing import List, Dict, Tuple, Optional, Any
import random
import matplotlib

matplotlib.use("TkAgg")  # Set backend explicitly
import matplotlib.pyplot as plt
from matplotlib.backends.backend_tkagg import FigureCanvasTkAgg
from matplotlib.figure import Figure
import matplotlib.animation as animation


class Philosopher:
    """Represents the state of a philosopher in the simulation."""

    def __init__(self, id_num: int):
        self.id = id_num
        self.state = "waiting"  # waiting, eating, thinking, sleeping, dead
        self.left_fork = False
        self.right_fork = False
        self.meals_eaten = 0
        self.last_meal_time = 0
        self.events = []  # List of (timestamp, event) tuples
        self.alive = True

    def update_state(self, new_state: str, timestamp: int):
        """Update philosopher state and record the event."""
        old_state = self.state
        self.state = new_state

        if new_state == "has taken a fork":
            if not self.left_fork:
                self.left_fork = True
            else:
                self.right_fork = True
        elif new_state == "is eating":
            self.last_meal_time = timestamp
            self.meals_eaten += 1
        elif new_state == "died":
            self.alive = False

        # Record event for timeline
        self.events.append((timestamp, new_state))

        return old_state != new_state


class PhiloVisualizer:
    """Main visualizer application for the Dining Philosophers problem."""

    def __init__(self, root: tk.Tk):
        self.root = root
        self.root.title("Philosophers Visualizer")
        self.root.geometry("1200x800")
        self.root.protocol("WM_DELETE_WINDOW", self.on_close)

        self.binary_path = ""
        self.process = None
        self.running = False
        self.philosophers = []
        self.total_philosophers = 0
        self.time_to_die = 0
        self.time_to_eat = 0
        self.time_to_sleep = 0
        self.meals_cap = 0
        self.start_time = 0
        self.last_update_time = 0
        self.simulation_status = "Not started"

        self.output_text = ""
        self.output_lock = threading.Lock()

        self.create_ui()

    def create_ui(self):
        """Create the user interface elements."""
        # Main frame
        main_frame = ttk.Frame(self.root, padding="10")
        main_frame.pack(fill=tk.BOTH, expand=True)

        # Control panel
        control_frame = ttk.LabelFrame(main_frame, text="Controls", padding="10")
        control_frame.pack(fill=tk.X, padx=5, pady=5)

        # File selection
        file_frame = ttk.Frame(control_frame)
        file_frame.pack(fill=tk.X, padx=5, pady=5)

        ttk.Label(file_frame, text="Philo Binary:").pack(side=tk.LEFT, padx=5)

        self.binary_entry = ttk.Entry(file_frame, width=40)
        self.binary_entry.pack(side=tk.LEFT, padx=5, fill=tk.X, expand=True)

        ttk.Button(file_frame, text="Browse", command=self.browse_binary).pack(
            side=tk.LEFT, padx=5
        )

        # Parameters frame
        params_frame = ttk.Frame(control_frame)
        params_frame.pack(fill=tk.X, padx=5, pady=5)

        # Parameters
        ttk.Label(params_frame, text="Philosophers:").grid(
            row=0, column=0, padx=5, pady=5, sticky=tk.W
        )
        self.philosophers_entry = ttk.Entry(params_frame, width=8)
        self.philosophers_entry.grid(row=0, column=1, padx=5, pady=5, sticky=tk.W)
        self.philosophers_entry.insert(0, "4")

        ttk.Label(params_frame, text="Time to die (ms):").grid(
            row=0, column=2, padx=5, pady=5, sticky=tk.W
        )
        self.time_to_die_entry = ttk.Entry(params_frame, width=8)
        self.time_to_die_entry.grid(row=0, column=3, padx=5, pady=5, sticky=tk.W)
        self.time_to_die_entry.insert(0, "800")

        ttk.Label(params_frame, text="Time to eat (ms):").grid(
            row=0, column=4, padx=5, pady=5, sticky=tk.W
        )
        self.time_to_eat_entry = ttk.Entry(params_frame, width=8)
        self.time_to_eat_entry.grid(row=0, column=5, padx=5, pady=5, sticky=tk.W)
        self.time_to_eat_entry.insert(0, "200")

        ttk.Label(params_frame, text="Time to sleep (ms):").grid(
            row=0, column=6, padx=5, pady=5, sticky=tk.W
        )
        self.time_to_sleep_entry = ttk.Entry(params_frame, width=8)
        self.time_to_sleep_entry.grid(row=0, column=7, padx=5, pady=5, sticky=tk.W)
        self.time_to_sleep_entry.insert(0, "200")

        ttk.Label(params_frame, text="Meals cap:").grid(
            row=0, column=8, padx=5, pady=5, sticky=tk.W
        )
        self.meals_cap_entry = ttk.Entry(params_frame, width=8)
        self.meals_cap_entry.grid(row=0, column=9, padx=5, pady=5, sticky=tk.W)
        self.meals_cap_entry.insert(0, "0")

        # Buttons
        buttons_frame = ttk.Frame(control_frame)
        buttons_frame.pack(fill=tk.X, padx=5, pady=5)

        self.start_button = ttk.Button(
            buttons_frame, text="Start Simulation", command=self.start_simulation
        )
        self.start_button.pack(side=tk.LEFT, padx=5)

        self.stop_button = ttk.Button(
            buttons_frame,
            text="Stop Simulation",
            command=self.stop_simulation,
            state=tk.DISABLED,
        )
        self.stop_button.pack(side=tk.LEFT, padx=5)

        ttk.Button(buttons_frame, text="Run Tests", command=self.run_tests).pack(
            side=tk.LEFT, padx=5
        )

        # Status
        status_frame = ttk.Frame(control_frame)
        status_frame.pack(fill=tk.X, padx=5, pady=5)

        ttk.Label(status_frame, text="Status:").pack(side=tk.LEFT, padx=5)
        self.status_label = ttk.Label(status_frame, text=self.simulation_status)
        self.status_label.pack(side=tk.LEFT, padx=5)

        # Notebook for different views
        self.notebook = ttk.Notebook(main_frame)
        self.notebook.pack(fill=tk.BOTH, expand=True, padx=5, pady=5)

        # Visualization tab
        viz_frame = ttk.Frame(self.notebook)
        self.notebook.add(viz_frame, text="Visualization")

        # Create Figure for philosopher visualization
        self.fig = Figure(figsize=(10, 6), dpi=100)
        self.ax = self.fig.add_subplot(111)
        self.canvas = FigureCanvasTkAgg(self.fig, master=viz_frame)
        self.canvas.get_tk_widget().pack(fill=tk.BOTH, expand=True)

        # Timeline tab
        timeline_frame = ttk.Frame(self.notebook)
        self.notebook.add(timeline_frame, text="Timeline")

        self.timeline_fig = Figure(figsize=(10, 6), dpi=100)
        self.timeline_ax = self.timeline_fig.add_subplot(111)
        self.timeline_canvas = FigureCanvasTkAgg(
            self.timeline_fig, master=timeline_frame
        )
        self.timeline_canvas.get_tk_widget().pack(fill=tk.BOTH, expand=True)

        # Output tab
        output_frame = ttk.Frame(self.notebook)
        self.notebook.add(output_frame, text="Output")

        self.output_text_widget = tk.Text(
            output_frame, wrap=tk.WORD, bg="black", fg="white"
        )
        self.output_text_widget.pack(fill=tk.BOTH, expand=True)

        # Statistics tab
        stats_frame = ttk.Frame(self.notebook)
        self.notebook.add(stats_frame, text="Statistics")

        self.stats_text = tk.Text(stats_frame, wrap=tk.WORD)
        self.stats_text.pack(fill=tk.BOTH, expand=True)

        # Start animation
        self.ani = animation.FuncAnimation(
            self.fig, self.update_plot, interval=50, blit=False
        )
        self.timeline_ani = animation.FuncAnimation(
            self.timeline_fig, self.update_timeline, interval=100, blit=False
        )

    def browse_binary(self):
        """Open file dialog to select philo binary."""
        filepath = filedialog.askopenfilename(
            title="Select Philosophers Binary", filetypes=[("All files", "*")]
        )
        if filepath:
            self.binary_entry.delete(0, tk.END)
            self.binary_entry.insert(0, filepath)

    def start_simulation(self):
        """Start the philosophers simulation."""
        self.binary_path = self.binary_entry.get().strip()
        if not self.binary_path:
            messagebox.showerror("Error", "Please select a philo binary")
            return

        if not os.path.exists(self.binary_path):
            messagebox.showerror("Error", f"Binary not found: {self.binary_path}")
            return

        try:
            self.total_philosophers = int(self.philosophers_entry.get())
            self.time_to_die = int(self.time_to_die_entry.get())
            self.time_to_eat = int(self.time_to_eat_entry.get())
            self.time_to_sleep = int(self.time_to_sleep_entry.get())
            self.meals_cap = int(self.meals_cap_entry.get())
        except ValueError:
            messagebox.showerror("Error", "All parameters must be integers")
            return

        # Initialize philosophers
        self.philosophers = [Philosopher(i + 1) for i in range(self.total_philosophers)]

        # Prepare command
        cmd = [
            self.binary_path,
            str(self.total_philosophers),
            str(self.time_to_die),
            str(self.time_to_eat),
            str(self.time_to_sleep),
        ]

        if self.meals_cap > 0:
            cmd.append(str(self.meals_cap))

        # Clear output
        self.output_text_widget.delete(1.0, tk.END)
        self.output_text = ""

        # Update UI
        self.start_button.config(state=tk.DISABLED)
        self.stop_button.config(state=tk.NORMAL)
        self.simulation_status = "Running"
        self.status_label.config(text=self.simulation_status)

        # Start the process
        self.running = True
        self.start_time = time.time() * 1000  # Convert to ms

        # Run in a separate thread to avoid blocking the UI
        threading.Thread(target=self.run_process, args=(cmd,), daemon=True).start()

    def run_process(self, cmd: List[str]):
        """Run the philosophers process and monitor output."""
        try:
            self.process = subprocess.Popen(
                cmd,
                stdout=subprocess.PIPE,
                stderr=subprocess.STDOUT,
                text=True,
                bufsize=1,
                universal_newlines=True,
            )

            # Read output line by line
            for line in iter(self.process.stdout.readline, ""):
                if not self.running:
                    break

                self.process_output_line(line)

            return_code = self.process.wait()

            # Update status after process completion
            if self.running:
                self.root.after(
                    0, self.update_status, f"Finished (return code: {return_code})"
                )
            else:
                self.root.after(0, self.update_status, "Stopped")

        except Exception as e:
            self.root.after(0, self.update_status, f"Error: {str(e)}")
        finally:
            self.root.after(0, self.enable_start_button)

    def process_output_line(self, line: str):
        """Process a line of output from the philosophers program."""
        line = line.strip()
        if not line:
            return

        with self.output_lock:
            self.output_text += line + "\n"

            # Update output text widget (thread-safe)
            self.root.after(0, self.update_output_text, line)

        # Parse line to update philosopher states
        self.parse_line(line)

    def parse_line(self, line: str):
        """Parse a line of output to update philosopher states."""
        # Expected format: timestamp philosopher_id action
        # Example: "200 1 has taken a fork"
        match = re.match(r"(\d+)\s+(\d+)\s+(.*)", line)
        if match:
            timestamp = int(match.group(1))
            philo_id = int(match.group(2))
            action = match.group(3)

            if 1 <= philo_id <= len(self.philosophers):
                philo = self.philosophers[philo_id - 1]
                philo.update_state(action, timestamp)

    def update_output_text(self, line: str):
        """Update the output text widget with a new line (called from main thread)."""
        self.output_text_widget.insert(tk.END, line + "\n")
        self.output_text_widget.see(tk.END)

    def update_status(self, status: str):
        """Update the status label (called from main thread)."""
        self.simulation_status = status
        self.status_label.config(text=status)

    def enable_start_button(self):
        """Enable the start button and disable stop button."""
        self.start_button.config(state=tk.NORMAL)
        self.stop_button.config(state=tk.DISABLED)
        self.running = False

    def stop_simulation(self):
        """Stop the running simulation."""
        if self.process and self.running:
            self.running = False
            try:
                # Send SIGTERM to the process group
                os.killpg(os.getpgid(self.process.pid), signal.SIGTERM)
            except:
                # Fallback to terminate()
                self.process.terminate()

            self.update_status("Stopping...")

    def update_plot(self, frame):
        """Update the visualization plot."""
        if not self.philosophers:
            return

        self.ax.clear()

        # Set up the table
        table_data = []
        table_colors = []

        # Header row
        table_data.append(["Philosopher", "State", "Meals", "Forks"])
        table_colors.append(["#CCCCCC", "#CCCCCC", "#CCCCCC", "#CCCCCC"])

        # Data rows
        for philo in self.philosophers:
            state = philo.state
            meals = str(philo.meals_eaten)
            forks = (
                f"{'L' if philo.left_fork else '-'}{'R' if philo.right_fork else '-'}"
            )

            row = [f"#{philo.id}", state, meals, forks]
            table_data.append(row)

            # Color based on state
            if state == "is eating":
                color = "#90EE90"  # Light green
            elif state == "is sleeping":
                color = "#ADD8E6"  # Light blue
            elif state == "is thinking":
                color = "#FFFACD"  # Light yellow
            elif state == "died":
                color = "#FF6347"  # Tomato red
            else:
                color = "#FFFFFF"  # White

            table_colors.append([color, color, color, color])

        # Create color list for table rows
        table = self.ax.table(
            cellText=table_data,
            cellColours=table_colors,
            loc="center",
            cellLoc="center",
        )

        # Set table properties
        table.auto_set_font_size(False)
        table.set_fontsize(10)
        table.scale(1, 1.5)

        # Remove axes
        self.ax.axis("off")

        # Set title with parameter info
        self.ax.set_title(
            f"Philosophers: {self.total_philosophers}, "
            f"Die: {self.time_to_die}ms, "
            f"Eat: {self.time_to_eat}ms, "
            f"Sleep: {self.time_to_sleep}ms, "
            f"Meals: {self.meals_cap}"
        )

        # Update statistics
        self.update_statistics()

    def update_timeline(self, frame):
        """Update the timeline visualization."""
        if not self.philosophers:
            return

        self.timeline_ax.clear()

        # Get current time
        current_time = time.time() * 1000 - self.start_time

        # Setup colors for different states
        colors = {
            "has taken a fork": "#FFD700",  # Gold
            "is eating": "#32CD32",  # Lime green
            "is sleeping": "#4169E1",  # Royal blue
            "is thinking": "#9370DB",  # Medium purple
            "died": "#FF0000",  # Red
        }

        y_ticks = []
        y_labels = []

        for i, philo in enumerate(self.philosophers):
            y_pos = i * 2
            y_ticks.append(y_pos)
            y_labels.append(f"Philo #{philo.id}")

            # Sort events by timestamp
            events = sorted(philo.events, key=lambda x: x[0])

            if not events:
                continue

            # Draw event segments
            for j in range(len(events)):
                start_time = events[j][0]
                state = events[j][1]

                # Determine end time
                if j < len(events) - 1:
                    end_time = events[j + 1][0]
                else:
                    end_time = current_time

                # Draw the segment
                color = colors.get(state, "#CCCCCC")  # Default to gray
                self.timeline_ax.barh(
                    y_pos,
                    end_time - start_time,
                    left=start_time,
                    height=0.8,
                    color=color,
                    alpha=0.7,
                )

                # Add text label for longer segments
                if end_time - start_time > self.time_to_die / 10:
                    text_x = start_time + (end_time - start_time) / 2
                    self.timeline_ax.text(
                        text_x,
                        y_pos,
                        state,
                        ha="center",
                        va="center",
                        fontsize=8,
                        color="black",
                    )

        # Set ticks and labels
        self.timeline_ax.set_yticks(y_ticks)
        self.timeline_ax.set_yticklabels(y_labels)

        # Set title and labels
        self.timeline_ax.set_title("Philosophers Timeline")
        self.timeline_ax.set_xlabel("Time (ms)")

        # Adjust x-axis limits
        if events:
            max_time = max(
                [event[0] for philo in self.philosophers for event in philo.events]
            )
            self.timeline_ax.set_xlim(0, max(max_time * 1.1, current_time))

        # Add grid
        self.timeline_ax.grid(True, axis="x", linestyle="--", alpha=0.3)

        # Add legend
        handles = [
            plt.Rectangle((0, 0), 1, 1, color=color) for color in colors.values()
        ]
        self.timeline_ax.legend(handles, colors.keys(), loc="upper right")

        self.timeline_fig.tight_layout()

    def update_statistics(self):
        """Update the statistics view with current data."""
        stats = []
        stats.append(f"Simulation Parameters:")
        stats.append(f"  Philosophers: {self.total_philosophers}")
        stats.append(f"  Time to die: {self.time_to_die} ms")
        stats.append(f"  Time to eat: {self.time_to_eat} ms")
        stats.append(f"  Time to sleep: {self.time_to_sleep} ms")
        stats.append(f"  Meals cap: {self.meals_cap}")
        stats.append("")

        stats.append(f"Simulation Status: {self.simulation_status}")
        stats.append("")

        if self.philosophers:
            # Count states
            states = {
                "eating": 0,
                "thinking": 0,
                "sleeping": 0,
                "dead": 0,
                "waiting": 0,
            }

            for philo in self.philosophers:
                if philo.state == "is eating":
                    states["eating"] += 1
                elif philo.state == "is thinking":
                    states["thinking"] += 1
                elif philo.state == "is sleeping":
                    states["sleeping"] += 1
                elif philo.state == "died":
                    states["dead"] += 1
                else:
                    states["waiting"] += 1

            stats.append(f"Current State Counts:")
            stats.append(f"  Eating: {states['eating']}")
            stats.append(f"  Thinking: {states['thinking']}")
            stats.append(f"  Sleeping: {states['sleeping']}")
            stats.append(f"  Dead: {states['dead']}")
            stats.append(f"  Waiting: {states['waiting']}")
            stats.append("")

            # Meal statistics
            meal_counts = [p.meals_eaten for p in self.philosophers]
            if meal_counts:
                avg_meals = sum(meal_counts) / len(meal_counts)
                min_meals = min(meal_counts)
                max_meals = max(meal_counts)

                stats.append(f"Meal Statistics:")
                stats.append(f"  Total meals eaten: {sum(meal_counts)}")
                stats.append(f"  Average meals per philosopher: {avg_meals:.2f}")
                stats.append(f"  Min meals eaten: {min_meals}")
                stats.append(f"  Max meals eaten: {max_meals}")

                if self.meals_cap > 0:
                    finished = sum(1 for m in meal_counts if m >= self.meals_cap)
                    stats.append(
                        f"  Philosophers finished eating: {finished}/{self.total_philosophers}"
                    )

        # Update the stats text widget
        self.stats_text.delete(1.0, tk.END)
        self.stats_text.insert(tk.END, "\n".join(stats))

    def run_tests(self):
        """Run a series of test cases."""
        if not self.binary_entry.get().strip():
            messagebox.showerror("Error", "Please select a philo binary first")
            return

        test_window = tk.Toplevel(self.root)
        test_window.title("Philosophers Test Suite")
        test_window.geometry("800x600")

        test_suite = PhilosophersTestSuite(self.binary_entry.get().strip(), test_window)
        test_suite.pack(fill=tk.BOTH, expand=True)

    def on_close(self):
        """Handle window close event."""
        self.stop_simulation()
        self.root.destroy()


class PhilosophersTestSuite(ttk.Frame):
    """A test suite for running multiple philosophers tests."""

    def __init__(self, binary_path: str, parent: tk.Toplevel):
        super().__init__(parent, padding="10")
        self.binary_path = binary_path
        self.parent = parent
        self.tests = []
        self.results = {"passed": 0, "failed": 0, "timeout": 0}
        self.current_test = None

        self.create_ui()
        self.generate_standard_tests()

    def create_ui(self):
        """Create the test suite UI."""
        # Title
        ttk.Label(
            self, text="Philosophers Test Suite", font=("Arial", 14, "bold")
        ).pack(pady=10)

        # Test control frame
        control_frame = ttk.LabelFrame(self, text="Test Controls", padding="10")
        control_frame.pack(fill=tk.X, padx=5, pady=5)

        ttk.Button(
            control_frame, text="Run All Tests", command=self.run_all_tests
        ).pack(side=tk.LEFT, padx=5)
        ttk.Button(
            control_frame, text="Add Custom Test", command=self.add_custom_test
        ).pack(side=tk.LEFT, padx=5)

        # Results summary
        self.summary_var = tk.StringVar(
            value="Tests: 0 | Passed: 0 | Failed: 0 | Timeout: 0"
        )
        ttk.Label(control_frame, textvariable=self.summary_var).pack(
            side=tk.RIGHT, padx=10
        )

        # Test list frame
        list_frame = ttk.LabelFrame(self, text="Test Cases", padding="10")
        list_frame.pack(fill=tk.BOTH, expand=True, padx=5, pady=5)

        # Create treeview for tests
        self.tree = ttk.Treeview(
            list_frame,
            columns=("name", "args", "expected", "status", "time"),
            show="headings",
        )

        # Define headings
        self.tree.heading("name", text="Test Name")
        self.tree.heading("args", text="Arguments")
        self.tree.heading("expected", text="Expected Result")
        self.tree.heading("status", text="Status")
        self.tree.heading("time", text="Time (s)")

        # Define columns
        self.tree.column("name", width=150)
        self.tree.column("args", width=250)
        self.tree.column("expected", width=100)
        self.tree.column("status", width=80)
        self.tree.column("time", width=80)

        # Add scrollbar
        scrollbar = ttk.Scrollbar(
            list_frame, orient=tk.VERTICAL, command=self.tree.yview
        )
        self.tree.configure(yscrollcommand=scrollbar.set)

        # Pack treeview and scrollbar
        self.tree.pack(side=tk.LEFT, fill=tk.BOTH, expand=True)
        scrollbar.pack(side=tk.RIGHT, fill=tk.Y)

        # Test output frame
        output_frame = ttk.LabelFrame(self, text="Test Output", padding="10")
        output_frame.pack(fill=tk.BOTH, expand=True, padx=5, pady=5)

        self.output_text = tk.Text(
            output_frame, wrap=tk.WORD, bg="black", fg="white", height=10
        )
        self.output_text.pack(fill=tk.BOTH, expand=True)

    def generate_standard_tests(self):
        """Generate standard test cases for common scenarios."""
        test_cases = [
            {
                "name": "Basic death test (1 philosopher)",
                "args": ["1", "800", "200", "200"],
                "expected": "die",
                "timeout": 2,
            },
            {
                "name": "No death test (enough time)",
                "args": ["4", "410", "200", "200"],
                "expected": "running",
                "timeout": 3,
            },
            {
                "name": "Death test (tight timing)",
                "args": ["4", "310", "200", "200"],
                "expected": "die",
                "timeout": 2,
            },
            {
                "name": "Meals limit test",
                "args": ["5", "800", "200", "200", "7"],
                "expected": "complete",
                "timeout": 10,
            },
            {
                "name": "Minimum values test",
                "args": ["2", "60", "30", "30"],
                "expected": "running",
                "timeout": 2,
            },
            {
                "name": "Performance test (20 philosophers)",
                "args": ["20", "800", "200", "200"],
                "expected": "running",
                "timeout": 5,
            },
            {
                "name": "Negative value test",
                "args": ["4", "-200", "200", "200"],
                "expected": "error",
                "timeout": 1,
            },
        ]

        for case in test_cases:
            self.add_test(
                name=case["name"],
                args=case["args"],
                expected=case["expected"],
                timeout=case["timeout"],
            )

    def add_test(self, name: str, args: List[str], expected: str, timeout: int = 5):
        """Add a test case to the suite."""
        test = {
            "name": name,
            "args": args,
            "expected": expected,
            "timeout": timeout,
            "status": "Pending",
            "time": 0,
            "output": "",
        }

        self.tests.append(test)

        # Add to treeview
        self.tree.insert(
            "", tk.END, values=(name, " ".join(args), expected, "Pending", "-")
        )

    def add_custom_test(self):
        """Add a custom test case."""
        custom_window = tk.Toplevel(self.parent)
        custom_window.title("Add Custom Test")
        custom_window.geometry("500x300")
        custom_window.grab_set()  # Modal window

        ttk.Label(custom_window, text="Test Name:").grid(
            row=0, column=0, padx=5, pady=5, sticky=tk.W
        )
        name_entry = ttk.Entry(custom_window, width=40)
        name_entry.grid(row=0, column=1, padx=5, pady=5, sticky=tk.W)

        ttk.Label(custom_window, text="Number of Philosophers:").grid(
            row=1, column=0, padx=5, pady=5, sticky=tk.W
        )
        philos_entry = ttk.Entry(custom_window, width=10)
        philos_entry.grid(row=1, column=1, padx=5, pady=5, sticky=tk.W)

        ttk.Label(custom_window, text="Time to die (ms):").grid(
            row=2, column=0, padx=5, pady=5, sticky=tk.W
        )
        die_entry = ttk.Entry(custom_window, width=10)
        die_entry.grid(row=2, column=1, padx=5, pady=5, sticky=tk.W)

        ttk.Label(custom_window, text="Time to eat (ms):").grid(
            row=3, column=0, padx=5, pady=5, sticky=tk.W
        )
        eat_entry = ttk.Entry(custom_window, width=10)
        eat_entry.grid(row=3, column=1, padx=5, pady=5, sticky=tk.W)

        ttk.Label(custom_window, text="Time to sleep (ms):").grid(
            row=4, column=0, padx=5, pady=5, sticky=tk.W
        )
        sleep_entry = ttk.Entry(custom_window, width=10)
        sleep_entry.grid(row=4, column=1, padx=5, pady=5, sticky=tk.W)

        ttk.Label(custom_window, text="Meals cap (0 for none):").grid(
            row=5, column=0, padx=5, pady=5, sticky=tk.W
        )
        meals_entry = ttk.Entry(custom_window, width=10)
        meals_entry.insert(0, "0")
        meals_entry.grid(row=5, column=1, padx=5, pady=5, sticky=tk.W)

        ttk.Label(custom_window, text="Expected result:").grid(
            row=6, column=0, padx=5, pady=5, sticky=tk.W
        )
        expected_var = tk.StringVar(value="running")
        expected_combo = ttk.Combobox(
            custom_window,
            textvariable=expected_var,
            values=["running", "die", "complete", "error"],
        )
        expected_combo.grid(row=6, column=1, padx=5, pady=5, sticky=tk.W)

        ttk.Label(custom_window, text="Timeout (seconds):").grid(
            row=7, column=0, padx=5, pady=5, sticky=tk.W
        )
        timeout_entry = ttk.Entry(custom_window, width=10)
        timeout_entry.insert(0, "5")
        timeout_entry.grid(row=7, column=1, padx=5, pady=5, sticky=tk.W)

        def save_test():
            try:
                args = [
                    philos_entry.get(),
                    die_entry.get(),
                    eat_entry.get(),
                    sleep_entry.get(),
                ]

                if meals_entry.get() and int(meals_entry.get()) > 0:
                    args.append(meals_entry.get())

                self.add_test(
                    name=name_entry.get(),
                    args=args,
                    expected=expected_var.get(),
                    timeout=int(timeout_entry.get()),
                )
                custom_window.destroy()

            except ValueError:
                messagebox.showerror(
                    "Error", "All numeric fields must contain valid numbers"
                )

        ttk.Button(custom_window, text="Add Test", command=save_test).grid(
            row=8, column=0, columnspan=2, pady=20
        )

    def run_test(self, test_index: int):
        """Run a single test case."""
        if test_index < 0 or test_index >= len(self.tests):
            return False

        test = self.tests[test_index]

        # Update tree status
        item_id = self.tree.get_children()[test_index]
        self.tree.item(
            item_id,
            values=(
                test["name"],
                " ".join(test["args"]),
                test["expected"],
                "Running",
                "-",
            ),
        )

        # Clear output
        self.output_text.delete(1.0, tk.END)
        self.output_text.insert(tk.END, f"Running test: {test['name']}\n")
        self.output_text.insert(
            tk.END, f"Command: {self.binary_path} {' '.join(test['args'])}\n\n"
        )

        cmd = [self.binary_path] + test["args"]
        start_time = time.time()

        try:
            if test["expected"] == "running":
                # For tests expected to keep running without death
                proc = subprocess.Popen(
                    cmd,
                    stdout=subprocess.PIPE,
                    stderr=subprocess.PIPE,
                    text=True,
                    preexec_fn=os.setsid,
                )

                # Process output for a while
                output = []
                start_read_time = time.time()

                while time.time() - start_read_time < test["timeout"] - 0.5:
                    # Try to read a line with timeout
                    if proc.poll() is not None:
                        break

                    # Read with timeout using non-blocking reads
                    line = None
                    try:
                        line = proc.stdout.readline()
                        if line:
                            output.append(line.strip())
                            self.output_text.insert(tk.END, line)
                            self.output_text.see(tk.END)
                    except:
                        time.sleep(0.1)
                        continue
                    # This replaced with the try-except block above

                # Check if still running
                if proc.poll() is None:
                    # Still running - good!
                    os.killpg(os.getpgid(proc.pid), signal.SIGTERM)
                    test["status"] = "Passed"
                    test["output"] = "\n".join(output)
                    self.output_text.insert(
                        tk.END, "\nProcess was still running as expected (no death)\n"
                    )
                else:
                    stdout, stderr = proc.communicate()
                    test["output"] = "\n".join(output) + "\n" + stdout + stderr
                    test["status"] = "Failed"
                    self.output_text.insert(
                        tk.END,
                        f"\nProcess terminated unexpectedly. Exit code: {proc.returncode}\n",
                    )
            else:
                # For tests expecting completion or death
                result = subprocess.run(
                    cmd, capture_output=True, text=True, timeout=test["timeout"]
                )
                test["output"] = result.stdout + result.stderr

                # Display output
                self.output_text.insert(tk.END, test["output"])

                if test["expected"] == "die" and "died" in test["output"]:
                    test["status"] = "Passed"
                elif test["expected"] == "complete" and "died" not in test["output"]:
                    test["status"] = "Passed"
                elif test["expected"] == "error" and result.returncode != 0:
                    test["status"] = "Passed"
                else:
                    test["status"] = "Failed"

        except subprocess.TimeoutExpired:
            if test["expected"] == "running":
                test["status"] = "Passed"
                test["output"] = "Process was still running as expected (timeout)"
                self.output_text.insert(
                    tk.END, "\nProcess was still running as expected (timeout)\n"
                )
            else:
                test["status"] = "Timeout"
                self.output_text.insert(
                    tk.END, f"\nTest timed out after {test['timeout']} seconds\n"
                )

        except Exception as e:
            test["status"] = "Failed"
            test["output"] = f"Exception: {str(e)}"
            self.output_text.insert(tk.END, f"\nTest error: {e}\n")

        # Calculate execution time
        test["time"] = time.time() - start_time

        # Update tree with results
        self.tree.item(
            item_id,
            values=(
                test["name"],
                " ".join(test["args"]),
                test["expected"],
                test["status"],
                f"{test['time']:.2f}",
            ),
        )

        # Update tree item color based on status
        if test["status"] == "Passed":
            self.tree.tag_configure("passed", background="#90EE90")  # Light green
            self.tree.item(item_id, tags=("passed",))
        elif test["status"] == "Failed":
            self.tree.tag_configure("failed", background="#FFB6C1")  # Light red
            self.tree.item(item_id, tags=("failed",))
        elif test["status"] == "Timeout":
            self.tree.tag_configure("timeout", background="#FFD700")  # Gold
            self.tree.item(item_id, tags=("timeout",))

        return test["status"] == "Passed"

    def run_all_tests(self):
        """Run all test cases in sequence."""
        if not self.tests:
            messagebox.showinfo("No Tests", "No tests available to run")
            return

        # Reset results
        self.results = {"passed": 0, "failed": 0, "timeout": 0}

        # Run each test
        for i in range(len(self.tests)):
            # Run the test and update self.results based on status
            success = self.run_test(i)
            status = self.tests[i]["status"]

            if status == "Passed":
                self.results["passed"] += 1
            elif status == "Timeout":
                self.results["timeout"] += 1
            else:
                self.results["failed"] += 1

            # Update summary
            self.summary_var.set(
                f"Tests: {len(self.tests)} | "
                f"Passed: {self.results['passed']} | "
                f"Failed: {self.results['failed']} | "
                f"Timeout: {self.results['timeout']}"
            )

            # Process events to update UI
            self.update()

        # Final summary
        messagebox.showinfo(
            "Test Results",
            f"Tests completed\n\n"
            f"Total tests: {len(self.tests)}\n"
            f"Passed: {self.results['passed']}\n"
            f"Failed: {self.results['failed']}\n"
            f"Timeout: {self.results['timeout']}",
        )


if __name__ == "__main__":
    if len(sys.argv) > 1:
        binary_path = sys.argv[1]
        root = tk.Tk()
        app = PhiloVisualizer(root)
        app.binary_entry.delete(0, tk.END)
        app.binary_entry.insert(0, binary_path)
        root.mainloop()
    else:
        root = tk.Tk()
        app = PhiloVisualizer(root)
        root.mainloop()

