/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation;
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

// Network topology
//
//            10 links       1 link          10 links
//       n0 ----------- n1 ----------- n2 ------------ n3
//            5 Mbps         1 Mbps          5 Mbps
//             10 ms          20 ms           10 ms
//
// - Dumbbell topology
// - Flow(s) from n0 to n3 using BulkSendApplication.
// - PacketSinkApplication at n3

#include <string>
#include <fstream>
#include "ns3/core-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/internet-module.h"
#include "ns3/applications-module.h"
#include "ns3/network-module.h"
#include "ns3/packet-sink.h"
#include "ns3/global-route-manager.h"
#include "ns3/ipv4-global-routing-helper.h"
#include "ns3/point-to-point-dumbbell.h"
#include "ns3/netanim-module.h"

using namespace ns3;

NS_LOG_COMPONENT_DEFINE ("ece6110_p1");

int
main (int argc, char *argv[])
{

  SeedManager::SetSeed(11223344);

  uint32_t maxBytes = 100000000;
  uint32_t segSize = 512;
  uint32_t queueSize = 2000;
  uint32_t maxWinSize = 2000;
  uint32_t tcpType = 0;
  uint32_t nFlows = 1;
  uint32_t j;
  double temp[10];

//
// Allow the user to override any of the defaults at
// run-time, via command-line arguments
//

  CommandLine cmd;
 cmd.AddValue ("segSize", "setting TCP segment size", segSize);
 cmd.AddValue ("queueSize", "setting queue size", queueSize);
 cmd.AddValue ("windowSize", "setting maximum advertised receiver window size", maxWinSize);
 cmd.AddValue ("nFlows", "Determines whether single flow or 10 flow output is being tested", nFlows);
 cmd.AddValue ("tcpType", "0 for TCP Tahoe, 1 for TCP Reno", tcpType);
 cmd.Parse (argc, argv);


if (tcpType == 0) {
Config::SetDefault("ns3::TcpL4Protocol::SocketType", StringValue("ns3::TcpTahoe"));
} else {
Config::SetDefault("ns3::TcpL4Protocol::SocketType", StringValue("ns3::TcpReno"));
}

//Config::SetDefault("ns3::TcpSocket::RcvBufSize", UintegerValue(maxWinSize));
Config::SetDefault("ns3::TcpSocketBase::MaxWindowSize", UintegerValue(maxWinSize));
Config::SetDefault("ns3::TcpSocketBase::WindowScaling", BooleanValue(false));
Config::SetDefault("ns3::DropTailQueue::Mode", EnumValue(DropTailQueue::QUEUE_MODE_BYTES));
Config::SetDefault("ns3::DropTailQueue::MaxBytes", UintegerValue(queueSize));
Config::SetDefault("ns3::TcpSocket::SegmentSize", UintegerValue(segSize));
//
// Explicitly create the nodes required by the topology (shown above).
//
  NS_LOG_INFO ("Create nodes.");
  //NodeContainer nodes;
  //nodes.Create (4);
//NodeContainer nlink1= NodeContainer (nodes.Get(0),nodes.Get(1));
//NodeContainer nlink2= NodeContainer (nodes.Get(1),nodes.Get(2));
//NodeContainer nlink3= NodeContainer (nodes.Get(2),nodes.Get(3));

  NS_LOG_INFO ("Create channels.");

//
// Explicitly create the point-to-point link required by the topology (shown above).
//
  PointToPointHelper pointToPoint1;
  PointToPointHelper pointToPoint2;
  pointToPoint1.SetDeviceAttribute ("DataRate", StringValue ("5Mbps"));
  pointToPoint1.SetChannelAttribute ("Delay", StringValue ("10ms"));
  pointToPoint2.SetDeviceAttribute("DataRate", StringValue ("1Mbps"));
  pointToPoint2.SetQueue("ns3::DropTailQueue");
  pointToPoint2.SetChannelAttribute("Delay", StringValue ("20ms"));
PointToPointDumbbellHelper dumbbell (nFlows, pointToPoint1, nFlows, pointToPoint1, pointToPoint2);

  //NetDeviceContainer devices1, devices2, devices3;
  //devices1 = pointToPoint1.Install (nlink1);
  //devices2 = pointToPoint2.Install (nlink2);
  //devices3 = pointToPoint1.Install (nlink3);

//
// Install the internet stack on the nodes
//
  InternetStackHelper internet;
  dumbbell.InstallStack (internet);

//
// We've got the "hardware" in place.  Now we need to add IP addresses.
//
  NS_LOG_INFO ("Assign IP Addresses.");
  //Ipv4AddressHelper ipv4;
  //ipv4.SetBase ("10.1.1.0", "255.255.255.0");
  //Ipv4InterfaceContainer i1 = ipv4.Assign (devices1);
  //ipv4.SetBase ("10.1.2.0", "255.255.255.0");
  //Ipv4InterfaceContainer i2 = ipv4.Assign (devices2);
  //ipv4.SetBase ("10.1.3.0", "255.255.255.0");
  //Ipv4InterfaceContainer i3 = ipv4.Assign (devices3);

Ipv4AddressHelper lIp = Ipv4AddressHelper ("10.1.1.0", "255.255.255.0");
Ipv4AddressHelper rIp = Ipv4AddressHelper ("10.2.1.0", "255.255.255.0");
Ipv4AddressHelper mIp = Ipv4AddressHelper ("10.3.1.0", "255.255.255.0");
dumbbell.AssignIpv4Addresses(lIp,rIp,mIp);
  NS_LOG_INFO ("Create Applications.");

//
// Create a BulkSendApplication and install it on node 0
//
  uint16_t port = 21;  // port number

Ptr<UniformRandomVariable> urv = CreateObject<UniformRandomVariable> ();
urv->SetAttribute("Min", DoubleValue(0.0));
urv->SetAttribute("Max", DoubleValue(0.1));
urv->SetAttribute("Stream", IntegerValue(6110));
for (j=0; j<10; j++) {

temp[j] = urv->GetValue();
}

ApplicationContainer sourceApps[10];
ApplicationContainer sinkApps[10];

for (j=0; j<nFlows; j++) {


BulkSendHelper source ("ns3::TcpSocketFactory", InetSocketAddress (dumbbell.GetRightIpv4Address (j), port));
source.SetAttribute ("MaxBytes", UintegerValue (maxBytes));
sourceApps[j] = source.Install(dumbbell.GetLeft(j));
sourceApps[j].Start (Seconds (temp[j]));
sourceApps[j].Stop (Seconds (10.0));

PacketSinkHelper sink ("ns3::TcpSocketFactory", InetSocketAddress (Ipv4Address::GetAny(), port));
sinkApps[j] = sink.Install (dumbbell.GetRight(j));
sinkApps[j].Start (Seconds (0));
sinkApps[j].Stop (Seconds (10.0));


}

//
// Now, do the actual simulation.
//

  ns3::Ipv4GlobalRoutingHelper::PopulateRoutingTables();
  NS_LOG_INFO ("Run Simulation.");
  Simulator::Stop (Seconds (10.0));

  Simulator::Run ();
  NS_LOG_INFO ("Done.");

Ptr<PacketSink> sink[10];

for (j=0; j<nFlows; j++) {

  sink[j] = DynamicCast<PacketSink> (sinkApps[j].Get (0));
  std::cout << "tcp," << tcpType << ",flow," << j << ",windowSize," << maxWinSize << ",queueSize," << queueSize << ",segSize," << segSize <<  ",goodput," << sink[j]->GetTotalRx()/(10.0-temp[j]) << std::endl; 
  // std::cout << temp[j] << std::endl;
}

Simulator::Destroy();
}
