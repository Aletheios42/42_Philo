/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   parser.c                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: alepinto <marvin@42.fr>                    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/06/15 16:56:42 by alepinto          #+#    #+#             */
/*   Updated: 2025/06/15 17:01:19 by alepinto         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../Inc/philo.h"

int	ft_atoi(const char *str)
{
	int				i;
	long			sign;
	long long		result;

	i = 0;
	result = 0;
	sign = 1;
	while ((str[i] >= 9 && str[i] <= 13) || str[i] == 32)
		i++;
	if (str[i] == '+' || str[i] == '-')
	{
		if (str[i] == '-')
			sign *= -1;
		i++;
	}
	while (str[i] && str[i] >= '0' && str[i] <= '9')
	{
		result = result * 10 + (str[i] - '0');
		if ((sign == 1 && result > INT_MAX)
			|| (sign == -1 && - result < INT_MIN))
			return (-1);
		i++;
	}
	return ((int)(result * sign));
}

static bool	ft_check_argument(char *str)
{
	int	i;

	i = 0;
	while (str[i] == ' ')
		i++;
	if (str[i] == '+')
		i++;
	if (str[i] < '0' || str[i] > '9')
		return (false);
	if (str[i] == '+')
		i++;
	while (str[i])
	{
		if (str[i] < '0' || str[i] > '9')
			return (false);
		i++;
	}
	return (true);
}

static void	ft_init_sim(t_sim *sim, char **av)
{
	sim->num_of_philos = ft_atoi(av[1]);
	sim->time_to_die = ft_atoi(av[2]);
	sim->time_to_eat = ft_atoi(av[3]);
	sim->time_to_sleep = ft_atoi(av[4]);
	if (av[5])
		sim->number_of_times_each_philosopher_must_eat = ft_atoi(av[5]);
	else
		sim->number_of_times_each_philosopher_must_eat = -1;
	sim->someone_died = 0;
	sim->full_philos = 0;
	pthread_mutex_init(&sim->write_lock, NULL);
	pthread_mutex_init(&sim->dead_lock, NULL);
	pthread_mutex_init(&sim->meal_lock, NULL);
	pthread_mutex_init(&sim->full_lock, NULL);
}

int	parse_data(char **av, t_sim *table)
{
	int	i;

	i = 1;
	while (av[i])
	{
		if (!ft_check_argument(av[i]))
			return (1);
		i++;
	}
	ft_init_sim(table, av);
	if (table->num_of_philos < 1 || table->num_of_philos > PHILO_MAX
		|| table->time_to_die <= 0 || table->time_to_eat <= 0
		|| table->time_to_sleep <= 0)
		return (1);
	table->start_time = get_current_time_ms();
	return (0);
}
