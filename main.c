#include "philo.h"

void	logmessage(t_list *table, char *msg)
{
	pthread_mutex_lock(&(table->info->m_write));
	printf("%ld %d %s", simul_time(table), table->id, msg);
	pthread_mutex_unlock(&(table->info->m_write));
}

void	logvalue(t_list *table, char *msg, long int value)
{
	pthread_mutex_lock(&(table->info->m_write));
	printf("%ld %d %s %ld\n", simul_time(table), table->id, msg, value);
	pthread_mutex_unlock(&(table->info->m_write));
}

int	permission_to_eat(t_list *table)
{
	int	i;
	i = 1;
	//m_status already locked in waiter
	pthread_mutex_lock(&(table->f_mutex));
	pthread_mutex_lock(&(table->next->f_mutex));
	pthread_mutex_lock(&(table->info->m_meal));
	//printf("0\n");
	//printf("01\n");
	if ((table->prev->status == 1
		&& table->prev->last_meal > table->last_meal)
		|| (table->next->status == 1
		&& table->next->last_meal > table->last_meal))
		i = 0;	
	//printf("02\n");
	logvalue(table, "prev last meal", table->prev->last_meal);
	logvalue(table, "act last meal", table->last_meal);
	logvalue(table, "next last meal", table->next->last_meal);
	logvalue(table, "i", i);
	pthread_mutex_lock(&(table->info->m_meal));
	pthread_mutex_unlock(&(table->next->f_mutex));
	pthread_mutex_unlock(&(table->f_mutex));
	return (i);
}

void	*waiter(void *arg)
{
	t_list	*table;

	table = (struct s_list*)arg;
	while (1)
	{
		logvalue(table, "waiter", 0);
		pthread_mutex_lock(&(table->info->m_status));
		if (table->status == 1 && permission_to_eat(table))
			table->perm_to_eat = 1;
		pthread_mutex_lock(&(table->info->m_meal));
		if (table->status != 2 
			&& (simul_time(table) - table->last_meal >= table->info->time_to_die))
		{
			logmessage(table, "died\n");
			pthread_mutex_lock(&(table->info->m_stop));
			table->info->stop_simul = 1;
			pthread_mutex_unlock(&(table->info->m_stop));
			return (NULL);
		}
		pthread_mutex_unlock(&(table->info->m_meal));
		pthread_mutex_unlock(&(table->info->m_status));
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
		//inside routine, no need to lock status
		pthread_mutex_lock(&(table->info->m_stop));
		if (table->info->stop_simul)
		{
			table->status = 0;
			return ;
		}
		pthread_mutex_unlock(&(table->info->m_stop));
		if (simul_time(table) - start >= timer)
			break ;
		//while doing stuff, every 100ms check for uptades about if anyone died
		usleep(100);
	}
	return ;
}

int	end_of_simulation(t_list *table)
{
	int	value;

	value = 0;

	logmessage(table, "end_of_simulation\n");
	pthread_mutex_lock(&(table->info->m_stop));
	if (table->info->stop_simul) //status == 0?
		value = 1;
	pthread_mutex_unlock(&(table->info->m_stop));
	return (value);
}

void	sleepthink(t_list *table)
{
	if (end_of_simulation(table))
		return ;
	pthread_mutex_lock(&(table->info->m_status));
	if (table->status == 3)
	{
		logmessage(table, "is sleeping\n");
		alertsleep(table->info->time_to_sleep, table);
		table->status = 1;
		logmessage(table, "is thinking\n");
	}
	pthread_mutex_unlock(&(table->info->m_status));
	return ;
}

int	readyeat(t_list *table)
{
	int	value;

	value = 0;
	if (end_of_simulation(table))
		return (value);
	pthread_mutex_lock(&(table->info->m_status));
	logvalue(table, "readyyeat", table->status);
	if (table->status == 1 && table->perm_to_eat)
	{
		//eat;
		pthread_mutex_lock(&(table->f_mutex));
		logmessage(table, "has taken a left fork\n");
		pthread_mutex_lock(&(table->next->f_mutex));
		logmessage(table, "has taken a right fork\n");
		table->status = 2;
	}
	pthread_mutex_unlock(&(table->info->m_status));
	pthread_mutex_lock(&(table->info->m_status));
	if (table->status == 2)
	{
		logmessage(table, "is eating\n");
		alertsleep(table->info->time_to_eat, table);
		pthread_mutex_lock(&(table->info->m_meal));
		printf("BROOOOWTF\n\n");
		table->last_meal = simul_time(table);
		pthread_mutex_unlock(&(table->info->m_meal));
		pthread_mutex_unlock(&(table->next->f_mutex));
		pthread_mutex_unlock(&(table->f_mutex));
		table->status = 3;
		table->perm_to_eat = 0;
		value++;
	}
	pthread_mutex_unlock(&(table->info->m_status));
	return (value);
}

