#include "philo.h"

void	logmessage(t_list *table, char *msg)
{
	pthread_mutex_lock(&(table->info->m_write));
	pthread_mutex_lock(&(table->info->m_stop));
	if (!(table->info->stop_simul))
		printf("%ld %d %s", simul_time(table), table->id, msg);
	pthread_mutex_unlock(&(table->info->m_stop));
	pthread_mutex_unlock(&(table->info->m_write));
}

void	protprint(t_list *table, char *msg)
{
	pthread_mutex_lock(&(table->info->m_write));
	printf("%s", msg);
	pthread_mutex_unlock(&(table->info->m_write));
}

void permission_to_eat(t_list *table)
{
	pthread_mutex_lock(&(table->info->m_pte));
	pthread_mutex_lock(&(table->info->m_meal));
	if ((table->prev->perm_to_eat == 1) || (table->next->perm_to_eat == 1))
		table->perm_to_eat = 0;
	else if (table->prev->last_meal < table->last_meal
		|| table->next->last_meal < table->last_meal)
		table->perm_to_eat = 0;
	else
		table->perm_to_eat = 1;
	pthread_mutex_unlock(&(table->info->m_meal));
	pthread_mutex_unlock(&(table->info->m_pte));
	return ;
}
void	*lonelyphil(void* arg)
{
	t_list	*table;

	table = (struct s_list*)arg;
	logmessage(table, "has taken a fork\n");
	return (NULL);
}

void	*solowaiter(void *arg)
{
	t_list	*table;

	table = (struct s_list*)arg;
	while (1)
	{
		if (simul_time(table) >= table->info->time_to_die)
		{
			logmessage(table, "died\n");
			pthread_mutex_lock(&(table->info->m_stop));
			table->info->stop_simul = 1;
			pthread_mutex_unlock(&(table->info->m_stop));
			return (NULL);
		}
		usleep(100);
	}
	return (NULL);
}

int		alleaten(int value, int i)
{
	if (value == -1)
		i++;
	else
		return (0);
	return (i);
}

void	*waiter(void *arg)
{
	int i;
	t_list	*table;

	table = (struct s_list*)arg;
	i = 0;
	while (1)
	{
		if (i == table->info->number_of_philo)
			return (protprint(table, "All philosophers satified\n"), NULL);
		pthread_mutex_lock(&(table->info->m_hungry));
		if (table->hungry == 1)
			permission_to_eat(table);
		pthread_mutex_lock(&(table->info->m_pte));
		pthread_mutex_lock(&(table->info->m_meal));
		i = alleaten(table->hungry, i);
		if (!(table->hungry == 0 && table->perm_to_eat == 1) && (table->hungry != -1)
		&& (simul_time(table) - table->last_meal >= table->info->time_to_die))
		{
			pthread_mutex_unlock(&(table->info->m_meal));
			pthread_mutex_unlock(&(table->info->m_pte));
			pthread_mutex_unlock(&(table->info->m_hungry));
			logmessage(table, "died\n");
			pthread_mutex_lock(&(table->info->m_stop));
			table->info->stop_simul = 1;
			pthread_mutex_unlock(&(table->info->m_stop));
			return (NULL);
		}
		pthread_mutex_unlock(&(table->info->m_meal));
		pthread_mutex_unlock(&(table->info->m_pte));
		pthread_mutex_unlock(&(table->info->m_hungry));
		table = table->next;
	}
	return (NULL);
}

suseconds_t	gettimems(void)
{
	struct timeval	tv;

	gettimeofday(&tv, NULL);
	return ((tv.tv_sec * 1000) + (tv.tv_usec / 1000));
}

suseconds_t	simul_time(t_list *table)
{
	return (gettimems() - table->info->start_time);
}

void	alertsleep(suseconds_t timer, t_list *table)
{
	suseconds_t	start;

	start = simul_time(table);
	while (1)
	{
		pthread_mutex_lock(&(table->info->m_stop));
		if (table->info->stop_simul)
		{
			pthread_mutex_lock(&(table->info->m_hungry));
			table->hungry = -1;
			pthread_mutex_unlock(&(table->info->m_hungry));
			pthread_mutex_unlock(&(table->info->m_stop));
			return ;
		}
		pthread_mutex_unlock(&(table->info->m_stop));
		if (simul_time(table) - start >= timer)
			return ;
		usleep(100);
	}
}

int	end_of_simulation(t_list *table)
{
	int	value;

	value = 0;

	pthread_mutex_lock(&(table->info->m_stop));
	if (table->info->stop_simul)
		value = 1;
	pthread_mutex_unlock(&(table->info->m_stop));
	return (value);
}

