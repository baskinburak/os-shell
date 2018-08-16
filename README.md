# A linux shell with tree like pipe mechanisms

This shell allows you to create pipes through which multiple processes can write and read. Every process can read from a single pipe while each process can write to any number of pipes.

* \>| creates an output pipe and the process writes on it. Just write the command, put "\>|" and write numbers that uniquely identify pipes.
* <| creates an input pipe and the process reads from it. Notice that there can be only one pipe process reads from. Just write the command, put "<|" and write numbers that uniquely identify pipes.

Whenever there is a reader and a write for each pipe in the network, all processes run.

## Example Usage


```
  make
  ./shell
  (shell starts)

  ls /tmp >| 12 13
  ls -l /tmp >| 12
  ls -la /usr/bin >| 13
  tr /a-z/ /A-Z/ <| 17
  cut -c 1-7 <| 12 >| 17
  grep a <| 13 >| 17
  quit

```

* The result of ls /tmp gets written to pipes 12 and 13.
* The result of ls -l /tmp gets written to pipe 12.
* The result of ls -la /usr/bin gets written to pipe 13.
* cut -c 1-7 reads from pipe 12 and writes to pipe 17.
* grep a reads from pipe 13 and writes to pipe 17.
* tr /a-z/ /A-Z/ reads from pipe 17 and outputs to stdout.

* quit stops the shell.
