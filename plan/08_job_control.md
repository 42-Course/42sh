# Job Control Module

## Purpose

Manage background jobs and process groups. Job control allows:

- Running commands in background (`&`)
- Suspending foreground jobs (Ctrl-Z)
- Resuming jobs in foreground (`fg`) or background (`bg`)
- Listing jobs (`jobs`)

## Concepts

### Process Groups
- A group of related processes (e.g., all commands in a pipeline)
- Each job runs in its own process group
- The shell is its own process group

### Session
- A collection of process groups sharing a controlling terminal
- One process group is the **foreground group** (receives terminal input and signals)
- Other groups are **background groups**

### Controlling Terminal
- The terminal associated with the session
- Only the foreground process group receives SIGINT/SIGTSTP from keyboard

```
┌─────────────────────────────────────────────────────────────────┐
│                          SESSION                                 │
├─────────────────────────────────────────────────────────────────┤
│                                                                 │
│  ┌──────────────────────┐    ┌──────────────────────┐          │
│  │   Shell (PGID 1000)  │    │   Job 1 (PGID 1001)  │          │
│  │   [Foreground]       │    │   ls | grep foo      │          │
│  │                      │    │   [Background]       │          │
│  └──────────────────────┘    └──────────────────────┘          │
│                                                                 │
│  ┌──────────────────────┐                                      │
│  │   Job 2 (PGID 1002)  │                                      │
│  │   sleep 100          │                                      │
│  │   [Stopped]          │                                      │
│  └──────────────────────┘                                      │
│                                                                 │
└─────────────────────────────────────────────────────────────────┘
```

## Interface

```c
int     job_control_init(t_shell *shell);
void    job_control_cleanup(t_shell *shell);

t_job   *job_create(t_shell *shell, const char *cmd_line);
void    job_add_process(t_job *job, pid_t pid);

int     job_launch_foreground(t_shell *shell, t_job *job);
int     job_launch_background(t_shell *shell, t_job *job);
int     job_continue_foreground(t_shell *shell, t_job *job);
int     job_continue_background(t_shell *shell, t_job *job);
int     job_wait(t_shell *shell, t_job *job);

void    job_update_statuses(t_shell *shell);
void    job_notify(t_shell *shell);

t_job   *job_find_by_id(t_shell *shell, int id);
t_job   *job_find_by_spec(t_shell *shell, const char *spec);
```

## Initialization

```
job_control_init(shell):
    terminal_fd = STDIN
    interactive = isatty(terminal_fd)
    if not interactive: return (no job control)

    # Wait until we are in the foreground
    while tcgetpgrp(terminal_fd) != getpgrp():
        kill(-getpgrp(), SIGTTIN)

    # Ignore job control signals in shell
    ignore: SIGINT, SIGQUIT, SIGTSTP, SIGTTIN, SIGTTOU

    # Put shell in its own process group and take terminal
    shell_pgid = getpid()
    setpgid(shell_pgid, shell_pgid)
    tcsetpgrp(terminal_fd, shell_pgid)

    # Save terminal attributes
    tcgetattr(terminal_fd, &original_termios)
```

## Launching Jobs

### Foreground

```
job_launch_foreground(shell, job):
    job.foreground = true

    # Give terminal to job's process group
    tcsetpgrp(terminal_fd, job.pgid)

    # Wait for job to complete or stop
    status = job_wait(shell, job)

    # Take terminal back
    tcsetpgrp(terminal_fd, shell_pgid)
    tcsetattr(terminal_fd, TCSADRAIN, &original_termios)

    return status
```

### Background

```
job_launch_background(shell, job):
    job.foreground = false
    print "[job_id] pgid"
    return 0  # don't wait
```

## Waiting for Jobs

```
job_wait(shell, job):
    loop:
        pid = waitpid(-job.pgid, &status, WUNTRACED)
        if pid < 0:
            if errno == ECHILD: break (no more children)
            return error

        find process in job with this pid
        update its status

        if WIFSTOPPED(status):
            mark process as stopped
            job.status = STOPPED
            print "[id]+ Stopped  command"
            return 128 + WSTOPSIG

        if WIFEXITED or WIFSIGNALED:
            mark process as completed
            if all processes in job completed:
                job.status = DONE
                break

    return exit status from last process in job
```

## Continuing Stopped Jobs

```
job_continue_foreground(shell, job):
    job.status = RUNNING
    job.foreground = true
    tcsetpgrp(terminal_fd, job.pgid)
    kill(-job.pgid, SIGCONT)        # send to whole process group
    mark all processes as not stopped
    status = job_wait(shell, job)
    tcsetpgrp(terminal_fd, shell_pgid)
    tcsetattr(terminal_fd, TCSADRAIN, &original_termios)
    return status

job_continue_background(shell, job):
    job.status = RUNNING
    job.foreground = false
    kill(-job.pgid, SIGCONT)
    mark all processes as not stopped
    return 0
```

## Background Job Monitoring

Called before each prompt to check on background jobs:

```
job_update_statuses(shell):
    loop with waitpid(-1, &status, WNOHANG | WUNTRACED):
        while a child has changed state:
            find job/process for this pid
            update process status (completed, stopped, signaled)
            update job status if all processes done
            mark job as needing notification

job_notify(shell):
    for each job:
        if needs notification:
            DONE:       print "[id]+ Done       command"
            TERMINATED: print "[id]+ Terminated command"
            STOPPED:    print "[id]+ Stopped    command"
            mark as notified

        if done/terminated and notified:
            remove job from list, free
```

## Child Process Setup

Every forked child in a job must set up its process group:

```
child_setup_job_control(shell, job, foreground):
    pid = getpid()

    # First child in job sets PGID, others join it
    if job.pgid == 0: job.pgid = pid
    setpgid(pid, job.pgid)

    # If foreground: take terminal
    if foreground and interactive:
        tcsetpgrp(terminal_fd, job.pgid)

    # Restore default signal handlers (shell ignores them)
    restore: SIGINT, SIGQUIT, SIGTSTP, SIGTTIN, SIGTTOU, SIGCHLD to SIG_DFL
```

## Job Spec Parsing

For `fg %1`, `bg %+`, etc:

```
job_find_by_spec(shell, spec):
    if spec is NULL or empty: return current job
    if spec starts with '%':
        %+ or %% or just %  → current job
        %-                   → previous job
        %N (digit)           → job with id N
        %string              → job whose command starts with string
    return NULL if not found
```

## Files

```
src/job_control/
├── job_control.c     # Init, cleanup
├── job.c             # Job creation, management
├── job_launch.c      # Launch foreground/background
├── job_wait.c        # Wait for jobs
├── job_continue.c    # Continue stopped jobs
├── job_notify.c      # Background job notification
└── job_utils.c       # Helper functions, job spec parsing
```
