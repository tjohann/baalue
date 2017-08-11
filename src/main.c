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

/* 8101-8114   Unassigned */
const char baalued_port[] = "8111";


static void
__attribute__((noreturn)) usage(int status)
{
	putchar('\n');
	baa_info_msg(_("Usage: %s [options]              "), program_name);
	baa_info_msg(_("Options:                                       "));
	baa_info_msg(_("        -h                       show help     "));
	baa_info_msg(_("        -s [server name]                       "));
	baa_info_msg(_("        -c [command]                           "));
	baa_info_msg(_("        -l                                     "));
	baa_info_msg(_("command:                                       "));
	baa_info_msg(_("        ping                     ping device   "));
	baa_info_msg(_("        halt                     halt device   "));
	baa_info_msg(_("        reboot                   reboot device "));
	putchar('\n');
	baa_info_msg(_("Examples:                                      "));
	baa_info_msg(_("%s -s baalue_master  <- connect to baalue_master"),
		     program_name);
	baa_info_msg(_("%s -l  <- connect to the local baalued          "),
		     program_name);
	baa_info_msg(_("%s -c reboot/halt/ping <- supported commands    "),
		     program_name);
	baa_info_msg(_("%s -c halt -s baalue-01 <- halt node baalue-01  "),
		     program_name);
	putchar('\n');

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
		baa_error_msg(_("could not detach signal handler -> ignore it"));

	int sig = EINVAL;
	int err = -1;
	for (;;) {
		err = sigwait(&mask, &sig);
		if (err != 0)
			baa_error_exit(_("sigwait() != 0 in %s"), __FUNCTION__);

		switch(sig) {
		case SIGTERM:
			baa_info_msg(_("catched signal \"%s\" (%d) -> exit now"),
				     strsignal(sig), sig);
			exit(EXIT_SUCCESS);
			break;
		case SIGHUP:
			baa_info_msg(_("signal \"%s\" (%d) -> ignore it"),
				     strsignal(sig), sig);
			break;
		default:
			baa_error_msg(_("unhandled signal \"%s\" (%d)"),
				      strsignal(sig), sig);
		}
	}

	return NULL;
}


static int
send_to_inet_server(char *server_name, char *command)
{
	if ((server_name == NULL) || (command == NULL)) {
		baa_error_msg(_("server_name or command == NULL"));
		return -1;
	} else {
		baa_info_msg(_("server: %s"), server_name);
		baa_info_msg(_("command: %s"), command);
	}


	int fds = baa_inet_dgram_client(server_name, baalued_port);
	if (fds == -1) {
		baa_error_msg(_("could not connect to %s"), &server_name);
		return -1;
	}

	int err = -1;
	if (strncmp(command, "reboot", strlen("reboot")) == 0) {
#ifdef __DEBUG__
		baa_info_msg(_("send cmd reboot"));
#endif
		err = baa_reboot_device(fds);
		if (err == -1) {
			baa_error_msg(_("could not reboot device %s"),
				&server_name);
			return -1;
		}
	}

	if (strncmp(command, "halt", strlen("halt")) == 0) {
#ifdef __DEBUG__
		baa_info_msg(_("send cmd halt"));
#endif
		err = baa_halt_device(fds);
		if (err == -1) {
			baa_error_msg(_("could not halt device %s"),
				&server_name);
			return -1;
		}
	}

	if (strncmp(command, "ping", strlen("ping")) == 0) {
#ifdef __DEBUG__
		baa_info_msg(_("send cmd ping"));
#endif
		err = baa_ping_device(fds);
		if (err == -1) {
			baa_error_msg(_("could not ping device %s"),
				&server_name);
			return -1;
		}
	}

#ifdef __DEBUG__
	baa_info_msg(_("%s seems to be alive"), server_name);
#endif
	return 0;
}

static void
send_to_local_server(char *command)
{
	baa_info_msg(_("command: %s command"));
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
			baa_error_msg(_("ERROR: no valid argument"));
			usage(EXIT_FAILURE);
		}
	}

	int err = atexit(cleanup);
	if (err != 0)
		exit(EXIT_FAILURE);

	if ((connect_to_inet_server == false) &&
	    (connect_to_local_server == false)) {
		baa_info_msg(_("neither inet nor local server selected -> exit"));
		usage(EXIT_FAILURE);
	}

	sigfillset(&mask);
	err = pthread_sigmask(SIG_BLOCK, &mask, NULL);
	if (err != 0)
		baa_th_error_exit(err, _("could not set sigmask"));

	err = pthread_create(&tid_signal_handler, NULL, signal_handler, 0);
	if (err != 0)
		baa_th_error_exit(err, _("could not create pthread"));

	if (connect_to_inet_server)
		send_to_inet_server(server_name, command);

	if (connect_to_local_server)
		send_to_local_server(command);

	exit(EXIT_SUCCESS);
}
