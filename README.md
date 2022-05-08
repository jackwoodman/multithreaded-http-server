```
./detect -f testcases/task1/resources-0.txt | diff - testcases/task1/resources-0-out.txt
./detect -f testcases/task1/resources-1.txt | diff - testcases/task1/resources-1-out.txt
./detect -e -f testcases/task2/resources-5.txt | diff - testcases/task2/resources-5-out.txt
./detect -e -f testcases/task2/resources-6.txt | diff - testcases/task2/resources-6-out.txt
./detect -e -f testcases/task2/resources-8.txt | diff - testcases/task2/resources-8-out.txt
./detect -f testcases/task3/resources-0.txt | diff - testcases/task3/resources-0-out.txt
./detect -f testcases/task3/resources-1.txt | diff - testcases/task3/resources-1-out.txt
./detect -f testcases/task3/resources-2.txt | diff - testcases/task3/resources-2-out.txt
./detect -f testcases/task3/resources-3.txt | diff - testcases/task3/resources-3-out.txt
./detect -f testcases/task4/resources-1.txt | diff - testcases/task4/resources-1-out.txt
./detect -f testcases/task4/resources-2.txt | diff - testcases/task4/resources-2-out.txt
./detect -f testcases/task4/resources-7.txt | diff - testcases/task4/resources-7-out.txt
./detect -f testcases/task5/resources-3.txt | diff - testcases/task5/resources-3-out.txt
./detect -f testcases/task5/resources-4.txt | diff - testcases/task5/resources-4-out.txt

./detect -c -f testcases/task6/resources-ch-0.txt
./detect -c -f testcases/task6/resources-ch-1.txt
./detect -c -f testcases/task6/resources-ch-2.txt
```
