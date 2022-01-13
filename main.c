
#include <unistd.h>
#include <signal.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <stdio.h>

#include <sys/wait.h>
#include <readline/readline.h>

#define MAX_ARGV 65

pid_t main_pid;

void sigintHandler() {
  if (getpid() != main_pid) {
    exit(0);
  }
  // 主shell这里显示效果不好
}

void runcmd(char *buf) {
  while (*buf != '\0' && isspace(*buf)) {
    buf++;
  }
  int len = strlen(buf);
  len--;
  while (len >= 0 && isspace(buf[len])) {
    buf[len] = '\0';
    len--;
  }
  if (len <= 0) {
    exit(0);
  }
  int argc = 0;
  char *argv[MAX_ARGV];
  while (*buf != '\0') {
    argv[argc++] = buf;
    while (*buf != '\0' && !isspace(*buf)) {
      buf++;
    }
    if (*buf != '\0') {
      *buf = '\0';
    } else {
      break;
    }
    buf++;
    while (*buf != '\0' && isspace(*buf)) {
      buf++;
    }
  }
  char *rein = NULL;
  char *reout = NULL;
  if (argc >= 2 && strcmp(argv[argc-2], ">") == 0) {
    reout = argv[argc-1];
    argc -= 2;
  }
  if (argc >= 2 && strcmp(argv[argc-2], "<") == 0) {
    rein = argv[argc-1];
    argc -= 2;
  }
  if (argc >= 2 && strcmp(argv[argc-2], ">") == 0) {
    reout = argv[argc-1];
    argc -= 2;
  }
  argv[argc] = NULL;

  if (rein != NULL) {
    freopen(rein, "r", stdin);
  }
  if (reout != NULL) {
    freopen(reout, "w", stdout);
  }

  char *first = argv[0];
  int lst_idx = -1;
  len = strlen(argv[0]);
  for (int i = 0; i < len; i++) {
    if (argv[0][i] == '/') {
      lst_idx = i;
    }
  }
  if (lst_idx >= 0) {
    argv[0] = argv[0] + lst_idx;
  }
  execvp(first, argv);
  
  exit(0);
}

// 在一个单独的进程中处理一行命令
void split_pipe(char *buf) {
  int p[2];
  char *first = buf;
  while (*first != '\0') {
    while (*buf != '\0' && *buf != '|') {
      buf++;
    }
    // 没有管道的情况，子进程执行，父进程监督执行以后退出
    if (*buf == '\0') {
      pid_t dfdf;
      if ((dfdf=fork()) == 0) {
        runcmd(first);
        break;
      }
      waitpid(dfdf, NULL, 0);
      break;
    }
    *buf = '\0';
    buf++;
    // 有管道的情况，子进程1执行左边，子进程2执行右边，父进程监督后退出
    pipe(p);
    pid_t pp1, pp2;
    if ((pp1=fork()) == 0) { // 管道左边
      close(p[0]);
      dup2(p[1], 1);
      runcmd(first);
    }
    if ((pp2=fork()) == 0) {
      close(p[1]);
      dup2(p[0], 0);
      first = buf;
    } else {
      close(p[0]);
      close(p[1]);
      waitpid(pp1, NULL, 0);
      waitpid(pp2, NULL, 0);
      break;
    }
  } 
}

int main() {
  signal(SIGINT, sigintHandler);
  main_pid = getpid();
  while (1) {
    char *buf = readline("poorpool @ poorpool\n$ ");
    pid_t sub;
    if (sub = fork()) {
      waitpid(sub, NULL, 0);
    } else {
      split_pipe(buf);
      exit(0);
    }
    free(buf);
  }
  return 0;
}