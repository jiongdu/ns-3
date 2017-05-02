#ifndef SFC_SWITCH_NET_DEVICE_H
#define SFC_SWITCH_NET_DEVICE_H

#include "ns3/simulator.h"
#include "ns3/log.h"
#include "ns3/mac48-address.h"

#include "ns3/ethernet-header.h"
#include "ns3/arp-header.h"
#include "ns3/tcp-header.h"
#include "ns3/udp-header.h"

#include "ns3/ipv4-l3-protocol.h"
#include "ns3/arp-l3-protocol.h"

#include "ns3/node.h"
#include "ns3/enum.h"
#include "ns3/string.h"
#include "ns3/integer.h"
#include "ns3/uinteger.h"
#include "ns3/callback.h"
#include "ns3/traced-callback.h"
#include "ns3/packet.h"

namespace s3 {
/**
 * \brief
 */
class SfcSwitchNetDevice : public NetDevice{
public:
	/**
	*Register this type
	*\return the TypeId
	*/
	static TypeId GetTypeId(void);
	
	/**
	*\brief SfcSwitchNetDevice Description Data
	*/
	static const char* GetManufactureDescription();
	static const char* GetHardwareDescription();
	static const char* GetSoftwareDescription();
	static const char* GetSerialNumber();
	
	SfcSwitchNetDevice();
	virtual ~SfcSwitchNetDevice();
	
	/*
	*\brief Add a 'port' to a switch device
	* This method adds a new switch port to a SfcSwitchNetDevice,
	* so that this new switch port NetDevice becomes part of the switch
	* and L2 frames start being forwarded to/from this NetDevice.
	*\return 0 if everything's ok, otherwise an error number
	*/
	//TODO: have an ip address?
	int AddSfcSwitchPort(Ptr<NetDevice> switchPort);
	
	/**
	*\return the number of switch ports attached to the switch
	*/
	uint32_t GetNSfcSwitchPorts(void) const;
	
	void AddHeader(Ptr<Packet> p, Mac48Address src, Mac48Address dest, uint16_t protocolNumber);
	
	virtual void SetAddress(Address address);
	virtual Address GetAddress(void) const;
	virtual Ptr<Node> GetNode(void) const;
	virtual void SetNode(Ptr<Node> node);
	virtual bool SetMtu(const uint16_t mtu);
	virtual uint16_t GetMtu(void) const;
	
	virtual void SetReceiveCallback(NetDevice::ReceiveCallback cb);
	virtual void SetPromiscReceiveCallback(NetDevice::PromiscReceiveCallback cb);
	
	virtual bool Send(Ptr<Packet> packet, const Address& dest, uint16_t protocolNumber);
	virtual bool SendFrom(Ptr<Packet> packet, const Address& src, const Address& dest, uint16_t protocolNumber);
	
	void Receive(Ptr<Packet> packet, Ptr<CsmaNetDevice> senderDevice);
private:

	uint16_t m_mtu;
	NetDevice::ReceiveCallback m_rxCallback;
	NetDevice::PromiscReceiveCallback m_promiscRxCallback;
	
	Mac48Address m_address;		///Address of this device
	Ptr<Node> m_node;			///Node that this device is installed on
};

}

#endif