# Operating System Project 2
## Introduction
A safer `OOM-killer` for Android kernel with **very low overhead**

Course and project information are provided in [handout](./OperatingSystemPrj2/CS356prj2.pdf).
Implementation details are explained in [project report](./OperatingSystemPrj2/ProjectReport.pdf).

- Baseline
    - [x] Implement a custom system call which can set memory limits for `UIDs`
    - [x] Define a global variable to store `UID` and `RSS` information
    - [x] Implement a naive `OOM-killer`triggered by `__alloc_page_nodemask` (not included in this version)

- Bonus
    - [x] Implement a `OOM-killer` triggerd by `timer`
    - [x] Allow users to exceed memory limits for a pre-set time period 

- Something More!
    - [x] Support using `make menueconfig` to add Yimin's `OOM-killer` feature in GUI
    - [x] Make `timer` ***dynamic*** to further decrease overhead
    - [x] Support ***passive detection*** of changes of `UIDs` using ***tracepoints*** to reduce the overhead of `OOM-killer` to the limit

## Special Thanks

  To my inspiring and gifted classmates @[Alex Chi](https://github.com/skyzh) @[Bugen Zhao](https://github.com/bugenzhao)
