// philo.h
#ifndef PHILO_H
#define PHILO_H

#include <limits.h>
#include <pthread.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include <unistd.h>

typedef struct s_input {
  long philosophers;
  long time_to_die;
  long time_to_eat;
  long time_to_sleep;
  long meals_cap;
} t_input;

typedef struct s_fork {
  pthread_mutex_t mutex;
  int id;
  struct s_fork *next;
} t_fork;

typedef struct s_end {
  pthread_mutex_t end_mutex;
  bool simulation_end;
} t_end;

typedef struct s_philo {
  int id;
  pthread_t thread;
  long meals_counter;
  long last_meal_time;
  t_fork *left_fork;
  t_fork *right_fork;
  pthread_mutex_t print_mutex;
  bool *simulation_end;
  pthread_mutex_t *end_mutex;
  t_input *params;
} t_philo;

// Function prototypes
bool parser(t_input *input, char **av, int ac);
void print_error(const char *error);
long ft_philo_atol(char *nbr);
long get_current_time(void);
void print_status(t_philo *philo, const char *status);
void *lifecycle(void *arg);
void create_forks(t_fork **forks, int nbr);
void create_philosophers(t_philo **philos, t_fork *forks, t_input *input,
                         pthread_mutex_t *print, t_end *end);
void free_resources(t_philo *philos, t_fork *forks, pthread_mutex_t *print,
                    int n);
void monitor_philos(t_philo *philos, t_input *input, t_end *end);

int check_starvation(t_philo *philos, t_input *input, t_end *end);
void msleep(long ms);

#endif
