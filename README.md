# **Understanding POSIX Interval Timers in Linux**

POSIX interval timers are defined in **<time.h>** and managed using system calls such as:

- `timer_create()` → Creates a new timer.
- `timer_settime()` → Starts or modifies an interval timer.
- `timer_gettime()` → Retrieves the remaining time.
- `timer_delete()` → Deletes a timer.

Timers are associated with **real-time signals**, allowing user-defined actions when timers expire.

## **Types of Timers**

POSIX provides several types of timers based on different time sources. Here’s an explanation of the different timer types:

### **`CLOCK_REALTIME`**
- **Purpose**: Represents the system's "wall-clock" time, aligned with real-world time (e.g., `2023-10-01 12:34:56 UTC`).
- **Behavior**: Affected by manual time changes (e.g., admin adjustments) and automatic synchronization (e.g., NTP). Time can jump forward/backward.
- **Use Case**: Timestamps for events, scheduling tasks at specific calendar times.
- **Caveat**: Unsuitable for measuring intervals if the system time might change during execution.

### **`CLOCK_MONOTONIC`**
- **Purpose**: Measures elapsed time from a fixed point (e.g., system boot).
- **Behavior**: Unaffected by manual time changes or NTP slewing. Always increases monotonically.
- **Use Case**: Measuring intervals (e.g., performance timing, timeouts).
- **Caveat**: May not count time when the system is suspended (varies by OS/configuration). Some systems offer variants like `CLOCK_MONOTONIC_RAW` to exclude NTP adjustments.

### **`CLOCK_PROCESS_CPUTIME_ID`**
- **Purpose**: Tracks CPU time consumed by the **entire process** (all threads).
- **Behavior**: Counts time the CPU spends executing the process's instructions, excluding sleep/I/O wait.
- **Use Case**: Profiling CPU usage, optimizing resource-heavy code.
- **Caveat**: Time accumulates across all CPU cores (e.g., 2 CPUs used for 1 second = 2 seconds of CPU time).

### **`CLOCK_THREAD_CPUTIME_ID`**
- **Purpose**: Tracks CPU time consumed by a **single thread**.
- **Behavior**: Similar to `CLOCK_PROCESS_CPUTIME_ID`, but per-thread.
- **Use Case**: Debugging/optimizing multi-threaded applications.
- **Caveat**: Requires thread-specific system calls (e.g., `clock_gettime()` with a thread ID).

---

### **Key Differences**

| Clock Type | Time Source | Adjustable? | Use Case |
| --- | --- | --- | --- |
| `CLOCK_REALTIME` | System clock (real-world time) | Yes | Timestamps, calendar scheduling |
| `CLOCK_MONOTONIC` | Time since boot | No | Interval measurement |
| `CLOCK_PROCESS_CPUTIME_ID` | Process CPU execution time | No | Process-level profiling |
| `CLOCK_THREAD_CPUTIME_ID` | Thread CPU execution time | No | Thread-level profiling |

---

### **Example Code (Linux)**

Here’s a simple example in C that demonstrates how to use `clock_gettime()` to retrieve the current time from different clocks:

```c
#include <time.h>
#include <stdio.h>

int main() {
    struct timespec ts;
    struct timespec res;

    // CLOCK_REALTIME example
    clock_gettime(CLOCK_REALTIME, &ts);
    clock_getres(CLOCK_REALTIME, &res);
    printf("Real-time: %ld seconds, %ld nanoseconds\n", ts.tv_sec, ts.tv_nsec);
    printf("Real-time resolution: %ld seconds, %ld nanoseconds\n", res.tv_sec, res.tv_nsec);

    // CLOCK_MONOTONIC example
    clock_gettime(CLOCK_MONOTONIC, &ts);
    clock_getres(CLOCK_MONOTONIC, &res);
    printf("Monotonic: %ld seconds since boot\n", ts.tv_sec);
    printf("Monotonic resolution: %ld seconds, %ld nanoseconds\n", res.tv_sec, res.tv_nsec);

    return 0;
}
```

---

### **Notes**
- **OS Support**: `CLOCK_MONOTONIC` and `CLOCK_REALTIME` are widely supported. CPU/time-thread clocks may require specific OS/hardware support.
- **Resolution**: Use `clock_getres()` to check timer precision (nanoseconds on modern systems).
- **Alternatives**: Functions like `gettimeofday()` (obsolete) or `std::chrono` (C++) often wrap these clocks.

---


# **POSIX Timers and Process Inheritance**

POSIX timers are **not inherited** by a child process created via `fork()`, and they are **disarmed and deleted** when executing a new program via `exec()`. Here's why:

1. **Not Inherited by `fork()`**:
   - When a process creates a timer using `timer_create()`, the timer is **associated with that process**.
   - If the process calls `fork()`, the child process gets a copy of the parent’s memory, **but not the timers**.
   - The reasoning is that timers are linked to kernel resources (e.g., signal delivery to the parent process). Inheriting them could lead to unexpected behaviors.
   - This means that if the child process needs a timer, it must create its own timers.

