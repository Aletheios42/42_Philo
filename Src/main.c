#include "../Inc/philo.h"
#include <pthread.h>

void print_error(const char *error) {
  printf("%s\n", error);
  exit(1);
}

bool parser(t_input *input, char **av, int ac) {
  input->philosophers = ft_philo_atol(av[1]);
  if (input->philosophers == -1)
    return (0);
  input->time_to_die = ft_philo_atol(av[2]);
  if (input->time_to_die == -1)
    return (0);
  input->time_to_eat = ft_philo_atol(av[3]);
  if (input->time_to_eat == -1)
    return (0);
  input->time_to_sleep = ft_philo_atol(av[4]);
  if (input->time_to_sleep == -1)
    return (0);
  if (ac == 5)
    input->meals_cap = 0;
  else if (ac == 6) {
    input->meals_cap = ft_philo_atol(av[5]);
    if (input->meals_cap == -1)
      return (0);
  }
  return (1);
}

void forge_forks(t_fork *forks, int nbr) {
  int i;

  i = -1;
  while (++i < nbr) {
    pthread_create(forks->fork, NULL, NULL, NULL);
  }
}

void *lifecicle(t_philo philo) {
  to_eat();
  to_thinnk();
  to_sleep();
}

int main(int ac, char **av) {
  t_input input;
  t_philo philos;
  t_fork forks;

  if (ac == 5 || ac == 6) {
    if (!parser(&input, av, ac))
      print_error("Bad Syntax");
  }
  forge_forks(forks, input.philosophers);
  int i = -1;
  while (++i < input.philosophers) {
    pthread_create(input.philosophers, NULL, life_cicle(), philos);
  }
  return (0);
}
