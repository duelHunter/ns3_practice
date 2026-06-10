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

    // Enable UDP Echo application logs
    LogComponentEnable("UdpEchoClientApplication", LOG_LEVEL_INFO);
    LogComponentEnable("UdpEchoServerApplication", LOG_LEVEL_INFO);

    // Create 3 nodes
    // Node 0 = Client
    // Node 1 = Router / intermediate node
    // Node 2 = Server
    NodeContainer nodes;
    nodes.Create(3);

    // Create two separate node pairs:
    // Link 1: Node 0 <--> Node 1
    // Link 2: Node 1 <--> Node 2
    NodeContainer node0node1 = NodeContainer(nodes.Get(0), nodes.Get(1));
    NodeContainer node1node2 = NodeContainer(nodes.Get(1), nodes.Get(2));

    // Create point-to-point link configuration
    PointToPointHelper pointToPoint;
    pointToPoint.SetDeviceAttribute("DataRate", StringValue("5Mbps"));
    pointToPoint.SetChannelAttribute("Delay", StringValue("2ms"));

    // Install point-to-point devices on each link
    NetDeviceContainer device0device1 = pointToPoint.Install(node0node1);
    NetDeviceContainer device1device2 = pointToPoint.Install(node1node2);

    // Install internet stack on all nodes
    InternetStackHelper stack;
    stack.Install(nodes);

    // Assign IP addresses for first network: Node 0 <--> Node 1
    Ipv4AddressHelper address1;
    address1.SetBase("10.1.1.0", "255.255.255.0");
    Ipv4InterfaceContainer interface0interface1 = address1.Assign(device0device1);

    // Assign IP addresses for second network: Node 1 <--> Node 2
    Ipv4AddressHelper address2;
    address2.SetBase("10.1.2.0", "255.255.255.0");
    Ipv4InterfaceContainer interface1interface2 = address2.Assign(device1device2);

    // Populate routing tables automatically
    // This allows Node 0 to reach Node 2 through Node 1
    Ipv4GlobalRoutingHelper::PopulateRoutingTables();

    // Create UDP Echo Server on Node 2
    UdpEchoServerHelper echoServer(9);

    ApplicationContainer serverApps = echoServer.Install(nodes.Get(2));
    serverApps.Start(Seconds(1.0));
    serverApps.Stop(Seconds(10.0));

    // Create UDP Echo Client on Node 0
    // Destination is Node 2 IP address on the 10.1.2.0 network
    UdpEchoClientHelper echoClient(interface1interface2.GetAddress(1), 9);
    echoClient.SetAttribute("MaxPackets", UintegerValue(5));
    echoClient.SetAttribute("Interval", TimeValue(Seconds(1.0)));
    echoClient.SetAttribute("PacketSize", UintegerValue(1024));

    ApplicationContainer clientApps = echoClient.Install(nodes.Get(0));
    clientApps.Start(Seconds(2.0));
    clientApps.Stop(Seconds(10.0));

    // Generate PCAP files for Wireshark
    pointToPoint.EnablePcapAll("project3");

    Simulator::Run();
    Simulator::Destroy();

    return 0;
}
