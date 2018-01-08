import os


base_name = raw_input("Please enter your baseName: ")

for i in range(1,6):
    fifo_in_name = base_name + "-" + str(i) + ".in"
    fifo_out_name = base_name + "-" + str(i) + ".out"
    os.system("mkfifo " + fifo_in_name)
    os.system("mkfifo " + fifo_out_name)


