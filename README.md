- Dijkstra's original paper on the Dining Philosophers problem

### Steps for core funtionality

1. Initialize mutexes for each fork
2. Create philosopher threads
3. Each philosopher thread should:
   a. Try to grab left fork (mutex lock)
   b. Try to grab right fork (mutex lock)
   c. If both acquired, eat for time_to_eat ms
   d. Release both forks (mutex unlock)
   e. Sleep for time_to_sleep ms
   f. Think (no specific duration)
   g. Repeat until full (if meals_cap specified)
4. Monitor thread checks if any philosopher hasn't eaten in time_to_die ms
5. Clean up mutexes and threads when simulation ends