2. **Timers are Disarmed and Deleted on `exec()`**:
   - When `exec()` is called, the current process image is **completely replaced** with a new program.
   - Since timers belong to the **old process image**, they are **automatically deleted**.
   - This prevents unwanted behavior where timers from an old program interfere with a new one.

3. **Timers are Deleted on Process Termination**:
   - If a process terminates (either normally or abnormally), all its timers are automatically deleted by the kernel.
   - This cleanup ensures that timers do not persist and interfere with other processes.

---

## **`lrt` is Required on Linux**

On Linux, POSIX timers are implemented in the **Realtime Library (`librt`)**, separate from the standard C library (`libc`).

- The **Realtime Extensions (librt)** provide additional functionalities such as **POSIX timers, message queues, and shared memory**.
- Since `timer_create()` and related functions are **not part of the default C library**, programs using POSIX timers **must explicitly link against `librt`** when compiling.

### **Example Compilation Command**

```
gcc my_timer_program.c -o my_timer_program -lrt
```

Without `-lrt`, you might see linker errors like:

```
undefined reference to `timer_create'
```

---

# **`timer_create()` System Call**

The `timer_create()` function is used to create a POSIX interval timer.

### **Function Signature**

```c
int timer_create(clockid_t clockid, struct sigevent *sevp, timer_t *timerid);
```

### **Arguments Explained**

1. **`clockid`** (`clockid_t`):
   - **Purpose**: Specifies the clock source the timer will use.
   - **Common Values**:
     - `CLOCK_REALTIME`: Wall-clock time (affected by system time changes).
     - `CLOCK_MONOTONIC`: Steady time since system boot (unaffected by time adjustments).
     - `CLOCK_PROCESS_CPUTIME_ID`: CPU time consumed by the process.
     - `CLOCK_THREAD_CPUTIME_ID`: CPU time consumed by a specific thread.
   - **Behavior**: The timer’s expiration is based on the chosen clock. For example:
     - Timers using `CLOCK_MONOTONIC` are ideal for measuring intervals.
     - Timers using `CLOCK_REALTIME` are tied to the system clock (e.g., for calendar-based events).

   Additionally, instead of using a predefined clock, `clockid` can be obtained dynamically using:

   - **`clock_getcpuclockid(pid_t pid)`**: Returns the `clockid` associated with a **specific process' CPU time**.
   - **`pthread_getcpuclockid(pthread_t thread)`**: Returns the `clockid` associated with a **specific thread’s CPU time**.

   **Example Usage: CPU time consumed by the process.**

   ```c
   #define _XOPEN_SOURCE 600
   #include <stdint.h>
   #include <stdio.h>
   #include <unistd.h>
   #include <stdlib.h>
   #include <time.h>

   int main(int argc, char *argv[])
   {
       clockid_t clockid;
       struct timespec ts;

       if (argc != 2) {
           fprintf(stderr, "%s <process-ID>\n", argv[0]);
           exit(EXIT_FAILURE);
       }

       if (clock_getcpuclockid(atoi(argv[1]), &clockid) != 0) {
           perror("clock_getcpuclockid");
           exit(EXIT_FAILURE);
       }

       if (clock_gettime(clockid, &ts) == -1) {
           perror("clock_gettime");
           exit(EXIT_FAILURE);
       }

       printf("CPU-time clock for PID %s is %jd.%09ld seconds\n",
              argv[1], (intmax_t) ts.tv_sec, ts.tv_nsec);
       exit(EXIT_SUCCESS);
   }
   ```

---

2. **`sevp`** (`struct sigevent *`)

- **Purpose**: Defines how the timer notifies the process when it expires.
- **Structure** (`struct sigevent`):
    ```c
    struct sigevent {
      int          sigev_notify;              // Notification method
      int          sigev_signo;               // Signal number (if using signals)
      union sigval sigev_value;               // Data passed to handler/thread
      void       (*sigev_notify_function)(union sigval); // Thread function
      void        *sigev_notify_attributes;   // Thread attributes (for SIGEV_THREAD)
      pid_t        sigev_notify_thread_id;    // Target thread ID (Linux-specific)
    };
    ```

- **Key Fields**:
  - **`sigev_notify`**: Specifies the notification mechanism:
      - `SIGEV_NONE`: No notification (timer expiration is silent).
      - `SIGEV_SIGNAL`: Send a signal (e.g., `SIGALRM`) on expiration.
      - `SIGEV_THREAD`: Invoke a thread function (`sigev_notify_function`) asynchronously.
      - `SIGEV_THREAD_ID` (Linux-specific): Send a signal to a specific thread.
    
      | Feature | `SIGEV_THREAD` | `SIGEV_THREAD_ID` |
      | --- | --- | --- |
      | Notification Mechanism | Calls a user-defined function in a thread-like context. | Sends a signal to a specific thread. |
      | Thread Context | Can create new threads or reuse a single thread. | No new threads are created; uses existing threads. |
      | Signal Usage | No actual signals involved. | Uses actual signals (e.g., `SIGUSR1`). |
      | Custom Data (`sigev_value`) | Passed as argument to the function. | Accessible via `siginfo_t` in the handler. |
      | Use Case | For event handling where threading is suitable. | For precise signal delivery to specific threads. |
  
  - **`sigev_signo`**: The signal number sent when the timer expires (e.g., `SIGALRM`, `SIGUSR1`), used when `sigev_notify` is `SIGEV_SIGNAL`.
  - **`sigev_value`**: User-defined data (passed to the signal handler or thread function).
      - A `union sigval` that holds accompanying data (integer or pointer) to pass to the signal handler or thread function.
  
  - **Example Usage**:
    ```c
    struct sigevent sevp = {
      .sigev_notify = SIGEV_SIGNAL,
      .sigev_signo = SIGALRM,
      .sigev_value.sival_int = 42 // Optional data
    };
    ```
  
---

### **`union sigval`**

The `union sigval` is used in the `struct sigevent` to pass data to a signal handler or thread function when a timer expires. Here’s its definition:

```c
union sigval {
  int    sival_int;   // Integer value
  void  *sival_ptr;   // Pointer value
};
```

### Example Usage:
- **Passing an integer**:

    ```c
    union sigval val_int;
    val_int.sival_int = 42;
    ```

- **Passing a pointer to a custom structure**:

    ```c
    typedef struct {
      int id;
      char msg[64];
    } CustomData;
    
    CustomData data = {.id = 1, .msg = "Timer expired!"};
    union sigval val_ptr;
    val_ptr.sival_ptr = &data;
    ```

---

3. **`timerid`** (`timer_t *`):

A **pointer to a buffer** where the kernel will store the created timer’s identifier.

- **Purpose**: Output parameter to store the ID of the newly created timer.
- **Behavior**:
    - On success, `timerid` points to a unique timer identifier.
    - This ID is used in other timer functions (e.g., `timer_settime()`, `timer_delete()`).

---

### **Return Value**

- **Success**: Returns `0`, and `timerid` is populated.
- **Failure**: Returns `1`, and `errno` is set to indicate the error (e.g., `EINVAL` for invalid `clockid`).

### **Example Usage**

```c
timer_t my_timer;
timer_create(CLOCK_REALTIME, NULL, &my_timer);
```

Here, `my_timer` will store the handle of the newly created timer, allowing it to be used in future timer-related calls.

---

### **Example**

```c
#include <signal.h>
#include <time.h>
#include <stdio.h>

