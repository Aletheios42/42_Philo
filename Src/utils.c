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

void msleep(long ms) {
  long start_time = get_current_time();

  while (get_current_time() - start_time < ms) {
    usleep(500); // Sleep for 0.5ms to reduce CPU usage
  }
}

int check_starvation(t_philo *philos, t_input *input, t_end *end) {

  int i;
  i = -1;
  while (++i < input->philosophers) {
    // Check if philosopher has died
    if (get_current_time() - philos[i].last_meal_time > input->time_to_die) {
      // Set simulation end flag
      pthread_mutex_lock(&(end->end_mutex));
      end->simulation_end = true;
      pthread_mutex_unlock(&(end->end_mutex));
      // Print death status
      pthread_mutex_lock(&philos[i].print_mutex);
      printf("%ld %d died\n", get_current_time(), philos[i].id);
      pthread_mutex_unlock(&philos[i].print_mutex);
      return 1;
    }
  }
  return 0;
}
