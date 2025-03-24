#include "../Inc/philo.h"

void print_error(const char *error) {
  printf("%s\n", error);
  exit(1);
}

void print_status(t_philo *philo, const char *status) {
  bool simulation_stopped;

  pthread_mutex_lock(philo->end_mutex);
  simulation_stopped = *philo->simulation_end;
  pthread_mutex_unlock(philo->end_mutex);

  if (!simulation_stopped) {
    pthread_mutex_lock(&philo->print_mutex);
    printf("%7ld %4d %s\n", get_current_time() - philo->start_time, philo->id,
           status);
    pthread_mutex_unlock(&philo->print_mutex);
  }
}
