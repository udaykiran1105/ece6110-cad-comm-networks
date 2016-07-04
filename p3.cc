/*Lab3

Team Members;
Uday Kiran Ravuri
Abhinav Murali
Mrunmayi Paranjape
Jayakrishna

*/


#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <string>
#include <vector>
#include <map>
#include <math.h>

#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/mobility-module.h"
#include "ns3/wifi-module.h"
#include "ns3/internet-module.h"
#include "ns3/aodv-helper.h"
#include "ns3/dsr-helper.h"
#include "ns3/olsr-helper.h"
#include "ns3/applications-module.h"
#include "ns3/stats-module.h"
#include "ns3/netanim-module.h"
#include "ns3/object.h"
#include "ns3/uinteger.h"
#include "ns3/traced-value.h"
#include "ns3/trace-source-accessor.h"

using namespace ns3;
using namespace std;

#define IEEE_80211_BANDWIDTH  20000000

uint32_t total = 0;

void Trace ( Ptr<const Packet> newValue)
{
  total = total + newValue->GetSize();
}


int main (int argc, char *argv[])
{
  
/*****************************************************************************************************************/
// Define the constants
/*****************************************************************************************************************/
  ns3::RngSeedManager::SetSeed(11223344);

  uint16_t runValue  = 0; 
  uint16_t pktSize   = 512;
  double   onTime    = 0.1;
  double   offTime   = 0.1; 		/*duty cycle is 0.5*/
  double   stopTime  = 30;

 
  uint16_t numNodes  = 50;
  string Size        = "1000";
  double   intensity = 0.5;
  string   routeProt = "AODV";
  double   txPower   = 100; 		// Entered In mW. 
  
  CommandLine cmd;
  cmd.AddValue ("runValue",  "Random seed run",                           runValue);
  cmd.AddValue ("numNodes",  "Number of nodes in simulation",             numNodes);
  cmd.AddValue ("pktSize",   "Size of packets sent",                      pktSize);
  cmd.AddValue ("Size",   "Size of simulation field in meters",        Size);
  cmd.AddValue ("intensity", "Traffic intensity (%)",                     intensity);
  cmd.AddValue ("routeProt", "Routing protocol to use: AODV, OLSR",       routeProt);
  cmd.AddValue ("txPower",   "Transmission power range in Watts",         txPower);
  cmd.AddValue ("onTime",    "Time ON for OnOffApplication in seconds",   onTime);
  cmd.AddValue ("offTime",    "Time OFF for OnOffApplication in seconds", offTime);
  cmd.Parse (argc,argv);

  SeedManager::SetRun(runValue);
  
  Ptr<UniformRandomVariable> rv_start = CreateObject<UniformRandomVariable> ();   // Random variable to determine random start
  rv_start->SetAttribute( "Stream", IntegerValue( 6110 ) );
  rv_start->SetAttribute ("Min", DoubleValue (0));
  rv_start->SetAttribute ("Max", DoubleValue (0.1)); 

  Config::SetDefault ("ns3::OnOffApplication::PacketSize", UintegerValue(pktSize));

  ostringstream onTimeStr;
  onTimeStr << "ns3::ConstantRandomVariable[Constant=" << onTime << "]";
  Config::SetDefault ("ns3::OnOffApplication::OnTime", StringValue(onTimeStr.str()));

  ostringstream offTimeStr;
  offTimeStr << "ns3::ConstantRandomVariable[Constant=" << offTime << "]";
  Config::SetDefault ("ns3::OnOffApplication::OffTime", StringValue(offTimeStr.str()));


  txPower = 10 * log10 (txPower);
  srand (11223344);
/*****************************************************************************************************************/
// Set up the topology
/*****************************************************************************************************************/
  NodeContainer nodes;
  nodes.Create(numNodes);

  /* Create random square based on provided size.*/
  MobilityHelper mobility;
  string posMax = "ns3::UniformRandomVariable[Min=0.0|Max=" + Size + "]";
  mobility.SetPositionAllocator ("ns3::RandomRectanglePositionAllocator","X", StringValue (posMax),"Y", StringValue (posMax));
  mobility.SetMobilityModel ("ns3::ConstantPositionMobilityModel");
  mobility.Install (nodes);

  //Set up the WiFi Net Device
  YansWifiPhyHelper wifiPhy = YansWifiPhyHelper::Default ();
  wifiPhy.Set ("TxPowerStart", DoubleValue(txPower));
  wifiPhy.Set ("TxPowerEnd", DoubleValue(txPower));

  //Define the wireless channel
  YansWifiChannelHelper wifiChannel = YansWifiChannelHelper::Default (); // Check the propagation loss model assumptions (5.5GHz default. Change it to 2.4GHz ??)
  wifiPhy.SetChannel (wifiChannel.Create ());

  
// Set Wifi to 802.11b and use a DSSS rate of 1Mbps.
  WifiHelper wifi = WifiHelper::Default ();
  wifi.SetStandard (WIFI_PHY_STANDARD_80211b);
  wifi.SetRemoteStationManager ("ns3::ConstantRateWifiManager", "DataMode", StringValue("DsssRate1Mbps"), "ControlMode", StringValue("DsssRate1Mbps"));

  /* Set the Wifi MAC as adhoc.*/
  NqosWifiMacHelper wifiMac = NqosWifiMacHelper::Default ();
  wifiMac.SetType ("ns3::AdhocWifiMac");

  //Install the WifiNet on all nodes in the network
  NetDeviceContainer devices = wifi.Install (wifiPhy, wifiMac, nodes);

  //Set up the internet stack 
  InternetStackHelper stack;
  Ipv4ListRoutingHelper list;

  if (routeProt.compare("AODV") == 0)
  {
      AodvHelper aodv;
      list.Add (aodv, 100);
      stack.SetRoutingHelper (list);
  }
  else if (routeProt.compare("OLSR") == 0)
  {
      OlsrHelper olsr;
      list.Add (olsr, 10);
      stack.SetRoutingHelper (list);
  }

  stack.Install (nodes);

  
  Ipv4AddressHelper ipv4;
  ipv4.SetBase ("10.0.0.0", "255.255.0.0");
  Ipv4InterfaceContainer interfaces = ipv4.Assign (devices);

/*****************************************************************************************************************/
// Choose the node pairs and configure the traffic generator
/*****************************************************************************************************************/
srand(6110);  
vector<uint32_t> vec;
map<uint32_t,uint32_t> nodePairs;
for (uint32_t j=0; j<numNodes; j++) vec.push_back(j); 	// Create a vector of node indices for pairing

random_shuffle(vec.begin(),vec.end());			// Randomize the list and connect the successive nodes

for(uint32_t i = 0 ; i <= vec.size() - 1 ; i ++ )
{
     if(i == (vec.size() - 1)){ 			// Pair the last node with the first node in the list
     nodePairs.insert(make_pair(vec[i],vec[0]));
     }
     else{						
     nodePairs.insert(make_pair(vec[i],vec[i+1]));
     }
} 

  
  /* Determine the data rate to be used based on the intensity and network capacity.*/
  Ptr<WifiNetDevice> wifiDevice = DynamicCast<WifiNetDevice> (devices.Get(0)); 
  uint64_t networkCap = IEEE_80211_BANDWIDTH * log (1 + wifiDevice->GetPhy()->CalculateSnr (wifiDevice->GetPhy()->GetMode(0), 0.1)) / log (2); //1 * 1024 * 1024;
 
  uint64_t dataRate  = (uint64_t)(networkCap * intensity) / (0.5*numNodes); /*duty cycle is 0.5

  Create OnOffApplications and PacketSinks*/
  ApplicationContainer sourceApps;
  ApplicationContainer sinkApps;
  
  for (uint32_t i=0; i<numNodes; ++i)
    {
      uint16_t port = 50000 + i;
      Ptr<Node> appSource 	= nodes.Get(nodePairs.find(i)->first);
      Ptr<Node> appSink 	= nodes.Get(nodePairs.find(i)->second);
      Ipv4Address remoteAddr 	= appSink->GetObject<Ipv4> ()->GetAddress (1, 0).GetLocal ();

      OnOffHelper onoff ("ns3::UdpSocketFactory", Address (InetSocketAddress (remoteAddr, port)));
      onoff.SetAttribute ("DataRate", DataRateValue (DataRate (dataRate)));

      ApplicationContainer tempApp (onoff.Install (appSource));
      tempApp.Start(Seconds(rv_start->GetValue()));
      sourceApps.Add(tempApp);

      PacketSinkHelper sink ("ns3::UdpSocketFactory", InetSocketAddress (Ipv4Address::GetAny (), port));
      sinkApps.Add(sink.Install (appSink));
    }
  sourceApps.Stop (Seconds (stopTime));
  sinkApps.Start (Seconds (0.0));

  for (uint32_t i=0; i < numNodes; ++i){
    Ptr<OnOffApplication> onoff1 = DynamicCast<OnOffApplication> (sourceApps.Get (i));
    onoff1->TraceConnectWithoutContext("Tx", MakeCallback(&Trace));
  }
  
 
  Simulator::Stop(Seconds(stopTime));  
  Simulator::Run ();

/*****************************************************************************************************************/
// Observations and Results
/*****************************************************************************************************************/
  
  uint32_t totRx = 0;
  for (uint32_t i=0; i < numNodes; ++i)
  {
      Ptr<PacketSink> sink1 = DynamicCast<PacketSink> (sinkApps.Get (i));
      totRx += sink1->GetTotalRx ();
  }
  /* Results output*/
  cout << Size << "\t" << numNodes << "\t" << pow(10, txPower/10) << "\t" << intensity << "\t" << routeProt << "\t" << (100 * (double)totRx / (double)total) << endl;

  ofstream outfile;
  outfile.open("scratch/p3.csv", std::ios_base::app);
  outfile << "routep," << routeProt << ",nodes," << numNodes << ",txpower," << pow(10, txPower/10) << ",intensity," << intensity << ",efficiency," << (100 * (double)totRx / (double)total) << endl;


  Simulator::Destroy ();
  return 0;
}