//status 0 = DEAD
//status 1 = THINKING
//status 2 = EATING
//status 3 = SLEEPING
void	*routine(void* arg)
{
	t_list	*table;
	int	times_eaten;

	times_eaten = 0;
	table = (struct s_list*)arg;
	while (1)
	{
		//logmessage(table, "routine\n");
		if (end_of_simulation(table))
			break ;
		if (table->info->times_to_satisfy != 0
			&& times_eaten == table->info->times_to_satisfy)
		{
			pthread_mutex_lock(&(table->info->m_status));
			table->status = 0;
			pthread_mutex_unlock(&(table->info->m_status));
		}
		times_eaten += readyeat(table);
		sleepthink(table);
		pthread_mutex_lock(&(table->info->m_status));
		if (table->status == 0) //simulation ender or safisfied
		{
			pthread_mutex_unlock(&(table->info->m_status));
			pthread_mutex_lock(&(table->info->m_write));
			printf("CU\n");
			pthread_mutex_unlock(&(table->info->m_write));
			break ;
		}
		pthread_mutex_unlock(&(table->info->m_status));
		usleep(100);
	}
	pthread_mutex_lock(&(table->info->m_write));
	printf("ANOS\n");
	pthread_mutex_unlock(&(table->info->m_write));
	return (NULL);
}

int	startroullete(t_list *table)
{
	int	i;
	t_list *tmp;

	i = 0;
	table->info->start_time = 0;
	tmp = table;
	printf("table->info->start_time: %ld\n", table->info->start_time);
	while (i < table->info->number_of_philo)
	{
		printf("ID %d lastmeal: %ld\n", tmp->id, tmp->last_meal);
		if (pthread_create(&(tmp->tid), NULL, &routine, tmp) != 0)
			perror("FAILED TO CREATE THREAD\n");
		tmp = tmp->next;
		i++;
	}
	usleep(table->info->number_of_philo);
	table->info->start_time = gettimems();
	if (pthread_create(&(table->info->check_death), NULL, &waiter, table) != 0)
		perror("FAILED TO CREATE THREAD WITH WAITER\n");
	i = 0;
	while (i < table->info->number_of_philo)
	{
		printf("FECHOU THREAD: %d\n", tmp->id);
		if (pthread_join(tmp->tid, NULL) != 0)
			perror("FAILED TO JOIN THREAD\n");
		tmp = tmp->next;
		i++;
	}
	printf("FECHOU EXTRA THREAD: %d\n", tmp->id);
	if (pthread_join(table->info->check_death, NULL) != 0)
		perror("FAILED TO JOIN THREAD CHECK_DEATH\n");
	pthread_mutex_destroy(&(tmp->info->m_status));
	pthread_mutex_destroy(&(tmp->info->m_write));
	pthread_mutex_destroy(&(tmp->info->m_meal));
	pthread_mutex_destroy(&(tmp->info->m_stop));
	return (0);
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
	startroullete(table);
/*	while (argc > 0)
	{
		gettimeofday(&et, NULL);
		//printf("CURRENT TIME: %ld\n", (et.tv_sec - st.tv_sec) * 1000);
		printf("CURRENT TIME: %ld\n", (et.tv_sec - st.tv_sec) * 1000 + (et.tv_usec - st.tv_usec) / 1000);
		argc--;
	}
	printf("totaln = %d\n", phil.totaln);
	printf("dietime = %d\n", phil.dietime);
	printf("eattime = %d\n", phil.eattime);
	printf("sleeptime = %d\n", phil.sleeptime);
	printf("eatcheck = %d\n", phil.eatcheck);*/
	freelist(&table, phil.number_of_philo);
	return (0);
}
