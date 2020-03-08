#include <ns3/output-stream-wrapper.h>
#include "RoutingTableEntry.h"
#include <map>
#include <ns3/event-id.h>
#include <ns3/simulator.h>
class RoutingTable{
	private:
		std::map<ns3::Ipv4Address, ns3::EventId> eventTable;
		std::map<ns3::Ipv4Address,RoutingTableEntry> rTable;
		ns3::Time holdTime;
		std::map<uint32_t,std::set<ns3::Ipv4Address>>knownZonesTable;
		bool deleteZoneIp(ns3::Ipv4Address ip,std::map<uint32_t,std::set<ns3::Ipv4Address>>::iterator findItr);
	public:
		RoutingTable();
		void getListOfAddressWithNextHop(ns3::Ipv4Address addr,std::map<ns3::Ipv4Address,RoutingTableEntry>); 
		void addEvent(ns3::Ipv4Address addr,ns3::EventId);
		void addRouteEntry(RoutingTableEntry& rte);
		bool search(ns3::Ipv4Address ip,RoutingTableEntry& entry) ;
		ns3::Time getHoldTime() const;
		void setHoldTime(ns3::Time time);
		uint32_t size() const;
		void clear();
		bool updateRoute(RoutingTableEntry& entry);
		void deleteRouteEntries(ns3::Ipv4InterfaceAddress interface);
		bool deleteRouteEntry(ns3::Ipv4Address ip);
		bool deleteEvent(ns3::Ipv4Address addr);
		bool forceDeleteEvent(ns3::Ipv4Address addr);
		void deleteAllInvalidRoutes();
		void getAllRoutes(std::map<ns3::Ipv4Address,RoutingTableEntry>& table);
		void deleteRoutesWithInterface(ns3::Ipv4InterfaceAddress addr);
		bool getKnownZones(std::set<ns3::Ipv4Address>& setIp,uint32_t zone);
		void addZoneIp(ns3::Ipv4Address ip,uint32_t zone);
		bool deleteZoneIp(ns3::Ipv4Address ip,uint32_t zone);
		bool deleteIpFromZoneMap(ns3::Ipv4Address ip);
		bool anyRunningEvent(ns3::Ipv4Address);
		void purge(std::map<ns3::Ipv4Address,RoutingTableEntry>& removedAddresses);
		void print(ns3::Ptr<ns3::OutputStreamWrapper> stream) const;
		bool getZonesForIp(std::set<uint32_t> zones,ns3::Ipv4Address addr);
		bool getAllIpforZone(uint32_t zone,std::set<ns3::Ipv4Address> set);
};
		
