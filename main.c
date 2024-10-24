#include "philo.h"


void	logmessage(t_list *table, char *msg)
{
	pthread_mutex_lock(&(table->info->m_write));
	printf("%ld %d %s", gettimems(), table->id, msg);
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
	if ((table->prev->status == 2
		&& table->prev->last_meal > table->last_meal)
		|| (table->next->status == 2
		&& table->next->last_meal > table->last_meal))
		i = 0;	
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
		pthread_mutex_lock(&(table->info->m_status));
		if (table->status == -1)
		{
			pthread_mutex_lock(&(table->info->m_stop));
			table->info->stop_simul = 1;
			pthread_mutex_unlock(&(table->info->m_stop));
			return (NULL);
		}
		if (table->status == 2 && permission_to_eat(table))
		{
			//lock perm_to_eat
			table->perm_to_eat = 1;
			//unlock perm_to_eat
		}
		pthread_mutex_unlock(&(table->info->m_status));
		table = table->next;
		usleep(1000); 
	}
	return (NULL);
}

suseconds_t	gettimems(void)
{
	struct timeval	tv;

	gettimeofday(&tv, NULL);
	return ((tv.tv_sec * 1000) + (tv.tv_usec / 1000));
}

void	alertsleep(suseconds_t timer, t_list *table)
{
	suseconds_t	start;

	start = gettimems();
	while (1)
	{
		//inside routine, no need to lock status
		
		if (table->status != 2
			&& (gettimems() - start >= table->info->time_to_die))
		{
			logmessage(table, "died\n");
			table->status = -1; 
			return ;
		}
		if (table->info->stop_simul)
		{
			table->status = 0;
			return ;;
		}
		if (gettimems() - start >= timer)
			break ;
		//while doing stuff, every 100ms check for uptades about if anyone died
		usleep(100);
	}
	return ;
}
/*
void	pickfork(t_list *table)
{
	//ask waiter if forks are available
	if (permission_to_eat(table))
	{
		printf("STARTED EATING\n");
		alertsleep(table->info->time_to_sleep, table);
	}
}



void	dropfork()
{

	
}*/

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
		pthread_mutex_lock(&(table->info->m_stop));
		if (table->info->stop_simul) //status == 0?
		{
			pthread_mutex_unlock(&(table->info->m_stop));
			break ;
		}
		pthread_mutex_unlock(&(table->info->m_stop));
		if (times_eaten == table->info->times_to_satisfy)
		{
			pthread_mutex_lock(&(table->info->m_status));
			table->status = 0;
			pthread_mutex_unlock(&(table->info->m_status));
		}
		//lock perm_to_eat
		if (table->status == 1 && table->perm_to_eat)
		{
			//eat;
			pthread_mutex_lock(&(table->f_mutex));
			logmessage(table, "has taken a left fork\n");
			pthread_mutex_lock(&(table->next->f_mutex));
			logmessage(table, "has taken a right fork\n");
			table->status = 2;
			logmessage(table, "is eating\n");
			alertsleep(table->info->time_to_eat, table);
			table->last_meal = gettimems();
			table->status = 3;
			table->perm_to_eat = 0;
			times_eaten++;
			pthread_mutex_unlock(&(table->f_mutex));
			pthread_mutex_unlock(&(table->next->f_mutex));
		}
		//unlock perm_to_eat
		pthread_mutex_unlock(&(table->p_mutex));
		if (table->status == 3)
		{
			logmessage(table, "is sleeping\n");
			alertsleep(table->info->time_to_sleep, table);
			pthread_mutex_lock(&(table->p_mutex));
			table->status = 1;
			logmessage(table, "is thinking\n");
			pthread_mutex_unlock(&(table->p_mutex));
		}
		if (table->status == 0) //simulation ender or safisfied
			break ;
	}
	return (NULL);
}

/*
	while (!table->philos->eatcheck || i < table->philos->eatcheck)
	{
		if (status == 1)
		{
			pthread_mutex_lock(&(table->forks->mutex));
			if (table->forks->status == 1)
			{
				pthread_mutex_lock(&(table->forks->next->mutex));
				if (table->forks->next->status == 1)
				{
					table->forks->status = 0;
					table->forks->next->status = 0;
					pthread_mutex_lock(&(table->phil->mutex));
					table->philos->status = 2;
					pthread_mutex_unlock(&(table->phil->mutex));
					usleep(table->eattime);
					table->forks->status = 1;
					table->forks->next->status = 1;
				
				}
				pthread_mutex_unlock(&(table->forks->next->mutex));
			}
			pthread_mutex_unlock(&(table->forks->mutex));
		}
		if (status == 2)
		{
			pthread_mutex_lock(&(table->phil->mutex));
			table->philos->status = 3;
			pthread_mutex_unlock(&(table->phil->mutex));
			table->philos->et = gettimeofday;
		}
		if (status == 3)
		{
			usleep(sleeptime);
			pthread_mutex_lock(&(table->phil->mutex));
				table->philos->status = 1;
			pthread_mutex_unlock(&(table->phil->mutex));
		}
		i++;
	}
	return (NULL);
}*/

void	*test_routine(void* arg)
{
	t_list	*table;
	int	i;

	table = (struct s_list*)arg;
	i = 0;
	while (i < 40)
	{
		printf("DENTRO DA ROTINA %d\n", table->id);
		i++;
	}
	return (NULL);
}

void	*test_waiter(void *arg)
{
	t_list	*table;
	int i;

	table = (struct s_list*)arg;
	i = 0;
	while (i < 3)
	{
		printf("DENTRO DO WAITER %d\n", table->id);
		i++;
	}
	return (NULL);
}

int	startroullete(t_list *table)
{
	int	i;
	t_list *tmp;

	i = 0;
	table->info->start_time = gettimems();
	tmp = table;
	while (i < table->info->number_of_philo)
	{
		printf("FOI THREAD: %d\n", tmp->id);
		tmp->last_meal = table->info->start_time;
		if (pthread_create(&(tmp->tid), NULL, &routine, tmp) != 0)
			perror("FAILED TO CREATE THREAD\n");
		tmp = tmp->next;
		i++;
	}
	printf("EXTRA THREAD: %d\n", tmp->id);
	usleep(table->info->number_of_philo + 1);
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
	pthread_mutex_destroy(&(tmp->info->waiter));
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
