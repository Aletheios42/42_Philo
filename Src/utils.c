#include "../Inc/philo.h"

long ft_philo_atol(char *nbr) {
  long result = 0;
  int i = 0;
  int sign = 1;

  while (nbr[i] == ' ' || (nbr[i] >= 9 && nbr[i] <= 13))
    i++;
  if (nbr[i] == '-' || nbr[i] == '+') {
    if (nbr[i] == '-')
      sign = -1;
    i++;
  }
  while (nbr[i] >= '0' && nbr[i] <= '9') {
    result = result * 10 + (nbr[i] - '0');
    i++;
    if ((sign == 1 && result > INT_MAX) ||
        (sign == -1 && result * sign < INT_MIN))
      return (-1);
  }
  if (nbr[i] != '\0')
    return (-1);
  return (result * sign);
}

long get_current_time(void) {
  struct timeval time;

  if (gettimeofday(&time, NULL) < 0)
    return (0);
  return ((time.tv_sec * 1000) + (time.tv_usec / 1000));
}

void print_status(t_philo *philo, const char *status) {
  bool simulation_stopped;

  pthread_mutex_lock(philo->end_mutex);
  simulation_stopped = *philo->simulation_end;
  pthread_mutex_unlock(philo->end_mutex);

  if (!simulation_stopped) {
    pthread_mutex_lock(&philo->print_mutex);
    printf("%ld %d %s\n", get_current_time(), philo->id, status);
    pthread_mutex_unlock(&philo->print_mutex);
  }
}

void msleep(long ms) {
  long start_time = get_current_time();

  while (get_current_time() - start_time < ms) {
    usleep(500); // Sleep for 0.5ms to reduce CPU usage
  }
}
