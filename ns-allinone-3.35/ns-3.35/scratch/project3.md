# Project 3: Three-node Network in ns-3

This project creates a simple **three-node wired network** in ns-3.

The goal is to understand how packets travel from one node to another through an intermediate node.

## Network Diagram

```text
Node 0                     Node 1                     Node 2
UDP Client  ------------   Router   ------------      UDP Server
10.1.1.1                   10.1.1.2
                            10.1.2.1                  10.1.2.2

        Network 1: 10.1.1.0/24        Network 2: 10.1.2.0/24
```

## Simple Explanation

In this simulation:

- **Node 0** works as the UDP Echo Client.
- **Node 1** works as the intermediate router.
- **Node 2** works as the UDP Echo Server.
- Node 0 sends UDP packets to Node 2.
- The packets pass through Node 1 before reaching Node 2.
- Node 2 replies back to Node 0.

This project introduces the idea of **multi-hop communication**.

---

## Full ns-3 Code

Save this file as:

```bash
scratch/project3.cc
```

```cpp
#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/internet-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/applications-module.h"

using namespace ns3;

NS_LOG_COMPONENT_DEFINE("Project3ThreeNodeNetwork");

int main(int argc, char *argv[])
{
    Time::SetResolution(Time::NS);

    LogComponentEnable("UdpEchoClientApplication", LOG_LEVEL_INFO);
    LogComponentEnable("UdpEchoServerApplication", LOG_LEVEL_INFO);

    NodeContainer nodes;
    nodes.Create(3);

    NodeContainer node0node1 = NodeContainer(nodes.Get(0), nodes.Get(1));
    NodeContainer node1node2 = NodeContainer(nodes.Get(1), nodes.Get(2));

    PointToPointHelper pointToPoint;
    pointToPoint.SetDeviceAttribute("DataRate", StringValue("5Mbps"));
    pointToPoint.SetChannelAttribute("Delay", StringValue("2ms"));

    NetDeviceContainer device0device1 = pointToPoint.Install(node0node1);
    NetDeviceContainer device1device2 = pointToPoint.Install(node1node2);

    InternetStackHelper stack;
    stack.Install(nodes);

    Ipv4AddressHelper address1;
    address1.SetBase("10.1.1.0", "255.255.255.0");
    Ipv4InterfaceContainer interface0interface1 = address1.Assign(device0device1);

    Ipv4AddressHelper address2;
    address2.SetBase("10.1.2.0", "255.255.255.0");
    Ipv4InterfaceContainer interface1interface2 = address2.Assign(device1device2);

    Ipv4GlobalRoutingHelper::PopulateRoutingTables();

    UdpEchoServerHelper echoServer(9);

    ApplicationContainer serverApps = echoServer.Install(nodes.Get(2));
    serverApps.Start(Seconds(1.0));
    serverApps.Stop(Seconds(10.0));

    UdpEchoClientHelper echoClient(interface1interface2.GetAddress(1), 9);
    echoClient.SetAttribute("MaxPackets", UintegerValue(5));
    echoClient.SetAttribute("Interval", TimeValue(Seconds(1.0)));
    echoClient.SetAttribute("PacketSize", UintegerValue(1024));

    ApplicationContainer clientApps = echoClient.Install(nodes.Get(0));
    clientApps.Start(Seconds(2.0));
    clientApps.Stop(Seconds(10.0));

    pointToPoint.EnablePcapAll("project3");

    Simulator::Run();
    Simulator::Destroy();

    return 0;
}
```

---

## Important Code Explanation

### 1. Create three nodes

```cpp
NodeContainer nodes;
nodes.Create(3);
```

This creates three simulated network nodes.

```text
nodes.Get(0) = Node 0
nodes.Get(1) = Node 1
nodes.Get(2) = Node 2
```

---

### 2. Create two links

```cpp
NodeContainer node0node1 = NodeContainer(nodes.Get(0), nodes.Get(1));
NodeContainer node1node2 = NodeContainer(nodes.Get(1), nodes.Get(2));
```

This creates two separate node pairs:

```text
Node 0 <--> Node 1
Node 1 <--> Node 2
```

Node 1 is shared by both links, so it acts as the middle node.

---

### 3. Configure the point-to-point link

```cpp
PointToPointHelper pointToPoint;
pointToPoint.SetDeviceAttribute("DataRate", StringValue("5Mbps"));
pointToPoint.SetChannelAttribute("Delay", StringValue("2ms"));
```

This sets the link speed and delay.

```text
DataRate = 5 Mbps
Delay    = 2 ms
```

---

### 4. Install network devices

