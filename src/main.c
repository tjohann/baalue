/*
  GPL
  (c) 2016-2017, thorsten.johannvorderbrueggen@t-online.de

  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; either version 2 of the License, or
  (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program; if not, write to the Free Software
  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.

 */

#include "common.h"

static char *program_name;
static sigset_t mask;

static pthread_t tid_signal_handler;
static pthread_t tid_local_client;
static pthread_t tid_inet_client;

/* 8101-8114   Unassigned */
const char baalued_port[] = "8111";

static void
show_some_infos(void)
{
	baa_show_package_name();
	baa_show_version_info();
}

static void
__attribute__((noreturn)) usage(int status)
{
	putchar('\n');
	baa_info_msg("Usage: %s [options]              ", program_name);
	baa_info_msg("Options:                                        ");
	baa_info_msg("        -h                       show help      ");
	baa_info_msg("        -s [server name]                        ");
	baa_info_msg("        -c [command]                            ");
	baa_info_msg("        -l                                      ");
	baa_info_msg("command:                                        ");
	baa_info_msg("        ping                     ping device    ");
	baa_info_msg("        halt                     halt device    ");
	baa_info_msg("        reboot                   reboot device  ");
	putchar('\n');
	baa_info_msg("Examples:                                       ");
	baa_info_msg("%s -s baalue_master  <- connect to baalue_master ",
		     program_name);
	baa_info_msg("%s -l  <- connect to the local baalued           ",
		     program_name);
	baa_info_msg("%s -c reboot/halt/ping <- cmd for baalued        ",
		     program_name);
	putchar('\n');

	show_some_infos();
	exit(status);
}

static void
cleanup(void)
{
#ifdef __DEBUG__
	baa_info_msg(_("finalize cleanup"));
#endif

	baa_info_msg(_("cheers %s"), getenv("USER"));
}

static void *
signal_handler(void *args)
{
	(void) args;

	if (pthread_detach(pthread_self()) != 0)
		baa_error_msg("could not detach signal handler -> ignore it");

	int sig = EINVAL;
	int err = -1;
	for (;;) {
		err = sigwait(&mask, &sig);
		if (err != 0)
			baa_error_exit("sigwait() != 0 in %s", __FUNCTION__);

		switch(sig) {
		case SIGTERM:
			baa_info_msg("catched signal \"%s\" (%d) -> exit now ",
				     strsignal(sig), sig);
			exit(EXIT_SUCCESS);
			break;
		case SIGHUP:
			baa_info_msg("signal \"%s\" (%d) -> ignore it",
				     strsignal(sig), sig);
			break;
		default:
			baa_error_msg("unhandled signal \"%s\" (%d)",
				      strsignal(sig), sig);
		}
	}

	return NULL;
}


void
send_to_inet_server(char *server_name, char *command)
{
	baa_info_msg("server: %s server_name");
	baa_info_msg("command: %s command");


	/*

	int fds = baa_inet_dgram_client(server_name, mgmt_port);
	if (fds == -1) {
		baa_error_msg("could not connect to %s", &server_name);
		usage(EXIT_FAILURE);
	}


	err = baa_ping_device(fds);
	if (err == -1) {
		baa_error_msg("could not ping device %s", &server_name);
		exit(EXIT_FAILURE);
	}

	baa_info_msg("%s is alive", server_name);
	*/

}

void
send_to_local_server(char *command)
{
	baa_info_msg("command: %s command");
}

int main(int argc, char *argv[])
{
	setlocale(LC_ALL, "");
	bindtextdomain(PACKAGE, LOCALEDIR);
	textdomain(PACKAGE);

	baa_set_program_name(&program_name, argv[0]);

	char *server_name = NULL;
	char *command = NULL;
	bool connect_to_inet_server = false;
	bool connect_to_local_server = false;

	int c;
	while ((c = getopt(argc, argv, "hlc:s:")) != -1) {
		switch (c) {
		case 's':
			connect_to_inet_server = true;
			server_name = optarg;
			break;
		case 'c':
			command = optarg;
			break;
		case 'l':
			connect_to_local_server = true;
			break;
		case 'h':
			usage(EXIT_SUCCESS);
			break;
		default:
			baa_error_msg("ERROR: no valid argument");
			usage(EXIT_FAILURE);
		}
	}

	int err = atexit(cleanup);
	if (err != 0)
		exit(EXIT_FAILURE);

	if ((connect_to_inet_server == false) &&
	    (connect_to_local_server == false))
		exit(EXIT_FAILURE);

	sigfillset(&mask);
	err = pthread_sigmask(SIG_BLOCK, &mask, NULL);
	if (err != 0)
		baa_th_error_exit(err, "could not set sigmask");

	err = pthread_create(&tid_signal_handler, NULL, signal_handler, 0);
	if (err != 0)
		baa_th_error_exit(err, "could not create pthread");

	if (connect_to_inet_server)
		send_to_inet_server(server_name, command);

	if (connect_to_local_server)
		send_to_local_server(command);

	/*
	if (connect_to_inet_server)
		(void) pthread_join(tid_inet_client, NULL);

	if (connect_to_local_server)
		(void) pthread_join(tid_local_client, NULL);
	*/

	(void) pthread_join(tid_signal_handler, NULL);

	exit(EXIT_SUCCESS);
}
