#include "../Inc/philo.h"

bool to_eat(t_philo *philo) {
  if (philo->id % 2 == 1) {
    pthread_mutex_lock(&philo->left_fork->mutex);
    print_status(philo, "has taken a fork");
    pthread_mutex_lock(&philo->right_fork->mutex);
    print_status(philo, "has taken a fork");
  } else {
    pthread_mutex_lock(&philo->right_fork->mutex);
    print_status(philo, "has taken a fork");

    pthread_mutex_lock(&philo->left_fork->mutex);
    print_status(philo, "has taken a fork");
  }

  print_status(philo, "is eating");
  philo->last_meal_time = get_current_time();
  philo->meals_counter++;

  if (philo->id % 2 == 1) {
    pthread_mutex_unlock(&philo->right_fork->mutex);
    pthread_mutex_unlock(&philo->left_fork->mutex);
  } else {
    pthread_mutex_unlock(&philo->left_fork->mutex);
    pthread_mutex_unlock(&philo->right_fork->mutex);
  }

  // Check if philosopher has eaten enough
  if (philo->params->meals_cap > 0 &&
      philo->meals_counter >= philo->params->meals_cap) {
    return true; // Return true if meal cap reached
  }
  return false; // Continue otherwise
}

void to_sleep(t_philo *philo) {
  print_status(philo, "is sleeping");
  msleep(philo->params->time_to_sleep);
}

void to_think(t_philo *philo) { print_status(philo, "is thinking"); }

bool should_exit(t_philo *philo) {
  bool simulation_stopped;
  pthread_mutex_lock(philo->end_mutex);
  simulation_stopped = *philo->simulation_end;
  pthread_mutex_unlock(philo->end_mutex);

  return simulation_stopped;
}

void *lifecycle(void *arg) {
  t_philo *philo = (t_philo *)arg;

  while (!should_exit(philo)) {
    if (to_eat(philo)) // Break if meal cap reached
      break;

    to_sleep(philo);
    to_think(philo);
  }

  return NULL;
}
