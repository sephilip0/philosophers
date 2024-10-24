#include "philo.h"

size_t	ft_strlen(const char *s)
{
	unsigned int	size;

	size = 0;
	while (s[size])
		size++;
	return (size);
}

int	ft_atoi(char *nbr)
{
	int		i;
	int	sig;
	long	res;

	i = 0;
	res = 0;
	sig = 1;
	if (nbr[i] == '+' || nbr[i] == '-')
	{
		if (nbr[i] == '-')
			sig = -sig;
		if (ft_strlen(nbr) == 1)
			return (-1);
		i++;
	}
	while (nbr[i] >= 48 && nbr[i] <= 57)
	{
		res = res * 10 + nbr[i] - 48;
		if ((res * sig) > INT_MAX || (res * sig) < INT_MIN)
			return (-1);
		i++;
	}
	if (nbr[i] && (nbr[i] < 48 || nbr[i] > 57))
		return (-1);
	return ((sig * res));
}

// ./philo totaln dietime eattime sleeptime eatcheck
int	correctinput(int argc, char *argv[])
{
	int	i;

	i = 1;
	if (argc < 5 || argc > 6)
	{
		printf("Wrong number of arguments\n");
		return (0);
	}
	while (i < argc)
	{
		if (ft_atoi(argv[i]) < 1)
		{
			printf("Wrong input\n");
			return (0);
		}
		i++;
	}
	return (1);
}
/*
t_list *addnode(t_list **first, t_list **last, int id)
{
	t_list *new;
	t_list *first;
	t_list *tmp;

	new = malloc(sizeof(t_list));
	if (!new)
		return (NULL);
	new->id = id;
	new->next = NULL;
	first = *head;
	tmp = *head;
}*/


void	freelist(t_list **list, int nbr)
{
	t_list	*tmp;
	int	i;

	i = 0;
	if (nbr == 0 || !list || !(*list))
		return ;
	while (i < nbr)
	{
		tmp = (*list);
		(*list) = (*list)->next;
		printf("DESTRUIDO MUTEX\n");
		pthread_mutex_destroy(&(tmp->p_mutex));
		pthread_mutex_destroy(&(tmp->f_mutex));
		free(tmp);
		i++;
	}
	return ;
}

t_list	*formlist(t_phil *phil)
{
	int	i;
	t_list	*new;
	t_list	*prev;
	t_list	*first;

	i = 1;
	prev = NULL;
	first = NULL;
	while (i <= phil->number_of_philo)
	{
		new = malloc(sizeof(t_list));
		if (!new)
			return (freelist(&new, i - 1), NULL);
		new->id = i;
		new->status = 1;
		new->perm_to_eat = 0;
		new->info = phil;
		new->next = first;
		new->prev = prev;
		printf("CRIADO MUTEX\n");
		printf("NEW: %p\n", new);
		printf("NEW ID: %d\n", new->id);
		printf("NEW PREV: %p\n", new->prev);
		printf("NEW NEXT: %p\n", new->next);
		pthread_mutex_init(&(new->p_mutex), NULL);
		pthread_mutex_init(&(new->f_mutex), NULL);
		if (!first)
			first = new;
		if (!prev)
			prev = new;
		else
		{
			prev->next = new;
			prev = new;
		}
		printf("ACTUAL PREV %p\n", prev);
		printf("ACTUAL FIRST %p\n", first);
		i++;
	}
	return (first);
}

void	spinbottle(t_list *head)
{
	int	i;

	i = 0;
	while (i < 20)
	{
		printf("%d\n", head->id);
		head = head->next;
		i++;
	}
}

t_list	*initphil(t_phil *phil, int argc, char *argv[])
{
	t_list	*table;

	phil->number_of_philo = ft_atoi(argv[1]);
	phil->time_to_die = ft_atoi(argv[2]);
	phil->time_to_eat = ft_atoi(argv[3]);
	phil->time_to_sleep = ft_atoi(argv[4]);
	pthread_mutex_init(&(phil->waiter), NULL);
	if (argc == 6)
		phil->times_to_satisfy = ft_atoi(argv[5]);
	else
		phil->times_to_satisfy = 0;
	table = formlist(phil);
	if (!table)
		return (NULL);
	//spinbottle(table);
	//freelist(&table, phil->number_of_philo);
	//exit (0);
	return (table);
}

