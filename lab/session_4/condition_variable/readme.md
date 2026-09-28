# In this example, I will implement the producer-consumer model using a mutex and a condition variable.

The producer continuously generates data; after each generation, it wakes up the consumer to check for and retrieve the data.

The consumer checks the data: if the value is 40 or greater, it retrieves the data; otherwise, it enters a sleep state and waits to wake up again.

The result when I run program is below:

```text
filled fuel ... 15
no fuel. waiting...
filled fuel ... 30
no fuel. waiting...
filled fuel ... 45
get fuel. Now left: 5
no fuel. waiting...
filled fuel ... 20
no fuel. waiting...
filled fuel ... 35
no fuel. waiting...
filled fuel ... 50
get fuel. Now left: 10
no fuel. waiting...
```
