# Flash Drive Benchmark


## Description

This repo contains a desktop application for Windows developed in C++ using WinAPI. The application allows you to test the read speed and write speed of a flash memory drive. Read and write operations can be performed using sequential and random methods. In real use cases, the read and write speeds of large amounts of data will be closer to the results of random operation tests. The difference between sequential and random operation principles is shown in the picture below.
  
![picture1](https://i.imgur.com/qUpCAZW.png)  
  
Testing is performed by writing and reading three files of different sizes: 25, 250 and 2500 MB for testing sequential access operations and 250, 2500, 25000 KB for testing random access operations. If you want to test your flash memory drive with this benchmark, you should format the flash drive before doing so. When comparing flash drives using this benchmark, you should use the same cluster size and file system.  
If you want to understand the algorithm of the application and the device of its functionality, you can read [this document](flash_drive_benchmark_guide_RUS.pdf) (in Russian only).
  
**App Language:** Russian  
**Programming Language:** C++  
 **API:** WinAPI  
 **Author:** Mikhail Bahamolau  

 ## Requirements

 **OS:** Windows (10 or newer)
 
 ## Screenshots
   
 ![screenshot1](https://i.imgur.com/UvUeweL.png)  
   
 ![screenshot2](https://i.imgur.com/sX5xbMl.png)  
   
 ![screenshot3](https://i.imgur.com/pKIzvNp.png)