void timer_handler(union sigval val) {
    printf("Timer expired! Value: %d\n", val.sival_int);
}

int main() {
    timer_t timerid;
    struct sigevent sevp = {
        .sigev_notify = SIGEV_THREAD,
        .sigev_notify_function = timer_handler,
        .sigev_value.sival_int = 1234
    };

    // Create a timer using CLOCK_MONOTONIC
    if (timer_create(CLOCK_MONOTONIC, &sevp, &timerid) == -1) {
        perror("timer_create");
        return 1;
    }

    // Configure and start the timer with timer_settime()...
    return 0;
}
```

---

### **Notes**

- **Thread Safety**: Use `SIGEV_THREAD` to run a function in a new thread on expiration.
- **Signal Handling**: For `SIGEV_SIGNAL`, ensure the signal is unblocked and has a handler.
- **Linux-Specific**: `SIGEV_THREAD_ID` allows targeting a specific thread (requires `sigev_notify_thread_id`).
- **Memory Leaks**: Always use `timer_delete()` to free timer resources.

---

### **Default Behavior When `evp` is `NULL`**

- If `evp == NULL`, the system uses default settings for notification. These defaults are:
    - **`sigev_notify = SIGEV_SIGNAL`**: The system sends a signal when the timer expires.
    - **`sigev_signo = SIGALRM`**: The signal sent is `SIGALRM`. (This signal typically indicates alarm clock notifications.)
        - **Note**: The signal number might vary on different systems because SUSv3 (Single UNIX Specification version 3) does not strictly specify which signal should be used as the default.
    - **`sigev_value.sival_int = timer_id`**: The `sival_int` field in the `sigval` union will contain the timer ID. This means the signal handler can use this value to determine which timer expired if multiple timers are in use.

---

### **Implications of Default Behavior**

If you don't explicitly provide a `struct sigevent` (by passing `NULL`), you:

- **Automatically get signal-based notifications.**
- **Rely on the `SIGALRM` signal.**
    - This could cause unintended conflicts if your application already uses `SIGALRM` for other purposes.
- **Cannot customize notification behavior.**
    - For example, you can’t send a different signal, execute a thread function, or disable notifications (e.g., `SIGEV_NONE`).

---

### **Example of Default Behavior**

Here’s an example of what happens when you pass `NULL` for `evp`:

```c
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <time.h>
#include <unistd.h>

