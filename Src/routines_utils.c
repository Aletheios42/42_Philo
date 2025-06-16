/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   routines_utils.c                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: alepinto <marvin@42.fr>                    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/06/15 17:02:54 by alepinto          #+#    #+#             */
/*   Updated: 2025/06/15 17:09:01 by alepinto         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../Inc/philo.h"

void	release_forks(t_philo *philo)
{
	pthread_mutex_unlock(philo->l_fork);
	pthread_mutex_unlock(philo->r_fork);
}

void	taken_forks(t_philo *philo)
{
	if (philo_has_died(philo))
		return ;
	if (philo->id % 2 == 0)
	{
		pthread_mutex_lock(philo->r_fork);
		print_status(philo, "has taken a fork", CYAN);
		pthread_mutex_lock(philo->l_fork);
		print_status(philo, "has taken a fork", CYAN);
	}
	else
	{
		usleep(500);
		pthread_mutex_lock(philo->l_fork);
		print_status(philo, "has taken a fork", CYAN);
		pthread_mutex_lock(philo->r_fork);
		print_status(philo, "has taken a fork", CYAN);
	}
}

void	check_and_print_full(t_philo *philo)
{
	long	timestamp;
	int		required_meals;

	required_meals = philo->sim->number_of_times_each_philosopher_must_eat;
	if (required_meals != -1
		&& philo->meals_eaten == required_meals)
	{
		pthread_mutex_lock(&philo->sim->full_lock);
		philo->sim->full_philos++;
		if (philo->sim->full_philos == philo->sim->num_of_philos)
		{
			timestamp = get_current_time_ms() - philo->sim->start_time;
			pthread_mutex_lock(&philo->sim->write_lock);
			printf(YELLOW "[%ld] All philosophers have eaten %d times "
				"🐷\n" RESET,
				timestamp,
				required_meals);
			pthread_mutex_unlock(&philo->sim->write_lock);
		}
		pthread_mutex_unlock(&philo->sim->full_lock);
	}
}

void	create_threads(t_sim *sim, pthread_t *monitor_thread)
{
	int	i;

	sim->start_time = get_current_time_ms();
	i = 0;
	while (i < sim->num_of_philos)
	{
		sim->philos[i].last_meal = sim->start_time;
		i++;
	}
	if (pthread_create(monitor_thread, NULL, monitor_func, sim) != 0)
		clean_up_all("Failed to create monitor thread\n", sim);
	i = 0;
	while (i < sim->num_of_philos)
	{
		if (pthread_create(&sim->philos[i].thread, NULL, philo_routine,
				&sim->philos[i]) != 0)
			clean_up_all("Failed to create philosopher thread\n", sim);
		i++;
	}
}

void	join_threads(t_sim *sim, pthread_t *monitor_thread)
{
	int	i;

	if (pthread_join(*monitor_thread, NULL) != 0)
		clean_up_all("Thread join error\n", sim);
	i = 0;
	while (i < sim->num_of_philos)
	{
		if (pthread_join(sim->philos[i].thread, NULL) != 0)
			clean_up_all("Thread join error\n", sim);
		i++;
	}
}
