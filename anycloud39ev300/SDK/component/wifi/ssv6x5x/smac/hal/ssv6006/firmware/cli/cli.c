#include "debug.h"
#include "cli.h"

#include <stdlib.h>
#include <string.h>
#include <stdio.h>

static struct cmd_struct *cmds;

static int do_help(int argc, const char **argv)
{
	struct cmd_struct *cmd = cmds;

	fprintf(stderr, "\nSupported commands:\n");
	do {
		fprintf(stderr, "  %s\n", cmd->cmd);
		cmd = cmd->next;

	} while (cmd);

	return 0;
}

static struct cmd_struct default_cmds[] = {

	{ .cmd = "help",	.fn = do_help,		.next = (void*)0 },
};

void cli_add_cmd(struct cmd_struct *cmd)
{
	cmd->next = cmds;
	cmds = cmd;
}

void cli_add_cmds(struct cmd_struct *cmds, int len)
{
	int i;

	for (i = 0; i < len; i++)
		cli_add_cmd(cmds++);
}

static int cli_process_cmd(int argc, const char **argv)
{
	struct cmd_struct *p = cmds;

	if (!argc)
		return 0;

	while (p) {

		if (!strcmp(argv[0], p->cmd))
			return p->fn(argc, argv);

		p = p->next;
	}

	fprintf(stderr, "Unknown command: %s\n", argv[0]);
	return 0;
}

void cli_main(void)
{
	char *buf;

	buf = malloc(1024);
	KASSERT(buf);

	cli_add_cmds(default_cmds, sizeof(default_cmds) / sizeof(struct cmd_struct));

	fprintf(stderr, "\n$ ");

        while (fgets(buf, 1024, stdin)) {

                const char *argv[10]; /* at most 10 arguments */
                char *p = buf;
                int argc = 0;

                buf[strlen(buf) - 1] = '\0';

s0:
                if (*p == '\n' || *p == '\0') {
                        goto s9;
                }
                else if (*p == ' ' || *p == '\t') {
                        *p++ = '\0';
                        goto s0;
                }
                else {
                        argv[argc++] = p;
                        goto s1;
                }
s1:
                if (*p == '\n' || *p == '\0') {
                        goto s9;
                }
                else if (*p == ' ' || *p == '\t') {
                        *p++ = '\0';
                        goto s0;
                }
                else {
                        p++;
                        goto s1;
                }

s9:
		cli_process_cmd(argc, argv);
		fprintf(stderr, "\n$ ");
        }

	free(buf);
}
