#include "./gzrppacket.h"
GzrpPacket::GzrpPacket(){}
GzrpPacket::GzrpPacket(ns3::Ipv4Address srcAddr,uint32_t zone,uint32_t seqNo,uint32_t KnownZones,uint32_t hops):srcIp(srcAddr),zoneId(zone),seqNum(seqNo),numKnownZones(KnownZones),numHops(hops){}

void GzrpPacket:: setSrcIp(ns3::Ipv4Address ip)
{
    srcIp=ip;
}
void GzrpPacket:: setSeqNo(uint32_t seqNo)
{
   seqNum=seqNo;
}
void GzrpPacket:: setZoneId(uint32_t zId)
{
   zoneId=zId;
}
void GzrpPacket:: setNumKnownZones(uint32_t knownZones)
{
    numKnownZones=knownZones;
}
void GzrpPacket::setNumHops(uint32_t hopsCount){
	numHops = hopsCount;
}
ns3::Ipv4Address GzrpPacket:: getSrcIp()
{
   return srcIp;
}
uint32_t GzrpPacket::getSeqNo()const{
  return seqNum;
}
uint32_t GzrpPacket:: getZoneId() const
{
  return zoneId;
}
uint8_t GzrpPacket::getNumKnownZones() const
{
  return numKnownZones;
}
void GzrpPacket::setNeighbourZones(std::vector<uint32_t>& zones) 
{
        if(zones.size())
        neighbourZones=zones;
}
uint32_t GzrpPacket::getNumHops()const {
	return numHops;
}
bool GzrpPacket::getNeighbourZones(std::vector<uint32_t>& zones) const
{
  if( numKnownZones != 0){
       zones=neighbourZones;
       return true;

  }
  else
  {
    return false;
  }
  
}
ns3::TypeId GzrpPacket::GetTypeId (void)
{
	static ns3::TypeId tid = ns3::TypeId ("ns3::dsdv::GzrpPacket")
		.SetParent<ns3::Header> ()
		.SetGroupName ("Dsdv")
		.AddConstructor<GzrpPacket> ();
	return tid;
}
	ns3::TypeId GzrpPacket::GetInstanceTypeId () const
{
	return GetTypeId ();
}
		uint32_t GzrpPacket::GetSerializedSize () const
{
	return (16 + (4*numKnownZones));
}
	void GzrpPacket::Serialize (ns3::Buffer::Iterator itr) const
{
	WriteTo (itr, srcIp);
	itr.WriteHtonU32 ( zoneId);
	itr.WriteHtonU32 (seqNum);
  itr.WriteHtonU32(numKnownZones);
	for(uint32_t i=0;i<numKnownZones;i++){
    itr.WriteHtonU32(neighbourZones[i]);
  }
	
}
uint32_t GzrpPacket::Deserialize (ns3::Buffer::Iterator start)
{
	ns3::Buffer::Iterator itr = start;
	ReadFrom (itr, srcIp);
	zoneId = itr.ReadNtohU32 ();
	seqNum= itr.ReadNtohU32 ();
	numKnownZones=itr.ReadNtohU32();
  for(uint32_t i=0;i<numKnownZones;i++){
    uint32_t zone = itr.ReadNtohU32();
    neighbourZones.push_back(zone);
  }
	uint32_t readSize = itr.GetDistanceFrom (start);
	NS_ASSERT (readSize == GetSerializedSize ());
	return readSize;
}
void GzrpPacket::Print(std::ostream &os) const{
  os << "My convention source Ipv4: " << srcIp
		<< " Zone id " << zoneId 
		<< "sequence number " << seqNum<<"number of known zones: "<<numKnownZones;
    for(uint32_t i=0;i<numKnownZones;i++){
      os<<"Neighbouring Zones: "<<neighbourZones[i]<<"\n";
    }
}

