# ransac
A simple RANSAC algorithm for fitting a 2D line

Random Sample Consesus (RANSAC) is a classic technique in big data analysis. It is an iterative method to estimate the parameters of a model which fits a ``significant" number of observations in a given dataset. The observations which fit into the model are known as \textit{inliers} and the observations which do not fit are known as \textit{outliers}. Note that this approach is different from other commonly used parameter estimation techniques such as least squares which try to estimate a model which best fits all the data points. 

The program is based on 10 threads, you can change it if you like. 

the client program is inside tool, along with the data generater, which can randomly generate the 2D data for RANSAC processing. 

Compile:
make

## before run
be sure the DATA_SIZE is same on both client and popcorn-ransac server and the real size of the data in byte

## How to run? 
```
# on machine A
./popcorn-ransac 
# on machine B
./client $ip_of_machine_A 

```
