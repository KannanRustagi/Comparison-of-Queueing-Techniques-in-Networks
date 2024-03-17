#include <bits/stdc++.h>
using namespace std;

// Simulation parameters set to their default values
int switchPortCount = 8;
int bufferSize = 4;
float packetGenProb = 0.5;
string queueType = "INQ";
string outputFile = "output.txt";
int maxTimeSlots = 10000;
float k = 0.6;
double total_delay = 0;
double packets_sent = 0;
double packets_generated = 0;
double n_trans = 0;
double link_util = 0;
int k_drop = 0;

//stores the delays encountered by every packet that is sent
vector<double> packet_delays;

// Define Packet structure
struct Packet
{
    int source;         // source port
    int destination;    // destination port
    double arrivalTime; // Arrival time of the packet
    double start_time;  // arrival_time+ offset

    // defining custom operator for multiset
    bool operator<(const Packet &a) const { return source == a.source ? start_time < a.start_time : start_time < a.start_time; }
};

// Traffic generation function
multiset<Packet> generateTraffic(int currentTime, mt19937 &gen, uniform_int_distribution<> &opPort, uniform_real_distribution<> &offset)
{
    multiset<Packet> generatedPackets;

    // Create a vector to hold port indices
    vector<int> ports(switchPortCount);
    for (int i = 0; i < switchPortCount; i++)
    {
        ports[i] = i; // Initialize ports with indices from 0 to switchPortCount-1
    }
    random_shuffle(ports.begin(), ports.end()); // Shuffle the ports randomly

    // Iterate through the shuffled ports
    for (int port : ports)
    {
        // Generate packet with probability packetGenProb
        if (rand() % 100 < packetGenProb * 100)
        { // Check if a packet is generated based on probability

            Packet packet;                                      // Create a new Packet object
            packet.source = port;                               // Set the source port of the packet
            packet.destination = opPort(gen) % switchPortCount; // Set the destination port uniformly randomly using opPort and modulo operation
            packet.arrivalTime = currentTime;                   // Set the arrival time of the packet
            packet.start_time = currentTime + offset(gen);      // Set the start time of the packet with an offset
            generatedPackets.insert(packet);                    // Insert the generated packet into the multiset
            packets_generated++;                                // Increment the counter for generated packets
        }
    }
    return generatedPackets; // Return the multiset of generated packets
}

// implementing INQ algorithm
void inq(multiset<Packet> &generatedPackets, vector<multiset<Packet>> &ip_ports, vector<vector<Packet>> &op_ports, int timeSlot)
{

    for (int i = 0; i < op_ports.size(); i++)
    {
        // Choose a packet randomly
        vector<Packet> port = op_ports[i];
        Packet packet;
        // if there are certain requests for that output port
        if (port.size() > 0)
        {
            // selecting a random packet for transmission
            int selectedIndex = rand() % port.size();
            packet = port[selectedIndex];

            // updating number of packets sent
            packets_sent += 1;
            cout << "Packet from port " << packet.source << " to port " << packet.destination << " which arrived at " << packet.arrivalTime << " has started transmitting" << endl;

            // updating total_delay
            total_delay += (double)(timeSlot - packet.arrivalTime);
            // as packet transmission will end by the next time slot
            total_delay += 1;
            packet_delays.push_back((double)(timeSlot - packet.arrivalTime + 1));

            // erasing the request from the output port and input port
            op_ports[i].erase(op_ports[i].begin() + selectedIndex);
            ip_ports[packet.source].erase(packet);
        }
    }
}

// Implementing the KOUQ algorithm
void kouq(multiset<Packet> &generatedPackets, int timeSlot, vector<queue<Packet>> &queues)
{

    // Iterate over each output port
    for (int i = 0; i < switchPortCount; i++)
    {
        vector<Packet> port;

        // Collect packets destined for op port i
        for (auto pkt : generatedPackets)
        {
            if (pkt.destination == i)
            {
                port.push_back(pkt); // Store packets destined for op port i
            }
        }

        // If there are packets destined for op port i
        if (port.size() > 0)
        {
            random_shuffle(port.begin(), port.end()); // Shuffle the packets randomly

            // Determine the number of packets to select for transmission, either k or the entire packets if less than k
            int x = min((int)(switchPortCount * (k)), (int)port.size());

            // more than k packets are destined for op port i
            if ((int)(switchPortCount * (k)) < (int)port.size())
            {
                k_drop++; // Increment the counter for dropped packets
            }

            // Select packets for transmission
            for (int j = 0; j < x; j++)
            {
                Packet selec = port.back(); // Select a packet from the end of the vector
                if (queues[i].size() < bufferSize)
                {                          // If the buffer of op port i is not full
                    queues[i].push(selec); // Push the selected packet into the queue for port i
                }
                port.pop_back(); // Remove the selected packet from the vector
            }
        }

        // If the queue for port i is not empty
        if (!queues[i].empty())
        {
            Packet selec = queues[i].front();                                                                                                                                    // Select a packet from the front of the queue
            queues[i].pop();                                                                                                                                                     // Remove the selected packet from the queue
            cout << "Packet from port " << selec.source << " to port " << selec.destination << " which arrived at " << selec.arrivalTime << " has started transmitting" << endl; // Print information about the transmitted packet
            total_delay += (timeSlot - selec.arrivalTime);                                                                                                                       // Update total delay
            total_delay += 1;                                                                                                                                                    // Add transmission time to total delay
            packet_delays.push_back((double)(timeSlot - selec.arrivalTime + 1));
            packets_sent += 1; // Increment the counter for sent packets
            n_trans++;         // Increment the counter for transmissions
        }
    }
}

