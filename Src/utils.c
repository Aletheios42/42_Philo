#include "../Inc/philo.h"

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
