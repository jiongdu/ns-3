// Network topology
//
//        n0     n1
//        |      |
//       ----------
//       | Switch |
//       ----------
//        |      |
//        n2     n3
//

#include <iostream>
#include <fstream>

#include "ns3/core-module.h"
#include "ns3/csma-module.h"
#include "ns3/network-module.h"
#include "ns3/internet-module.h"
#include "ns3/application-module.h"
#include "ns3/log.h"

using namespace ns3;

NS_LOG_COMPONENT_DEFINE("SfcSwitchExample");

int main(int argc, char* argv[])
{
	NS_LOG_INFO("Create nodes.");
	NodeContainer terminals;
	terminals.Create(4);
	
	NodeContainer sfcSwitch;
	sfcSwitch.Create(1);
	
	NG_LOG_INFO("Build Topology");
	CsmaHelper csma;
	csma.SetChannelAttribute("DataRate", DataRateValue(5000000));
	csma.SetChannelAttribute("Delay", TimeValue(MilliSeconds(2)));
	
	//create the csma links between terminal and switch
	NetDeviceContainer terminalDevices;
	NetDeviceContainer switchDevices;
	
	for(int i=0;i<4;i++){
		NetDeviceContainer link = csma.Install(NodeContainer(terminals.Get(i)), sfcSwitch));
		terminalDevices.Add(link.Get(0));
		switchDevices.Add(link.Get(1));
	}
	
	//create the switch netdevice
	Ptr<Node> switchNode = sfcSwitch.Get(0);
	SfcSwitchHelper switch;
	
	switch.Install(switchNode, switchDevices);
	
}



