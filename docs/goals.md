# Goals

## Performance

An important goal is to achieve high performance. This encompasses optimizing for speed, minimizing memory usage, and efficiently managing threads.

The choice of programming language is crucial to this objective. Although no language can guarantee top-tier performance on its own, it significantly influences the maximum performance potential we can reach. In balancing performance and simplicity, we have selected C++ as our language of choice.

Some amount of latency, e.g. communicating with the hardware, will be unavoidable. Our goal will be to keep any additional latency well below a reasonable budget.

Latency goals -
- Measuring: Knowing the latency will help us track and reduce it.
- Budget: IMHO, for competitive gaming <2ms is an acceptable tolerance. We will attempt to keep the additional overhead below that.


## Simplicity & Power

The program should be powerful to express all basic needs from a keyboard remapping software. Please see the "Use Cases" section below.

The configuration system should be as simple and intuitive as it can be, but no simpler.

## Linux First

The current scope of the project is restricted only to Linux.

That said, a large part of the codebase is re-usable OS independent, meaning it may come to other OS eventually.

## Inspiration: KMonad

KMonad is a fantastic tool with great functionality, and if it works well for
you, I encourage you to continue using it!

This new project was inspired by a desire to create an alternative that focuses
on performance and simplicity.

My aim was to develop something specifically for gaming - efficient and
powerful.

As I experimented, this idea evolved into a new project. In addition to enhanced
performance, it now features a user-friendly configuration language that offers
a different approach from KMonad. For these reasons I decided to experiment, and
it grew into a new project. Along with performance, this now also offers a
simple and easy to use config language which is different from kmonad.

### Comparison with KMonad

Both KMonad and KeyShift offer unique advantages tailored to different user
needs. While KMonad excels in various areas, KeyShift aims to provide a more
streamlined experience for gaming. Below is a comparison of some key properties:


|                                 | KMonad              | KeyShift                                         |
| ------------------------------- | ------------------- | ------------------------------------------------ |
| Functionality: Layering support | ✓                   | ✓ E.g. `CAPSLOCK+1=F1`                           |
| Functionality: Dual function    | ✓                   | ✓ E.g. `CAPSLOCK+1=F1;CAPSLOCK+nothing=CAPSLOCK` |
| Functionality: Snap tap         | ✓                   | ✓ E.g. `^A=~D ^A;^D=~A ^D`                       |
| Functionality: Key timeouts     | ✓                   | ✗                                                |
| Threads ¹                       | 34                  | 1                                                |
| RAM ¹                           | 52M (RES)           | < 10M (RES)                                      |
| Latency overhead ²              | Unknown             | < 0.02 ms                                        |
| Language                        | Haskell             | C++ 23                                           |
| OS Support                      | Linux, Windows, Mac | Linux only                                       |

Note 1: Threads & RAM usage were measured on equivalent key-mapping configuration, on same hardware.

Note 2: The profiling of KeyShift is based on average time spent in
`Process(int, int)` on the commit 62f3421, based on an artificial load (see
profile.cpp), on an i9-9900k.