void takeforks(t_list *table)
{
	if (table->id % 2 == 0)
	{
		pthread_mutex_lock(&(table->f_mutex));
		logmessage(table, "has taken a fork\n");
		pthread_mutex_lock(&(table->next->f_mutex));
		logmessage(table, "has taken a fork\n");
	}
	else
	{	
		pthread_mutex_lock(&(table->next->f_mutex));
		logmessage(table, "has taken a fork\n");
		pthread_mutex_lock(&(table->f_mutex));
		logmessage(table, "has taken a fork\n");
	}
}


void putforks(t_list *table)
{
	if (table->id % 2 == 0)
	{
		pthread_mutex_unlock(&(table->next->f_mutex));
		pthread_mutex_unlock(&(table->f_mutex));
	}
	else
	{	
		pthread_mutex_unlock(&(table->f_mutex));
		pthread_mutex_unlock(&(table->next->f_mutex));
	}
}
void	thinking(t_list *table)
{
	logmessage(table, "is thinking\n");
	pthread_mutex_lock(&(table->info->m_hungry));
	table->hungry = 1;
	pthread_mutex_unlock(&(table->info->m_hungry));
}


int	readyeat(t_list *table)
{
	pthread_mutex_lock(&(table->info->m_pte));
	if (table->perm_to_eat)
	{
		pthread_mutex_unlock(&(table->info->m_pte));
		takeforks(table);
		pthread_mutex_lock(&(table->info->m_hungry));
		table->hungry = 0;
		pthread_mutex_unlock(&(table->info->m_hungry));
		logmessage(table, "is eating\n");
		pthread_mutex_lock(&(table->info->m_meal));
		table->last_meal = simul_time(table);
		pthread_mutex_unlock(&(table->info->m_meal));
		alertsleep(table->info->time_to_eat, table);
		putforks(table);
		pthread_mutex_lock(&(table->info->m_pte));
		table->perm_to_eat = 0;
		pthread_mutex_unlock(&(table->info->m_pte));
		logmessage(table, "is sleeping\n");
		alertsleep(table->info->time_to_sleep, table);
		return (1);
	}
	pthread_mutex_unlock(&(table->info->m_pte));
	return (0);
}

//hungry -1 = DEAD
//hungry 0 = NOTHING
//hungry 1 = THINKING and WANT TO EAT
void	*routine(void* arg)
{
	t_list	*table;
	int	times_eaten;

	times_eaten = 0;
	table = (struct s_list*)arg;
	while (1)
	{
		if (end_of_simulation(table))
			break ;
		if (readyeat(table))
		{
			times_eaten++;
			if (table->info->times_to_satisfy != 0
				&& times_eaten == table->info->times_to_satisfy)
			{
				pthread_mutex_lock(&(table->info->m_hungry));
				table->hungry = -1;
				pthread_mutex_unlock(&(table->info->m_hungry));
				break ;
			}
			thinking(table);
		}
		usleep(100);
	}
	return (NULL);
}

void	endthreads(t_list *table)
{
	int i;
	t_list *tmp;

	i = 0;
	tmp = table;
	while (i < table->info->number_of_philo)
	{
		if (pthread_join(tmp->tid, NULL) != 0)
			perror("FAILED TO JOIN THREAD\n");
		tmp = tmp->next;
		i++;
	}
	if (pthread_join(table->info->check_death, NULL) != 0)
		perror("FAILED TO JOIN THREAD CHECK_DEATH\n");
	pthread_mutex_destroy(&(tmp->info->m_pte));
	pthread_mutex_destroy(&(tmp->info->m_hungry));
	pthread_mutex_destroy(&(tmp->info->m_write));
	pthread_mutex_destroy(&(tmp->info->m_meal));
	pthread_mutex_destroy(&(tmp->info->m_stop));
	return ;
}

void startthreads(t_list *table)
{
	int	i;
	t_list *tmp;

	i = 0;
	tmp = table;
	while (i < table->info->number_of_philo)
	{
		if (pthread_create(&(tmp->tid), NULL, &routine, tmp) != 0)
			perror("FAILED TO CREATE THREAD\n");
		tmp = tmp->next;
		i++;
	}
	usleep(table->info->number_of_philo);
	table->info->start_time = gettimems();
	if (table->info->number_of_philo == 1)
	{
		if (pthread_create(&(table->info->check_death), NULL, &solowaiter, table) != 0)
			perror("FAILED TO CREATE THREAD WITH WAITER\n");
	}
	else
	{
		if (pthread_create(&(table->info->check_death), NULL, &waiter, table) != 0)
			perror("FAILED TO CREATE THREAD WITH WAITER\n");
	}
}

int	main(int argc, char *argv[])
{
	t_phil	phil;
	t_list	*table;

	if (!correctinput(argc, argv))
		return (1);
	table = initphil(&phil, argc, argv);
	if (!table)
		return (1);
	table->info->start_time = 0;	
	startthreads(table);
	endthreads(table);
	freelist(&table, phil.number_of_philo);
	return (0);
}
