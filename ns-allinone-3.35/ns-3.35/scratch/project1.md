# ns-3 UDP Echo Network Diagram

Create two nodes and send UDP packets from one node to another.

```text
Node 0                                              Node 1
UDP Client  -------- point-to-point link --------- UDP Server
10.1.1.1                                            10.1.1.2
```

Description
- Node 0 acts as the UDP Echo Client.
- Node 1 acts as the UDP Echo Server.
- The two nodes are connected using a point-to-point link.
- Node 0 sends UDP packets to Node 1.
- Node 1 receives the packets and sends replies back to Node 0.