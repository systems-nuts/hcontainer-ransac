### How to use? 
```
gcc client.c -o client
gcc generate.c -o generate


generate will help you generate the test 2D set, or you can use your own one
please change the DATA_SIZE or OBJ_NUM in source file accordingly 

The program is free for use, and client only takes one parameter, the destination ip address of the ransac program. It will send the ransac_data to the ransac serer then receive a acknowledge after the processing. 

```
