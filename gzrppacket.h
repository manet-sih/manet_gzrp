#include "ns3/ipv4-address.h"
#include "ns3/nstime.h"
#include "ns3/object-base.h"
#include "ns3/address-utils.h"
#include "ns3/packet.h"
#include <vector>
#include "./Metric.h"
//typeid is not yet discusssed and implemented
class GzrpPacket: public ns3::Header{
	private:
		NS_OBJECT_ENSURE_REGISTERED(GzrpPacket);
		ns3::Ipv4Address srcIp;
		uint32_t zoneId;
		uint32_t seqNum;
		uint32_t numKnownZones;
		Metric packetMetric;
        	std::vector<uint32_t> neighbourZones;
		static ns3::TypeId GetTypeId(void);
		virtual ns3::TypeId GetInstanceTypeId(void) const;
		virtual uint32_t GetSerializedSize() const;
		virtual void Serialize(ns3::Buffer::Iterator start) const;
		virtual uint32_t Deserialize(ns3::Buffer::Iterator start);
		virtual void Print(std::ostream &os) const;
	 
	public:
		GzrpPacket();
		GzrpPacket(ns3::Ipv4Address srcAddr,uint32_t zone,uint32_t seqNo,uint32_t KnownZones,Metric met);
		void setSrcIp(ns3::Ipv4Address ip);
		void setSeqNo(uint32_t seqNo);
		void setZoneId(uint32_t zId);
		void setNumKnownZones(uint32_t knownZones);
		void setNeighbourZones(std::vector<uint32_t>& zones);
		void setMetric(Metric& met);
		ns3::Ipv4Address getSrcIp();
		uint32_t getSeqNo()const;
		uint32_t getZoneId()const;
		uint8_t getNumKnownZones() const;
		bool getNeighbourZones(std::vector<uint32_t>& zones) const;
		Metric getMetric()const;
};



	

