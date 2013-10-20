fano_algorithm
==============

Utility for encode and decode ASCII files.
Command format
-----------
**encode**
```
  fano a [-t <probability table>] <input file> <output file>
```
**decode**
```
  fano a [-t <probability table>] <input file> <output file>
```
**help**
```
  fano h
```

Probability table
-----------
Probability table is a text file with pairs:
```
<ASCII character> <probability>  
```
*ASCII character can be:* 
* ```<character symbol>``` for example ```a```
* ```<character hex-code>``` for example ```\x0A```
