#ifndef SFC_SWITCH_HELPER
#define SFC_SWITCH_HELPER

#include "ns3/net-device-container.h"
#include "ns3/object-factory.h"
#include <string>

namespace ns3{

class Node;
class AttributeValue;

/**
 *\brief SfcSwitchHelper
 */
class SfcSwitchHelper{
public:
	SfcSwitchHelper();
	void SetDeviceAttribute(std::string n1, const AttributeValue& v1);
	/**
	 * adds the device to the node, and attaches the given NetDevices as ports of switch
	 */
	NetDeviceContainer Install(Ptr<Node> node, NetDeviceContainer c);
	NetDeviceContainer Install(std::string nodeName, NetDeviceContainer c);
private:
	ObjectFactory m_deviceFactory;
};

}

#endif