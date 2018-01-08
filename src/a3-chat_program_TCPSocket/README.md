## Assignment3
---
### Instruction of running program

1. 

	The port number is not specified
	
2. 

	You have to run server first, then clients
	You have to close all clients first, then server

	It may occurs some issues, when you do the wrong order

3.

	You have to type "make" first to get a3chat program first

4.

	Running server by typing
		./a3chat -s portnumber nclient

		[portnumber]: Your port number you want
		[nclient]: MAXIMUM_CLIENT. You have to enter the reasonable number like [1-5], although it will detect the wrong number when running program

	Running client by typing
		./a3chat -c portnumber serverAddress

		[portnumber]: 9613
		[serverAddress]: The IP Address of server.

5.

	When running program, it will happens some issues that you can't receive response from server when you type you command. Just click "enter" in keyboard, it will recovered. But you may have to re-type the command.

---
### Program Basic Skeleton
![a3-skeleton](https://github.com/Raymundo1/xinlei-Project_OS/blob/master/img/a3-img1.png)

---
### Acknowledgements
* Advanced Programming in the UNIX® Environment by W. Richard Stevens and Stephen A. Rago
* [Difference between read() & recv()](https://stackoverflow.com/questions/1790750/what-is-the-difference-between-read-and-recv-and-between-send-and-write)
* [Understand INNADDR_ANY](https://stackoverflow.com/questions/16508685/understanding-inaddr-any-for-socket-programming)

