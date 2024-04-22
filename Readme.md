# Contributors
1. Vejandala Bhavya Teja - 20CS01022
2. Rishithaa Gottipati - 20CS01023
3. Sakshi Shashikant Yeole - 20CS01047

# Theory
**Lamport’s Distributed Mutual Exclusion Algorithm** is a permission based algorithm proposed by Lamport as an illustration of his synchronization scheme for distributed systems. In this scheme, permission based timestamp is used to order critical section requests and to resolve any conflict between requests. In Lamport’s Algorithm critical section requests are executed in the increasing order of timestamps i.e a request with smaller timestamp will be given permission to execute critical section first than a request with larger timestamp.


# Implementation
Our implementation is a C++ realization of Lamport's Distributed Mutual Exclusion algorithm where N number of processes share a critical section which only one should access at a time.Our implementation uses sockets to communicate using the TCP protocol to exchange messages.These messages are of three kinds:
Request(Q): A process which wants to access the critical section sends this message to every other peer,requesting access to critical section.
Reply(R): A process responds to the request message with a reply after adding the request in it's priority_queue.
Open(O): A process sends the open message when it releases the critical section.

For ordering we use timestamps produced by Lamport's logical clock,we have used atomic integers to synchronise the sender and receiver threads.We store requests in a min heap according to smallest timestamp first.

## Usage

To run the program with N devices,first you need to compile the c++ code using g++(or clang):

`g++ LamportsExclusion.cpp`

Following which you need to run it in N devices inside the terminal using the following command(in Linux):

`./a.out`

Enter the required details asked further.