// implementing iSlip algorithm
void iSlip(vector<multiset<Packet>> &ip_ports, vector<vector<Packet>> &op_ports, int timeSlot, vector<int> &op_ptr, vector<int> &ip_ptr, vector<vector<queue<Packet>>> &voq)
{

    // Initialize vectors to store requests and grants for output ports and input ports
    vector<vector<int>> op_requests(switchPortCount);
    vector<vector<int>> ip_grant(switchPortCount);

    // Collect requests from VOQs to output ports
    for (int i = 0; i < switchPortCount; i++)
    {
        for (int j = 0; j < switchPortCount; j++)
        {
            if (voq[i][j].size() > 0)
            {
                op_requests[j].push_back(voq[i][j].front().source); // Store source of the front packet in the VOQ
            }
        }
    }

    // Iterate over output ports
    for (int port = 0; port < switchPortCount; port++)
    {
        int ptr = op_ptr[port]; // Current pointer position for output port
        int choice = -1;        // Initialize choice for grant
        int closest = INT_MAX;  // Initialize distance to closest request
        // Iterate over requests for the current output port
        for (auto req : op_requests[port])
        {
            int distance = (req - ptr + switchPortCount) % switchPortCount; // Calculate distance to request from pointer
            if (distance < closest)
            {
                closest = distance; // Update closest distance
                choice = req;       // Update choice for grant
            }
        }
        if (choice != -1)
            ip_grant[choice].push_back(port); // Store grant for input port corresponding to the choice
    }

    // Iterate over input ports
    for (int port = 0; port < switchPortCount; port++)
    {
        int ptr = ip_ptr[port]; // Current pointer position for input port
        int choice = -1;        // Initialize choice for grant
        int closest = INT_MAX;  // Initialize distance to closest grant
        // Iterate over grants for the current input port
        for (auto grant : ip_grant[port])
        {
            int distance = (grant - ptr + switchPortCount) % switchPortCount; // Calculate distance to grant from pointer
            if (distance < closest)
            {
                closest = distance; // Update closest distance
                choice = grant;     // Update choice for grant
            }
        }
        if (choice != -1)
        {
            ip_ptr[port] = (choice + 1) % switchPortCount; // Update pointer for current input port
            op_ptr[choice] = (port + 1) % switchPortCount; // Update pointer for corresponding output port

            // Transmit the packet corresponding to the grant
            Packet packet = voq[port][choice].front();      // Get the packet from the VOQ
            voq[port][choice].pop();                        // Remove the packet from the VOQ
            total_delay += (timeSlot - packet.arrivalTime); // Update total delay
            total_delay += 1;                               // Add transmission time to total delay
            packet_delays.push_back((double)(timeSlot - packet.arrivalTime + 1));
            cout << "Packet from port " << packet.source << " to port " << packet.destination << " which arrived at " << packet.arrivalTime << " has started transmitting" << endl; // Print information about the transmitted packet
            packets_sent += 1;                                                                                                                                                      // Increment the counter for sent packets
            n_trans++;                                                                                                                                                              // Increment the counter for transmissions
            ip_ports[port].erase(ip_ports[port].find(packet));                                                                                                                      // Remove the packet from the IP port
        }
    }
}