void signal_handler(int sig, siginfo_t *si, void *uc) {
    printf("Signal %d received. Timer ID: %d\n", sig, si->si_value.sival_int);
}

int main() {
    struct sigaction sa;
    timer_t timerid;

    // Set up the signal handler for SIGALRM
    sa.sa_flags = SA_SIGINFO;
    sa.sa_sigaction = signal_handler;
    sigemptyset(&sa.sa_mask);

    if (sigaction(SIGALRM, &sa, NULL) == -1) {
        perror("sigaction");
        exit(EXIT_FAILURE);
    }

    // Create a timer with default behavior
    if (timer_create(CLOCK_REALTIME, NULL, &timerid) == -1) {
        perror("timer_create");
        exit(EXIT_FAILURE);
    }

    // Set the timer to expire after 2 seconds
    struct itimerspec its;
    its.it_value.tv_sec = 2;
    its.it_value.tv_nsec = 0;
    its.it_interval.tv_sec = 0;
    its.it_interval.tv_nsec = 0;

    if (timer_settime(timerid, 0, &its, NULL) == -1) {
        perror("timer_settime");
        exit(EXIT_FAILURE);
    }

    printf("Timer set. Waiting for expiration...\n");

    // Wait for the signal
    pause();
    return 0;
}
```

**Output**:

```
Timer set. Waiting for expiration...
Signal 14 received. Timer ID: 12345678
```

---

### **5. Summary of Defaults**

- If `evp` is `NULL`, a signal-based notification is used (`SIGEV_SIGNAL`).
- The signal sent is `SIGALRM`, but this may vary by system.
- The timer ID is passed in `sigev_value.sival_int`, making it accessible in the signal handler.

Passing a `NULL` `evp` is convenient for simple timer-based applications but limits your control over the notification mechanism. If you need customized behavior (e.g., specific signals, thread execution, or no notification), you should provide a properly initialized `struct sigevent`.
---


# **`sigwaitinfo()` and `sigtimedwait()`**

Both `sigwaitinfo()` and `sigtimedwait()` are system calls in Linux that allow a program to **synchronously wait for signals** and retrieve detailed information about the signals received. They are alternatives to asynchronous signal handling using `sigaction()` or `signal()`.

---

### **1. `sigwaitinfo()`**

#### **Purpose**:

- Waits for a signal specified in a signal set and retrieves detailed information about it.
- Unlike a signal handler, it does not interrupt program flow. Instead, the program pauses and waits for a signal synchronously.

#### **Function Signature**:

```c
#include <signal.h>

int sigwaitinfo(const sigset_t *set, siginfo_t *info);
```

- **`set`**: Specifies the set of signals the process is waiting for (e.g., `SIGUSR1`, `SIGUSR2`).
- **`info`**: Pointer to a `siginfo_t` structure that will hold information about the received signal.

#### **Returns**:

- On success: The signal number that was received.
- On failure: `-1`, and `errno` is set appropriately.

#### **Use Case**:

`sigwaitinfo()` is typically used when you want **detailed signal information** (e.g., custom data passed through `si_value`) but do not want to use a signal handler.

---

### **2. `sigtimedwait()`**

#### **Purpose**:

- Similar to `sigwaitinfo()`, but allows specifying a **timeout** for waiting on signals.
- Useful when you want to wait for a signal but avoid blocking indefinitely.

#### **Function Signature**:

```c
#include <signal.h>
#include <time.h>

int sigtimedwait(const sigset_t *set, siginfo_t *info, const struct timespec *timeout);
```

- **`set`**: Specifies the set of signals to wait for.
- **`info`**: Pointer to a `siginfo_t` structure to retrieve signal details.
- **`timeout`**: A `timespec` structure that specifies how long to wait for a signal.

#### **Returns**:

- On success: The signal number that was received.
- On timeout: `-1`, and `errno` is set to `EAGAIN`.

#### **Use Case**:

`sigtimedwait()` is used when you want **non-blocking behavior** while waiting for signals, so the program can proceed after a specific period if no signal arrives.

---

### **Detailed Signal Information: `siginfo_t`**

Both functions fill a `siginfo_t` structure with details about the received signal. This structure includes:

- **`si_signo`**: The signal number.
- **`si_code`**: The source of the signal (e.g., `SI_USER`, `SI_TIMER`, etc.).
- **`si_value`**: Custom data attached to the signal (set by `sigev_value` or `sigqueue()`).
- **`si_pid`**: PID of the process that sent the signal (if applicable).
- **`si_uid`**: UID of the sending process (if applicable).

---

### **Example: `sigwaitinfo()`**

#### Scenario:

Wait for a `SIGUSR1` signal and retrieve its associated custom value.

```c
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>

