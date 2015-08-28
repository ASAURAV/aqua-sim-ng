//...

#include "aqua-sim-mac.h"

#include "ns3/log.h"
#include "ns3/pointer.h"
#include "ns3/address.h"

//...

namespace ns3 {

NS_LOG_COMPONENT_DEFINE("AquaSimMac");
NS_OBJECT_ENSURE_REGISTERED(AquaSimMac);

/*
* Base class for Aqua Sim MAC
*/
AquaSimMac::AquaSimMac() : m_node(NULL),
	m_phy(NULL), m_callback(NULL),
	m_recvChannel(NULL), m_recvHandler(this)
{
}

void
AquaSimMac::SetNode(Ptr<AquaSimNode> node){
	NS_LOG_FUNCTION(this << node);
	m_node = node;
	
	//if needed set up handler here, should be set up such as a callback
	//i.e. m_node->RegisterProtocolHandler (MakeCallBack (&AquaSimMac::Recv, this), ...);
}

void
AquaSimMac::SetPhy(Ptr<AquaSimPhy> phy){
	NS_LOG_FUNCTION(this << phy);
	m_phy = phy;
}

void
AquaSimMac::SetRouting(Ptr<AquaSimRouting> rout){
	NS_LOG_FUNCTION(this << rout);
	m_rout = rout;
}

void
AquaSimMac::RecvProcess(Ptr<Packet> p){
	NS_LOG_FUNCTION (this << " a dummy version.");
}

void
AquaSimMac::TxProcess(Ptr<Packet> p){
	NS_LOG_FUNCTION(this << " a dummy version.");
}

void
AquaSimMac::SetForwardUpCallback(Callback<void, Ptr<Packet>, Address> upCallback)
{
	//not currently used.
	m_callback = upCallback;
}

void
AquaSimMac::SendUp(Ptr<Packet> p)
{
	NS_ASSERT(m_node && m_phy && m_rout);

	m_rout->Recv(p);
}

void
AquaSimMac::SendDown(Ptr<Packet> p)
{
	NS_ASSERT(m_node && m_phy && m_rout);

	m_phy->Recv(p);
}

void
AquaSimMac::HandleIncomingPkt(Ptr<Packet> p) { 	
	//m_recvChannel->AddNewPacket(p);
	double txTime = p->GetTxTime();
	if (Phy()->Status() != PHY_SEND) {
		Node()->SetCarrierSense(true);
	}
	Simulator::Schedule(txTime, &m_recvHandler, p);
	return;
}

void
AquaSimMac::HandleOutgoingPkt(Ptr<Packet> p) {
	m_callback = h;
	/*
	*  TODO Handle busy terminal problem before trying to tx packet
	*/
	TxProcess(p);
}

void
AquaSimMac::Recv(Ptr<Packet> p) {
	//assert(initialized());
	NS_ASSERT(m_node && m_phy && m_rout);

	if (p->GetDirection == DOWN){		//TODO make sure this works correctly
		// Handle outgoing packets.
		HandleOutgoingPkt(p);
	}
	else {
		// Handle incoming packets.
		HandleIncomingPkt(p);
	}
}

TypeId
AquaSimMac::GetTypeId(void)
{
	static TypeId tid = TypeId("ns3::AquaSimMac")
		.SetParent<Object>()
		.AddConstructor<AquaSimMac>()
	;
	return tid;
}

void
AquaSimMac::PowerOn()
{
	Phy()->PowerOn();
}

double
AquaSimMac::GetPreamble()
{
	return Phy()->Preamble();
}

void
AquaSimMac::PowerOff()
{
	Phy()->PowerOff();
}

/**
	* @param pkt_len length of packet, in byte
	* @param mod_name	modulation name
	*
	* @return txtime of a packet of size pkt_len using the modulation scheme
	* 		specified by mod_name
	*/
double
AquaSimMac::GetTxTime(int pktLen, Ptr<std::string> modName)
{
	return Phy()->CalcTxTime(pktLen, modName);
}

double
AquaSimMac::GetTxTime(Ptr<Packet> pkt, Ptr<std::string> modName) {
	return GetTxTime(pkt->GetSize(), modName);
}

int AquaSimMac::GetSizeByTxTime(double txTime, Ptr<std::string> modName) {
	return Phy()->CalcPktSize(txTime, modName);
}

void AquaSimMac::InterruptRecv(double txTime){
	//assert(initialized());
	NS_ASSERT(m_node && m_phy && m_rout);

	if (PHY_RECV == Phy()->Status()){
		Phy()->StatusShift(txTime);
	}
}

} // namespace ns3
