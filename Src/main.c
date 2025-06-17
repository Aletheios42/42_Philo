/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.c                                             :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: alepinto <marvin@42.fr>                    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/06/15 16:56:01 by alepinto          #+#    #+#             */
/*   Updated: 2025/06/15 16:58:18 by alepinto         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../Inc/philo.h"

void	error_exit(const char *msg)
{
	printf(RED "%s" RESET, msg);
	exit(EXIT_FAILURE);
}

void	clean_up_all(char *msg, t_sim *sim)
{
	int	i;

	if (msg)
	{
        pthread_mutex_lock(&sim->write_lock);
		write(2, msg, ft_strlen(msg));
		write(2, "\n", 1);
        pthread_mutex_unlock(&sim->write_lock);
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
