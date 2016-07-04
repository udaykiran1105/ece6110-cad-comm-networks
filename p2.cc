#include <iostream>
#include <string>
#include <fstream>
#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/internet-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/netanim-module.h"
#include "ns3/applications-module.h"
#include "ns3/packet-sink-helper.h"
#include "ns3/uinteger.h"
#include "ns3/point-to-point-dumbbell.h"
#include "ns3/drop-tail-queue.h"
#include "ns3/random-variable-stream.h"
#include "ns3/constant-position-mobility-model.h"

using namespace ns3;
using namespace std;

NS_LOG_COMPONENT_DEFINE ("p2");

int
main (int argc, char *argv[])
{

Time::SetResolution (Time::NS);

  // Default parameters
  uint nNodes = 11; // initial value of nodes
  std::string animFile = "P2.xml" ; // Name of file for animation output
  std::string queueType = "DropTail"; // default queue type
  std::string udp1_rate = "500kb/s"; // initial udp rate
  std::string udp2_rate = "500kb/s"; // initial udp rate
  std::string router_bw = "1Mbps"; // router link bandwidth
  std::string router_delay = "10ms"; // router link delay

  //DropTail parameters
  uint32_t maxBytes = 10000000; //infinity (initial value)
  uint32_t segSize = 512; // initial value of maximum segment size (maximum bytes sent in a TCP segment)
  uint32_t queueSize = 64000; // queue limit (in bytes) for both DropTail and RED
  uint32_t windowSize = 64000; // receiver advertised maximum window size
  uint32_t packetSize = 256; // packet size

  //Red parameters
  double minTh = 30; // Threshold to trigger probabalistic drops
  double maxTh = 90; // Threshold to trigger forced drops
  
// To change the value during run-time  
  CommandLine cmd;
  cmd.AddValue ("queueType","Set type of queue", queueType);
  cmd.AddValue ("segSize","Size of each segment (in bytes)", segSize);
  cmd.AddValue ("queueSize","Size of queue (in bytes)", queueSize);
  cmd.AddValue ("windowSize","Size of window (in bytes)", windowSize);
  cmd.AddValue ("packetSize","Set packet size (in bytes)", packetSize);
  cmd.AddValue ("udp1_rate","Set UDP Flow 1 Data Rate", udp1_rate);
  cmd.AddValue ("udp2_rate","Set UDP Flow 2 Data Rate", udp2_rate);
  cmd.AddValue ("router_bw","Set bottle-neck link bandwidth", router_bw);
  cmd.AddValue ("router_delay","Set bottle-neck link delay", router_delay);
  cmd.AddValue ("minTh","Set threshold to trigger probabalistic drops (# packets)", minTh);
  cmd.AddValue ("maxTh","Set threshold to trigger forced drops (# packets)", maxTh);
  cmd.Parse (argc, argv);

  double simtime = 10.0; 
  double recvUDP1;  
  double recvUDP2;  
  double recvTCP1;  
  double recvTCP2;  
  double opUDP1; 
  double opUDP2; 
  double opTCP1; 
  double opTCP2;
  
  // Select queue type
  std::string queuetype;
  if (queueType == "DropTail") {
    queuetype = "ns3::DropTailQueue";
  }
  else if (queueType == "RED") {
    queuetype = "ns3::RedQueue";
  } 
  else {
    NS_ABORT_MSG ("Invalid queue type: Use --queueType=RED or --queueType=DropTail");
  } 

  // Setting TCP characteristics.
  Config::SetDefault("ns3::TcpL4Protocol::SocketType", StringValue ("ns3::TcpTahoe"));
  Config::SetDefault("ns3::TcpSocket::SegmentSize", UintegerValue(segSize));
  Config::SetDefault("ns3::TcpSocketBase::MaxWindowSize", UintegerValue(windowSize));
  Config::SetDefault("ns3::TcpSocketBase::WindowScaling", BooleanValue(false));

  // Setting DropTailQueue characteristics
  Config::SetDefault ("ns3::DropTailQueue::Mode", EnumValue(DropTailQueue::QUEUE_MODE_BYTES));
  Config::SetDefault ("ns3::DropTailQueue::MaxBytes", UintegerValue(queueSize));

  //Setting RedQueue characteristics
  minTh *= packetSize; 
  maxTh *= packetSize;
  Config::SetDefault ("ns3::RedQueue::Mode", EnumValue (RedQueue::QUEUE_MODE_BYTES));
  Config::SetDefault ("ns3::RedQueue::MinTh", DoubleValue (minTh));
  Config::SetDefault ("ns3::RedQueue::MaxTh", DoubleValue (maxTh));
  Config::SetDefault ("ns3::RedQueue::QueueLimit", UintegerValue (queueSize)); 
  Config::SetDefault ("ns3::RedQueue::LinkBandwidth", StringValue (router_bw));
  Config::SetDefault ("ns3::RedQueue::LinkDelay", StringValue (router_delay));
  
    // Creating nodes.
    NodeContainer nodes;
    nodes.Create(nNodes);

    NodeContainer r0r1 (nodes.Get(0), nodes.Get(1));
    NodeContainer r1r2 (nodes.Get(1), nodes.Get(2));
    NodeContainer r0a1 (nodes.Get(0), nodes.Get(3));
    NodeContainer r0a2 (nodes.Get(0), nodes.Get(4));
    NodeContainer r0a3 (nodes.Get(0), nodes.Get(5));
    NodeContainer r1b1 (nodes.Get(1), nodes.Get(6));
    NodeContainer r1b2 (nodes.Get(1), nodes.Get(7));
    NodeContainer r2c1 (nodes.Get(2), nodes.Get(8));
    NodeContainer r2c2 (nodes.Get(2), nodes.Get(9));
	NodeContainer r2c3 (nodes.Get(2), nodes.Get(10));

    // To set positions of each node in netAnim.
    Ptr<Node> node0 = nodes.Get(0);
    Ptr<ConstantPositionMobilityModel> pos0 = node0->GetObject<ConstantPositionMobilityModel>();
    pos0 = CreateObject<ConstantPositionMobilityModel>();
    node0->AggregateObject(pos0);
    Vector pos0V(0, 2, 0);
    pos0->SetPosition(pos0V);

    Ptr<Node> node1 = nodes.Get(1);
    Ptr<ConstantPositionMobilityModel> pos1 = node1->GetObject<ConstantPositionMobilityModel>();
    pos1 = CreateObject<ConstantPositionMobilityModel>();
    node1->AggregateObject(pos1);
    Vector pos1V(1, 2, 0);
    pos1->SetPosition(pos1V);

    Ptr<Node> node2 = nodes.Get(2);
    Ptr<ConstantPositionMobilityModel> pos2 = node2->GetObject<ConstantPositionMobilityModel>();
    pos2 = CreateObject<ConstantPositionMobilityModel>();
    node2->AggregateObject(pos2);
    Vector pos2V(2, 2, 0);
    pos2->SetPosition(pos2V);

    Ptr<Node> node3 = nodes.Get(3);
    Ptr<ConstantPositionMobilityModel> pos3 = node3->GetObject<ConstantPositionMobilityModel>();
    pos3 = CreateObject<ConstantPositionMobilityModel>();
    node3->AggregateObject(pos3);
    Vector pos3V(-2, 1, 0);
    pos3->SetPosition(pos3V);

    Ptr<Node> node4 = nodes.Get(4);
    Ptr<ConstantPositionMobilityModel> pos4 = node4->GetObject<ConstantPositionMobilityModel>();
    pos4 = CreateObject<ConstantPositionMobilityModel>();
    node4->AggregateObject(pos4);
    Vector pos4V(-2, 2, 0);
    pos4->SetPosition(pos4V);

	Ptr<Node> node5 = nodes.Get(5);
    Ptr<ConstantPositionMobilityModel> pos5 = node5->GetObject<ConstantPositionMobilityModel>();
    pos5 = CreateObject<ConstantPositionMobilityModel>();
    node5->AggregateObject(pos5);
    Vector pos5V(-2, 3, 0);
    pos5->SetPosition(pos5V);

    Ptr<Node> node6 = nodes.Get(6);
    Ptr<ConstantPositionMobilityModel> pos6 = node6->GetObject<ConstantPositionMobilityModel>();
    pos6 = CreateObject<ConstantPositionMobilityModel>();
    node6->AggregateObject(pos6);
    Vector pos6V(1, 0, 0);
    pos6->SetPosition(pos6V);

    Ptr<Node> node7 = nodes.Get(7);
    Ptr<ConstantPositionMobilityModel> pos7 = node7->GetObject<ConstantPositionMobilityModel>();
    pos7 = CreateObject<ConstantPositionMobilityModel>();
    node7->AggregateObject(pos7);
    Vector pos7V(1, 4, 0);
    pos7->SetPosition(pos7V);

    Ptr<Node> node8 = nodes.Get(8);
    Ptr<ConstantPositionMobilityModel> pos8 = node8->GetObject<ConstantPositionMobilityModel>();
    pos8 = CreateObject<ConstantPositionMobilityModel>();
    node8->AggregateObject(pos8);
    Vector pos8V(4, 1, 0);
    pos8->SetPosition(pos8V);

    Ptr<Node> node9 = nodes.Get(9);
    Ptr<ConstantPositionMobilityModel> pos9 = node9->GetObject<ConstantPositionMobilityModel>();
    pos9 = CreateObject<ConstantPositionMobilityModel>();
    node9->AggregateObject(pos9);
    Vector pos9V(4, 2, 0);
    pos9->SetPosition(pos9V);

    Ptr<Node> node10 = nodes.Get(10);
    Ptr<ConstantPositionMobilityModel> pos10 = node10->GetObject<ConstantPositionMobilityModel>();
    pos10 = CreateObject<ConstantPositionMobilityModel>();
    node10->AggregateObject(pos10);
    Vector pos10V(4, 3, 0);
    pos10->SetPosition(pos10V);


    //Creating links
    // Set parameters for links connecting routers r0 and r1
    PointToPointHelper RouterLink;
    RouterLink.SetDeviceAttribute ("DataRate", StringValue (router_bw)); 
    RouterLink.SetChannelAttribute ("Delay", StringValue (router_delay)); 
    RouterLink.SetQueue (queuetype);
    NetDeviceContainer d_r0r1 = RouterLink.Install(r0r1);
    NetDeviceContainer d_r1r2 = RouterLink.Install(r1r2);

    // Set parameters for links connecting the left leaf
    PointToPointHelper LeftLink;
    LeftLink.SetDeviceAttribute ("DataRate", StringValue ("5Mbps"));
    LeftLink.SetChannelAttribute ("Delay", StringValue ("2ms"));
    NetDeviceContainer d_r0a1 = LeftLink.Install(r0a1);
    NetDeviceContainer d_r0a2 = LeftLink.Install(r0a2);
    NetDeviceContainer d_r0a3 = LeftLink.Install(r0a3);
   
   // Set parameters for links connecting the middle segment nodes
    PointToPointHelper MiddleLink;
    MiddleLink.SetDeviceAttribute ("DataRate", StringValue ("5Mbps"));
    MiddleLink.SetChannelAttribute ("Delay", StringValue ("2ms"));
    NetDeviceContainer d_r1b1 = MiddleLink.Install(r1b1);
    NetDeviceContainer d_r1b2 = MiddleLink.Install(r1b2);
   
    // Set parameters for links connecting the right leaf
    PointToPointHelper RightLink;
    RightLink.SetDeviceAttribute ("DataRate", StringValue ("5Mbps"));
    RightLink.SetChannelAttribute ("Delay", StringValue ("2ms"));
    NetDeviceContainer d_r2c1 = RightLink.Install(r2c1);
    NetDeviceContainer d_r2c2 = RightLink.Install(r2c2);
    NetDeviceContainer d_r2c3 = RightLink.Install(r2c3);
	
    InternetStackHelper stack; // Install internet stack
    stack.Install (nodes);

    // Assigning addresses to all devices.
    Ipv4AddressHelper address;
 
    address.SetBase ("10.1.1.0", "255.255.255.0");
    Ipv4InterfaceContainer interface1 = address.Assign (d_r0r1);

    address.SetBase ("10.1.2.0", "255.255.255.0");
    Ipv4InterfaceContainer interface2 = address.Assign (d_r1r2);

    address.SetBase ("10.1.3.0", "255.255.255.0");
    Ipv4InterfaceContainer interface3 = address.Assign (d_r0a1);

    address.SetBase ("10.1.4.0", "255.255.255.0");
    Ipv4InterfaceContainer interface4 = address.Assign (d_r0a2);

    address.SetBase ("10.1.5.0", "255.255.255.0");
    Ipv4InterfaceContainer interface5 = address.Assign (d_r0a3);

    address.SetBase ("10.1.6.0", "255.255.255.0");
    Ipv4InterfaceContainer interface6 = address.Assign (d_r1b1);

    address.SetBase ("10.1.7.0", "255.255.255.0");
    Ipv4InterfaceContainer interface7 = address.Assign (d_r1b2);

    address.SetBase ("10.1.8.0", "255.255.255.0");
    Ipv4InterfaceContainer interface8 = address.Assign (d_r2c1);

    address.SetBase ("10.1.9.0", "255.255.255.0");
    Ipv4InterfaceContainer interface9 = address.Assign (d_r2c2);

    address.SetBase ("10.1.10.0", "255.255.255.0");
    Ipv4InterfaceContainer interface10 = address.Assign (d_r2c3);
	
    // Install TCP and UDP applications to get both flows on the bottle-neck links.

    uint16_t port = 9;
    
	// On Off UDP applications
    OnOffHelper Udp1("ns3::UdpSocketFactory", Address(InetSocketAddress(interface10.GetAddress(1), port)));
    //Udp1.SetConstantRate(DataRate(udp1_rate)); 
    Udp1.SetAttribute("DataRate", DataRateValue(udp1_rate)); // variable
	Udp1.SetAttribute ("PacketSize", UintegerValue (packetSize)); 
    Udp1.SetAttribute("OnTime", StringValue("ns3::ConstantRandomVariable[Constant=0.5]")); 
    Udp1.SetAttribute("OffTime", StringValue("ns3::ConstantRandomVariable[Constant=0.5]"));
    Udp1.SetAttribute ("MaxBytes", UintegerValue (maxBytes));

    OnOffHelper Udp2("ns3::UdpSocketFactory", Address(InetSocketAddress(interface9.GetAddress(1), port)));
    //Udp2.SetConstantRate(DataRate(udp2_rate)); 
    Udp2.SetAttribute("DataRate", DataRateValue(udp2_rate)); // variable
	Udp2.SetAttribute ("PacketSize", UintegerValue (packetSize)); 
    Udp2.SetAttribute("OnTime", StringValue("ns3::ConstantRandomVariable[Constant=0.5]")); 
    Udp2.SetAttribute("OffTime", StringValue("ns3::ConstantRandomVariable[Constant=0.5]"));
    Udp2.SetAttribute ("MaxBytes", UintegerValue (maxBytes));	
	
    BulkSendHelper Tcp1 ("ns3::TcpSocketFactory",InetSocketAddress (interface8.GetAddress(1), port));
    Tcp1.SetAttribute ("MaxBytes", UintegerValue (maxBytes));
    Tcp1.SetAttribute ("SendSize", UintegerValue (packetSize)); // variable

    BulkSendHelper Tcp2 ("ns3::TcpSocketFactory",InetSocketAddress (interface6.GetAddress(1), port));
    Tcp2.SetAttribute ("MaxBytes", UintegerValue (maxBytes));
    Tcp2.SetAttribute ("SendSize", UintegerValue (packetSize));  // variable    
	
    ApplicationContainer sourceApp1 = Udp1.Install (nodes.Get (5));
    ApplicationContainer sourceApp2 = Udp2.Install (nodes.Get (7));
    ApplicationContainer sourceApp3 = Tcp1.Install (nodes.Get (3));
    ApplicationContainer sourceApp4 = Tcp2.Install (nodes.Get (4));
	
    sourceApp1.Start (Seconds (0.0));
    sourceApp1.Stop (Seconds (simtime));

    sourceApp2.Start (Seconds (0.0));
    sourceApp2.Stop (Seconds (simtime));

    sourceApp3.Start (Seconds (0.0));
    sourceApp3.Stop (Seconds (simtime));

    sourceApp4.Start (Seconds (0.0));
    sourceApp4.Stop (Seconds (simtime));
    
	// Creating UDP and TCP sink apps
    // UDP sink 1
    PacketSinkHelper UdpSink1 ("ns3::UdpSocketFactory", InetSocketAddress(Ipv4Address::GetAny(), port));

    // UDP sink 2
    PacketSinkHelper UdpSink2 ("ns3::UdpSocketFactory", InetSocketAddress(Ipv4Address::GetAny(), port));

    // TCP sink 1
    PacketSinkHelper TcpSink1 ("ns3::TcpSocketFactory", InetSocketAddress(Ipv4Address::GetAny(), port));
    
    // TCP sink 2
    PacketSinkHelper TcpSink2 ("ns3::TcpSocketFactory", InetSocketAddress(Ipv4Address::GetAny(), port));

	ApplicationContainer sinkApp1;
    sinkApp1.Add (UdpSink1.Install (nodes.Get (10)));

    ApplicationContainer sinkApp2;
    sinkApp2.Add (UdpSink2.Install (nodes.Get (9)));

    ApplicationContainer sinkApp3;
    sinkApp3.Add (TcpSink1.Install (nodes.Get (8)));

    ApplicationContainer sinkApp4;
    sinkApp4.Add (TcpSink2.Install (nodes.Get (6)));

	sinkApp1.Start (Seconds (0.0));
    sinkApp1.Stop (Seconds (simtime));

    sinkApp2.Start (Seconds (0.0));
    sinkApp2.Stop (Seconds (simtime));

    sinkApp3.Start (Seconds (0.0));
    sinkApp3.Stop (Seconds (simtime));

    sinkApp4.Start (Seconds (0.0));
    sinkApp4.Stop (Seconds (simtime));

    // Create the animation object and configure for specified output
    AnimationInterface anim (animFile);

	// God routing
    Ipv4GlobalRoutingHelper::PopulateRoutingTables ();

    Simulator::Stop (Seconds (simtime));
    Simulator::Run ();
  
    cout <<"\nGoodput of individual flows: " <<endl; 
    Ptr<PacketSink> sink1 = DynamicCast<PacketSink> (sinkApp1.Get(0));
    recvUDP1 = sink1->GetTotalRx ();
    opUDP1 = recvUDP1/simtime;
    cout << "Goodput of UDP flow 1: " << opUDP1 <<endl;

    Ptr<PacketSink> sink2 = DynamicCast<PacketSink> (sinkApp2.Get(0));
    recvUDP2 = sink2->GetTotalRx ();
    opUDP2 = recvUDP2/simtime;
    cout << "Goodput of UDP flow 2: " << opUDP2 <<endl;

    Ptr<PacketSink> sink3 = DynamicCast<PacketSink> (sinkApp3.Get(0));
    recvTCP1 = sink3->GetTotalRx ();
    opTCP1 = recvTCP1/simtime;
    cout << "Goodput of TCP flow 1: " << opTCP1 <<endl;

    Ptr<PacketSink> sink4 = DynamicCast<PacketSink> (sinkApp4.Get(0));
    recvTCP2 = sink4->GetTotalRx ();
    opTCP2 = recvTCP2/simtime;
    cout << "Goodput of TCP flow 2: " << opTCP2 <<endl;

	double avgopt = (opTCP1+opTCP2+opUDP1+opUDP2)/4.0;
	cout << "Average Goodput: " << avgopt <<endl;

	Simulator::Destroy ();

return 0;

}