int main() {
    sigset_t set;
    siginfo_t info;

    // Block SIGUSR1 and add it to the signal set
    sigemptyset(&set);
    sigaddset(&set, SIGUSR1);
    sigprocmask(SIG_BLOCK, &set, NULL); // Block SIGUSR1

    printf("Waiting for SIGUSR1...\n");

    // Wait for SIGUSR1 synchronously
    if (sigwaitinfo(&set, &info) == -1) {
        perror("sigwaitinfo");
        exit(EXIT_FAILURE);
    }

    // Print details about the received signal
    printf("Received signal: %d\n", info.si_signo);
    printf("Custom value (int): %d\n", info.si_value.sival_int);
    printf("Custom value (ptr): %p\n", info.si_value.sival_ptr);

    return 0;
}
```

---

### **Example: `sigtimedwait()`**

#### Scenario:

Wait for a signal for up to 5 seconds.

```c
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <time.h>

int main() {
    sigset_t set;
    siginfo_t info;
    struct timespec timeout;

    // Block SIGUSR1 and add it to the signal set
    sigemptyset(&set);
    sigaddset(&set, SIGUSR1);
    sigprocmask(SIG_BLOCK, &set, NULL); // Block SIGUSR1

    // Set timeout to 5 seconds
    timeout.tv_sec = 5;
    timeout.tv_nsec = 0;

    printf("Waiting for SIGUSR1 with timeout...\n");

    // Wait for SIGUSR1 or timeout
    int result = sigtimedwait(&set, &info, &timeout);
    if (result == -1) {
        if (errno == EAGAIN) {
            printf("Timeout reached, no signal received.\n");
        } else {
            perror("sigtimedwait");
        }
        exit(EXIT_FAILURE);
    }

    // Print details about the received signal
    printf("Received signal: %d\n", info.si_signo);
    printf("Custom value (int): %d\n", info.si_value.sival_int);

    return 0;
}
```

---

### **Key Differences**

| Feature | `sigwaitinfo()` | `sigtimedwait()` |
| --- | --- | --- |
| Blocking behavior | Blocks indefinitely. | Waits for a signal or timeout. |
| Timeout support | Not supported. | Supported (via `timeout`). |
| Use case | Simple, synchronous signal wait. | Non-blocking signal wait. |

---

Here’s how you can add the new section to your README:

---

# **What happens when a timer is created?**

- For each POSIX timer created using `timer_create()`, the kernel **preallocates one queued realtime signal structure**.
    - This structure is used to store the information necessary to queue a signal when the timer expires.
    - By preallocating this resource, the kernel guarantees that the signal for that timer can always be queued, even under heavy system load or signal queue saturation.

---

### **Why does the kernel preallocate this structure?**

- **Ensures reliability:** The preallocation ensures that the system can always handle the timer expiration notification, even if the signal queue is full when the timer expires. Without this preallocation, the signal might fail to be queued, resulting in unreliable behavior.
- **Real-time guarantees:** This aligns with the requirements of real-time systems, where timely and predictable behavior is critical.

---

### **How does this affect the number of timers?**

- The number of timers you can create is **limited by the maximum number of realtime signals that can be queued**.
    - This limit is controlled by the system parameter **`RLIMIT_SIGPENDING`**, which specifies the maximum number of signals that can be queued for a user.
    - If this limit is reached, no more timers can be created because the kernel cannot preallocate a signal structure for them.

---

### **Practical Implications**

- **System-wide limit:** The limit on realtime signal queuing applies system-wide for all processes belonging to a user. If many signals are queued or many POSIX timers are created, the limit may be exhausted.
- **Resource exhaustion:** If the limit is reached, attempts to create new timers using `timer_create()` will fail, typically returning an error (`ENOMEM`).

---

### **5. How can you check or adjust the limit?**

- **Check the current limit:** Use the `ulimit` command or inspect `/proc/<pid>/limits` to see the `Max pending signals` value.

or
    
    ```bash
    ulimit -i
    ```
    
    ```bash
    cat /proc/self/limits | grep "Max pending signals"
    ```
    
- **Adjust the limit:** You can increase the limit using the `ulimit` command (for temporary changes) or by modifying system-wide configuration files (for permanent changes).
    
    ```bash
    ulimit -i <new_limit>
    ```
    

---

### **Summary**

- **POSIX timer creation depends on the availability of realtime signal structures.**
- **The number of timers is constrained by the `RLIMIT_SIGPENDING` limit on queued realtime signals.**
- **Preallocation ensures reliable operation for each timer's notification.**
- If your application requires many timers, you may need to monitor and adjust the system's signal queue limits to accommodate them.

---

# `si_timerid` in Linux

In Linux, the `siginfo_t` structure includes a nonstandard field called `si_timerid`, which is different from the `timerid` returned by `timer_create()`. This field is an **internal kernel identifier** for the timer and is **not meant for use by applications**.

### **Key Points About `si_timerid`**

1. **Internal Kernel Use**:
    - The kernel assigns `si_timerid` to uniquely track a timer internally.
    - It is **not the same** as the `timer_t` handle returned by `timer_create()`.
2. **Not Part of POSIX Standard**:
    - The POSIX standard (`SUSv3` and later) does **not** specify `si_timerid`, making it a Linux-specific extension.
    - Portable applications should avoid using it.
3. **Why Is It Not Useful to Applications?**
    - Applications interact with timers using the `timer_t` identifier provided by `timer_create()`.
    - The kernel uses `si_timerid` internally to manage the system's timer queue.
    - It is not exposed in user-space APIs, so there is no documented way to use it meaningfully in an application.

---

### **Example: Why You Should Use `si_value.sival_ptr` Instead of `si_timerid`**

Instead of relying on `si_timerid`, you should use `sigev_value.sival_ptr` to associate custom data with a timer.

```c
#include <stdio.h>
#include <signal.h>
#include <time.h>
#include <stdlib.h>

