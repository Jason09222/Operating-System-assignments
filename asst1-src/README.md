# ASST1: Synchronisation

---

## 1. Introduction
This assignment focuses on **synchronisation** and **concurrent programming** within the OS/161 kernel.  
By completing it, you will gain the skills required to write thread-safe concurrent kernel code using semaphores, locks, and condition variables.

You will implement and test four synchronisation problems that represent real OS concurrency challenges.

---

## 2. Setup Instructions

### 2.1 Obtain the ASST1 Source
```bash
% cd ~/cs3231
% git clone https://zXXXXXXX@gitlab.cse.unsw.edu.au/COMP3231/23T1/zXXXXXXX-asst1.git asst1-src
```

### 2.2 Configure and Build
```bash
% cd ~/cs3231/asst1-src
% ./configure && bmake && bmake install
% cd kern/conf
% ./config ASST1
% cd ../compile/ASST1
% bmake depend
% bmake
% bmake install
```

### 2.3 Check sys161.conf
```bash
% cd ~/cs3231/root
% wget http://cgi.cse.unsw.edu.au/~cs3231/23T1/assignments/asst1/sys161.conf
```

### 2.4 Run the Kernel
```bash
% cd ~/cs3231/root
% sys161 kernel
```

You should see a running OS/161 kernel with the ASST1 menu options (e.g., 1a, 1b, 1c, 1d).

---

## 3. Kernel Menu Commands
You can run menu tests automatically using command-line arguments.  
Example:
```bash
sys161 kernel "1a;q"
sys161 kernel "1b;q"
```

> Do not change the provided kernel menu option strings.

---

## 4. Debugging Concurrent Programs
- `thread_yield()` is automatically invoked at random intervals to simulate context switches.  
- To reproduce results, fix the random seed in `sys161.conf` (e.g. `random seed=1`).  
- To explore race conditions, switch to `autoseed` for testing variable timing.

---

## 5. Tutorial and Preparation
Before coding, review the Week 3 tutorial exercises on synchronisation primitives (locks, semaphores, CVs).  
Understanding OS/161’s **thread subsystem** and **wait channels** will help with debugging and reasoning about concurrency.

---

## 6. Assignment Parts

### Part 1: Concurrent Counter (6 marks)
Implement a **thread-safe counter** in `kern/asst1/counter.c` and `counter.h`.

Functions to implement:
- `counter_initialise()`
- `counter_increment()`
- `counter_decrement()`
- `counter_read_and_destroy()`

Test with:
```bash
OS/161 kernel [? for menu]: 1a
```

Expected output:
```
Starting 10 incrementer threads
The final count value was 10000 (expected 10000)
```

---

### Part 2: Simple Deadlock (4 marks)
Modify `kern/asst1/twolocks.c` to apply **resource ordering** to prevent deadlock between threads using two locks (`lock_a`, `lock_b`).

Constraints:
- Maintain the same locks and function calls.  
- Document the lock ordering chosen.  
- Code must terminate without deadlock.

Expected output (order may vary):
```
Locking frenzy starting up
Hi, I'm Bill
Hi, I'm Bruce
Hi, I'm Ben
Hi, I'm Bob
Bruce says 'bye'
Bob says 'bye'
Ben says 'bye'
Bill says 'bye'
Locking frenzy finished
```

---

### Part 3: Bounded Buffer Producer/Consumer (8 marks)
Synchronise the **producer-consumer** circular buffer using locks and condition variables.

Implement in:
- `producerconsumer.c` and `producerconsumer.h`  
- Use FIFO semantics and avoid busy-waiting.

Key functions:
- `producer_send()`
- `consumer_receive()`
- `producerconsumer_startup()`
- `producerconsumer_shutdown()`

Test with:
```bash
OS/161 kernel [? for menu]: 1c
```

Expected behaviour: all producers and consumers finish cleanly with correct data transfer.

---

### Part 4: The Soup Kitchen (12 marks)
Simulate a **soup kitchen** with customer and cook threads.

Functions to implement in `kitchen.c`:
- `fill_bowl()` → only call `get_serving_from_pot()` when soup remains.  
- `do_cooking()` → only call `cook_soup_in_pot()` when the pot is empty.

Requirements:
- No busy-waiting.  
- Use locks and CVs for coordination.  
- Ensure correctness under concurrent access.

Expected output:
```
Starting 20 dining threads who eat 10 serves each
Starting cooking thread
The total number of servings served was 200 (expected 200)
```

---

## 7. Hints and Recommendations
- Prefer **locks + condition variables** to manage shared state.  
- Identify proper waiting and signalling conditions.  
- Use `cv_signal()` vs `cv_broadcast()` carefully.  
- Avoid using disallowed primitives (e.g. spinlocks, thread_yield, atomic ops).

---

## 8. Submission Instructions
Submit via the **CSE give** system as a git bundle:
```bash
% cd ~
% give cs3231 asst1 asst1.bundle
```

The system will perform a **test build and simple execution check**.  
You can resubmit until the deadline, and only the latest successful submission counts.

> If your submission does not compile, you may receive **zero marks**.

---

## 9. Summary
By completing ASST1, you will:
- Implement and test multiple synchronisation mechanisms.  
- Learn how to coordinate threads safely using OS/161 primitives.  
- Debug race conditions, deadlocks, and concurrency timing issues.

> For full details and guidance, refer to the **official COMP3231 ASST1 specification** and tutorial material.