```cpp
NetDeviceContainer device0device1 = pointToPoint.Install(node0node1);
NetDeviceContainer device1device2 = pointToPoint.Install(node1node2);
```

This creates network interfaces for both links.

```text
Node 0 has one network device
Node 1 has two network devices
Node 2 has one network device
```

Node 1 has two devices because it connects to both Node 0 and Node 2.

---

### 5. Install internet stack

```cpp
InternetStackHelper stack;
stack.Install(nodes);
```

This installs IP, UDP, TCP, and routing support on all nodes.

Without this, the nodes cannot communicate using IP addresses.

---

### 6. Assign IP addresses to link 1

```cpp
Ipv4AddressHelper address1;
address1.SetBase("10.1.1.0", "255.255.255.0");
Ipv4InterfaceContainer interface0interface1 = address1.Assign(device0device1);
```

This gives IP addresses to the first link.

Usually:

```text
Node 0 = 10.1.1.1
Node 1 = 10.1.1.2
```

---

### 7. Assign IP addresses to link 2

```cpp
Ipv4AddressHelper address2;
address2.SetBase("10.1.2.0", "255.255.255.0");
Ipv4InterfaceContainer interface1interface2 = address2.Assign(device1device2);
```

This gives IP addresses to the second link.

Usually:

```text
Node 1 = 10.1.2.1
Node 2 = 10.1.2.2
```

---

### 8. Enable routing

```cpp
Ipv4GlobalRoutingHelper::PopulateRoutingTables();
```

This is very important.

Node 0 and Node 2 are in different networks:

```text
Node 0 side = 10.1.1.0/24
Node 2 side = 10.1.2.0/24
```

So Node 0 does not directly know how to reach Node 2.

This line automatically creates routing table entries, so packets can travel like this:

```text
Node 0 -> Node 1 -> Node 2
```

Without this line, packets may not reach Node 2.

---

### 9. Create server on Node 2

```cpp
UdpEchoServerHelper echoServer(9);

ApplicationContainer serverApps = echoServer.Install(nodes.Get(2));
serverApps.Start(Seconds(1.0));
serverApps.Stop(Seconds(10.0));
```

This creates a UDP Echo Server on Node 2.

The server listens on UDP port `9`.

---

### 10. Create client on Node 0

```cpp
UdpEchoClientHelper echoClient(interface1interface2.GetAddress(1), 9);
echoClient.SetAttribute("MaxPackets", UintegerValue(5));
echoClient.SetAttribute("Interval", TimeValue(Seconds(1.0)));
echoClient.SetAttribute("PacketSize", UintegerValue(1024));
```

This creates a UDP Echo Client.

The destination is:

```cpp
interface1interface2.GetAddress(1)
```

This is the IP address of Node 2, usually:

```text
10.1.2.2
```

The client sends:

```text
5 packets
1 second interval
1024 bytes per packet
```

---

### 11. Install client on Node 0

```cpp
ApplicationContainer clientApps = echoClient.Install(nodes.Get(0));
clientApps.Start(Seconds(2.0));
clientApps.Stop(Seconds(10.0));
```

This installs the UDP Echo Client on Node 0.

The client starts at `2s`, after the server starts at `1s`.

---

### 12. Enable PCAP files

```cpp
pointToPoint.EnablePcapAll("project3");
```

This creates PCAP files for Wireshark.

Example output files:

```text
project3-0-0.pcap
project3-1-0.pcap
project3-1-1.pcap
project3-2-0.pcap
```

You can open these files in Wireshark to see the packets.

---

## How to Run

Go to your ns-3 folder:

```bash
cd /home/lahiru/Desktop/ns3_practice/ns-allinone-3.35/ns-3.35
```

Create the code file:

```bash
nano scratch/project3.cc
```

Build:

```bash
./waf build
```

Run:

```bash
./waf --run scratch/project3
```

---

## Expected Output

You should see output similar to this:

```text
At time +2s client sent 1024 bytes to 10.1.2.2 port 9
At time +2.006s server received 1024 bytes from 10.1.1.1 port ...
At time +2.006s server sent 1024 bytes to 10.1.1.1 port ...
At time +2.012s client received 1024 bytes from 10.1.2.2 port ...
```

The exact time can be slightly different.

---

## What We Learn

From this project, we learn:

- How to create more than two nodes
- How to connect nodes using multiple links
- How to assign different IP networks
- How an intermediate node can forward packets
- Why routing is needed
- How to use PCAP files to inspect packets

---

## Summary

This project demonstrates a basic three-node network in ns-3.  
Node 0 sends UDP packets to Node 2 through Node 1.  
Node 1 acts as a router between two different IP networks.

This is the first step toward learning multi-hop networks, routing, and larger network topologies in ns-3.
