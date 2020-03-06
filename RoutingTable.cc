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
bool RoutingTable::getKnownZones(std::set<ns3::Ipv4Address>& knownZones,uint32_t zone)
{
	auto itr = knownZonesTable.find(zone);
	if(itr == knownZonesTable.end())
		return false;
	else{
		knownZones = itr->second;
		return true;
	}

}
void RoutingTable::addZoneIp(ns3::Ipv4Address ip,uint32_t zone){
	auto findItr = knownZonesTable.find(zone);
	if(findItr == knownZonesTable.end()){
		knownZonesTable[zone] = std::set<ns3::Ipv4Address>();
	}
	knownZonesTable[zone].insert(ip);
}
bool RoutingTable::deleteZoneIp(ns3::Ipv4Address ip,uint32_t zone){
	auto findItr = knownZonesTable.find(zone);
	if(findItr == knownZonesTable.end()){
		return false;
	}
	std::set<ns3::Ipv4Address>& ipSet = findItr->second;
	auto findIp = ipSet.find(ip);
	if(findIp == ipSet.end()) return false;
	ipSet.erase(findIp);
	if(ipSet.size() == 0) knownZonesTable.erase(findItr);
	return true;
}
bool RoutingTable::deleteZoneIp(ns3::Ipv4Address ip,std::map<uint32_t,std::set<ns3::Ipv4Address>>::iterator findItr){
	std::set<ns3::Ipv4Address>& ipSet = findItr->second;
	auto findIp = ipSet.find(ip);
	if(findIp == ipSet.end()) return false;
	ipSet.erase(findIp);
	if(ipSet.size() == 0) knownZonesTable.erase(findItr);
	return true;
}

bool RoutingTable::deleteIpFromZoneMap(ns3::Ipv4Address addr){
	bool result = false;
	for(auto itr = knownZonesTable.begin();itr!=knownZonesTable.end();itr++){
		result = result||(deleteZoneIp(addr,itr));
	}
	return result;
}
bool RoutingTable::anyRunningEvent(ns3::Ipv4Address addr){
	auto itr = eventTable.find(addr);
	if(eventTable.empty()) return false;
	if(itr== eventTable.end()) return false;
	if(itr->second.IsRunning()) return true;
	return false;
}
void RoutingTable::purge(std::map<ns3::Ipv4Address,RoutingTableEntry>& removedAddresses){
	if(rTable.empty()) return;
	for(auto i = rTable.begin();i!=rTable.end();i++){
		auto iTmp = i;
		if(i->second.getLifeTime()>holdTime && (i->second.getMetric().getMagnitude()>0)){
			for(auto j = rTable.begin();j!=rTable.end();j++){
				if((j->second.getNextHop() == i->second.getDsptIp())&&(i->second.getMetric().getMagnitude() != j->second.getMetric().getMagnitude())){
					auto jTmp = j;
					removedAddresses.insert(std::make_pair(j->first,j->second));
					j++;
					rTable.erase(jTmp);
				}else{
					j++;
				}
			}
			removedAddresses.insert(std::make_pair(i->first,i->second));
			i++;
			rTable.erase(iTmp);
		}else{
			i++;
		}
	}
	return;
}
void RoutingTable::getListOfAddressWithNextHop(ns3::Ipv4Address addr,std::map<ns3::Ipv4Address,RoutingTableEntry> map){
	map.clear();
	for(auto i= rTable.begin();i!=rTable.end();i++){
		if(i->second.getNextHop() == addr) 
			map.insert(std::make_pair(i->first,i->second));
	}
}
void RoutingTable::print(ns3::Ptr<ns3::OutputStreamWrapper> stream) const{
	*stream->GetStream()<<"\nRouting Table\n"<<"Destination\tGateway\tInterface\t\tMetric\t\tSeqNum\t\tLifeTime\t\tSettlingTime\n";
	for(auto i = rTable.begin();i!=rTable.end();i++){
		i->second.print(stream);
	}
	*stream->GetStream()<<"\n";
}
