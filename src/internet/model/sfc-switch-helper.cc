#include "sfc-switch-helper.h"
#include "ns3/log.h"
#include "ns3/sfc-switch-net-device.h"
#include "ns3/node.h"
#include "ns3/names.h"

namespace ns3{

NS_LOG_COMPONENT_DEFINE("SfcSwitchHelper");

SfcSwitchHelper::SfcSwitchHelper(){
	NS_LOG_FUNCTION_NOARGS();
	m_deviceFactory.SetTypeId("ns3::SfcSwitchNetDevice");
}
void SfcSwitchHelper::SetDeviceAttribute(std::string n1, const AttributeValue& v1){
	NS_LOG_FUNCTION_NOARGS();
	m_deviceFactory.Set(n1, v1);
}

NetDeviceContainer SfcSwitchHelper::Install(Ptr<Node> node, NetDeviceContainer c){	//c: switch port/netdevice
	NS_LOG_FUNCTION_NOARGS();
	NS_LOG_INFO("*** Install sfc-switch device on node " << node->GetId());
	
	NetDeviceContainer devs;
	Ptr<SfcSwitchNetDevice> dev = m_deviceFactory.Create<SfcSwitchNetDevice>();
	devs.Add(dev);
	node->AddDevice(dev);
	
	//TODO
	for(NetDeviceContainer::Iterator i=c.Begin(); i!=c.End(); ++i){
		NS_LOG_INFO("**** Add SwitchPort " << *i);
		dev->AddSwitchPort(*i);							//add switch port(NetDeviceContainer c) to SfcSwitchNetDevice
	}
	return devs;
}

NetDeviceContainer SfcSwitchHelper::Install(std::string nodeName, NetDeviceContainer c){
	NS_LOG_FUNCTION_NOARGS();
	Ptr<Node> node = Names::Find<Node> (nodeName);
	return Install(node, c);
}