#include "../Inc/philo.h"

void create_forks(t_fork **forks, int nbr) {
  int i;
  t_fork *first = NULL;
  t_fork *current = NULL;
  t_fork *prev = NULL;

  *forks = NULL;

  for (i = 0; i < nbr; i++) {
    current = (t_fork *)malloc(sizeof(t_fork));
    if (!current) {
      print_error("Failed to allocate memory for fork");
    }

    // Initialize fork mutex
    pthread_mutex_init(&current->mutex, NULL);
    current->id = i + 1;
    current->next = NULL;

    if (!*forks) {
      *forks = current;
      first = current;
    } else {
      prev->next = current;
    }

    prev = current;
  }

  // Make the list circular by connecting last fork to first
  if (current)
    current->next = first;
}

void create_philosophers(t_philo **philos, t_fork *forks, t_input *input,
                         pthread_mutex_t *print) {
  int i;

  // Allocate memory for philosophers
  *philos = (t_philo *)malloc(sizeof(t_philo) * input->philosophers);
  if (!*philos) {
    print_error("Failed to allocate memory for philosophers");
  }

  // Initialize philosophers
  for (i = 0; i < input->philosophers; i++) {
    (*philos)[i].id = i + 1;
    (*philos)[i].meals_counter = 0;
    (*philos)[i].last_meal_time = 0;
    (*philos)[i].print_mutex = *print;
    (*philos)[i].params = input;

    // Assign forks to philosophers
    t_fork *current = forks;
    for (int j = 0; j < i; j++) {
      current = current->next;
    }

    (*philos)[i].left_fork = current;
    (*philos)[i].right_fork = current->next;
  }
}

void free_resources(t_philo *philos, t_fork *forks, pthread_mutex_t *print,
                    int n) {
  int i;
  t_fork *current;
  t_fork *next;

  // Destroy print mutex
  pthread_mutex_destroy(print);

  // Free philosophers
  if (philos) {
    free(philos);
  }

  // Free forks and destroy mutexes
  if (forks) {
    current = forks;
    for (i = 0; i < n; i++) {
      next = current->next;
      pthread_mutex_destroy(&current->mutex);
      free(current);
      current = next;
      if (current == forks)
        break;
    }
  }
}

void *lifecycle(void *arg) {
  t_philo *philo = (t_philo *)arg;

  // Stagger philosophers to prevent deadlock
  // Odd-numbered philosophers start by thinking a bit
  if (philo->id % 2 != 0) {
    print_status(philo, "is thinking");
    msleep(philo->params->time_to_eat / 2);
  }

  while (1) {
    bool simulation_stopped;
    pthread_mutex_lock(philo->end_mutex);
    simulation_stopped = *philo->simulation_end;
    pthread_mutex_unlock(philo->end_mutex);

    if (simulation_stopped)
      break;

    // Try to take left fork
    pthread_mutex_lock(&philo->left_fork->mutex);
    print_status(philo, "has taken a fork");

    // Try to take right fork
    pthread_mutex_lock(&philo->right_fork->mutex);
    print_status(philo, "has taken a fork");

    // Eating
    print_status(philo, "is eating");
    philo->last_meal_time = get_current_time();
    msleep(philo->params->time_to_eat);
    philo->meals_counter++;

    // Release forks
    pthread_mutex_unlock(&philo->right_fork->mutex);
    pthread_mutex_unlock(&philo->left_fork->mutex);

    // Check if philosopher has eaten enough
    if (philo->params->meals_cap > 0 &&
        philo->meals_counter >= philo->params->meals_cap) {
      break;
    }

    // Sleeping
    print_status(philo, "is sleeping");
    msleep(philo->params->time_to_sleep);

    // Thinking
    print_status(philo, "is thinking");
  }

  return (NULL);
}

void monitor_philos(t_philo *philos, t_input *input, bool *simulation_end,
                    pthread_mutex_t *end_mutex) {
  int i;
  int full_philos;

  while (1) {
    i = 0;
    full_philos = 0;

    while (i < input->philosophers) {
      // Check if philosopher has died
      if (get_current_time() - philos[i].last_meal_time > input->time_to_die) {
        // Set simulation end flag
        pthread_mutex_lock(end_mutex);
        *simulation_end = true;
        pthread_mutex_unlock(end_mutex);

        // Print death status
        pthread_mutex_lock(&philos[i].print_mutex);
        printf("%ld %d died\n", get_current_time(), philos[i].id);
        pthread_mutex_unlock(&philos[i].print_mutex);

        return;
      }

      // Check if philosopher is full
      if (input->meals_cap > 0 && philos[i].meals_counter >= input->meals_cap) {
        full_philos++;
      }

      i++;
    }

    // Check if all philosophers are full
    if (input->meals_cap > 0 && full_philos == input->philosophers) {
      pthread_mutex_lock(end_mutex);
      *simulation_end = true;
      pthread_mutex_unlock(end_mutex);
      return;
    }

    // Sleep a bit to reduce CPU usage
    usleep(1000);
  }
}
