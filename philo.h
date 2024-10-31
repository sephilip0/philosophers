#ifndef PHILO_H
# define PHILO_H

# include <stdio.h>
# include <unistd.h>
# include <stdlib.h>
# include <sys/time.h>
# include <pthread.h>
# include <limits.h>

typedef struct s_phil
{
	suseconds_t	start_time;
	int	number_of_philo;
	int	time_to_die;
	int	time_to_eat;
	int	time_to_sleep;
	int	times_to_satisfy;
	int	stop_simul;
	pthread_t	check_death;
	//permission to eat
	pthread_mutex_t	m_pte;
	//writing on screen //AND PERM_TO_EAT? //protect from waiter
	pthread_mutex_t	m_hungry;
	//writing on screen //protect from other philo on writing in the screen atst
	pthread_mutex_t	m_write;
	//check last_meal //protect between waiter and philo
	pthread_mutex_t	m_meal;
	//check if someone died //protect between waiter and philo
	pthread_mutex_t	m_stop;
}	t_phil;

typedef struct s_list
{
	int	id;
	int	hungry;
	int	perm_to_eat;
	suseconds_t	last_meal;
	pthread_t	tid;
	pthread_mutex_t f_mutex;
	struct s_list	*next;
	struct s_list	*prev;
	struct s_phil	*info;
}	t_list;

suseconds_t	gettimems(void);
suseconds_t	simul_time(t_list *table);
void	logmessage(t_list *table, char *msg);
void 	permission_to_eat(t_list *table);
void	*waiter(void *arg);
void	alertsleep(suseconds_t timer, t_list *table);
void	*routine(void* arg);
int	startroullete(t_list *table);

size_t	ft_strlen(const char *s);
int	ft_atoi(char *nbr);
int	correctinput(int argc, char *argv[]);
void	freelist(t_list **list, int nbr);
t_list	*formlist(t_phil *phil, int i);
t_list	*initphil(t_phil *phil, int argc, char *argv[]);

#endif // PHILO_H
