#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/internet-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/applications-module.h"

using namespace ns3;

NS_LOG_COMPONENT_DEFINE("Project5PcapAnalysis");

int main(int argc, char *argv[])
{
    Time::SetResolution(Time::NS);

    LogComponentEnable("UdpEchoClientApplication", LOG_LEVEL_INFO);
    LogComponentEnable("UdpEchoServerApplication", LOG_LEVEL_INFO);

    // Simulation parameters
    std::string dataRate = "5Mbps";
    std::string delay = "2ms";
    uint32_t packetSize = 1024;
    uint32_t maxPackets = 5;
    double interval = 1.0;

    CommandLine cmd;
    cmd.AddValue("dataRate", "Point-to-point link data rate", dataRate);
    cmd.AddValue("delay", "Point-to-point link delay", delay);
    cmd.AddValue("packetSize", "UDP packet size in bytes", packetSize);
    cmd.AddValue("maxPackets", "Number of packets to send", maxPackets);
    cmd.AddValue("interval", "Interval between packets in seconds", interval);
    cmd.Parse(argc, argv);

    std::cout << "Project 05: PCAP Packet Analysis" << std::endl;
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

    // Install network devices
    NetDeviceContainer devices;
    devices = pointToPoint.Install(nodes);

    // Install internet stack
    InternetStackHelper stack;
    stack.Install(nodes);

    // Assign IP addresses
    Ipv4AddressHelper address;
    address.SetBase("10.1.1.0", "255.255.255.0");

    Ipv4InterfaceContainer interfaces;
    interfaces = address.Assign(devices);

    std::cout << "Node 0 IP Address: " << interfaces.GetAddress(0) << std::endl;
    std::cout << "Node 1 IP Address: " << interfaces.GetAddress(1) << std::endl;

    // Create UDP Echo Server on Node 1
    UdpEchoServerHelper echoServer(9);

    ApplicationContainer serverApps = echoServer.Install(nodes.Get(1));
    serverApps.Start(Seconds(1.0));
    serverApps.Stop(Seconds(10.0));

    // Create UDP Echo Client on Node 0
    UdpEchoClientHelper echoClient(interfaces.GetAddress(1), 9);
    echoClient.SetAttribute("MaxPackets", UintegerValue(maxPackets));
    echoClient.SetAttribute("Interval", TimeValue(Seconds(interval)));
    echoClient.SetAttribute("PacketSize", UintegerValue(packetSize));

    ApplicationContainer clientApps = echoClient.Install(nodes.Get(0));
    clientApps.Start(Seconds(2.0));
    clientApps.Stop(Seconds(10.0));

    // Enable PCAP file generation
    // This will create packet capture files that can be opened in Wireshark
    pointToPoint.EnablePcapAll("project5-pcap");

    Simulator::Run();
    Simulator::Destroy();

    std::cout << "Simulation completed." << std::endl;
    std::cout << "PCAP files generated with prefix: project5-pcap" << std::endl;

    return 0;
}
