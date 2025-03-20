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
      printf("%ld %d died\n", get_current_time() - philos[i].start_time,
             philos[i].id);
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
    // Verificar si algún filósofo murió de hambre
    if (check_starvation(philos, input, end))
      return;
    // Solo comprueba si todos han comido lo suficiente si hay un límite de
    // comidas
    if (input->meals_cap > 0) {
      i = 0;
      full_philos = 0;

      // Recorrer TODOS los filósofos
      while (i < input->philosophers) {
        if (philos[i].meals_counter >= input->meals_cap)
          full_philos++;
        i++;
      }

      // Si todos han comido lo suficiente, terminar simulación
      if (full_philos == input->philosophers) {
        pthread_mutex_lock(&(end->end_mutex));
        end->simulation_end = true;
        pthread_mutex_unlock(&(end->end_mutex));
        return;
      }
    }
  }
}
