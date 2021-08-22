#ifndef __CLI_H__
#define __CLI_H__

struct cmd_struct{

	const char *cmd;
	int (*fn)(int argc, const char **argv);
	struct cmd_struct *next;
};

extern void cli_add_cmd(struct cmd_struct *cmd);
extern void cli_add_cmds(struct cmd_struct *cmds, int len);
extern void cli_main(void);

#endif /* __CLI_H__ */
