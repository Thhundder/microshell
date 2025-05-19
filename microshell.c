/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   microshell.c                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: emorreal <emorreal@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2021/12/15 12:15:17 by shackbei          #+#    #+#             */
/*   Updated: 2025/05/19 17:06:31 by emorreal         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <unistd.h>

void	ft_putstr_fd2(char *str, char *arg)
{
	while (*str)
		write(2, str++, 1);
	if (arg)
		while (*arg)
			write(2, arg++, 1);
	write(2, "\n", 1);
}

void	ft_execute(char *argv[], int argc, int tmp_fd, char *env[])
{
	argv[argc] = NULL;
	dup2(tmp_fd, STDIN_FILENO);
	close(tmp_fd);
	execve(argv[0], argv, env);
	ft_putstr_fd2("error: cannot execute ", argv[0]);
	exit(1);
}

void	handle_cd(char *argv[], int argc)
{
	if (argc != 2)
		ft_putstr_fd2("error: cd: bad arguments", NULL);
	else if (chdir(argv[1]) != 0)
		ft_putstr_fd2("error: cd: cannot change directory to ", argv[1]);
}

void	handle_simple(char *argv[], int argc, int *tmp_fd, char *env[])
{
	pid_t	pid;

	pid = fork();
	if (pid < 0)
	{
		ft_putstr_fd2("error: fatal", NULL);
		exit(1);
	}
	if (pid == 0)
		ft_execute(argv, argc, *tmp_fd, env);
	close(*tmp_fd);
	while (waitpid(-1, NULL, WUNTRACED) != -1)
		;
	*tmp_fd = dup(STDIN_FILENO);
}

void	handle_pipe(char *argv[], int argc, int *tmp_fd, char *env[])
{
	int		fd[2];
	pid_t	pid;

	if (pipe(fd) < 0)
	{
		ft_putstr_fd2("error: fatal", NULL);
		exit(1);
	}
	pid = fork();
	if (pid < 0)
	{
		ft_putstr_fd2("error: fatal", NULL);
		exit(1);
	}
	if (pid == 0)
	{
		dup2(fd[1], STDOUT_FILENO);
		close(fd[0]);
		close(fd[1]);
		ft_execute(argv, argc, *tmp_fd, env);
	}
	close(fd[1]);
	close(*tmp_fd);
	*tmp_fd = fd[0];
}

int	main(int argc, char *argv[], char *env[])
{
	int		i;
	int		tmp_fd;

	(void)argc;
	tmp_fd = dup(STDIN_FILENO);
	i = 0;
	while (argv[i] && argv[i + 1])
	{
		argv = &argv[i + 1];
		i = 0;
		while (argv[i] && strcmp(argv[i], ";") && strcmp(argv[i], "|"))
			i++;
		if (strcmp(argv[0], "cd") == 0)
			handle_cd(argv, i);
		else if (i > 0 && (argv[i] == NULL || strcmp(argv[i], ";") == 0))
			handle_simple(argv, i, &tmp_fd, env);
		else if (i > 0 && strcmp(argv[i], "|") == 0)
			handle_pipe(argv, i, &tmp_fd, env);
	}
	close(tmp_fd);
	return (0);
}
