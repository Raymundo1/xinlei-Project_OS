## Assignment #2 

---
### Instruction of running program

1.

	 [python = python2]
	 
	 There is a fifo folder, which contains a "mkfifo.py" python script file.
	 	And before you running the a2rchat program, you have to run the python2 file
		by "python mkfifo". 
	
	 Once you run the mkfifo, it will shows "Please enter you baseName: ", and you have to 
	 	enter the baseName you want. Then it will automatically create the FIFOs for you.

2.

	You have to run server first, then clients.
	You have to close all clients first, then server

	It may occurs some issues, when you do the wrong order


3.

	You have to type "make" first to get a2rchat program first


4.

	Running server by typing:
		./a2rchat -s baseName-part number-part

		[baseName-part]: The name which you entered when running mkfifo.py file
		[number-part]: MAXIMUM_CLIENT. You have to enter the reasonable number like [1-5], although it will detect the wrong number when running program


	Running client by typing:
		./a2rchat -c baseName-part

		[baseName-part]: The name which you entered when running mkfifo.py file


5. 

	Username CAN NOT have space inside
	
	
---

### Program Basic Skeleton
![a2-skeleton](https://github.com/Raymundo1/xinlei-Project_OS/blob/master/img/a2-img1.png)

---
### Acknowledgements
* Advanced Programming in the UNIX® Environment by W. Richard Stevens and Stephen A. Rago
* [lockf() function](http://man7.org/linux/man-pages/man3/lockf.3.html)
* [combine multiple string by Jacob](https://stackoverflow.com/questions/5889880/better-way-to-concatenate-multiple-strings-in-c)






