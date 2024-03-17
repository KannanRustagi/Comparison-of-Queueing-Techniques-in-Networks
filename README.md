## Topic- Comparing Different Scheduling Algorithms for Packet transmission through a crossbar switch
You can find detailed description of the problem statement for this project in the pdf file "problem statement".<br>
<br>
Input Format-<br>
Inputs to the program,i.e, the simulation parameters will be taken as command line arguments-
1. N- Number of switch input and output ports
2. B- Buffer size
3. p- Packet generation probability
4. queue- queue scheduling technique being used{INQ/KOUQ/ISLIP}
5. K- Maximum packets arriving at an output port in KOUQ that will be served
6. out- output file name
7. maxtimeslots- simulation time.
<br>
Default values to the above parameters(as instructed in the problem statement) have been initialized in the program gloablly in the start itself.<br>

=====================================================================================================================================================

Execution-<br>
Extract the zip file and open the terminal, go to the location where you have extracted the zip. The code should be compiled and run on a system in which a C++ compiler is installed.<br>

To compile the program-<br>
~ g++ Assignment1.cpp -o Assignment1<br>
To run the program-<br>
~ ./Assignment1  -N  switchportcount  -B  buffersize  -p  packetgenprob  -queue  INQ/KOUQ/iSLIP  -Kknockout -out -outputfile -T maxtimeslots<br>
(You may or may not give any argument, then it will assume the default value for processing)<br>

An output file will be generated which will contain the following values in a line(as instructed in the problem statement)<br>

~ N   p   QueueType   Avg PD  Std Dev of PD   Avg link utilization<br>
where-<br>
N-Number of switch input and output ports<br>
p-Packet generation probability<br>
QueueType-queue scheduling technique being used{INQ/KOUQ/ISLIP}<br>
Average PD-average packet delay<br>
Std Dev of PD-Standard deviation of packet delay<br>
Avg link utilisation- Average link utilisation<br>

=====================================================================================================================================================

Explanation-<br>
The code contains the following major structures and functions-<br>
1. Packet structure- defining the parameters required to define a packet
2. generateTraffic function- will generate packets per slot and store them in a multiset sorted by arrival times
3. inq function- implements the inq scheduling algorithm
4. kouq function- implements the kouq scheduling algorithm
5. iSlip function- implements the iSlip scheduling algorithm<br>

The program has detailed comments explaining how we have implemented the logic for all the algorithms.<br>

Formula used for calculating average packet delay= summation of all the (packet start transmission time+1(transmission time)- packet arrival time) divided by the total packets sent<br>

Similarly standard deviation of packet delay has been calculated as sqrt(summation of (packet delay of each packet)/ total number of packets sent).<br>

Formula used for calculating average packet delay- packets_sent/(switchPortCount * maxTimeSlots). Herein, the denominator signifies the maximum number of packets that the switch can transmit over the entire simulation time assuming that it can transmit N(number of switch ports) packets each timeslot.<br>
