#include "../Inc/philo.h"

void create_forks(t_fork **forks, int nbr) {
  int i;
  t_fork *first = NULL;
  t_fork *current = NULL;
  t_fork *prev = NULL;

  *forks = NULL;
  i = -1;
  while (++i < nbr) {
    current = (t_fork *)malloc(sizeof(t_fork));
    if (!current) {
      print_error("Failed to allocate memory for fork");
    }
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
  if (current)
    current->next = first;
}

void create_philosophers(t_philo **philos, t_fork *forks, t_input *input,
                         pthread_mutex_t *print, t_end *end) {
  int i;
  int j;

  *philos = (t_philo *)malloc(sizeof(t_philo) * input->philosophers);
  if (!*philos) {
    print_error("Failed to allocate memory for philosophers");
  }

  // Initialize philosophers
  i = -1;
  while (++i < input->philosophers) {
    (*philos)[i].id = i + 1;
    (*philos)[i].meals_counter = 0;
    (*philos)[i].last_meal_time = 0;
    (*philos)[i].print_mutex = *print;
    (*philos)[i].params = input;

    t_fork *current = forks;
    j = -1;
    while (++j < i)
      current = current->next;

    (*philos)[i].left_fork = current;
    (*philos)[i].right_fork = current->next;
  }
  i = -1;
  while (++i < input->philosophers) {
    (*philos)[i].simulation_end = &(end->simulation_end);
    (*philos)[i].end_mutex = &(end->end_mutex);
  }
}

void free_resources(t_philo *philos, t_fork *forks, pthread_mutex_t *print,
                    int nbr_philos) {
  int i;
  t_fork *current;
  t_fork *next;

  pthread_mutex_destroy(print);
  if (philos) {
    free(philos);
  }

  if (forks) {
    current = forks;
    i = -1;
    while (++i < nbr_philos) {
      next = current->next;
      pthread_mutex_destroy(&current->mutex);
      free(current);
      current = next;
      if (current == forks)
        break;
    }
  }
}
