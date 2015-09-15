#ifndef WIN32
#define FORK_TEST
#endif

#ifdef FORK_TEST
#include <sys/types.h>
#include <sys/wait.h>
#endif
#include <unistd.h>
#include "test.h"
#include "../src/list.h"


struct test {
	struct list_head list;
	char *name;
	int (*func)(void);
};

static LIST_HEAD(test_list);

static char *color_fail = "";
static char *color_pass = "";
static char *color_test = "";
static char *color_none = "";

static int num_tests = 0;

#define add_test(x) _add_test(#x, _test_func_##x)

void _add_test(char *name, int (*func)(void))
{
	struct test *t;

	t = malloc(sizeof (struct test));
	if (t == NULL)
		return;
	t->name = name;
	t->func = func;
	list_add_tail(&t->list, &test_list);
	num_tests++;
}

void init_colors()
{
	if (isatty(STDOUT_FILENO)) {
		color_fail = "\x1b[1;31m";
		color_pass = "\x1b[1;32m";
		color_test = "\x1b[1m";
		color_none = "\x1b[0m";
	}
}

#ifdef FORK_TEST

int run_tests()
{
	struct list_head *tmp;
	int total, fail;
	pid_t pid;
	int status;

	total = fail = 0;

	init_colors();

	list_for_each(tmp, &test_list) {
		struct test *t = list_entry(tmp, struct test, list);

		printf("%stest %d:%s %s: ",
				color_test, total + 1, color_none, t->name);
		fflush(stdout);

		if ((pid = fork()) == 0) {
			exit(t->func());
		}

		waitpid(pid, &status, 0);

		if (status != 0) {
			fail++;
			if (WIFSIGNALED(status)) {
				printf("%s: ", strsignal(WTERMSIG(status)));
			}
			printf("%s**fail**%s\n", color_fail, color_none);
		} else {
			printf("%spass%s\n", color_pass, color_none);
		}
		total++;
	}

	printf("%stotal:%d  passed:%d (%4.1f%%)  failed:%d (%4.1f%%)%s\n",
		color_test, total,
		(total - fail), 100.0 * (total - fail) / total,
		fail, 100.0 * fail / total, color_none);

	return -fail;
}

#else

int run_test(int num)
{
	struct list_head *tmp;
	int i;

	i = 0;
	list_for_each(tmp, &test_list) {
		struct test *t = list_entry(tmp, struct test, list);
		int res;

		if (i == num) {
			printf("test %d: %s: ", num + 1, t->name);
			res = t->func();
			if (res != 0) {
				printf("**fail**\n");
				return -1;
			} else {
				printf("pass\n");
				return 0;
			}
		}
		
		i++;
	}

	return -2;
}

#endif

int main(int argc, char **argv)
{
#define declare_test(x) add_test(x)
#include "all_tests.c"
#undef declare_test

#ifdef FORK_TEST

	if (run_tests() == 0) {
		exit(EXIT_SUCCESS);
	} else {
		exit(EXIT_FAILURE);
	}

#else
	int i;
	char cmd[512];
	int total = 0, fail = 0;

	/* Run specific tests */
	if (argc > 1) {
		int res = run_test(strtoul(argv[1], NULL, 0));
		exit(-res);
	}

	for (i = 0; i < num_tests; i++) {
		snprintf(cmd, 512, "%s %d", argv[0], i);
		if (system(cmd) != 0) {
			fail++;
		}
		total++;
	}

	printf("total:%d  passed:%d (%4.1f%%)  failed:%d (%4.1f%%)\n",
		total, (total - fail), 100.0 * (total - fail) / total,
		fail, 100.0 * fail / total);

	if (fail == 0) {
		exit(EXIT_SUCCESS);
	} else {
		exit(EXIT_FAILURE);
	}
#endif
}