typedef struct {
    timer_t timerid;
    int custom_data;
} TimerInfo;

void timer_handler(int sig, siginfo_t *si, void *uc) {
    TimerInfo *info = (TimerInfo *)si->si_value.sival_ptr;
    printf("Timer expired!\n");
    printf("Application Timer ID: %ld\n", (long)info->timerid);
    printf("Custom Data: %d\n", info->custom_data);
}

int main() {
    struct sigevent sev;
    struct sigaction sa;
    struct itimerspec its;
    TimerInfo *info = malloc(sizeof(TimerInfo));  // Allocate memory for TimerInfo

    // Set up signal handler
    sa.sa_flags = SA_SIGINFO;
    sa.sa_sigaction = timer_handler;
    sigemptyset(&sa.sa_mask);
    sigaction(SIGRTMIN, &sa, NULL);

    // Set up sigevent structure
    sev.sigev_notify = SIGEV_SIGNAL;
    sev.sigev_signo = SIGRTMIN;
    sev.sigev_value.sival_ptr = info;  // Pass structure pointer

    // Create timer
    timer_create(CLOCK_REALTIME, &sev, &info->timerid);
    info->custom_data = 42;

    // Start timer (2 seconds)
    its.it_value.tv_sec = 2;
    its.it_value.tv_nsec = 0;
    its.it_interval.tv_sec = 0;
    its.it_interval.tv_nsec = 0;
    timer_settime(info->timerid, 0, &its, NULL);

    printf("Waiting for timer...\n");
    pause();  // Wait for signal

    free(info);  // Clean up memory
    return 0;
}

```

---

### **Summary**

- `si_timerid` is **internal** and should not be used in applications.
- Instead, applications should use `sival_ptr` in `sigev_value` to track timers.
- `timer_t` returned by `timer_create()` is the correct way to reference timers in user space.

---

Here’s how you can add the new section to your README:

---

# **`timer_settime()`**

**Purpose:** Sets (starts or modifies) the expiration time of a timer.

**Syntax:**

```c
int timer_settime(timer_t timerid, int flags,
                  const struct itimerspec *new_value,
                  struct itimerspec *old_value);
```

**Parameters:**

- `timerid` – The timer ID returned by `timer_create()`.
- `flags` –
    - `0` (default): The time is set relative to the current time.
    - `TIMER_ABSTIME`: The time is set as an absolute time (depends on the selected clock).
- `new_value` – Specifies the new expiration time and interval.
- `old_value` – If non-null, stores the previous timer settings.

**Behavior:**

- If `it_value` (expiration time) is nonzero, the timer starts or resets.
- If `it_interval` is nonzero, the timer reloads and repeats after expiration (periodic mode).

**Example:**

```c
struct itimerspec ts;
ts.it_value.tv_sec = 5;   // Expire in 5 seconds
ts.it_value.tv_nsec = 0;
ts.it_interval.tv_sec = 2; // Repeat every 2 seconds
ts.it_interval.tv_nsec = 0;
timer_settime(timerid, 0, &ts, NULL);
```

---

# **`timer_gettime()`**

**Purpose:** Retrieves the remaining time until the next expiration of a timer.

**Syntax:**

```c
int timer_gettime(timer_t timerid, struct itimerspec *curr_value);
```

**Parameters:**

- `timerid` – The timer ID returned by `timer_create()`.
- `curr_value` – Stores the remaining time until expiration and the interval.

**Example:**

```c
struct itimerspec ts;
timer_gettime(timerid, &ts);
printf("Time remaining: %ld sec, %ld nsec\n", ts.it_value.tv_sec, ts.it_value.tv_nsec);
```

---

# **`timer_delete()`**

**Purpose:** Deletes a previously created timer, stopping it and freeing resources.

**Syntax:**

```c
int timer_delete(timer_t timerid);
```

**Parameters:**

- `timerid` – The timer ID returned by `timer_create()`.

**Example:**

```c
timer_delete(timerid);
```

- After this call, the `timerid` is no longer valid.

---

# **`timer_getoverrun()`**

**Purpose:** Returns the number of missed expirations due to process scheduling delays.

**Syntax:**

```c
int timer_getoverrun(timer_t timerid);
```

**Parameters:**

- `timerid` – The timer ID returned by `timer_create()`.

**Behavior:**

- If a timer expires multiple times before the process can handle the signal, this function reports how many expirations were missed.
- If no overruns occurred, it returns `0`.
- If the implementation cannot track overruns, it returns `1`.

**Example:**

```c
int overrun = timer_getoverrun(timerid);
if (overrun > 0) {
    printf("Missed %d timer expirations\n", overrun);
}
```

---


# Timer Examples with Signal Handling and Timer Overruns

This program demonstrates the use of POSIX timers in combination with signals and the detection of timer overruns.

### **Example of Using a Timer with a Signal**

```c
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <time.h>
#include <unistd.h>

