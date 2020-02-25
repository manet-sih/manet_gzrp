#include "RoutingTable.h"
RoutingTable::RoutingTable(){}
void RoutingTable::addEvent(ns3::Ipv4Address addr,ns3::EventId id){
	auto itr = eventTable.find(addr);
	if(itr==eventTable.end()){
		eventTable[addr] = id;
	}else{
		itr->second = id;
	}
}
void RoutingTable::addRouteEntry(RoutingTableEntry& entry){
	rTable[entry.getDsptIp()] = entry;
}
bool RoutingTable::search(ns3::Ipv4Address addr,RoutingTableEntry& entry){
	if(rTable.empty()) return false;
	auto itr = rTable.find(addr);
	if(itr==rTable.end()) return false;
	entry = itr->second;
	return true;
}
ns3::Time RoutingTable::getHoldTime() const{
	return holdTime;
}
void RoutingTable::setHoldTime(ns3::Time time){
	holdTime = time;
}
uint32_t RoutingTable::size() const{
	return rTable.size();
}
bool RoutingTable::updateRoute(RoutingTableEntry& route){
	auto itr = rTable.find(route.getDsptIp());
	if(itr==rTable.end()){
		return false;
	}
	route = itr->second;
	return true;
}
void RoutingTable::deleteRouteEntries(ns3::Ipv4InterfaceAddress interface){
	for(auto itr = rTable.begin(); itr!=rTable.end();itr++){
		if(itr->second.getLink() == interface){
			auto tmpIterator = itr;
			itr++;
			rTable.erase(tmpIterator);
		}else{
			itr++;
		}
	}
}
bool RoutingTable::deleteRouteEntry(ns3::Ipv4Address ip){
	auto itr = rTable.find(ip);
	if(itr==rTable.end()) return false;
	rTable.erase(itr);
	return true;
}
bool RoutingTable::deleteEvent(ns3::Ipv4Address addr){
	auto itr = eventTable.find(addr);
	if(eventTable.empty()) return false;
	else if(itr == eventTable.end()) return false;
	else if(itr->second.IsRunning()) return false;
	else if(itr->second.IsExpired()) {
		itr->second.Cancel();
		eventTable.erase(itr);
		return true;
	}
	else{
		eventTable.erase(itr);
		return true;
	}
}
bool RoutingTable::forceDeleteEvent(ns3::Ipv4Address addr){
	auto itr = eventTable.find(addr);
	if(itr == eventTable.end()) return false;
	ns3::Simulator::Cancel(itr->second);
	eventTable.erase(itr);
	return true;
}
void RoutingTable::deleteRoutesWithInterface(ns3::Ipv4InterfaceAddress addr){
	for(auto itr = rTable.begin();itr!=rTable.end();){
		if(itr->second.getLink() == addr) {
			auto tmp = itr;
			itr++;
			rTable.erase(tmp);
		}else{
			itr++;
		}
	}
}
void RoutingTable::getAllRoutes(std::map<ns3::Ipv4Address,RoutingTableEntry>& table){
	for(auto itr = rTable.begin();itr!=rTable.end();itr++){
		if(itr->first != ns3::Ipv4Address("127.0.0.1") && itr->second.getEntryState()!=EntryState::INVALID){
			table[itr->first] = itr->second;
		}
	}
}

void RoutingTable::deleteAllInvalidRoutes(){
	for(auto itr = rTable.begin();itr!=rTable.end();){
		if(itr->second.getEntryState() == EntryState::INVALID) {
			auto tmp = itr;
			itr++;
			rTable.erase(tmp);
		}else if(itr->second.getLifeTime()>holdTime){
			auto tmp = itr;
			itr++;
			rTable.erase(tmp);
		}
		else itr++;
	}
}
