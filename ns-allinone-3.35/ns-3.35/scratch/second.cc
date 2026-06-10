#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/internet-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/applications-module.h"

using namespace ns3;

NS_LOG_COMPONENT_DEFINE("Project2ChangeLinkParameters");

int main(int argc, char *argv[])
{
    Time::SetResolution(Time::NS);

    // Default values
    std::string dataRate = "5Mbps";
    std::string delay = "2ms";
    uint32_t packetSize = 1024;
    uint32_t maxPackets = 5;
    double interval = 1.0;

    // Allow values to be changed from terminal
    CommandLine cmd;
    cmd.AddValue("dataRate", "Link data rate", dataRate);
    cmd.AddValue("delay", "Link delay", delay);
    cmd.AddValue("packetSize", "UDP packet size in bytes", packetSize);
    cmd.AddValue("maxPackets", "Number of packets to send", maxPackets);
    cmd.AddValue("interval", "Time interval between packets in seconds", interval);
    cmd.Parse(argc, argv);

    LogComponentEnable("UdpEchoClientApplication", LOG_LEVEL_INFO);
    LogComponentEnable("UdpEchoServerApplication", LOG_LEVEL_INFO);

    std::cout << "Simulation Settings:" << std::endl;
    std::cout << "Data Rate   : " << dataRate << std::endl;
    std::cout << "Delay       : " << delay << std::endl;
    std::cout << "Packet Size : " << packetSize << " bytes" << std::endl;
    std::cout << "Max Packets : " << maxPackets << std::endl;
    std::cout << "Interval    : " << interval << " seconds" << std::endl;

    // Create 2 nodes
    NodeContainer nodes;
    nodes.Create(2);

    // Create point-to-point link
    PointToPointHelper pointToPoint;
    pointToPoint.SetDeviceAttribute("DataRate", StringValue(dataRate));
    pointToPoint.SetChannelAttribute("Delay", StringValue(delay));

    // Install devices
    NetDeviceContainer devices;
    devices = pointToPoint.Install(nodes);

    // Install internet stack
    // Install networking protocols on the nodes so they can communicate using IP, UDP, and TCP.
    InternetStackHelper stack;
    stack.Install(nodes);
 
    // Assign IP addresses
    Ipv4AddressHelper address;
    address.SetBase("10.1.1.0", "255.255.255.0");

    Ipv4InterfaceContainer interfaces;
    interfaces = address.Assign(devices);

    // Create UDP echo server on Node 1
    UdpEchoServerHelper echoServer(9);

    ApplicationContainer serverApps = echoServer.Install(nodes.Get(1));
    serverApps.Start(Seconds(1.0));
    serverApps.Stop(Seconds(20.0));

    // Create UDP echo client on Node 0
    UdpEchoClientHelper echoClient(interfaces.GetAddress(1), 9);
    echoClient.SetAttribute("MaxPackets", UintegerValue(maxPackets));
    echoClient.SetAttribute("Interval", TimeValue(Seconds(interval)));
    echoClient.SetAttribute("PacketSize", UintegerValue(packetSize));

    ApplicationContainer clientApps = echoClient.Install(nodes.Get(0));
    clientApps.Start(Seconds(2.0));
    clientApps.Stop(Seconds(20.0));

    // Generate PCAP files for Wireshark
    pointToPoint.EnablePcapAll("project2");

    Simulator::Run();
    Simulator::Destroy();

    return 0;
}