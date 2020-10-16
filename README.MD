Compile using gcc -o parallelsort parallelsort.c -pthread

Run it as ./parallelsort x y 
where x is a number that is a factor of 2, and y is a number such that y % x == 0

Utilizes multithreaded programming to split an array of randomly generated numbers into the input parameters and sorts it in parallel and then merges the results.
