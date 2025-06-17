/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   routines.c                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: alepinto <marvin@42.fr>                    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/06/15 16:56:54 by alepinto          #+#    #+#             */
/*   Updated: 2025/06/15 17:06:20 by alepinto         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../Inc/philo.h"

void	start_threads(t_sim *sim)
{
	pthread_t	monitor_thread;

	create_threads(sim, &monitor_thread);
	join_threads(sim, &monitor_thread);
}

void	philo_eat(t_philo *philo)
{
	taken_forks(philo);
	pthread_mutex_lock(&philo->sim->meal_lock);
	philo->eating = 1;
	philo->last_meal = get_current_time_ms();
	philo->meals_eaten++;
	pthread_mutex_unlock(&philo->sim->meal_lock);
	print_status(philo, "is eating 🍽️", GREEN);
	ft_usleep(philo->sim->time_to_eat, philo);
	pthread_mutex_lock(&philo->sim->meal_lock);
	check_and_print_full(philo);
	philo->eating = 0;
	pthread_mutex_unlock(&philo->sim->meal_lock);
	release_forks(philo);
}

void	philo_sleep(t_philo *philo)
{
	if (philo_has_died(philo))
		return ;
	print_status(philo, "is sleeping 😴", BLUE);
	ft_usleep(philo->sim->time_to_sleep, philo);
}

void	philo_think(t_philo *philo)
{
	if (philo_has_died(philo))
		return ;
	print_status(philo, "is thinking 🤔", MAGENTA);
}

void	*philo_routine(void *arg)
{
	t_philo	*philo;

	philo = (t_philo *)arg;
	if (philo->sim->num_of_philos == 1)
	{
		pthread_mutex_lock(philo->l_fork);
		print_status(philo, "has taken a fork", CYAN);
		ft_usleep(philo->sim->time_to_die, philo);
		pthread_mutex_unlock(philo->l_fork);
		pthread_mutex_lock(&philo->sim->dead_lock);
		philo->sim->someone_died = 1;
		pthread_mutex_unlock(&philo->sim->dead_lock);
		return (NULL);
	}
	if (philo->id % 2 == 0)
		ft_usleep(70, philo);
	while (!philo_has_died(philo))
	{
		philo_eat(philo);
		philo_sleep(philo);
		philo_think(philo);
	}
	return (NULL);
}
