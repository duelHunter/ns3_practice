```cpp
#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/internet-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/applications-module.h"

using namespace ns3;

NS_LOG_COMPONENT_DEFINE("Project4StaticRouting");

int main(int argc, char *argv[])
{
    Time::SetResolution(Time::NS);

    LogComponentEnable("UdpEchoClientApplication", LOG_LEVEL_INFO);
    LogComponentEnable("UdpEchoServerApplication", LOG_LEVEL_INFO);

    // Create 3 nodes
    // Node 0 = Client
    // Node 1 = Router / intermediate node
    // Node 2 = Server
    NodeContainer nodes;
    nodes.Create(3);

    // Create two point-to-point links
    NodeContainer node0node1 = NodeContainer(nodes.Get(0), nodes.Get(1));
    NodeContainer node1node2 = NodeContainer(nodes.Get(1), nodes.Get(2));

    // Configure point-to-point links
    PointToPointHelper pointToPoint;
    pointToPoint.SetDeviceAttribute("DataRate", StringValue("5Mbps"));
    pointToPoint.SetChannelAttribute("Delay", StringValue("2ms"));

    // Install devices on both links
    NetDeviceContainer device0device1 = pointToPoint.Install(node0node1);
    NetDeviceContainer device1device2 = pointToPoint.Install(node1node2);

    // Install internet stack on all nodes
    InternetStackHelper stack;
    stack.Install(nodes);

    // Assign IP addresses to first link: Node 0 <--> Node 1
    Ipv4AddressHelper address1;
    address1.SetBase("10.1.1.0", "255.255.255.0");
    Ipv4InterfaceContainer interface0interface1 = address1.Assign(device0device1);

    // Assign IP addresses to second link: Node 1 <--> Node 2
    Ipv4AddressHelper address2;
    address2.SetBase("10.1.2.0", "255.255.255.0");
    Ipv4InterfaceContainer interface1interface2 = address2.Assign(device1device2);

    /*
        IP Address Summary

        Node 0:
            10.1.1.1

        Node 1:
            10.1.1.2
            10.1.2.1

        Node 2:
            10.1.2.2
    */

    // ------------------------------------------------------------
    // Manual Static Routing
    // ------------------------------------------------------------

    Ipv4StaticRoutingHelper staticRouting;

    // Static route for Node 0
    // Node 0 wants to reach network 10.1.2.0/24.
    // It must send packets through Node 1 at 10.1.1.2.
    Ptr<Ipv4> ipv4Node0 = nodes.Get(0)->GetObject<Ipv4>();
    Ptr<Ipv4StaticRouting> staticRoutingNode0 = staticRouting.GetStaticRouting(ipv4Node0);

    staticRoutingNode0->AddNetworkRouteTo(
        Ipv4Address("10.1.2.0"),
        Ipv4Mask("255.255.255.0"),
        Ipv4Address("10.1.1.2"),
        1
    );

    // Static route for Node 2
    // Node 2 wants to reach network 10.1.1.0/24.
    // It must send packets through Node 1 at 10.1.2.1.
    Ptr<Ipv4> ipv4Node2 = nodes.Get(2)->GetObject<Ipv4>();
    Ptr<Ipv4StaticRouting> staticRoutingNode2 = staticRouting.GetStaticRouting(ipv4Node2);

    staticRoutingNode2->AddNetworkRouteTo(
        Ipv4Address("10.1.1.0"),
        Ipv4Mask("255.255.255.0"),
        Ipv4Address("10.1.2.1"),
        1
    );

    // Node 1 is directly connected to both networks.
    // Therefore, Node 1 does not need a manual static route.

    // ------------------------------------------------------------
    // UDP Echo Server on Node 2
    // ------------------------------------------------------------

    UdpEchoServerHelper echoServer(9);

    ApplicationContainer serverApps = echoServer.Install(nodes.Get(2));
    serverApps.Start(Seconds(1.0));
    serverApps.Stop(Seconds(10.0));

    // ------------------------------------------------------------
    // UDP Echo Client on Node 0
    // ------------------------------------------------------------

    UdpEchoClientHelper echoClient(interface1interface2.GetAddress(1), 9);
    echoClient.SetAttribute("MaxPackets", UintegerValue(5));
    echoClient.SetAttribute("Interval", TimeValue(Seconds(1.0)));
    echoClient.SetAttribute("PacketSize", UintegerValue(1024));

    ApplicationContainer clientApps = echoClient.Install(nodes.Get(0));
    clientApps.Start(Seconds(2.0));
    clientApps.Stop(Seconds(10.0));

    // Enable PCAP output for Wireshark analysis
    pointToPoint.EnablePcapAll("project4-static-routing");

    Simulator::Run();
    Simulator::Destroy();

    return 0;
}
```
