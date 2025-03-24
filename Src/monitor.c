#include "../Inc/philo.h"

int check_starvation(t_philo *philos, t_input *input, t_end *end) {

  int i;
  i = -1;
  while (++i < input->philosophers) {
    // Check if philosopher has died
    if (get_current_time() - philos[i].last_meal_time >= input->time_to_die) {
      // Set simulation end flag
      pthread_mutex_lock(&(end->end_mutex));
      end->simulation_end = true;
      pthread_mutex_unlock(&(end->end_mutex));
      // Print death status
      pthread_mutex_lock(&philos[i].print_mutex);
      printf("%7ld %4d \033[1;31mdied\033[0m\n",
             get_current_time() - philos[i].start_time, philos[i].id);
      pthread_mutex_unlock(&philos[i].print_mutex);
      return 1;
    }
  }
  return 0;
}

void monitor_philos(t_philo *philos, t_input *input, t_end *end) {
  int i;
  int full_philos;

  while (1) {
    if (check_starvation(philos, input, end))
      return;

    if (input->meals_cap > 0) {
      full_philos = 0;
      for (i = 0; i < input->philosophers; i++) {
        if (philos[i].meals_counter >= input->meals_cap)
          full_philos++;
      }

      if (full_philos == input->philosophers) {
        pthread_mutex_lock(&(end->end_mutex));
        end->simulation_end = true;
        pthread_mutex_unlock(&(end->end_mutex));
        return;
      }
    }
    usleep(1000);
  }
}
