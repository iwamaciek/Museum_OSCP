# Museum_OSCP
Project for OS&amp;CP laboratories made by Maciej Iwaszkiewicz

In order to run this program you need to:
1. Use "make" to compile the program
2. Use "./Museum N Na Nb" where N is the (int) number of visitors; Na is the (int) capacity of Hall A; Nb is the (int) capacity of Hall B or
3. Use "./Museum" to run with default settings (N=20; Na=4; Nb=3).

How the program works:
It creates n threads simulating one person visiting a museum. Each thread waits outside the museum and checks if it can enter. Once it can, it enters Hall A and watches the exhibition for a random time (1-10 seconds) and then randomly (50-50) decides if it exits the museum or enters Hall B. It waits until it can enter and after it watches the exhibition for a random time (1-5 seconds) and notifies the system that it wants to exit. It then waits until there is space in Hall A and has priority to enter there over those waiting outside. Then it immediately exits.
There is also another "control" thread which enables the switch between people waiting to get to Hall B from Hall A and those waiting to go to Hall A from Hall B - in short it prevents the deadlock of the process.

Important notes:
Sometimes it may seem like there is nothing happening this is due to two possible things:
1. The visitors are watching the exhibition for a long time or
2. The system got stuck on something. Unfortunately it sometimes happens that when there are a lot of people on the border between Hall A and Hall B the program can just not do anything. As I have not managed to find the reason for such behaviour, I suggest terminating the program and executing it again, it should now finish the execution as this bug rarely happened during testing.
