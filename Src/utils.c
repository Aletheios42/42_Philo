#include "../Inc/philo.h"

long ft_philo_atol(char *nbr) {
  long result = 0;
  int i = -1;
  while (nbr[++i] == ' ')
    i++;
  while (nbr[i] >= '0' && nbr[i] <= '9') {
    result = result * 10 + (nbr[i] - '0');
    i++;
  }
  if (nbr[i] != '\0' || result > INT_MAX || result < INT_MIN)
    return (-1);
  return (result);
}
