make distclean
make 
#timeout 1 ../build.linux/nachos -e hw3_consoleIO_1 -e hw3_consoleIO_2 -e hw3_consoleIO_3 -e hw3_consoleIO_4 -e
##11/29 
#timeout 1 ../build.linux/nachos -e consoleIO_test1 -e consoleIO_test2
##original(hw3)
#../build.linux/nachos -ep hw3_consoleIO_1 60 -ep hw3_consoleIO_2 70 
##12/20
# timeout 1 ../build.linux/nachos -ep hw4_normal_test1 0 -ep hw4_normal_test2 0 


timeout 3 ../build.linux/nachos -d z -ep hw4_delay_test1 50 -ep hw4_normal_test2 45

# timeout 1 ../build.linux/nachos -e consoleIO_test1
echo "done"
