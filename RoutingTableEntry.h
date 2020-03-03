#include <ns3/ipv4-address.h>
#include <ns3/ipv4-interface-container.h>
#include <ns3/nstime.h>
#include <ns3/net-device.h>
#include <ns3/ipv4-route.h>
#include <ns3/simulator.h>
#include "./Metric.h"
enum class EntryState{
	VALID,INVALID
};
class RoutingTableEntry{
	private: 
		ns3::Ipv4Address srcIp;
		uint32_t seqNumber;
		uint32_t zoneId;
		Metric metric;
		ns3::Ipv4InterfaceAddress outputLinkInterface;
		ns3::Ptr<ns3::Ipv4Route> route;
		ns3::Time lifeTime;
		ns3::Time settlingTime;
		bool changed;
		EntryState state;
	public:
		RoutingTableEntry(ns3::Ptr<ns3::NetDevice> dev,ns3::Ipv4Address destIp,uint32_t seqNumber,Metric met,ns3::Ipv4InterfaceAddress interface,ns3::Ipv4Address nextHop,ns3::Time lifetime=ns3::Simulator::Now(),ns3::Time settlingTime = ns3::Simulator::Now(),bool changed = false);
		~RoutingTableEntry();
		RoutingTableEntry();
		bool compareMetric(Metric& newMetric){
			return newMetric<metric;
		}
		inline uint32_t getSeqNumber() const;
		inline EntryState getEntryState() const;
		inline ns3::Ptr<ns3::NetDevice> getDevice() const;
		inline ns3::Ipv4Address getDsptIp() const;
		inline ns3::Ipv4InterfaceAddress getLink() const;
		inline ns3::Ptr<ns3::Ipv4Route> getRoute() const;
		inline ns3::Time getLifeTime() const;
		inline ns3::Ipv4Address getNextHop() const;
		inline void setSeqNumber(uint32_t seq_number);
		inline void setDevice(ns3::Ptr<ns3::NetDevice> dev);
		inline void setLink(ns3::Ipv4InterfaceAddress link);
		inline void setEntryState(EntryState state);
		inline void setLifeTime(ns3::Time time);
		inline bool isChanged();
		inline ns3::Time getSettlingTime();
		inline void setSettlingTime(ns3::Time);
		inline void setChangedState(bool changed);
		void setMetric(Metric&);
		Metric getMetric();
		bool operator==(const RoutingTableEntry& entry){
			return srcIp == entry.srcIp;
		}
		inline void setNextHop(ns3::Ipv4Address hop) ;
		inline void setRoute(ns3::Ptr<ns3::Ipv4Route> route);

};


