/******************************************************************************************
 Final Project : Comparison of ns3 propagation loss models

Team Members;
Abhinav Murali
Mrunmayi Paranjape
Uday Kiran
Jayakrishna

*******************************************************************************************/
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
#include "ns3/dsdv-helper.h"
#include "ns3/olsr-helper.h"
#include "ns3/applications-module.h"
#include "ns3/stats-module.h"
#include "ns3/object.h"
#include "ns3/uinteger.h"
#include "ns3/traced-value.h"
#include "ns3/trace-source-accessor.h"

using namespace ns3;
using namespace std;

uint32_t total 		= 0;

void selectmodel(YansWifiChannelHelper &channel, string model){
	if(!model.compare("FixedRSSDefault")){
		channel.AddPropagationLoss ("ns3::FixedRssLossModel");
	}
	else if(!model.compare("FixedRSSMod")){
		channel.AddPropagationLoss ("ns3::FixedRssLossModel","Rss",DoubleValue(-50.0));
	}
	else if(!model.compare("MatrixDefault")) {
		channel.AddPropagationLoss ("ns3::MatrixPropagationLossModel");
	}
	else if(!model.compare("MatrixMod")) {
		channel.AddPropagationLoss ("ns3::MatrixPropagationLossModel","DefaultLoss",DoubleValue(-50.0));
	}
	else if(!model.compare("Range")) {
		channel.AddPropagationLoss ("ns3::RangePropagationLossModel");
	}
	else if(!model.compare("RandomDefault")) {
		channel.AddPropagationLoss ("ns3::RandomPropagationLossModel");
	}
	else if(!model.compare("RandomModified")) {		
		Ptr<NormalRandomVariable> param = CreateObject<NormalRandomVariable> ();   // Gaussian Random variable ~ N(50,25)
  		param->SetAttribute ("Mean", DoubleValue (50.0));
		param->SetAttribute ("Variance", DoubleValue (25.0));
		channel.AddPropagationLoss ("ns3::RandomPropagationLossModel","Variable",PointerValue(param));
	}
	else if(!model.compare("COSTHata")) {
		channel.AddPropagationLoss ("ns3::Cost231PropagationLossModel");
	}
	else if(!model.compare("Friis")) {
		channel.AddPropagationLoss ("ns3::FriisPropagationLossModel");
	}
	else if(!model.compare("LogDistance")) {
		channel.AddPropagationLoss ("ns3::LogDistancePropagationLossModel");		
	}
	else if(!model.compare("ThreeLogDistance")) {
		channel.AddPropagationLoss ("ns3::ThreeLogDistancePropagationLossModel");		
	}
	else if(!model.compare("TwoRayGround")) {
		channel.AddPropagationLoss ("ns3::TwoRayGroundPropagationLossModel");
	}
	else if(!model.compare("TwoRayGroundMod")) {
		channel.AddPropagationLoss ("ns3::TwoRayGroundPropagationLossModel","HeightAboveZ",DoubleValue(1.0));
	}
	else if(!model.compare("JakesonFriis")) {		
		channel.AddPropagationLoss ("ns3::FriisPropagationLossModel");
	  	channel.AddPropagationLoss ("ns3::JakesPropagationLossModel");		
	}
	else if(!model.compare("NakagamionFriis")) {
		channel.AddPropagationLoss ("ns3::FriisPropagationLossModel");
		channel.AddPropagationLoss ("ns3::NakagamiPropagationLossModel");				
	}
}


void Trace ( Ptr<const Packet> newValue)
{
  total = total + newValue->GetSize();
}


