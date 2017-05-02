#include "sfc-switch-net-device.h"

namespace ns3 {

NS_LOG_COMPONENT_DEFINE("SfcSwitchNetDevice");
NS_OBJECT_ENSURE_REGISTERED(SfcSwitchNetDevice);

const char* SfcSwitchNetDevice::GetManufactureDescription(){
	return "uestc kb310";
}

const char* SfcSwitchNetDevice::GetHardwareDescription(){
	return "N/A";
}

const char* SfcSwitchNetDevice::GetSoftwareDescription(){
	return "Simulated Sfc Switch";
}

const char* SfcSwitchNetDevice::GetSerialNumber(){
	return "N/A";
}

void SfcSwitchNetDevice::SetAddress(Address address){
	NS_LOG_FUNCTION_NOARGS();
	m_address = Mac48Address::ConvertFrom(address);
}

Address SfcSwitchNetDevice::GetAddress(void) const{
	NS_LOG_FUNCTION_NOARGS();
	return m_address;
}

Ptr<Node> SfcSwitchNetDevice::GetNode(void) const{
	NS_LOG_FUNCTION_NOARGS();
	return m_node;
}

void SfcSwitchNetDevice::SetNode(Ptr<Node> node){
	NS_LOG_FUNCTION_NOARGS();
	m_node=node;
}

bool SetMtu(const uint16_t mtu){
	NS_LOG_FUNTION(this << mtu);
	m_mtu = mtu;
	NS_LOG_LOGIC("m_mtu= " << m_mtu);
	return true;
}

uint16_t GetMtu(void) const{
	NS_LOG_FUNCTION_NOARGS();
	return m_mtu;
}


void SfcSwitchNetDevice::SetReceiveCallback(NetDevice::ReceiveCallback cb){
	NS_LOG_FUNCTION_NOARGS();
	m_rxCallback = cb;
}

void SfcSwitchNetDevice::SetPromiscReceiveCallback(NetDevice::PromiscReceiveCallback cb){
	NS_LOG_FUNCTION_NOARGS();
	mm_promiscRxCallback = cb;
}

void SfcSwitchNetDevice::AddHeader(Ptr<Packet> p, Mac48Address src, Mac48Address dest, uint16_t protocolNumber){
	NS_LOG_FUNCTION(p << src << dest << protocolNumber);
	
	EthernetHeader header(false);
	header.SetSource(source);
	header.SetDestination(dest);
	
	EthernetTrailer trailer;
	NS_LOG_LOGIC ("p->GetSize () = " << p->GetSize ());
	NS_LOG_LOGIC ("m_encapMode = " << m_encapMode);
	NS_LOG_LOGIC ("m_mtu = " << m_mtu);
	
	uint16_t lengthType = 0;
	switch(m_encapMode){
		case DIX:
			NS_LOG_LOGIC("Encapsulating packet as DIX (type interpretation)");
			lengthType = protocolNumber;
			if(p->GetSize()<46){
				uint8_t buffer[46];
				memset(buffer, 0, 46);
				Ptr<Packet> padd = Create<Packet>(buffer, 46-p->GetSize());
				p->AddAtEnd(padd);
			}
		break;
		case LLC:
			NS_LOG_LOGIC ("Encapsulating packet as LLC (length interpretation)");
			LlcSnapHeader llc;
			llc.SetType(protocolNumber);
			p->AddHeader(llc);
			lengthType = protocolNumber;
			if(p->GetSize()<46){
				uint8_t buffer[46];
				memset(buffer, 0, 46);
				Ptr<Packet> padd = Create<Packet>(buffer, 46-p->GetSize());
				p->AddAtEnd(padd);
			}
			NS_ASSERT_MSG (p->GetSize () <= GetMtu (),
                       "CsmaNetDevice::AddHeader(): 802.3 Length/Type field with LLC/SNAP: "
                       "length interpretation must not exceed device frame size minus overhead");
		break;
		case ILLEGAL:
		default:
			NS_FATAL_ERROR("CsmaNetDevice::AddHeader(): Unknown packet encapsulation mode");
			break;
	}
	NS_LOG_LOGIC("header.SetLengthType (" << lengthType << ")");
	header.SetLengthType(lengthType);
	p->AddHeader(header);
	if(Node::ChecksumEnabled()){
		trailer.EnableFcs(true);
	}
	trailer.CalcFcs(p);
	p->AddTrailer(trailer);
}

bool SfcSwitchNetDevice::Send(Ptr<Packet> packet, const Address& dest, uint16_t protocolNumber){
	NS_LOG_FUNCTION_NOARGS();
	return SendFrom(packet, m_address, dest, protocolNumber);		//TODO:m_address : source mac?
}

bool SfcSwitchNetDevice::SendFrom(Ptr<Packet> packet, const Address& src, const Address& dest, uint16_t protocolNumber){
	NS_LOG_FUNCTION_NOARGS(packet << src << dest << protocolNumber);
	NS_LOG_LOGIC("packet = " << packet);
	NS_LOG_LOGIC("UID is " << packet->GetUid() << ")");
	
	Mac48Address destination = Mac48Address::ConvertFrom(dest);
	Mac48Address source = Mac48Address::ConvertFrom(src);
		
	AddHeader(packet, source, destination, protocolNumber);
	
	m_macTxTrace(packet);
	
	//TODO
	if(m_queue->Enqueue(Create(QueueItem(packet))==false){
		m_macDropTrace(packet);
		return false;
	}
	
	if(m_txMachineState == READY){
		if(m_queue->IsEmpty()==false){
			//TODO
			Ptr<QueueItem> item = m_queue->Dequeue();
			m_currentPkt = item->GetPacket();
			m_promiscSnifferTrace(m_currentPkt);
			m_snifferTrace(m_currentPkt);
			TransmitStart();
		}
	}
	
}


//TODO
int SfcSwitchNetDevice::AddSfcSwitchPort(Ptr<NetDevice> switchPort){
	NS_LOG_FUNCTION_NOARGS();
	NS_ASSERT(switchPort != this);
	if(!Mac48Address::IsMatchingType(switchPort->GetAddress())){
		NS_FATAL_ERROR("Device does not support eui 48 addresses: cannot be added to switch.");
	}
	if(!switchPort->SupportsSendFrom()){
		NS_FATAL_ERROR("Device does not support SendFrom: cannot be added to switch.");
	}
	if(m_address==Mac48Address()){
		m_address = Mac48Address::ConvertFrom(switchPort->GetAddress());
	}
	
}

void SfcSwitchNetDevice::Receive(Ptr<Packet> packet, Ptr<CsmaNetDevice> senderDevice){
	NS_LOG_FUNCTION(packet << senderDevice);
	NS_LOG_LOGIC("UID is " << packet->GetUid());
	
	if(senderDevice == this){
		return;
	}
	
	m_phyRxEndTrace(packet);
	
	if(IsReceiveEnabled() == false){
		m_phyRxDropTrace(packet);
		return;
	}
	Ptr<Packet> original = packet->Copy();
	EthernetTrailer trailer;
	packet->RemoveTrailer(trailer);
	if(Node::ChecksumEnabled()){
		trailer.EnbaleFcs(true);
	}
	bool crcGood = trailer.CheckFcs(packet);
	if(!crcGood){
		m_phyRxDropTrace(packet);
		return;
	}
	EthernetHeader header(false);
	packet->RemoveHeader(header);
	
	NS_LOG_LOGIC ("Pkt source is " << header.GetSource ());
	NS_LOG_LOGIC ("Pkt destination is " << header.GetDestination ());
	
	//TODO : test src mac drop
	
	uint16_t protocol;
	if(header.GetLengthType()<=1500){
		NS_ASSERT (packet->GetSize () >= header.GetLengthType ());
		uint32_t padlen = packet->GetSize () - header.GetLengthType ();
		NS_ASSERT (padlen <= 46);
		if(padlen > 0){
			packet->RemoveAtEnd(padlen);
		}
		LlcSnapHeader llc;
		packet->RemoveHeader (llc);
		protocol = llc.GetType ();
	}else{
		protocol = header.GetLengthType();
	}
	PacketType packetType;
	if(header.GetDestination().IsBroadcast()){
		packetType = PPACKET_BROADCAST£»
	}else if(header.GetDestination().IsGroup()){
		packetType = PACKET_MULTICAST;
	}else if(header.GetDestination()==m_address){
		packetType = PACKET_HOST;
	}else{
		packetType = PACKET_OTHERHOST;
	}
	
	//TODO
	m_promiscSnifferTrace (originalPacket);
	if(!m_promiscRxCallback.IsNull()){
		m_macPromiscRxTrace(originalPacket);
		m_promiscRxCallback(this, packet, protocol, header.GetSource(), header.GetDestination(), packetType);
	}
	if(packetType!=PACKET_OTHERHOST){
		m_snifferTrace(originalPacket);
		m_macRxTrace(originalPacket);
		m_rxCallback(this, packet, protocol, header.GetSource());
	}
}

}	//end namespace ns3