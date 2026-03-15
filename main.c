/*
 * Copyright (C) 2015-2016 Omar Sandoval
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <assert.h>
#include <errno.h>
#include <getopt.h>
#include <inttypes.h>
#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>
#include <signal.h>
#include <unistd.h>
#include <sys/epoll.h>
#include <sys/signalfd.h>
#include <sys/timerfd.h>
#include <sys/types.h>

#include "backend.h"
#include "verbar_internal.h"
#include "render.h"

static const char *progname = "verbar";
static const struct backend *const backends[] = {
#if WITH_X11
	&x11_backend,
#endif
	&stdout_text_backend,
};

static const char *config[] = {
#if WITH_DROPBOX
	"dropbox",
#endif
#if WITH_NET
	"net",
#endif
#if WITH_VOLUME
	"volume",
#endif
	"cpu",
	"mem",
	"power",
	"clock",
};

static bool quit, update, wordy;

static struct str status_str;
#if WITH_X11
static const struct backend *backend = &x11_backend;
#else
static const struct backend *backend = &stdout_text_backend;
#endif
static void *backend_data;

void request_update(void)
{
	update = true;
}

static int update_statusbar(void)
{
	if (render_status(&status_str, wordy))
		return -1;

	return backend->write_status(backend_data, status_str.buf);
}

static int signal_fd_callback(int fd, void *data, uint32_t events)
{
	struct signalfd_siginfo ssi;
	ssize_t ssret;

	ssret = read(fd, &ssi, sizeof(ssi));
	if (ssret == -1) {
		perror("read(signalfd)");
		return -1;
	}
	assert(ssret == sizeof(ssi));
	if (ssi.ssi_signo == SIGUSR1) {
		wordy = !wordy;
		update = true;
	} else {
		fprintf(stderr, "got signal %s; exiting\n",
			strsignal(ssi.ssi_signo));
		quit = true;
	}
	return 0;
}

static struct epoll_callback signal_cb = {
	.callback = signal_fd_callback,
	.fd = -1,
};

static int signal_fd_init(int epoll_fd)
{
	struct epoll_event ev;
	sigset_t mask;
	int ret;
	int fd;

	sigemptyset(&mask);
	sigaddset(&mask, SIGHUP);
	sigaddset(&mask, SIGINT);
	sigaddset(&mask, SIGTERM);
	sigaddset(&mask, SIGUSR1);
	ret = sigprocmask(SIG_BLOCK, &mask, NULL);
	if (ret == -1) {
		perror("sigprocmask");
		return -1;
	}

	fd = signalfd(-1, &mask, SFD_CLOEXEC);
	if (fd == -1) {
		perror("signalfd");
		return -1;
	}

	signal_cb.fd = fd;
	ev.events = EPOLLIN;
	ev.data.ptr = &signal_cb;

	return epoll_ctl(epoll_fd, EPOLL_CTL_ADD, fd, &ev);
}

static int timer_fd_callback(int fd, void *data, uint32_t events)
{

	uint64_t times;
	ssize_t ssret;
	int ret;

	ssret = read(fd, &times, sizeof(times));
	if (ssret == -1) {
		perror("read(timerfd)");
		return -1;
	}
	assert(ssret == sizeof(times));
	if (times > 1) {
		fprintf(stderr, "warning: missed %" PRIu64 " ticks\n",
			times - 1);
	}
	ret = update_timer_sections();
	if (ret)
		return -1;
	update = true;
	return 0;
}

static struct epoll_callback timer_cb = {
	.callback = timer_fd_callback,
	.fd = -1,
};

static int timer_fd_init(int epoll_fd)
{
	struct epoll_event ev;
	int fd;

	fd = timerfd_create(CLOCK_MONOTONIC, TFD_CLOEXEC);
	if (fd == -1) {
		perror("timerfd_create");
		return -1;
	}

	timer_cb.fd = fd;
	ev.events = EPOLLIN;
	ev.data.ptr = &timer_cb;

	return epoll_ctl(epoll_fd, EPOLL_CTL_ADD, fd, &ev);
}

static const struct backend *find_backend(const char *name)
{
	size_t i;

	for (i = 0; i < sizeof(backends) / sizeof(*backends); i++) {
		if (strcmp(backends[i]->name, name) == 0)
			return backends[i];
	}

	return NULL;
}

static void usage(bool error)
{
	fprintf(error ? stderr : stdout,
		"usage: %s [--backend NAME] [--icons PATH] [--wordy]\n"
		"\n"
		"Gather system information and write status text\n"
		"\n"
		"Options:\n"
#if WITH_X11
		"  -b, --backend NAME  output backend (x11, stdout-text)\n"
#else
		"  -b, --backend NAME  output backend (stdout-text)\n"
#endif
		"  -i, --icons PATH    directory containing icon files\n"
		"  -w, --wordy         enable wordy output on startup\n"
		"\n"
		"Miscellaneous:\n"
		"  -h, --help     display this help message and exit\n",
		progname);
	exit(error ? EXIT_FAILURE : EXIT_SUCCESS);
}

int main(int argc, char **argv)
{
	struct option long_options[] = {
		{"backend", required_argument, NULL, 'b'},
		{"icons", required_argument, NULL, 'i'},
		{"wordy", no_argument, NULL, 'w'},
		{"help", no_argument, NULL, 'h'},
	};
	int epoll_fd = -1;
	struct itimerspec it;
	int ret;
	int status = EXIT_SUCCESS;

	if (argc > 0)
		progname = argv[0];

	for (;;) {
		int c;

		c = getopt_long(argc, argv, "b:i:wh", long_options, NULL);
		if (c == -1)
			break;

		switch (c) {
		case 'b':
			backend = find_backend(optarg);
			if (!backend) {
				fprintf(stderr, "unknown backend \"%s\"\n",
					optarg);
				usage(true);
			}
			break;
		case 'i':
			icon_path = optarg;
			break;
		case 'w':
			wordy = true;
			break;
		case 'h':
			usage(false);
		default:
			usage(true);
		}
	}
	if (optind != argc)
		usage(true);

	backend_data = backend->init();
	if (!backend_data) {
		status = EXIT_FAILURE;
		goto out;
	}

	epoll_fd = epoll_create1(EPOLL_CLOEXEC);
	if (epoll_fd == -1) {
		perror("epoll_create1");
		status = EXIT_FAILURE;
		goto out;
	}

	ret = signal_fd_init(epoll_fd);
	if (ret == -1) {
		status = EXIT_FAILURE;
		goto out;
	}

	ret = timer_fd_init(epoll_fd);
	if (ret == -1) {
		status = EXIT_FAILURE;
		goto out;
	}

	if (init_sections(epoll_fd, config, sizeof(config) / sizeof(*config))) {
		status = EXIT_FAILURE;
		goto out;
	}
	if (update_timer_sections()) {
		status = EXIT_FAILURE;
		goto out;
	}

	it.it_interval.tv_sec = 1;
	it.it_interval.tv_nsec = 0;
	it.it_value.tv_sec = 1;
	it.it_value.tv_nsec = 0;
	ret = timerfd_settime(timer_cb.fd, 0, &it, NULL);
	if (ret == -1) {
		perror("timerfd_settime");
		status = EXIT_FAILURE;
		goto out;
	}

	while (!quit) {
		struct epoll_event events[10];
		int i;

		update = false;

		ret = epoll_wait(epoll_fd, events,
				 sizeof(events) / sizeof(events[0]),
				 -1);
		if (ret == -1) {
			if (errno == EINTR)
				continue;
			perror("epoll_wait");
			status = EXIT_FAILURE;
			goto out;
		}

		for (i = 0; i < ret; i++) {
			struct epoll_callback *cb = events[i].data.ptr;

			ret = cb->callback(cb->fd, cb->data, events[i].events);
			if (ret) {
				status = EXIT_FAILURE;
				goto out;
			}
		}

		if (update) {
			ret = update_statusbar();
			if (ret) {
				status = EXIT_FAILURE;
				goto out;
			}
		}
	}

	status = EXIT_SUCCESS;
out:
	if (epoll_fd != -1)
		close(epoll_fd);
	free_sections();
	if (timer_cb.fd != -1)
		close(timer_cb.fd);
	if (signal_cb.fd != -1)
		close(signal_cb.fd);
	str_free(&status_str);
	backend->shutdown(backend_data);
	return status;
}