int main (int argc, char *argv[])
{
  
/*****************************************************************************************************************/
// Define the constants
/*****************************************************************************************************************/
  uint16_t pktSize   = 512;
  double   stopTime  = 20.0;
  string  lossmodel  = "ThreeLogDistance";
  int 	  data_rate  = 500000; 			// in bps
  double  onTime     = (pktSize * 8.0)/(data_rate); 	 
  double  offTime    = 49*onTime;

  uint16_t numNodes  = 50;
  uint16_t subnodes  = 20;
  string   routeProt = "AODV";
  double   txPower   = 100; 			// Entered In mW. 
  
  CommandLine cmd;
  cmd.AddValue ("numNodes",  "Number of nodes in simulation",             numNodes);
  cmd.AddValue ("routeProt", "Routing protocol to use: AODV, OLSR, DSDV", routeProt);
  cmd.AddValue ("txPower",   "Transmission power range in milliwatts",    txPower);
  cmd.AddValue ("lossmodel", "Propagation Loss model to be used",         lossmodel);
  cmd.Parse (argc,argv);

  /*
  Ptr<UniformRandomVariable> rv_start = CreateObject<UniformRandomVariable> ();   // Random variable to determine random start
  rv_start->SetAttribute( "Stream", IntegerValue(11223344) );
  rv_start->SetAttribute ("Min", DoubleValue (0));
  rv_start->SetAttribute ("Max", DoubleValue (0.1)); 
*/

  Config::SetDefault ("ns3::OnOffApplication::PacketSize", UintegerValue(pktSize));
ostringstream onTimeStr;
  onTimeStr << "ns3::ConstantRandomVariable[Constant=" << onTime << "]";
  Config::SetDefault ("ns3::OnOffApplication::OnTime", StringValue(onTimeStr.str()));

  ostringstream offTimeStr;
  offTimeStr << "ns3::ConstantRandomVariable[Constant=" << offTime << "]";
  Config::SetDefault ("ns3::OnOffApplication::OffTime", StringValue(offTimeStr.str()));
  /*
  Config::SetDefault ("ns3::OnOffApplication::OnTime", RandomVariableValue (ConstantVariable (onTime)) );
  Config::SetDefault ("ns3::OnOffApplication::OffTime", RandomVariableValue (ConstantVariable (offTime)) );
*/

  txPower = 10 * log10 (txPower);  

/*****************************************************************************************************************/
// Set up the topology
/*****************************************************************************************************************/
  NodeContainer nodes;
  nodes.Create(numNodes);

  /* Create random square based on provided size.*/
  MobilityHelper mobility;
  string posMax = "ns3::UniformRandomVariable[Min=0.0|Max=1000]";
  //mobility.SetPositionAllocator ("ns3::RandomRectanglePositionAllocator","X", RandomVariableValue (UniformVariable (0,1000)), "Y", RandomVariableValue (UniformVariable (0,1000)) );
   mobility.SetPositionAllocator ("ns3::RandomRectanglePositionAllocator","X", StringValue (posMax),"Y", StringValue (posMax));
  mobility.SetMobilityModel ("ns3::ConstantPositionMobilityModel");
  mobility.Install (nodes);

  //Set up the WiFi Net Device
  YansWifiPhyHelper wifiPhy = YansWifiPhyHelper::Default ();
  wifiPhy.Set ("TxPowerStart", DoubleValue(txPower));
  wifiPhy.Set ("TxPowerEnd", DoubleValue(txPower));
  //wifiPhy.SetPcapDataLinkType (YansWifiPhyHelper::DLT_IEEE802_11_RADIO);

  //Define the wireless channel
  YansWifiChannelHelper wifiChannel;
  selectmodel(wifiChannel,lossmodel);
  wifiChannel.SetPropagationDelay ("ns3::ConstantSpeedPropagationDelayModel");
  wifiPhy.SetChannel (wifiChannel.Create ());

  // Set Wifi to 802.11a and use a DSSS rate of 1Mbps.
  WifiHelper wifi;
  wifi.SetStandard (WIFI_PHY_STANDARD_80211a);
  wifi.SetRemoteStationManager ("ns3::ConstantRateWifiManager", "DataMode", StringValue("OfdmRate6Mbps"), "ControlMode", StringValue("OfdmRate6Mbps"));

  /* Set the Wifi MAC as adhoc.*/
  NqosWifiMacHelper wifiMac = NqosWifiMacHelper::Default ();
  wifiMac.SetType ("ns3::AdhocWifiMac");

  //Install the WifiNet on all nodes in the network
  NetDeviceContainer devices = wifi.Install (wifiPhy, wifiMac, nodes);

  //Set up the internet stack 
  InternetStackHelper stack;

  if (routeProt.compare("AODV") == 0)
  {
      AodvHelper aodv;
      stack.SetRoutingHelper (aodv);
  }
  else if (routeProt.compare("OLSR") == 0)
  {
      OlsrHelper olsr;
      stack.SetRoutingHelper (olsr);
  }
  else if (routeProt.compare("DSDV") == 0)
  {
      DsdvHelper dsdv;
      stack.SetRoutingHelper (dsdv);
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
vector<uint32_t> subNodes;
map<uint32_t,uint32_t> nodePairs;
for (uint32_t j=0; j<numNodes; j++) vec.push_back(j); 	// Create a vector of node indices for pairing
random_shuffle(vec.begin(),vec.end());			// Randomize the list and connect the successive nodes

for (uint32_t j=0; j<subnodes; j++) subNodes.push_back(vec[j]); 	// Create a vector of node indices for pairing and pick the first n pairs

for(uint32_t i = 0 ; i <= subNodes.size() - 1 ; i++ )
{	

if (i==subNodes.size()-1)
	subNodes[i+1]=subNodes[0];
    
	nodePairs.insert(pair<uint32_t,uint32_t>(subNodes[i],subNodes[i+1]));
     //cout << subNodes[i] << "--" << subNodes[i+1] << endl;
}
 
//cout << "\n" << endl;
  
ApplicationContainer sourceApps[subnodes];
ApplicationContainer sinkApps[subnodes];

uint32_t indx = 0;
map<uint32_t,uint32_t>::iterator iter;

  for (iter=nodePairs.begin(); iter != nodePairs.end(); iter++)
    {
      uint16_t port = 50000 + indx;
      Ptr<Node> appSource 	= nodes.Get(iter->first);
      Ptr<Node> appSink 	= nodes.Get(iter->second);
      //cout << iter->first << "--**" << iter->second << endl;

      Ipv4Address remoteAddr 	= appSink->GetObject<Ipv4> ()->GetAddress (1, 0).GetLocal ();
      OnOffHelper onoff ("ns3::UdpSocketFactory", Address (InetSocketAddress (remoteAddr, port)));
      // onoff.SetAttribute ("DataRate", DataRateValue (DataRate (dataRate)));
	  sourceApps[indx] = onoff.Install (appSource);
      sourceApps[indx].Start(Seconds(0.0));
      sourceApps[indx].Stop (Seconds (stopTime));

      PacketSinkHelper sink ("ns3::UdpSocketFactory", InetSocketAddress (Ipv4Address::GetAny (), port));
      sinkApps[indx] = sink.Install (appSink);
      sinkApps[indx].Start(Seconds(0.0));
      sinkApps[indx].Stop (Seconds (stopTime));

      indx++;
    }

for (uint32_t i=0; i < subnodes; ++i){
    Ptr<OnOffApplication> onoff1 = DynamicCast<OnOffApplication> (sourceApps[i].Get (0));
    onoff1->TraceConnectWithoutContext("Tx", MakeCallback(&Trace));
  }
  
Simulator::Stop(Seconds(stopTime));  
Simulator::Run ();

/*****************************************************************************************************************/
// Observations and Results
/*****************************************************************************************************************/
  
  uint32_t totRx = 0;
  for (uint32_t i=0; i < subnodes; ++i)
  {
      Ptr<PacketSink> sink1 = DynamicCast<PacketSink> (sinkApps[i].Get (0));
      totRx += sink1->GetTotalRx ();
  }
  /* Results output*/
  cout << lossmodel << "\t\t" << numNodes << "\t" << pow(10, txPower/10) << "\t" << routeProt << "\t" << ((double)totRx / (double)total) << endl;

  ofstream outfile;
  outfile.open("scratch/p4_AODV.csv", std::ios_base::app);
  outfile << "LossModel," << lossmodel << ",routep," << routeProt << ",nodes," << numNodes << ",txpower," << pow(10, txPower/10) << ",efficiency," << ((double)totRx / (double)total) << endl;
 

  Simulator::Destroy ();
  return 0;
}