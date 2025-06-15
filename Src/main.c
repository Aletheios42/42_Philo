#include "../Inc/philo.h"


void	error_exit(const char *msg)
{
	printf(RED "%s" RESET, msg);
	exit(EXIT_FAILURE);
}

void	clean_up_all(char *str, t_sim *sim)
{
	int	i;

	if (str)
	{
		write(2, str, ft_strlen(str));
		write(2, "\n", 1);
	}
	pthread_mutex_destroy(&sim->write_lock);
	pthread_mutex_destroy(&sim->meal_lock);
	pthread_mutex_destroy(&sim->dead_lock);
	if (sim->forks)
	{
		i = 0;
		while (i < sim->num_of_philos)
			pthread_mutex_destroy(&sim->forks[i++]);
		free(sim->forks);
	}
	if (sim->philos)
	{
		i = 0;
		while (i < sim->num_of_philos)
			pthread_mutex_destroy(&sim->philos[i++].mutex);
		free(sim->philos);
	}
}


int	main(int ac, char **av)
{
	t_sim	sim;

	if (ac != 5 && ac != 6)
		error_exit("Wrong argument count\n");
	if (parse_data(av, &sim))
		error_exit("Error: parsing error\n");
	init_forks(&sim);
	init_philos(&sim);
	start_threads(&sim);
	clean_up_all(NULL, &sim);
	return (EXIT_SUCCESS);
}
