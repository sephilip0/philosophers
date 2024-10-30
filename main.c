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
	return (i);
}

void	*solowaiter(void *arg)
{
	t_list	*table;

	table = (struct s_list*)arg;
	logmessage(table, "has taken a fork\n");
	while (1)
	{
		if (simul_time(table) >= table->info->time_to_die)
		{
			logmessage(table, "died\n");
			pthread_mutex_lock(&(table->info->m_stop));
			table->info->stop_simul = 1;
			pthread_mutex_unlock(&(table->info->m_stop));
		}
		usleep(100);
	}

	return (NULL)
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
			return (logmessage(table, "ALL SATISFIED (tirar o numero do filosofo)\n"), NULL);
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
		//inside routine, no need to lock status
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
		//while doing stuff, every 100ms check for uptades about if anyone died
		usleep(100);
	}
}

int	end_of_simulation(t_list *table)
{
	int	value;

	value = 0;

	//logmessage(table, "end_of_simulation\n");
	pthread_mutex_lock(&(table->info->m_stop));
	if (table->info->stop_simul) //status == 0?
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
		//logmessage(table, "has put a fork\n");
		pthread_mutex_unlock(&(table->f_mutex));
		//logmessage(table, "has put a left fork\n");
	}
	else
	{	
		pthread_mutex_unlock(&(table->f_mutex));
		//logmessage(table, "has put a left fork\n");
		pthread_mutex_unlock(&(table->next->f_mutex));
		//logmessage(table, "has put a right fork\n");
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
		//logmessage(table, "routine\n");
		if (end_of_simulation(table))
			break ;
		if (readyeat(table))
		{
			times_eaten++;
			if (table->info->times_to_satisfy != 0
				&& times_eaten == table->info->times_to_satisfy)
			{
				pthread_mutex_lock(&(table->info->m_hungry));
				//logmessage(table, "SATISFIED\n");
				table->hungry = -1;
				pthread_mutex_unlock(&(table->info->m_hungry));
				break ;
			} lvscan | grep -qc ACTIVE && echo "yes" || echo "no"
			thinking(table);
		}
		usleep(100);
	}
	return (NULL);
}

int	startroullete(t_list *table)
{
	int	i;
	t_list *tmp;

	i = 0;
	table->info->start_time = 0;
	tmp = table;
	while (i < table->info->number_of_philo)
	{
		//printf("ID %d lastmeal: %ld\n", tmp->id, tmp->last_meal);
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
		if (pthread_join(tmp->tid, NULL) != 0)
			perror("FAILED TO JOIN THREAD\n");
		tmp = tmp->next;
		i++;
	}
	printf("FECHOU EXTRA THREAD: %d\n", tmp->id);
	if (pthread_join(table->info->check_death, NULL) != 0)
		perror("FAILED TO JOIN THREAD CHECK_DEATH\n");
	pthread_mutex_destroy(&(tmp->info->m_pte));
	pthread_mutex_destroy(&(tmp->info->m_hungry));
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
