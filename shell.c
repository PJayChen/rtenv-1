#include "myio.h"
#include <stddef.h>
#include "kernel.h"

#define MAX_CMDNAME 19
#define MAX_ARGC 19
#define MAX_CMDHELP 1023
#define HISTORY_COUNT 20
#define CMDBUF_SIZE 100
#define MAX_ENVCOUNT 30
#define MAX_ENVNAME 15
#define MAX_ENVVALUE 127



/*Global Variables*/
char next_line[3] = {'\n','\r','\0'};
char cmd[HISTORY_COUNT][CMDBUF_SIZE];
int cur_his=0;
//int fdout;
int fdin;
extern size_t task_count;

/*Functions*/
void check_keyword(void);
void find_events(void);

/* Command handlers. */
void export_envvar(int argc, char *argv[]);
void show_echo(int argc, char *argv[]);
void show_cmd_info(int argc, char *argv[]);
void show_task_info(int argc, char *argv[]);
void show_man_page(int argc, char *argv[]);
void show_history(int argc, char *argv[]);
void exec_program(int argc, char *argv[]);

/* Enumeration for command types. */
enum {
	CMD_ECHO = 0,
	CMD_EXPORT,
	CMD_HELP,
	CMD_HISTORY,
	CMD_MAN,
	CMD_PS,
	CMD_EXEC,
	CMD_COUNT
} CMD_TYPE;

/* Structure for command handler. */
typedef struct {
	char cmd[MAX_CMDNAME + 1];
	void (*func)(int, char**);
	char description[MAX_CMDHELP + 1];
} hcmd_entry;

const hcmd_entry cmd_data[CMD_COUNT] = {
	[CMD_ECHO] = {.cmd = "echo", .func = show_echo, .description = "Show words you input."},
	[CMD_EXPORT] = {.cmd = "export", .func = export_envvar, .description = "Export environment variables."},
	[CMD_HELP] = {.cmd = "help", .func = show_cmd_info, .description = "List all commands you can use."},
	[CMD_HISTORY] = {.cmd = "history", .func = show_history, .description = "Show latest commands entered."}, 
	[CMD_MAN] = {.cmd = "man", .func = show_man_page, .description = "Manual pager."},
	[CMD_PS] = {.cmd = "ps", .func = show_task_info, .description = "List all the processes."},
	[CMD_EXEC] = {.cmd = "exec", .func = exec_program, .description = "Execute a specific program."}
};

/* Structure for environment variables. */
typedef struct {
	char name[MAX_ENVNAME + 1];
	char value[MAX_ENVVALUE + 1];
} evar_entry;
evar_entry env_var[MAX_ENVCOUNT];
int env_count = 0;


void vShell_task()
{
	char put_ch[2]={'0','\0'};
	char hint[] =  USER_NAME "@" USER_NAME "-STM32:~$ ";
	int hint_length = sizeof(hint);
	char *p = NULL;
	int cmd_count = 0;
	char q[2] = {'q','\0'};
	//fdout = mq_open("/tmp/mqueue/out", 0);
	fdin = open("/dev/tty0/in", 0);
	
	for (;; cur_his = (cur_his + 1) % HISTORY_COUNT) {
		p = cmd[cur_his];
		
		printf("%s", hint);

		while (1) {
			read(fdin, put_ch, 1);

			if (put_ch[0] == '\r' || put_ch[0] == '\n') {
				*p = '\0';
				printf("%s", next_line);
				break;
			}
			else if (put_ch[0] == 127 || put_ch[0] == '\b') {
				if (p > cmd[cur_his]) {
					p--;
					printf("\b \b");
				}
			}
			else if (p - cmd[cur_his] < CMDBUF_SIZE - 1) {
				*p++ = put_ch[0];
				printf("%s", put_ch);
			}
		}
		check_keyword();	
	}
}

