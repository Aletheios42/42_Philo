#include "../Inc/philo.h"

int main(int ac, char **av) {
  int i;
  t_input input;
  t_philo *philos = NULL;
  t_fork *forks = NULL;
  pthread_mutex_t print;
  t_end end;

  if (ac != 5 && ac != 6) {
    print_error("Usage: ./philo number_of_philosophers time_to_die time_to_eat "
                "time_to_sleep [number_of_times_each_philosopher_must_eat]");
    return (1);
  }

  if (!parser(&input, av, ac))
    print_error("Invalid input parameters");

  // Create forks
  create_forks(&forks, input.philosophers);

  // Initialize mutexes
  pthread_mutex_init(&print, NULL);
  pthread_mutex_init(&end.end_mutex, NULL);

  // Create philosophers
  create_philosophers(&philos, forks, &input, &print, &end);

  // Initialize last_meal_time for all philosophers
  long start_time = get_current_time();
  i = -1;
  while (++i < input.philosophers) {
    philos[i].last_meal_time = start_time;
    philos[i].start_time = start_time;
  }

  // Start philosopher threads
  i = -1;
  while (++i < input.philosophers) {
    if (pthread_create(&philos[i].thread, NULL, lifecycle, &philos[i]) != 0) {
      print_error("Failed to create philosopher thread");
    }
  }

  // Monitor philosophers for death
  monitor_philos(philos, &input, &end);

  // Wait for philosopher threads to finish
  i = -1;
  while (++i < input.philosophers)
    pthread_join(philos[i].thread, NULL);

  // Clean up resources
  free_resources(philos, forks, &print, input.philosophers);

  return (0);
}
