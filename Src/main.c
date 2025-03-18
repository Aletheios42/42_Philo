#include "../Inc/philo.h"

void monitor_philos(t_philo *philos, t_input *input, t_end *end) {

  int i;
  int full_philos;

  while (1) {
    i = 0;
    full_philos = 0;
    if (check_starvation(philos, input, end))
      return;

    if (input->meals_cap > 0 && philos[i].meals_counter >= input->meals_cap)
      full_philos++;
    i++;
  }

  if (input->meals_cap > 0 && full_philos == input->philosophers) {
    pthread_mutex_lock(&(end->end_mutex));
    end->simulation_end = true;
    pthread_mutex_unlock(&(end->end_mutex));
    return;
  }
  usleep(1000);
}
bool parser(t_input *input, char **av, int ac) {
  input->philosophers = ft_philo_atol(av[1]);
  if (input->philosophers <= 0)
    return (false);

  input->time_to_die = ft_philo_atol(av[2]);
  if (input->time_to_die <= 0)
    return (false);

  input->time_to_eat = ft_philo_atol(av[3]);
  if (input->time_to_eat <= 0)
    return (false);

  input->time_to_sleep = ft_philo_atol(av[4]);
  if (input->time_to_sleep <= 0)
    return (false);

  if (ac == 6) {
    input->meals_cap = ft_philo_atol(av[5]);
    if (input->meals_cap <= 0)
      return (false);
  } else {
    input->meals_cap = 0;
  }
  return (true);
}

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
  while (++i < input.philosophers)
    philos[i].last_meal_time = start_time;

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