int main(int argc, char *argv[])
{
    packet_delays.clear();
    srand(time(0)); // Seed the random number generator with the current time

    // opening output file
    ofstream outputfile(outputFile);

    // Command line argument handling
    for (int i = 1; i < argc; i += 2)
    {
        string arg = argv[i]; // Extract the argument
        if (arg == "-N")
        {
            switchPortCount = stoi(argv[i + 1]); // Set switchPortCount from command line argument
        }
        else if (arg == "-B")
        {
            bufferSize = stoi(argv[i + 1]); // Set bufferSize from command line argument
        }
        else if (arg == "-p")
        {
            packetGenProb = stof(argv[i + 1]); // Set packetGenProb from command line argument
        }
        else if (arg == "-K")
        {
            k = stof(argv[i + 1]); // Set k from command line argument
        }
        else if (arg == "-queue")
        {
            queueType = argv[i + 1]; // Set queueType from command line argument
        }
        else if (arg == "-out")
        {
            outputFile = argv[i + 1]; // Set outputFile from command line argument
        }
        else if (arg == "-T")
        {
            maxTimeSlots = stoi(argv[i + 1]); // Set maxTimeSlots from command line argument
        }
        else
        {
            cout << "Unknown option: " << arg << endl; // Print error message for unknown option
            exit(1);                                   // Exit the program with error code 1
        }
    }
    cout << switchPortCount << " " << bufferSize << " " << packetGenProb << " " << queueType << " " << outputFile << " " << maxTimeSlots << endl; // Print parameters

    random_device rd;  // A seed source for the random number engine
    mt19937 gen(rd()); // Mersenne Twister engine seeded with rd()

    // Initializing uniform distribution for output port
    uniform_int_distribution<> opPort(0, 30 * (switchPortCount) + 1);
    // Initializing uniform distribution for offset
    uniform_real_distribution<> offset(0.001, 0.01);

    vector<int> op_ptr(switchPortCount); // Initialize output port pointers for islip
    vector<int> ip_ptr(switchPortCount); // Initialize input port pointers for islip

    vector<multiset<Packet>> ip_ports(switchPortCount); // Initialize input port packets
    vector<vector<Packet>> op_ports(switchPortCount);   // Initialize output port requests

    // Initialize queues for each op port for kouq algorithm
    vector<queue<Packet>> queues(switchPortCount);

    // Initialize virtual output queues for iSLIP algorithm
    vector<vector<queue<Packet>>> voq(switchPortCount, vector<queue<Packet>>(switchPortCount));

    // Loop over each time slot
    for (int timeSlot = 1; timeSlot <= maxTimeSlots; ++timeSlot)
    {
        // Phase 1: Traffic Generation
        multiset<Packet> generatedPackets = generateTraffic(timeSlot, gen, opPort, offset); // Generate traffic for the current time slot
        // Process generated packets (e.g., schedule, transmit)
        for (const Packet &packet : generatedPackets)
        {
            // Process each generated packet
            if (ip_ports[packet.source].size() < bufferSize)
            {
                ip_ports[packet.source].insert(packet);              // Insert packet into input port queue
                voq[packet.source][packet.destination].push(packet); // Insert packet into VOQ
                op_ports[packet.destination].push_back(packet);      // Insert packet into output port queue
            }
        }

        // Other logic for each time slot
        if (queueType == "INQ")
        {
            inq(generatedPackets, ip_ports, op_ports, timeSlot); // Call INQ scheduling algorithm
        }
        else if (queueType == "KOUQ")
        {
            kouq(generatedPackets, timeSlot, queues); // Call KOUQ scheduling algorithm
        }
        else if (queueType == "ISLIP")
        {
            iSlip(ip_ports, op_ports, timeSlot, op_ptr, ip_ptr, voq); // Call iSLIP scheduling algorithm
        }
    }

    // Print performance metrics
    cout << endl;
    cout << "Average Packet delay is " << total_delay / packets_sent << endl; // Print average packet delay
    link_util = packets_sent / (double)(switchPortCount * maxTimeSlots);      // Calculate link utilization
    cout << "Link utilization is " << link_util << endl;                      // Print link utilization

    // calculating standard deviation
    double deviation = 0;

    for (auto x : packet_delays)
    {
        //average packet delay
        double avg = (double)total_delay / (double)packets_sent;
        double y = (x - avg) * (x - avg);
        deviation += y;
    }

    double std_dev = sqrt((double)deviation / (double)packets_sent);

    cout << "Standard Deviation: " << std_dev << endl;

    if (queueType == "KOUQ")
    {
        cout << "Drop probability " << (double)k_drop / (double)(packets_generated) << endl; // Print drop probability for KOUQ
    }

    // printing performance metrics in output file
    outputfile << switchPortCount << "\t" << packetGenProb << "\t" << queueType << "\t" << total_delay / packets_sent << "\t" << std_dev << "\t" << link_util << endl;

    return 0; // Exit the program
}