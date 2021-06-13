# -Super-shell
As part of the Operating Systems course I had to implement a "Super" shell. 
This "Super" shell need to support all the standart shell commands + additonal special commands like:
1. encryptFile & decryptFile - given a ASCII character and a text file we need to encrypt/decrypt the file using it.
2. lockCmdForTime - lock a command for a given time (in seconds) - for this command we also implemented a data structure to manage all the locked commands. 
3. letterFreq - given a text file we need to count the letter frequency.
4. compressFile - given a text file we need to compress it (every character that appears 3+ times in a row has to be compressed to this format: llll -> l4).

