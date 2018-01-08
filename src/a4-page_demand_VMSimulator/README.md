******* VERY IMPORTANT ******
Instruction of running program

1.
	You have to type "make" to get the a4vmsim program


2.
	If you occured the error which said "./mrefgen: cannot execute binary file", please use rm to remove mrefgen executable file. And "make" again

3. 
	Any modification should be taken place in run.sh file


4. 
	when you want to run program.
	Please use "./run.sh"

	if run.sh doesn't runnable at first, please type 
		"chmod +x run.sh"

5.
	It would be better to use small value of [$memsize / $page].
	Best value range(0,10000]. Otherwise it will take much time to run.
