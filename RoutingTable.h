#include "RoutingTableEntry.h"
#include <map>
#include <ns3/event-id.h>
#include <ns3/simulator.h>
class RoutingTable{
	private:
		std::map<ns3::Ipv4Address, ns3::EventId> eventTable;
		std::map<ns3::Ipv4Address,RoutingTableEntry> rTable;
		ns3::Time holdTime;
	public:
		RoutingTable();
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
		
 

};
		
