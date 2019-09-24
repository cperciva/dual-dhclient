#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

static volatile sig_atomic_t gotsig = 0;

static void
sigterm_handler(int sig)
{
	(void)sig; /* UNUSED */
	gotsig = 1;
}

static void
spawn(char * path, const char * const argv[])
{

	/* Fork into background. */
	switch (fork()) {
	case -1:
		/* Fork failed. */
		exit(1);
	case 0:
		/* In child process. */
		break;
	default:
		/* In parent process. */
		return;
	}

	/* Execute process. */
	execv(path, (char * const *)argv);

	/* Fatal error. */
	fprintf(stderr, "Failed to spawn %s\n", path);
	exit(1);
}

static void
daemonize(const char * pidfile)
{
	FILE * f;

	/* Fork into background. */
	switch (fork()) {
	case -1:
		/* Fork failed. */
		exit(1);
	case 0:
		/* In child process. */
		break;
	default:
		/* In parent process. */
		_exit(0);
	}

	/* Become a session leader. */
	if (setsid() == -1)
		_exit(1);

	/* Write out our pid file. */
	if ((f = fopen(pidfile, "w")) == NULL)
		exit(1);
	if (fprintf(f, "%d", getpid()) < 0)
		exit(1);
	fclose(f);
}

static void
killpidfile(const char * pidfile, int sig)
{
	FILE * f;
	int pid;

	/* Open the pidfile and read it. */
	if ((f = fopen(pidfile, "r")) == NULL)
		exit(1);
	fscanf(f, "%d", &pid);
	fclose(f);

	/* Send the signal. */
	kill(pid, sig);
}

int
main(int argc, char * argv[])
{
	const char * nargv[9];
	char * pidfile;
	char * pidfile4;
	char * pidfile6;

	/* We should have a single argument -- the interface name. */
	if (argc != 2) {
		fprintf(stderr, "usage: dual-dhclient ifname\n");
		exit(1);
	}

	if (asprintf(&pidfile, "/var/run/dhclient/dhclient.%s.pid", argv[1]) < 0)
		exit(1);
	if (asprintf(&pidfile4, "/var/run/dhclient/dhclient.%s.4.pid", argv[1]) < 0)
		exit(1);
	if (asprintf(&pidfile6, "/var/run/dhclient/dhclient.%s.6.pid", argv[1]) < 0)
		exit(1);

	/* Spawn /sbin/dhclient. */
	nargv[0] = "/sbin/dhclient";
	nargv[1] = "-p";
	nargv[2] = pidfile4;
	nargv[3] = argv[1];
	nargv[4] = NULL;
	spawn("/sbin/dhclient", nargv);

	/* Spawn /usr/local/sbin/dhclient. */
	nargv[0] = "/usr/local/sbin/dhclient";
	nargv[1] = "-6";
	nargv[2] = "-nw";
	nargv[3] = "-cf";
	nargv[4] = "/dev/null";
	nargv[5] = "-pf";
	nargv[6] = pidfile6;
	nargv[7] = argv[1];
	nargv[8] = NULL;
	spawn("/usr/local/sbin/dhclient", nargv);

	/* Daemonize. */
	daemonize(pidfile);

	/* Catch SIGTERM. */
	signal(SIGTERM, sigterm_handler);

	/* Spin until it arrives. */
	while (!gotsig) {
		sleep(1);
	}
	
	/* Signal the two dhclient processes. */
	killpidfile(pidfile4, SIGTERM);
	killpidfile(pidfile6, SIGTERM);

	/* Remove our pidfile. */
	unlink(pidfile);

	/* We're done. */
	exit(0);
}
