#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <ctype.h>
#include <fcntl.h>
#include <sys/wait.h>
#include <signal.h>
#include "clash.h"

#define PROMPT_SIZE 256
#define INPUT_SIZE 1024

#define KNRM  "\x1B[0m"
#define KRED  "\x1B[31m"
#define KGRN  "\x1B[32m"
#define KYEL  "\x1B[33m"
#define KBLU  "\x1B[34m"
#define KMAG  "\x1B[35m"
#define KCYN  "\x1B[36m"
#define KWHT  "\x1B[37m"
#define RESET "\033[0m"

char *prompt;

ssize_t safe_write(int fd, const void *buf, size_t count) {
    ssize_t res = write(fd, buf, count);
    if (res == -1) perror("write");
    return res;
}

void error(char *msg) {
    safe_write(STDERR_FILENO, msg, strlen(msg));
}

void get_prompt(void) {
	char buf[PROMPT_SIZE];
	char *cwd = getcwd(buf, PROMPT_SIZE);
	if (!cwd) {
		perror("getcwd");
		exit(1);
	}

	int i = 0;
	while (cwd[i]) {
		if (cwd[i] == '/') {
			cwd += i;
			cwd ++;
			i = 0;
		}
		i ++;
	}

	ssize_t len = snprintf(prompt, PROMPT_SIZE,
		"\033[34m[clash]\033[36m (%s)>\033[0m ", cwd);
	if (len <= 0 || len > PROMPT_SIZE) {
		error("get_prompt: error acquiring current directory");
	}
	prompt[len] = 0;
}

int leading_whitespace(char *input) {
	while (input[0]) {
		if (!isspace(input[0]))
			return 0;
		input ++;
	}
	return 1;
}

int get_redirect(char *input, char *str, char *redirect_file) {
	int redirected = 0;
	int i = 0;
	int preceded_by_ws = 1;
	int l = strlen(str);
	while (input[i] != 0) {
		if (isspace(input[i]))
			preceded_by_ws = 1;
		else
			preceded_by_ws = 0;
		if (!strncmp(input + i, str, l)) {
			if (preceded_by_ws && isspace(input[i + l])) {
				if (!redirected)
					get_next_word(input, i + l, redirect_file);
				else

			}
		}
	}
}

int parse(char *input, char *command, int *argc, char **argv,
		char *append_file, char *in_file, char *out_file) {
	safe_write(STDOUT_FILENO, input, strlen(input));

	if (leading_whitespace(input))
		return 1;

	int err = 0;
	err += get_redirect(input, ">>", append_file);
	err += get_redirect(input, ">", out_file);
	err += get_redirect(input, "<", in_file);
	if (err < 0)
		return 1;
	if (append_file && out_file) {
		error("parse: cannot redirect output twice");
		return 1;
	}

	return 0;
}

void eval(char *input) {
	char *command = 0;
	int argc = 0;
	char **argv = 0;
	char *append_file = 0;
	char *in_file = 0;
	char *out_file = 0;

	int parse_error = parse(input, command, &argc, argv,
		append_file, in_file, out_file);
	if (parse_error)
		return;


}

int repl(void) {
	while(1) {
		char input[INPUT_SIZE];
		safe_write(STDOUT_FILENO, prompt, strlen(prompt));
		ssize_t len = read(STDIN_FILENO, input, INPUT_SIZE);
		if (len == -1) {
			perror("read");
			free(prompt);
			exit(1);
		} else if (len == 0) {
			safe_write(STDOUT_FILENO, "\n", 1);
			break;
		} else {
			input[len] = 0;
			eval(input);
		}
	}

	return 0;
}

int main(void) {
	prompt = malloc(PROMPT_SIZE);
	if (!prompt) {
		perror("malloc");
		exit(1);
	}
	get_prompt();
	int result = repl();
	free(prompt);
	return result;
}
