## Project 2: Change Data Rate, Delay, and Packet Size

This project extends the basic two-node UDP Echo simulation in ns-3.  
The main goal is to understand how **data rate**, **link delay**, **packet size**, **packet interval**, and **number of packets** affect packet transmission.

### Simple Network Diagram

```text
Node 0                                      Node 1
UDP Echo Client  ----------------------->  UDP Echo Server
10.1.1.1                                    10.1.1.2

              Point-to-point link
              DataRate = 5Mbps
              Delay    = 2ms
```

In this simulation, **Node 0** works as the UDP Echo Client and **Node 1** works as the UDP Echo Server.  
The client sends UDP packets to the server, and the server sends the same packets back as replies.

---

### Important Code Parts

#### 1. Set simulation parameters

```cpp
std::string dataRate = "5Mbps";
std::string delay = "2ms";
uint32_t packetSize = 1024;
uint32_t maxPackets = 5;
double interval = 1.0;
```

These variables define the main network settings.

- `dataRate` controls the speed of the link.
- `delay` controls how long a packet takes to travel through the link.
- `packetSize` controls the size of each UDP packet.
- `maxPackets` controls how many packets are sent.
- `interval` controls the time gap between packets.

---

#### 2. Create the point-to-point link

```cpp
PointToPointHelper pointToPoint;
pointToPoint.SetDeviceAttribute("DataRate", StringValue(dataRate));
pointToPoint.SetChannelAttribute("Delay", StringValue(delay));
```

This creates a direct link between the two nodes.

For example:

```text
Node 0  --------  Node 1
        5Mbps
        2ms
```

Changing `DataRate` and `Delay` allows us to observe different network conditions.

---

#### 3. Assign IP addresses

```cpp
Ipv4AddressHelper address;
address.SetBase("10.1.1.0", "255.255.255.0");

Ipv4InterfaceContainer interfaces;
interfaces = address.Assign(devices);
```

This assigns IP addresses to the two nodes.

Usually:

```text
Node 0 = 10.1.1.1
Node 1 = 10.1.1.2
```

These IP addresses are needed for UDP communication.

---

#### 4. Create UDP Echo Server

```cpp
UdpEchoServerHelper echoServer(9);

ApplicationContainer serverApps = echoServer.Install(nodes.Get(1));
serverApps.Start(Seconds(1.0));
serverApps.Stop(Seconds(20.0));
```

This creates a UDP Echo Server on **Node 1**.

- The server listens on UDP port `9`.
- The server starts at simulation time `1s`.
- The server stops at simulation time `20s`.

---

#### 5. Create UDP Echo Client

```cpp
UdpEchoClientHelper echoClient(interfaces.GetAddress(1), 9);
echoClient.SetAttribute("MaxPackets", UintegerValue(maxPackets));
echoClient.SetAttribute("Interval", TimeValue(Seconds(interval)));
echoClient.SetAttribute("PacketSize", UintegerValue(packetSize));
```

This creates a UDP Echo Client that sends packets to Node 1.

- `interfaces.GetAddress(1)` gets the IP address of Node 1.
- `MaxPackets` sets the number of packets.
- `Interval` sets the time gap between packets.
- `PacketSize` sets the size of each packet.

---

#### 6. Install and start the client

```cpp
ApplicationContainer clientApps = echoClient.Install(nodes.Get(0));
clientApps.Start(Seconds(2.0));
clientApps.Stop(Seconds(20.0));
```

This installs the UDP Echo Client on **Node 0**.

The client starts at `2s`, after the server has already started at `1s`.

---

### Running the Simulation

Run with default values:

```bash
./waf --run scratch/project2
```

Run with custom parameters:

```bash
./waf --run "scratch/project2 --dataRate=1Mbps --delay=10ms --packetSize=2048 --maxPackets=10 --interval=0.5"
```

This means:

```text
Data rate   = 1Mbps
Delay       = 10ms
Packet size = 2048 bytes
Packets     = 10
Interval    = 0.5 seconds
```

---

### Important Note About Large Packet Size

If you use a very large packet size like:

```bash
./waf --run "scratch/project2 --packetSize=1024000"
```

the server may not receive any packets.

This happens because `1024000 bytes` is too large for one UDP packet.  
Instead of sending 1 MB as one packet, send many smaller packets:

```bash
./waf --run "scratch/project2 --packetSize=1024 --maxPackets=1000 --interval=0.01"
```

This sends about 1 MB total data:

```text
1024 bytes × 1000 packets = 1,024,000 bytes
```

---

### What We Learn

From this project, we can understand:

- Higher delay causes packets to arrive later.
- Lower data rate increases transmission time.
- Larger packet size takes more time to send.
- Very large UDP packets may not be delivered.
- Many small packets are better than one extremely large UDP packet.

---

### Summary

This project helps beginners understand how basic network parameters affect communication in ns-3.  
By changing data rate, delay, packet size, packet count, and interval, we can observe different packet transmission behaviors in a simple two-node UDP Echo network.