// Timer handler function
void timer_handler(int sig, siginfo_t *si, void *uc) {
    // Accessing the value passed to the signal handler (sival_int)
    printf("Timer expired! Signal %d received.\n", sig);
    printf("The passed value is: %d\n", si->si_value.sival_int);
}

int main() {
    struct sigevent sev;
    struct sigaction sa;
    struct itimerspec ts;
    timer_t timerid;

    // Set up the signal handler with additional information (siginfo_t)
    sa.sa_sigaction = timer_handler;  // Use the handler function with siginfo_t
    sa.sa_flags = SA_SIGINFO;         // Ensure we get extra signal info
    sigemptyset(&sa.sa_mask);         // No additional signals are blocked
    if (sigaction(SIGUSR1, &sa, NULL) == -1) {
        perror("sigaction");
        exit(EXIT_FAILURE);
    }

    // Set up the sigevent structure to send SIGUSR1 signal with value
    sev.sigev_notify = SIGEV_SIGNAL;      // Notification type: signal
    sev.sigev_signo = SIGUSR1;            // Signal to send: SIGUSR1
    sev.sigev_value.sival_int = 123;      // Pass integer value (123) to signal handler

    // Create the timer
    if (timer_create(CLOCK_REALTIME, &sev, &timerid) == -1) {
        perror("timer_create");
        exit(EXIT_FAILURE);
    }

    // Set the timer to expire after 3 seconds
    ts.it_value.tv_sec = 3;      // First expiration after 3 seconds
    ts.it_value.tv_nsec = 0;     // No additional nanoseconds
    ts.it_interval.tv_sec = 0;   // No periodic expiration
    ts.it_interval.tv_nsec = 0;

    // Start the timer
    if (timer_settime(timerid, 0, &ts, NULL) == -1) {
        perror("timer_settime");
        exit(EXIT_FAILURE);
    }

    printf("Timer set. Waiting for signal...\n");

    // Wait for the signal (SIGUSR1)
    pause();  // Blocks and waits for signals

    printf("Program exiting.\n");
    return 0;
}
```

---

### **Key Explanation**

1. **Signal Configuration**:
    - Uses SIGUSR1 for timer notifications.
    - Sets up a `sigaction()` handler to process the signal and retrieve additional information via `siginfo_t`.
2. **Timer Creation**:
    - Creates a timer that will notify the process using a signal when it expires.
    - Timer set to expire after 3 seconds.
3. **Timer Specification**:
    - Timer setup specifies no periodic interval for expiration.
4. **Handler Implementation**:
    - The handler prints out the signal and the passed value.

---

### **Additional Examples**

#### Example 2:

```c
#define _POSIX_C_SOURCE 199309
#include <signal.h>
#include <time.h>
#include "curr_time.h"      /* Custom time formatting function */
#include "itimerspec_from_str.h" /* For parsing timer specs */
#include "tlpi_hdr.h"       /* Error handling functions */

/* Signal used for timer notifications */
#define TIMER_SIG SIGRTMAX  /* Use highest real-time signal */

static void handler(int sig, siginfo_t *si, void *uc) {
    timer_t *tidptr;
    tidptr = si->si_value.sival_ptr;

    printf("[%s] Got signal %d\n", currTime("%T"), sig);
    printf("    *sival_ptr = %ld\n", (long)*tidptr);
    printf("    timer_getoverrun() = %d\n", timer_getoverrun(*tidptr));
}

