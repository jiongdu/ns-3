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

#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/csma-module.h"
#include "ns3/internet-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/applications-module.h"
#include "ns3/ipv4-global-routing-helper.h"

// Default Network Topology
//
//       10.1.1.0
// n0 -------------- n1   n2   n3   n4
//    point-to-point  |    |    |    |
//                    ================
//                      LAN 10.1.2.0


using namespace ns3;

NS_LOG_COMPONENT_DEFINE ("SecondScriptExample");

int 
main (int argc, char *argv[])
{
  bool verbose = true;
  uint32_t nCsma = 3;

  CommandLine cmd;
  cmd.AddValue ("nCsma", "Number of \"extra\" CSMA nodes/devices", nCsma);
  cmd.AddValue ("verbose", "Tell echo applications to log if true", verbose);

  cmd.Parse (argc,argv);

  if (verbose)
    {
      LogComponentEnable ("UdpEchoClientApplication", LOG_LEVEL_ALL);
      LogComponentEnable ("UdpEchoClientApplication", LOG_PREFIX_ALL);
      
	  LogComponentEnable ("UdpEchoServerApplication", LOG_LEVEL_ALL);
	  LogComponentEnable ("UdpEchoServerApplication", LOG_PREFIX_ALL);

      LogComponentEnable ("PointToPointHelper", LOG_LEVEL_INFO);
    }

  nCsma = nCsma == 0 ? 1 : nCsma;

  NodeContainer p2pNodes;
  p2pNodes.Create (2);

  NodeContainer csmaNodes;
  csmaNodes.Add (p2pNodes.Get (1));   
  csmaNodes.Create (nCsma);    
//pointTopoint
  PointToPointHelper pointToPoint;
  pointToPoint.SetDeviceAttribute ("DataRate", StringValue ("5Mbps"));
  pointToPoint.SetChannelAttribute ("Delay", StringValue ("2ms"));

  NetDeviceContainer p2pDevices;
  p2pDevices = pointToPoint.Install (p2pNodes);   //@return : NetDeviceContainer(contains two PointToPointNetDevice)
//ethernet
  CsmaHelper csma;
  csma.SetChannelAttribute ("DataRate", StringValue ("100Mbps"));
  csma.SetChannelAttribute ("Delay", TimeValue (NanoSeconds (6560)));

  NetDeviceContainer csmaDevices;
  csmaDevices = csma.Install (csmaNodes);	//@return : NetDeviceContainer(contains csmaNetDevices)
//add stack
  InternetStackHelper stack;
  stack.Install (p2pNodes.Get (0));		
  stack.Install (csmaNodes);

  Ipv4AddressHelper address;
  address.SetBase ("10.1.1.0", "255.255.255.0");	//SetBase(Ipv4Address network, Ipv4Mask mask, Ipv4Address base = "0.0.0.1");  start from 10.0.0.1
  Ipv4InterfaceContainer p2pInterfaces;
  p2pInterfaces = address.Assign (p2pDevices);		//Assign IP address to the net devices specified in the container based on the address set

  address.SetBase ("10.1.2.0", "255.255.255.0");
  Ipv4InterfaceContainer csmaInterfaces;
  csmaInterfaces = address.Assign (csmaDevices);	//Assign IP address to csmaDevices

  UdpEchoServerHelper echoServer (9);   //port number

  ApplicationContainer serverApps = echoServer.Install (csmaNodes.Get (nCsma));		//UdpEchoServer Application installed on csmaNodes[nCsma](the last node)
  serverApps.Start (Seconds (1.0));
  serverApps.Stop (Seconds (10.0));

  UdpEchoClientHelper echoClient (csmaInterfaces.GetAddress (nCsma), 9);  //remote ipaddress, port of UdpEchoServer
  echoClient.SetAttribute ("MaxPackets", UintegerValue (1));
  echoClient.SetAttribute ("Interval", TimeValue (Seconds (1.0)));
  echoClient.SetAttribute ("PacketSize", UintegerValue (10000));	//65535

  ApplicationContainer clientApps = echoClient.Install (p2pNodes.Get (0));	//UdpEchoClient Application installed on p2pNodes[0](the first node)
  clientApps.Start (Seconds (2.0));
  clientApps.Stop (Seconds (10.0));

//  Ipv4GlobalRoutingHelper::PopulateRoutingTables ();	//route

  //pointToPoint.EnablePcapAll ("second");
  //csma.EnablePcap ("second", csmaDevices.Get (1), true);
  //AsciiTraceHelper ascii;
  //pointToPoint.EnableAsciiAll(ascii.CreateFileStream("second.tr"));  
  
  Simulator::Run ();
  Simulator::Destroy ();
  return 0;
}
