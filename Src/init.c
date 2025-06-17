/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   init.c                                             :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: alepinto <marvin@42.fr>                    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/06/15 16:55:39 by alepinto          #+#    #+#             */
/*   Updated: 2025/06/15 17:06:08 by alepinto         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../Inc/philo.h"

void	assign_forks(t_philo *philo, int index, t_sim *sim)
{
	philo->l_fork = &sim->forks[index];
	philo->r_fork = &sim->forks[(index + 1) % sim->num_of_philos];
}

void	init_forks(t_sim *sim)
{
	int	i;

	sim->forks = malloc(sizeof(pthread_mutex_t) * sim->num_of_philos);
	if (!sim->forks)
		error_exit("Error in the forks\n");
	i = 0;
	while (i < sim->num_of_philos)
	{
		pthread_mutex_init(&sim->forks[i], NULL);
		i++;
	}
	pthread_mutex_init(&sim->write_lock, NULL);
	pthread_mutex_init(&sim->dead_lock, NULL);
}

void	init_philos(t_sim *sim)
{
	int			i;

	sim->philos = malloc(sizeof(t_philo) * sim->num_of_philos);
	if (!sim->philos)
		error_exit("Error: Failed to allocate memory for philosophers\n");
	sim->start_time = get_current_time_ms();
	i = -1;
	while (++i < sim->num_of_philos)
	{
		sim->philos[i].id = i + 1;
		sim->philos[i].eating = 0;
		sim->philos[i].meals_eaten = 0;
		sim->philos[i].last_meal = sim->start_time;
		sim->philos[i].sim = sim;
		pthread_mutex_init(&sim->philos[i].mutex, NULL);
		assign_forks(&sim->philos[i], i, sim);
	}
}