int main(int argc, char *argv[]) {
    struct itimerspec ts;
    struct sigaction sa;
    struct sigevent sev;
    timer_t *tidlist;
    int j;

    if (argc < 2)
        usageErr("%s secs[/nsecs][:int-secs[/int-nsecs]]...\n", argv[0]);

    tidlist = calloc(argc - 1, sizeof(timer_t));
    if (tidlist == NULL)
        errExit("malloc");

    sa.sa_flags = SA_SIGINFO;
    sa.sa_sigaction = handler;
    sigemptyset(&sa.sa_mask);

    if (sigaction(TIMER_SIG, &sa, NULL) == -1)
        errExit("sigaction");

    sev.sigev_notify = SIGEV_SIGNAL;
    sev.sigev_signo = TIMER_SIG;

    for (j = 0; j < argc - 1; j++) {
        itimerspecFromStr(argv[j + 1], &ts);
        sev.sigev_value.sival_ptr = &tidlist[j];

        if (timer_create(CLOCK_REALTIME, &sev, &tidlist[j]) == -1)
            errExit("timer_create");

        printf("Timer ID: %ld (%s)\n", (long)tidlist[j], argv[j + 1]);

        if (timer_settime(tidlist[j], 0, &ts, NULL) == -1)
            errExit("timer_settime");
    }

    for (;;)
        pause();
}
```

---

### **Key Explanation**

1. **Signal Handling**: Uses `SIGRTMAX` to avoid conflicts with standard signals. The signal handler processes the timer expiration and checks for overruns.
2. **Timers Creation**: Multiple timers are created from command-line arguments using `itimerspecFromStr`.
3. **Overrun Detection**: Utilizes `timer_getoverrun()` to track how many times a timer expired without being handled.

---

### **itimerspecFromStr**

This function parses a string representing timer values and populates a `struct itimerspec` with the parsed values.

#### **Input Format**

```
"value.sec[/value.nanosec][:interval.sec[/interval.nanosec]]"
```

- `value.sec`: Required, specifies seconds for the timer's initial expiration.
- `/value.nanosec`: Optional, specifies nanoseconds for the initial expiration.
- `:interval.sec`: Optional, specifies seconds for the periodic interval.
- `/interval.nanosec`: Optional, specifies nanoseconds for the periodic interval.

#### **Steps of the Function**

1. Parse the initial expiration and interval, handling the separators (`/` and `:`).
2. Defaults are assigned if parts of the string are missing.

```c
void itimerspecFromStr(char *str, struct itimerspec *tsp) {
    char *cptr, *sptr;
    cptr = strchr(str, ':');
    if (cptr != NULL) *cptr = '\0';

    sptr = strchr(str, '/');
    if (sptr != NULL) *sptr = '\0';

    tsp->it_value.tv_sec = atoi(str);
    tsp->it_value.tv_nsec = (sptr != NULL) ? atoi(sptr + 1) : 0;

    if (cptr == NULL) {
        tsp->it_interval.tv_sec = 0;
        tsp->it_interval.tv_nsec = 0;
    } else {
        sptr = strchr(cptr + 1, '/');
        if (sptr != NULL) *sptr = '\0';
        tsp->it_interval.tv_sec = atoi(cptr + 1);
        tsp->it_interval.tv_nsec = (sptr != NULL) ? atoi(sptr + 1) : 0;
    }
}
```

#### **Example Usage**

```c
char *input = "5/500000000:2/100000000";
struct itimerspec ts;
itimerspecFromStr(input, &ts);
```

---

### **Timer Overruns**

Timer overruns occur when multiple expirations happen before the process handles the signal for one of the expirations.

1. **Cause**: Process scheduling delays or blocked signals.
2. **Detection**: 
    - Use `timer_getoverrun()` to retrieve the number of missed expirations.

#### **Why Can't Realtime Signals Solve This Problem?**

- Realtime signals are queued, but they have a limited queue size.
- **Solution**: Use `timer_getoverrun()` to detect missed expirations without queueing signals.

#### **Detecting Timer Overruns**

```c
int overrun_count = timer_getoverrun(timerid);
```

Alternatively, `si_overrun` in `siginfo_t` can be used on Linux systems for a more efficient detection method.

---

### **Key Takeaways**

- **Timer Overruns**: Ensure that timer expirations are accounted for, even if signals are delayed.
- **Overrun Detection**: Use `timer_getoverrun()` or `si_overrun` for handling missed expirations.
- **Signal Handling**: Use `sigaction()` to capture extended signal information, especially for timers.

Here’s a template for writing closing statements for a repository:

---

## Closing Remarks

Thank you for exploring this repository! We hope that you found it useful and that it serves your needs. If you have any questions, suggestions, or run into issues, feel free to contact me.It's best not to share personal contact information like email publicly for privacy and security reasons. But if you're looking to provide a contact email in your repository, you could include it in the closing remarks like this:

---

## Contact

For inquiries or suggestions, feel free to reach out to me at [baselinux2024@gmail.com](mailto:baselinux2024@gmail.com).