/* Split command into tokens. */
char *cmdtok(char *cmd)
{
	static char *cur = NULL;
	static char *end = NULL;
	if (cmd) {
		char quo = '\0';
		cur = cmd;
		for (end = cmd; *end; end++) {
			if (*end == '\'' || *end == '\"') {
				if (quo == *end)
					quo = '\0';
				else if (quo == '\0')
					quo = *end;
				*end = '\0';
			}
			else if (isspace(*end) && !quo)
				*end = '\0';
		}
	}
	else
		for (; *cur; cur++)
			;

	for (; *cur == '\0'; cur++)
		if (cur == end) return NULL;
	return cur;
}

void check_keyword(void)
{
	char *argv[MAX_ARGC + 1] = {NULL};
	char cmdstr[CMDBUF_SIZE];
	char buffer[CMDBUF_SIZE * MAX_ENVVALUE / 2 + 1];
	char *p = buffer;
	int argc = 1;
	int i;

	find_events();
	strcpy(cmdstr, cmd[cur_his]);
	argv[0] = cmdtok(cmdstr);
	if (!argv[0])
		return;

	while (1) {
		argv[argc] = cmdtok(NULL);
		if (!argv[argc])
			break;
		argc++;
		if (argc >= MAX_ARGC)
			break;
	}

	for(i = 0; i < argc; i++) {
		int l = fill_arg(p, argv[i]);
		argv[i] = p;
		p += l + 1;
	}

	for (i = 0; i < CMD_COUNT; i++) {
		if (!strcmp(argv[0], cmd_data[i].cmd)) {
			cmd_data[i].func(argc, argv);
			break;
		}
	}
	if (i == CMD_COUNT) {
		printf("%s: command not found\n", argv[0]);
	}
}

void find_events(void)
{
	char buf[CMDBUF_SIZE];
	char *p = cmd[cur_his];
	char *q;
	int i;

	for (; *p; p++) {
		if (*p == '!') {
			q = p;
			while (*q && !isspace(*q))
				q++;
			for (i = cur_his + HISTORY_COUNT - 1; i > cur_his; i--) {
				if (!strncmp(cmd[i % HISTORY_COUNT], p + 1, q - p - 1)) {
					strcpy(buf, q);
					strcpy(p, cmd[i % HISTORY_COUNT]);
					p += strlen(p);
					strcpy(p--, buf);
					break;
				}
			}
		}
	}
}

char *find_envvar(const char *name)
{
	int i;

	for (i = 0; i < env_count; i++) {
		if (!strcmp(env_var[i].name, name))
			return env_var[i].value;
	}

	return NULL;
}

/* Fill in entire value of argument. */
int fill_arg(char *const dest, const char *argv)
{
	char env_name[MAX_ENVNAME + 1];
	char *buf = dest;
	char *p = NULL;

	for (; *argv; argv++) {
		if (isalnum(*argv) || *argv == '_') {
			if (p)
				*p++ = *argv;
			else
				*buf++ = *argv;
		}
		else { /* Symbols. */
			if (p) {
				*p = '\0';
				p = find_envvar(env_name);
				if (p) {
					strcpy(buf, p);
					buf += strlen(p);
					p = NULL;
				}
			}
			if (*argv == '$')
				p = env_name;
			else
				*buf++ = *argv;
		}
	}
	if (p) {
		*p = '\0';
		p = find_envvar(env_name);
		if (p) {
			strcpy(buf, p);
			buf += strlen(p);
		}
	}
	*buf = '\0';

	return buf - dest;
}

//export
void export_envvar(int argc, char *argv[])
{
	char *found;
	char *value;
	int i;

	for (i = 1; i < argc; i++) {
		value = argv[i];
		while (*value && *value != '=')
			value++;
		if (*value)
			*value++ = '\0';
		found = find_envvar(argv[i]);
		if (found)
			strcpy(found, value);
		else if (env_count < MAX_ENVCOUNT) {
			strcpy(env_var[env_count].name, argv[i]);
			strcpy(env_var[env_count].value, value);
			env_count++;
		}
	}
}

/*Command Function: ps*/
void show_task_info(int argc, char* argv[])
{
	char ps_message[]="PID STATUS \t\tPRIORITY";
	int ps_message_length = sizeof(ps_message);
	int task_i;
	int task;

	printf("%s\n", ps_message);

	for (task_i = 0; task_i < task_count; task_i++) {
		
		char *task_info_status;
		
		switch(tasks[task_i].status){
			case TASK_READY:
				task_info_status = "TASK_READY";
				break;
			case TASK_WAIT_READ:
				task_info_status = "TASK_WAIT_READ";
				break;
			case TASK_WAIT_WRITE:
				task_info_status = "TASK_WAIT_WRITE";
				break;
			case TASK_WAIT_INTR:
				task_info_status = "TASK_WAIT_INTR";
				break;
			case TASK_WAIT_TIME:
				task_info_status = "TASK_WAIT_TIME";
				break;
		}
		
		printf("%d   %d %s\t%d\n", tasks[task_i].pid, tasks[task_i].status, task_info_status, tasks[task_i].priority);
	
	}
}


/*Command Function: help*/
void show_cmd_info(int argc, char* argv[])
{
	const char help_desp[] = "This system has commands as follow\n\r\0";
	int i;

	printf("%s", help_desp);
	
	for (i = 0; i < CMD_COUNT; i++) {
		printf("%s: %s\n", cmd_data[i].cmd, cmd_data[i].description);
	}
}

/*Command Function: echo*/
void show_echo(int argc, char* argv[])
{
	const int _n = 1; /* Flag for "-n" option. */
	int flag = 0;
	int i;

	for (i = 1; i < argc; i++) {
		if (!strcmp(argv[i], "-n"))
			flag |= _n;
		else
			break;
	}

	for (; i < argc; i++) {
		printf("%s", argv[i]);
		if (i < argc - 1)
			printf(" "); 
	}

	if (~flag & _n)
		printf("\n");
}

/*Command Function: man*/
void show_man_page(int argc, char *argv[])
{
	int i;

	if (argc < 2)
		return;

	for (i = 0; i < CMD_COUNT && strcmp(cmd_data[i].cmd, argv[1]); i++)
		;

	if (i >= CMD_COUNT)
		return;

	printf("NAME: %s\nDESCRIPTION: %s\n", cmd_data[i].cmd, cmd_data[i].description);
}

/*Command Function: history*/
void show_history(int argc, char *argv[])
{
	int i;

	for (i = cur_his + 1; i <= cur_his + HISTORY_COUNT; i++) {
		if (cmd[i % HISTORY_COUNT][0]) {
			printf("%s\n", cmd[i % HISTORY_COUNT]);
		}
	}
}

/*below is Progeam loader*/
enum 
{
	DISABLE = 0, 
	ENABLE 
}exec;

typedef struct {
	unsigned int fork;
	void (*func)(void);
} loader_info;

loader_info l_info = {
	.fork = DISABLE,
	.func = NULL
};

/*Task want be created*/
void vNew_task(void){
	printf("New task is running\n");	
	int fd = open("/dev/tty0/in", 0);
	write(fd, "\n", 1);
	while(1);
}

void vNew_task1(void){
	printf("New task1 is running\n");	
	int fd = open("/dev/tty0/in", 0);
	write(fd, "\n", 1);
	while(1);	
}

/*responseble for fork specific task*/
void vProgram_loader_task(void){

	while(1){
		
		if(l_info.fork){
			l_info.fork = DISABLE;

			if(task_count < TASK_LIMIT){
				if (!fork()) setpriority(0, PRIORITY_DEFAULT), l_info.func();
			}else {
				printf("Error, reach the tasks maximun.\n");
				int fd = open("/dev/tty0/in", 0);
				write(fd, "\n", 1);
			}
		}
	}
}

/*Command Function: exec*/
void exec_program(int argc, char *argv[])
{
	printf("Execute program %c \n", *argv[1]);
	switch(*argv[1]){
		case '1':
			l_info.func = vNew_task;
			l_info.fork = ENABLE;
			break;
		case '2':
			l_info.func = vNew_task1;
			l_info.fork = ENABLE;
			break;
	}
}